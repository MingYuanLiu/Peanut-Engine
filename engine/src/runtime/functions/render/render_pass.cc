#include "runtime/functions/render/render_pass.h"
#include "runtime/functions/rhi/vulkan/vulkan_rhi.h"

#include <array>

namespace peanut_old {

void MainRenderPass::Initialize() {
  rhi_ = GlobalEngineContext::GetContext()->GetRenderSystem()->GetRHI();
  env_map_levels_ = RenderUtils::NumMipmapLevels(kEnvMapSize, kEnvMapSize);
  g_descriptor_layouts_.resize(kNumDescriptorType);
  g_pipeline_layouts_.resize(kNumDescriptorType);

  display_width_ = static_cast<VulkanRHI *>(rhi_.get())->GetDisplayWidth();
  display_height_ = static_cast<VulkanRHI *>(rhi_.get())->GetDisplayHeight();

  CreateRenderTarget();

  preparePassData();

  CreateUniformBuffer();

  SetupSamplers();

  SetupComputeDescriptorPool();

  SetupComputeDescriptorSets();

  SetupUniformDescriptorSets();

  SetupRenderpass();

  CreateFrameBuffer();

  SetupToneMapPipeline();

  SetupPBRPipeline();

  SetupSkyboxPipeline();

  LoadAndProcessEnvironmentMap();

  ComputeDiffuseIrradianceMap();

  ComputeCookTorranceLut();
}

void MainRenderPass::DeInitialize() 
{
    PEANUT_LOG_INFO("DeInitialize main render pass");

    rhi_->DestroyTexture(environment_map_);
    rhi_->DestroyTexture(irradiance_map_);
    rhi_->DestroyTexture(brdf_lut_);
    rhi_->DestroyTexture(albedo_texture_);
    rhi_->DestroyTexture(normal_texture_);
    rhi_->DestroyTexture(metalness_texture_);
    rhi_->DestroyTexture(roughness_texture_);

    AssetsManager::GetInstance().DestroyMeshBuffer(*pbr_mesh_);
    AssetsManager::GetInstance().DestroyMeshBuffer(*skybox_mesh_);

    DestroyUniformBuffer(uniform_buffer_);

    // destroy sampler
    rhi_->DestroySampler(&default_sampler_);
    rhi_->DestroySampler(&brdf_sampler_);

    rhi_->DestroyPipelineLayout(g_pipeline_layouts_[Pbr]);
    rhi_->DestroyPipeline(pbr_pipeline_);

    rhi_->DestroyPipelineLayout(g_pipeline_layouts_[Skybox]);
    rhi_->DestroyPipeline(skybox_pipeline_);

    rhi_->DestroyPipelineLayout(g_pipeline_layouts_[ToneMap]);
    rhi_->DestroyPipeline(tonemap_pipeline_);

    rhi_->DestroyRenderPass(g_render_pass_);

    // destroy render target
    int32_t num_frames = rhi_->GetNumberFrames();
    for (int32_t i = 0; i < num_frames; ++i) 
    {
        DestroyRenderTarget(g_render_targets_[i]);

        rhi_->DestroyFrameBuffer(g_frame_buffers_[i]);
    }
}

void MainRenderPass::RenderTick(const ViewSettings &view,
                                const SceneSettings &scene) {
  const VkDeviceSize zero_offset = 0;

  glm::mat4 projection_mat =
      glm::perspectiveFov(view.fov, (float)(display_width_),
                          (float)(display_height_), 1.0f, 1000.0f);

  projection_mat[1][1] *= -1.0f;  // Vulkan uses right handed NDC with Y axis
                                  // pointing down, compensate for that.

  // PEANUT_LOG_INFO("pitch of view {0}, yaw of view {1}", view.pitch,
  // view.yaw);

  const glm::mat4 view_rotation_mat =
      glm::eulerAngleXY(glm::radians(view.pitch), glm::radians(view.yaw));

  const glm::mat4 scene_rotation_mat =
      glm::eulerAngleXY(glm::radians(scene.pitch), glm::radians(scene.yaw));

  const glm::mat4 view_mat =
      glm::translate(glm::mat4{1.0f}, {0.0f, 0.0f, -view.distance}) *
      view_rotation_mat;

  const glm::vec3 eye_position = glm::inverse(view_mat)[3];

  uint32_t current_frame_index =
      static_cast<VulkanRHI *>(rhi_.get())->GetCurrentFrameIndex();
  // update transform uniform buffer
  {
    TransformUniforms *const transfer_uniform =
        transform_uniform_buffer_[current_frame_index].as<TransformUniforms>();
    transfer_uniform->viewProjectionMatrix = projection_mat * view_mat;
    transfer_uniform->skyProjectionMatrix = projection_mat * view_rotation_mat;
    transfer_uniform->sceneRotationMatrix = scene_rotation_mat;
  }
  // update shading uniform buffer
  {
    ShadingUniforms *const shading_uniform =
        shading_uniform_buffer_[current_frame_index].as<ShadingUniforms>();
    shading_uniform->eye_position = glm::vec4{eye_position, 0.0f};
    for (int i = 0; i < SceneSettings::kNumLights; ++i) {
      const SceneSettings::Light &light = scene.lights[i];
      shading_uniform->lights[i].direction = glm::vec4{light.direction, 0.0f};
      if (light.enabled)
        shading_uniform->lights[i].radiance = glm::vec4{light.radiance, 0.0f};
      else
        shading_uniform->lights[i].radiance = glm::vec4{};
    }
  }

  // Begin recording current frame command buffer.
  VkCommandBuffer command_buffer = rhi_->BeginImmediateComputePassCommandBuffer();
  // vkResetCommandBuffer(command_buffer, 0);
  // begin render pass
  std::array<VkClearValue, 2> clear_value = {};
  clear_value[1].depthStencil.depth = 1.0f;

  VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  begin_info.renderPass = g_render_pass_;
  begin_info.framebuffer = g_frame_buffers_[current_frame_index];
  begin_info.renderArea = VkRect2D({0, 0, display_width_, display_height_});
  begin_info.clearValueCount = static_cast<uint32_t>(clear_value.size());
  begin_info.pClearValues = clear_value.data();
  vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

  // draw skybox
  VkDescriptorSet uniforms_descriptorset =
      uniform_descriptor_sets_[current_frame_index];

  const std::array<VkDescriptorSet, 2> skybox_descriptorsets = {
      uniforms_descriptorset, skybox_descriptor_set_};

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    skybox_pipeline_);
  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          g_pipeline_layouts_[DescriptorSetType::Skybox], 0,
                          static_cast<uint32_t>(skybox_descriptorsets.size()),
                          skybox_descriptorsets.data(), 0, nullptr);

  vkCmdBindVertexBuffers(command_buffer, 0, 1,
                         &skybox_mesh_->vertex_buffer.resource, &zero_offset);

  vkCmdBindIndexBuffer(command_buffer, skybox_mesh_->index_buffer.resource, 0,
                       VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(command_buffer, skybox_mesh_->num_elements, 1, 0, 0, 0);

  // draw pbr model
  const std::array<VkDescriptorSet, 1> pbr_descriptorsets = {
      pbr_descriptor_set_};

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pbr_pipeline_);

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          g_pipeline_layouts_[DescriptorSetType::Pbr], 1,
                          static_cast<uint32_t>(pbr_descriptorsets.size()),
                          pbr_descriptorsets.data(), 0, nullptr);

  vkCmdBindVertexBuffers(command_buffer, 0, 1,
                         &pbr_mesh_->vertex_buffer.resource, &zero_offset);

  vkCmdBindIndexBuffer(command_buffer, pbr_mesh_->index_buffer.resource, 0,
                       VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(command_buffer, pbr_mesh_->num_elements, 1, 0, 0, 0);

  vkCmdNextSubpass(command_buffer, VK_SUBPASS_CONTENTS_INLINE);

  // draw a full screen triangle for postprocessing/tone mapping
  VkDescriptorSet tonemap_descriptorset =
      tonemap_descriptor_sets_[current_frame_index];

  const std::array<VkDescriptorSet, 1> tonemap_descriptorsets = {
      tonemap_descriptorset};

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    tonemap_pipeline_);

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          g_pipeline_layouts_[DescriptorSetType::ToneMap], 0,
                          static_cast<uint32_t>(tonemap_descriptorsets.size()),
                          tonemap_descriptorsets.data(), 0, nullptr);

  vkCmdDraw(command_buffer, 3, 1, 0, 0);

  vkCmdEndRenderPass(command_buffer);
  vkEndCommandBuffer(command_buffer);

  // submite command buffer
  {
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    rhi_->QueueSubmit(1, current_frame_index, &submit_info);
  }

  rhi_->PresentFrame();
}

void MainRenderPass::preparePassData() 
{
    // load pbr model's assets
    auto &asset_manager = peanut::AssetsManager::GetInstance();

    pbr_mesh_ = asset_manager.LoadMeshBuffer(kPBRModelFile);
    skybox_mesh_ = asset_manager.LoadMeshBuffer(kSkyBoxModelFile);

    albedo_texture_ = asset_manager.LoadTextureData(kPbrAlbedoTextureFile,
                                                    VK_FORMAT_R8G8B8A8_SRGB);
    normal_texture_ = asset_manager.LoadTextureData(kPbrNormalTextureFile,
                                                    VK_FORMAT_R8G8B8A8_UNORM);
    metalness_texture_ = asset_manager.LoadTextureData(kMetalnessTextureFile,
                                                        VK_FORMAT_R8_UNORM, 1);
    roughness_texture_ = asset_manager.LoadTextureData(kRoughnessTextureFile,
                                                        VK_FORMAT_R8_UNORM, 1);

    environment_map_ = rhi_->CreateTexture(kEnvMapSize, kEnvMapSize, 6, 0,
                                            VK_FORMAT_R16G16B16A16_SFLOAT,
                                            VK_IMAGE_USAGE_STORAGE_BIT);
    irradiance_map_ = rhi_->CreateTexture(kIrradianceMapSize, kIrradianceMapSize,
                                        6, 1, VK_FORMAT_R16G16B16A16_SFLOAT,
                                        VK_IMAGE_USAGE_STORAGE_BIT);
    brdf_lut_ =
        rhi_->CreateTexture(kBrdfLutSize, kBrdfLutSize, 1, 1,
                            VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT);
}

void MainRenderPass::CreateUniformBuffer() 
{
    uniform_buffer_.buffer =
        rhi_->CreateBuffer(kUniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniform_buffer_.capacity = kUniformBufferSize;
    uniform_buffer_.cursor = 0;
    rhi_->MapMemory(uniform_buffer_.buffer.memory, 0, VK_WHOLE_SIZE, 0,
                    &uniform_buffer_.host_mem_ptr);
}

void MainRenderPass::DestroyUniformBuffer(UniformBuffer uniform_buffer) 
{
    if (uniform_buffer.host_mem_ptr != nullptr &&
        uniform_buffer.buffer.memory != VK_NULL_HANDLE) 
    {
        rhi_->UnMapMemory(uniform_buffer.buffer.memory);
    }

    rhi_->DestroyBuffer(uniform_buffer.buffer);
}

void MainRenderPass::SetupSamplers() {
  VkSamplerCreateInfo create_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  create_info.minFilter = VK_FILTER_LINEAR;
  create_info.magFilter = VK_FILTER_LINEAR;
  create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  rhi_->CreateSampler(&create_info, &comput_sampler_);

  // Linear, anisotropic sampler, wrap address mode (rendering)
  create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  create_info.anisotropyEnable = VK_TRUE;
  create_info.maxAnisotropy =
      rhi_->GetPhysicalDevice().properties.limits.maxSamplerAnisotropy;
  create_info.minLod = 0;
  create_info.maxLod = FLT_MAX;
  rhi_->CreateSampler(&create_info, &default_sampler_);

  // Linear, non-anisotropic sampler, clamp address mode (sampling BRDF LUT)
  create_info.anisotropyEnable = VK_FALSE;
  create_info.addressModeU =
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;  // TODO: what mean of addressModeU
  create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  rhi_->CreateSampler(&create_info, &brdf_sampler_);
}

void MainRenderPass::SetupComputeDescriptorPool() {
  const std::array<VkDescriptorPoolSize, 2> pool_size = {
      {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
       {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, env_map_levels_}}};

  VkDescriptorPoolCreateInfo create_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  create_info.maxSets = 2;
  create_info.poolSizeCount = (uint32_t)pool_size.size();
  create_info.pPoolSizes = pool_size.data();
  rhi_->CreateDescriptorPool(&create_info, &compute_descriptor_pool_);
}

void MainRenderPass::SetupComputeDescriptorSets() {
  const std::vector<VkDescriptorSetLayoutBinding>
      descriptorset_layout_bindings = {
          {BindingsInputTexture, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_COMPUTE_BIT, &comput_sampler_},
          {BindingsOutputTexture, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1,
           VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
          {BindingsOutputMipTail, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
           env_map_levels_ - 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
      };

  g_descriptor_layouts_[DescriptorSetType::Compute] =
      rhi_->CreateDescriptorSetLayout(descriptorset_layout_bindings);
  compute_descriptor_set_ = rhi_->AllocateDescriptor(
      compute_descriptor_pool_,
      g_descriptor_layouts_[DescriptorSetType::Compute]);
  const std::vector<VkDescriptorSetLayout> pipeline_descriptor_layouts = {
      g_descriptor_layouts_[DescriptorSetType::Compute]};
  const std::vector<VkPushConstantRange> pipeline_push_const = {
      {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(SpecularFilterPushConstants)}};
  g_pipeline_layouts_[DescriptorSetType::Compute] = rhi_->CreatePipelineLayout(
      pipeline_descriptor_layouts, pipeline_push_const);
}

// uniform buffer for rendering viewport and light input
void MainRenderPass::SetupUniformDescriptorSets()
{
  const std::vector<VkDescriptorSetLayoutBinding>
      descriptorset_layout_bindings = {
          {BindingsTransformUniforms, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
           VK_SHADER_STAGE_VERTEX_BIT, nullptr},
          {BindingsShadingUniforms, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
      };
  g_descriptor_layouts_[DescriptorSetType::Uniforms] =
      rhi_->CreateDescriptorSetLayout(descriptorset_layout_bindings);

  auto num_frames = rhi_->GetNumberFrames();
  uniform_descriptor_sets_.resize(num_frames);
  for (uint32_t i = 0; i < num_frames; ++i) {
    uniform_descriptor_sets_[i] = rhi_->AllocateDescriptor(
        g_descriptor_layouts_[DescriptorSetType::Uniforms]);

    transform_uniform_buffer_.push_back(
        AllocateSubStorageFromUniformBuffer<TransformUniforms>(
            uniform_buffer_));

    rhi_->UpdateBufferDescriptorSet(
        uniform_descriptor_sets_[i], BindingsTransformUniforms,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        {transform_uniform_buffer_[i].descriptor_info});

    shading_uniform_buffer_.push_back(
        AllocateSubStorageFromUniformBuffer<ShadingUniforms>(uniform_buffer_));

    rhi_->UpdateBufferDescriptorSet(
        uniform_descriptor_sets_[i], BindingsShadingUniforms,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        {shading_uniform_buffer_[i].descriptor_info});
  }
}

UniformBufferAllocation MainRenderPass::AllocateSubStorageFromUniformBuffer(
    UniformBuffer &buffer, VkDeviceSize size)
{
    const auto &properties = rhi_->GetPhysicalDevice().properties;
    const VkDeviceSize min_alignment =
        properties.limits.minUniformBufferOffsetAlignment;
    const VkDeviceSize align_size =
        RenderUtils::RoundToPowerOfTwo(size, min_alignment);
    if (align_size > properties.limits.maxUniformBufferRange) 
    {
        PEANUT_LOG_FATAL(
            "Request uniform buffer sub-allocation size exceeds "
            "maxUniformBufferRange of current physical device");
    }

    if (buffer.cursor + align_size > buffer.capacity) 
    {
        PEANUT_LOG_FATAL("Failed to allocate with out-of-capacity unifor buffer");
    }

    UniformBufferAllocation allocation;
    allocation.descriptor_info.buffer = buffer.buffer.resource;
    allocation.descriptor_info.offset = buffer.cursor;
    allocation.descriptor_info.range = align_size;
    allocation.host_mem_ptr = reinterpret_cast<uint8_t *>(buffer.host_mem_ptr) + buffer.cursor;

    buffer.cursor += align_size;

    return allocation;
}

void MainRenderPass::SetupRenderpass() {
  std::vector<VkAttachmentDescription> attachments = {
      // index0: main color attachment
      {
          0,
          g_render_targets_[0].color_format,
          static_cast<VkSampleCountFlagBits>(render_samples_),
          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          VK_ATTACHMENT_STORE_OP_DONT_CARE,
          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          VK_ATTACHMENT_STORE_OP_DONT_CARE,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      },
      // index1: depth-stencil attachment
      {0, g_render_targets_[0].depth_format,
       static_cast<VkSampleCountFlagBits>(render_samples_),
       VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
       VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
       VK_IMAGE_LAYOUT_UNDEFINED,
       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
      // index 2: swapchain color attchment
      {
          0,
          VK_FORMAT_B8G8R8A8_UNORM,
          VK_SAMPLE_COUNT_1_BIT,
          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          VK_ATTACHMENT_STORE_OP_STORE,
          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          VK_ATTACHMENT_STORE_OP_DONT_CARE,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      }};
  if (render_samples_ > 1) {
    const VkAttachmentDescription resolveAttachment = {
        0,
        g_resolve_render_targets_[0].color_format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    attachments.push_back(resolveAttachment);
  }
  const std::array<VkAttachmentReference, 1> mainpass_color_refs = {
      {MainColorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
  const std::array<VkAttachmentReference, 1> mainpass_resolve_refs = {
      {ResolveColorAttachment, VK_IMAGE_LAYOUT_GENERAL}};
  const VkAttachmentReference mainpass_depth_stencil_refs = {
      MainDepthStencilAttachment,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  // main pass
  VkSubpassDescription main_pass = {};
  main_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  main_pass.colorAttachmentCount =
      static_cast<uint32_t>(mainpass_color_refs.size());
  main_pass.pColorAttachments = mainpass_color_refs.data();
  main_pass.pDepthStencilAttachment = &mainpass_depth_stencil_refs;
  if (render_samples_ > 1) {
    main_pass.pResolveAttachments = mainpass_resolve_refs.data();
  }
  // Tonemapping subpass
  const std::array<VkAttachmentReference, 1> tonemap_subpass_input_refs = {
      {MainColorAttachment, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};
  const std::array<VkAttachmentReference, 1> tonemap_subpass_multisample_refs =
      {{ResolveColorAttachment, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};
  const std::array<VkAttachmentReference, 1> tonemap_subpass_color_refs = {
      {SwapchainColorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
  VkSubpassDescription tonemap_subpass = {};
  tonemap_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  tonemap_subpass.colorAttachmentCount =
      static_cast<uint32_t>(tonemap_subpass_color_refs.size());
  tonemap_subpass.pColorAttachments = tonemap_subpass_color_refs.data();
  if (render_samples_ > 1) {
    tonemap_subpass.inputAttachmentCount =
        static_cast<uint32_t>(tonemap_subpass_multisample_refs.size());
    tonemap_subpass.pInputAttachments = tonemap_subpass_multisample_refs.data();
  } else {
    tonemap_subpass.inputAttachmentCount =
        static_cast<uint32_t>(tonemap_subpass_input_refs.size());
    tonemap_subpass.pInputAttachments = tonemap_subpass_input_refs.data();
  }

  const std::array<VkSubpassDescription, 2> subpasses = {main_pass,
                                                         tonemap_subpass};

  const VkSubpassDependency subpass_dependency = {
      0,
      1,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_ACCESS_SHADER_READ_BIT,
      VK_DEPENDENCY_BY_REGION_BIT};
  VkRenderPassCreateInfo renderpass_create_info = {
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  renderpass_create_info.attachmentCount =
      static_cast<uint32_t>(attachments.size());
  renderpass_create_info.pAttachments = attachments.data();
  renderpass_create_info.subpassCount = static_cast<uint32_t>(subpasses.size());
  renderpass_create_info.pSubpasses = subpasses.data();
  renderpass_create_info.dependencyCount = 1;
  renderpass_create_info.pDependencies = &subpass_dependency;

  rhi_->CreateRenderPass(&renderpass_create_info, &g_render_pass_);
}

void MainRenderPass::CreateRenderTarget() {
  const VkFormat color_format = VK_FORMAT_R16G16B16A16_SFLOAT;
  const VkFormat depth_format = VK_FORMAT_D32_SFLOAT;

  auto QueryFormatMaxSamples = [this](VkFormat format,
                                      VkImageUsageFlags usage) -> uint32_t {
    VkImageFormatProperties properties;
    rhi_->GetPhysicalDeviceImageFormatProperties(format, VK_IMAGE_TYPE_2D,
                                                 VK_IMAGE_TILING_OPTIMAL, usage,
                                                 0, &properties);

    for (VkSampleCountFlags max_sample_count = VK_SAMPLE_COUNT_64_BIT;
         max_sample_count > VK_SAMPLE_COUNT_1_BIT; max_sample_count >>= 1)
    {
      if (properties.sampleCounts & max_sample_count) {
        return static_cast<uint32_t>(max_sample_count);
      }
    }
    return 1;
  };

  const uint32_t max_color_samples =
      QueryFormatMaxSamples(color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
  const uint32_t max_depth_samples = QueryFormatMaxSamples(
      depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

  render_samples_ =
      ((max_color_samples < max_depth_samples) ? (max_color_samples)
                                               : (max_depth_samples));
  assert(render_samples_ >= 1);

  uint32_t num_frames = rhi_->GetNumberFrames();
  if (num_frames <= 0) {
    PEANUT_LOG_FATAL("Number of frame used must be larger than one");
  }

  // create texture internal

  g_render_targets_.resize(num_frames);
  g_resolve_render_targets_.resize(num_frames);
  for (uint32_t i = 0; i < num_frames; ++i) {
    CreateRenderTargetInternal(g_render_targets_[i], display_width_,
                               display_height_, render_samples_, color_format,
                               depth_format);
    if (render_samples_ > 1) {
      CreateRenderTargetInternal(g_resolve_render_targets_[i], display_width_,
                                 display_height_, 1, color_format,
                                 VK_FORMAT_UNDEFINED);
    }
  }
}

void MainRenderPass::CreateRenderTargetInternal(RenderTarget &target,
                                                uint32_t width, uint32_t height,
                                                uint32_t samples,
                                                VkFormat color_format,
                                                VkFormat depth_format)
{
  RenderTarget render_target = {};
  render_target.width = width;
  render_target.height = height;
  render_target.color_format = color_format;
  render_target.depth_format = depth_format;
  render_target.samples = samples;

  VkImageUsageFlags color_image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (samples == 1) {
    color_image_usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
  }

  if (color_format != VK_FORMAT_UNDEFINED) {
    render_target.color_image = rhi_->CreateImage(
        width, height, 1, 1, samples, color_format, color_image_usage);
  }

  if (depth_format != VK_FORMAT_UNDEFINED) {
    render_target.depth_image =
        rhi_->CreateImage(width, height, 1, 1, samples, depth_format,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  if (render_target.color_image.resource != VK_NULL_HANDLE) {
    render_target.color_view =
        rhi_->CreateImageView(render_target.color_image.resource, color_format,
                              VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1);
  }
  if (render_target.depth_image.resource != VK_NULL_HANDLE) {
    render_target.depth_view =
        rhi_->CreateImageView(render_target.depth_image.resource, depth_format,
                              VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 1);
  }
  target = render_target;
}

void MainRenderPass::DestroyRenderTarget(RenderTarget &render_target) {
  rhi_->DestroyImage(render_target.color_image);
  rhi_->DestroyImage(render_target.depth_image);

  if (render_target.color_view != VK_NULL_HANDLE) {
    rhi_->DestroyImageView(render_target.color_view);
  }

  if (render_target.depth_view != VK_NULL_HANDLE) {
    rhi_->DestroyImageView(render_target.depth_view);
  }

  render_target = {};
}

void MainRenderPass::CreateFrameBuffer() {
  const std::vector<VkImageView> &swapchain_image_view =
      static_cast<VulkanRHI *>(rhi_.get())->GetSwapchainImageView();
  uint32_t num_frames = rhi_->GetNumberFrames();
  g_frame_buffers_.resize(num_frames);
  for (uint32_t i = 0; i < num_frames; ++i) {
    std::vector<VkImageView> attachments = {g_render_targets_[i].color_view,
                                            g_render_targets_[i].depth_view,
                                            swapchain_image_view[i]};
    if (render_samples_ > 1) {
      attachments.push_back(g_resolve_render_targets_[i].color_view);
    }

    VkFramebufferCreateInfo framebuffer_create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebuffer_create_info.renderPass = g_render_pass_;
    framebuffer_create_info.attachmentCount =
        static_cast<uint32_t>(attachments.size());
    framebuffer_create_info.pAttachments = attachments.data();
    framebuffer_create_info.width = display_width_;
    framebuffer_create_info.height = display_height_;
    framebuffer_create_info.layers = 1;
    rhi_->CreateFrameBuffer(&framebuffer_create_info, &g_frame_buffers_[i]);
  }
}

void MainRenderPass::SetupPBRPipeline()
{
  const std::vector<VkVertexInputBindingDescription> vertex_input_bindings = {
      {0, sizeof(Mesh::Vertex), VK_VERTEX_INPUT_RATE_VERTEX},
  };

  const std::vector<VkVertexInputAttributeDescription> vertex_attributes = {
      {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},   // position
      {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 12},  // normal
      {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 24},  // tangent
      {3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 36},  // bitangent
      {4, 0, VK_FORMAT_R32G32_SFLOAT, 48},        // texcoord
  };

  const std::vector<VkDescriptorSetLayoutBinding>
      descriptorset_layout_bindings = {

          {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT, &default_sampler_},  // Albedo texture

          {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT, &default_sampler_},  // Normal texture

          {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT,
           &default_sampler_},  // Metalness texture

          {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT,
           &default_sampler_},  // Roughness texture

          {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT,
           &default_sampler_},  // specular env map texture

          {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT,
           &default_sampler_},  // irradiance map texture

          {6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT, &brdf_sampler_},  // specular brdf lut

      };

  g_descriptor_layouts_[DescriptorSetType::Pbr] =
      rhi_->CreateDescriptorSetLayout(descriptorset_layout_bindings);

  const std::vector<VkDescriptorSetLayout> pipeline_descriptor_set_layouts = {
      g_descriptor_layouts_[DescriptorSetType::Uniforms],
      g_descriptor_layouts_[DescriptorSetType::Pbr]};

  std::vector<VkPushConstantRange> push_constants;
  g_pipeline_layouts_[DescriptorSetType::Pbr] = rhi_->CreatePipelineLayout(
      pipeline_descriptor_set_layouts, push_constants);

  VkPipelineMultisampleStateCreateInfo multi_sample_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

  multi_sample_state_create_info.rasterizationSamples =
      static_cast<VkSampleCountFlagBits>(g_render_targets_[0].samples);

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
  depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
  depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

  VkShaderModule pbr_vs = rhi_->CreateShaderModule(kPbrVertexShaderFile);
  VkShaderModule pbr_fs = rhi_->CreateShaderModule(kPbrFragmentShaderFile);

  pbr_pipeline_ = rhi_->CreateGraphicsPipeline(
      g_render_pass_, 0, pbr_vs, pbr_fs,
      g_pipeline_layouts_[DescriptorSetType::Pbr], &vertex_input_bindings,
      &vertex_attributes, &multi_sample_state_create_info,
      &depth_stencil_state_create_info);

  const std::vector<VkDescriptorImageInfo> textures = {
      {VK_NULL_HANDLE, albedo_texture_->image_view,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      {VK_NULL_HANDLE, normal_texture_->image_view,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      {VK_NULL_HANDLE, metalness_texture_->image_view,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      {VK_NULL_HANDLE, roughness_texture_->image_view,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      /*add env map, irradiance map, lut*/
      {VK_NULL_HANDLE, environment_map_->image_view,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      {VK_NULL_HANDLE, irradiance_map_->image_view,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      {VK_NULL_HANDLE, brdf_lut_->image_view,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};

  pbr_descriptor_set_ =
      rhi_->AllocateDescriptor(g_descriptor_layouts_[DescriptorSetType::Pbr]);
  rhi_->UpdateImageDescriptorSet(pbr_descriptor_set_, 0,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 textures);

  // TODO: update descriptor set
  rhi_->DestroyShaderModule(pbr_vs);
  rhi_->DestroyShaderModule(pbr_fs);
}

void MainRenderPass::SetupToneMapPipeline()
{
  const std::vector<VkDescriptorSetLayoutBinding>
      descriptorset_layout_bindings = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                        1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                        nullptr}};
  g_descriptor_layouts_[DescriptorSetType::ToneMap] =
      rhi_->CreateDescriptorSetLayout(descriptorset_layout_bindings);
  const std::vector<VkDescriptorSetLayout> pipeline_descriptorset_layout = {
      g_descriptor_layouts_[DescriptorSetType::ToneMap]};
  std::vector<VkPushConstantRange> const_range;
  g_pipeline_layouts_[DescriptorSetType::ToneMap] =
      rhi_->CreatePipelineLayout(pipeline_descriptorset_layout, const_range);

  VkShaderModule tonemap_vs =
      rhi_->CreateShaderModule(kTonemapVertexShaderFile);
  VkShaderModule tonemap_fs =
      rhi_->CreateShaderModule(kTonemapFragmentShaderFile);

  tonemap_pipeline_ = rhi_->CreateGraphicsPipeline(
      g_render_pass_, 1, tonemap_vs, tonemap_fs,
      g_pipeline_layouts_[DescriptorSetType::ToneMap]);

  uint32_t num_frames = rhi_->GetNumberFrames();
  tonemap_descriptor_sets_.resize(num_frames);
  for (uint32_t i = 0; i < num_frames; ++i) {
    const VkDescriptorImageInfo image_info = {
        VK_NULL_HANDLE,
        (render_samples_ > 1) ? g_resolve_render_targets_[i].color_view
                              : g_render_targets_[i].color_view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    tonemap_descriptor_sets_[i] = rhi_->AllocateDescriptor(
        g_descriptor_layouts_[DescriptorSetType::ToneMap]);
    rhi_->UpdateImageDescriptorSet(tonemap_descriptor_sets_[i], 0,
                                   VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                   {image_info});
  }

  rhi_->DestroyShaderModule(tonemap_vs);
  rhi_->DestroyShaderModule(tonemap_fs);
}
void MainRenderPass::SetupSkyboxPipeline()
{
  const std::vector<VkVertexInputBindingDescription> vertex_input_bindings = {
      {0, sizeof(Mesh::Vertex), VK_VERTEX_INPUT_RATE_VERTEX},
  };
  const std::vector<VkVertexInputAttributeDescription> vertex_attributes = {
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},  // Position
  };

  const std::vector<VkDescriptorSetLayoutBinding>
      descriptor_set_layout_bindings = {
          {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
           VK_SHADER_STAGE_FRAGMENT_BIT,
           &default_sampler_},  // Environment texture
      };

  g_descriptor_layouts_[DescriptorSetType::Skybox] =
      rhi_->CreateDescriptorSetLayout(descriptor_set_layout_bindings);
  const std::vector<VkDescriptorSetLayout> pipeline_descriptor_set_layout = {
      g_descriptor_layouts_[DescriptorSetType::Uniforms],
      g_descriptor_layouts_[DescriptorSetType::Skybox]};

  std::vector<VkPushConstantRange> const_push_range;
  g_pipeline_layouts_[DescriptorSetType::Skybox] = rhi_->CreatePipelineLayout(
      pipeline_descriptor_set_layout, const_push_range);

  VkPipelineMultisampleStateCreateInfo multisample_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisample_create_info.rasterizationSamples =
      static_cast<VkSampleCountFlagBits>(g_render_targets_[0].samples);

  VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depth_stencil_create_info.depthTestEnable = VK_FALSE;

  VkShaderModule skybox_vs = rhi_->CreateShaderModule(kSkyboxVertexShaderFile);
  VkShaderModule skybox_fs =
      rhi_->CreateShaderModule(kSkyboxFragmentShaderFile);

  skybox_pipeline_ = rhi_->CreateGraphicsPipeline(
      g_render_pass_, 0, skybox_vs, skybox_fs,
      g_pipeline_layouts_[DescriptorSetType::Skybox], &vertex_input_bindings,
      &vertex_attributes, &multisample_create_info, &depth_stencil_create_info);

  const VkDescriptorImageInfo skybox_texture_image_info = {
      VK_NULL_HANDLE, environment_map_->image_view,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  skybox_descriptor_set_ = rhi_->AllocateDescriptor(
      g_descriptor_layouts_[DescriptorSetType::Skybox]);
  rhi_->UpdateImageDescriptorSet(skybox_descriptor_set_, 0,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 {skybox_texture_image_info});

  rhi_->DestroyShaderModule(skybox_vs);
  rhi_->DestroyShaderModule(skybox_fs);
}
void MainRenderPass::LoadAndProcessEnvironmentMap()
{
  std::shared_ptr<TextureData> env_texture_unfiltered = rhi_->CreateTexture(
      kEnvMapSize, kEnvMapSize, 6, 0, VK_FORMAT_R16G16B16A16_SFLOAT,
      VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

  VkShaderModule envmap_cs =
      rhi_->CreateShaderModule(kEnvMapEquirectShaderFile);
  VkPipeline pipeline = rhi_->CreateComputePipeline(
      envmap_cs, g_pipeline_layouts_[DescriptorSetType::Compute]);

  auto &asset_manager = AssetsManager::GetInstance();
  std::shared_ptr<TextureData> env_texture_equirect =
      asset_manager.LoadTextureData(kEnvMapTextureFile,
                                    VK_FORMAT_R32G32B32A32_SFLOAT, 4, 1);
  const VkDescriptorImageInfo input_texture = {
      VK_NULL_HANDLE, env_texture_equirect->image_view,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  const VkDescriptorImageInfo output_texture = {
      VK_NULL_HANDLE, env_texture_unfiltered->image_view,
      VK_IMAGE_LAYOUT_GENERAL};
  rhi_->UpdateImageDescriptorSet(compute_descriptor_set_, BindingsInputTexture,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 {input_texture});
  rhi_->UpdateImageDescriptorSet(compute_descriptor_set_, BindingsOutputTexture,
                                 VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                 {output_texture});
  VkCommandBuffer command_buffer = rhi_->BeginImmediateComputePassCommandBuffer();
  {
    const auto prev_dispatch_barrier =
        TextureMemoryBarrier(*env_texture_unfiltered, 0,
                             VK_ACCESS_SHADER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)
            .MipLevels(0, env_texture_unfiltered->levels);
    rhi_->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             {prev_dispatch_barrier});
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            g_pipeline_layouts_[DescriptorSetType::Compute], 0,
                            1, &compute_descriptor_set_, 0, nullptr);
    vkCmdDispatch(command_buffer, kEnvMapSize / 32, kEnvMapSize / 32, 6);

    const auto post_dispatch_barrier =
        TextureMemoryBarrier(
            *env_texture_unfiltered, VK_ACCESS_SHADER_WRITE_BIT, 0,
            VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
            .MipLevels(0, env_texture_unfiltered->levels);
    rhi_->CmdPipelineBarrier(
        command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {post_dispatch_barrier});
  }
  rhi_->ExecImmediateComputePassCommandBuffer(command_buffer);
  rhi_->DestroyPipeline(pipeline);
  rhi_->DestroyTexture(env_texture_equirect);

  rhi_->GenerateMipmaps(*env_texture_unfiltered);
  // copy mipmap level into destination environment map
  {
    const uint32_t num_mip_tail_levels = env_map_levels_ - 1;
    const VkSpecializationMapEntry specialization_map = {0, 0,
                                                         sizeof(uint32_t)};
    const uint32_t specialization_data[] = {num_mip_tail_levels};
    const VkSpecializationInfo specialization_info = {
        1, &specialization_map, sizeof(specialization_data),
        specialization_data};
    VkShaderModule envmap_vs =
        rhi_->CreateShaderModule(kSpecularEnvMapShaderFile);
    VkPipeline pipeline = rhi_->CreateComputePipeline(
        envmap_vs, g_pipeline_layouts_[DescriptorSetType::Compute],
        &specialization_info);

    VkCommandBuffer command_buffer = rhi_->BeginImmediateComputePassCommandBuffer();
    std::vector<VkImageView> env_map_tail_views;
    std::vector<VkDescriptorImageInfo> env_map_tail_descriptor;
    {
      const std::vector<TextureMemoryBarrier> prev_copy_barriers = {
          TextureMemoryBarrier(*env_texture_unfiltered, 0,
                               VK_ACCESS_TRANSFER_READ_BIT,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
              .MipLevels(0, env_texture_unfiltered->levels),
          TextureMemoryBarrier(
              *environment_map_, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
              .MipLevels(0, environment_map_->levels)};
      rhi_->CmdPipelineBarrier(
          command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT, prev_copy_barriers);

      VkImageCopy copy_region = {};
      copy_region.extent = {environment_map_->width, environment_map_->height,
                            1};
      copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      copy_region.srcSubresource.layerCount = environment_map_->layers;
      copy_region.dstSubresource = copy_region.srcSubresource;
      vkCmdCopyImage(command_buffer, env_texture_unfiltered->image.resource,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     environment_map_->image.resource,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

      const std::vector<TextureMemoryBarrier> post_copy_barriers = {
          TextureMemoryBarrier(
              *env_texture_unfiltered, VK_ACCESS_TRANSFER_READ_BIT,
              VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
              .MipLevels(0, env_texture_unfiltered->levels),
          TextureMemoryBarrier(*environment_map_, VK_ACCESS_TRANSFER_WRITE_BIT,
                               VK_ACCESS_SHADER_WRITE_BIT,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_GENERAL)
              .MipLevels(0, environment_map_->levels),
      };
      rhi_->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                               post_copy_barriers);

      const VkDescriptorImageInfo input_texture = {
          VK_NULL_HANDLE, env_texture_unfiltered->image_view,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
      rhi_->UpdateImageDescriptorSet(
          compute_descriptor_set_, BindingsInputTexture,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, {input_texture});
      for (uint32_t level = 1; level < env_map_levels_; ++level) {
        env_map_tail_views.push_back(rhi_->CreateTextureView(
            environment_map_, VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_IMAGE_ASPECT_COLOR_BIT, level, 1));
        env_map_tail_descriptor.push_back(
            VkDescriptorImageInfo{VK_NULL_HANDLE, env_map_tail_views[level - 1],
                                  VK_IMAGE_LAYOUT_GENERAL});
      }
      rhi_->UpdateImageDescriptorSet(
          compute_descriptor_set_, BindingsOutputMipTail,
          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, env_map_tail_descriptor);
      vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        pipeline);
      vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              g_pipeline_layouts_[DescriptorSetType::Compute],
                              0, 1, &compute_descriptor_set_, 0, nullptr);

      const float delta_roughness =
          1.0f / std::max((float)num_mip_tail_levels, 1.0f);
      for (uint32_t level = 1, size = kEnvMapSize / 2; level < env_map_levels_;
           ++level, size /= 2) {
        const uint32_t num_groups = std::max<uint32_t>(1, size / 32);
        const SpecularFilterPushConstants push_consts = {
            level - 1, level * delta_roughness};
        vkCmdPushConstants(command_buffer,
                           g_pipeline_layouts_[DescriptorSetType::Compute],
                           VK_SHADER_STAGE_COMPUTE_BIT, 0,
                           sizeof(SpecularFilterPushConstants), &push_consts);
        vkCmdDispatch(command_buffer, num_groups, num_groups, 6);
      }
      const auto barrier = TextureMemoryBarrier(
          *environment_map_, VK_ACCESS_SHADER_WRITE_BIT, 0,
          VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      rhi_->CmdPipelineBarrier(command_buffer,
                               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                               VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {barrier});
    }
    rhi_->ExecImmediateComputePassCommandBuffer(command_buffer);

    for (VkImageView tail_view : env_map_tail_views) {
      rhi_->DestroyImageView(tail_view);
    }
    rhi_->DestroyPipeline(pipeline);
    rhi_->DestroyShaderModule(envmap_cs);
    rhi_->DestroyShaderModule(envmap_vs);
    rhi_->DestroyTexture(env_texture_unfiltered);
  }
}

void MainRenderPass::ComputeDiffuseIrradianceMap() {
  VkShaderModule irradiance_map_shader =
      rhi_->CreateShaderModule(kDiffuseIrradianceMapShaderFile);
  VkPipeline pipeline = rhi_->CreateComputePipeline(
      irradiance_map_shader, g_pipeline_layouts_[DescriptorSetType::Compute]);
  const VkDescriptorImageInfo intput_texture = {
      VK_NULL_HANDLE, environment_map_->image_view,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  const VkDescriptorImageInfo out_texture = {
      VK_NULL_HANDLE, irradiance_map_->image_view, VK_IMAGE_LAYOUT_GENERAL};
  rhi_->UpdateImageDescriptorSet(compute_descriptor_set_, BindingsInputTexture,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 {intput_texture});
  rhi_->UpdateImageDescriptorSet(compute_descriptor_set_, BindingsOutputTexture,
                                 VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                 {out_texture});

  VkCommandBuffer command_buffer = rhi_->BeginImmediateComputePassCommandBuffer();
  const auto prev_pipeline_barrier =
      TextureMemoryBarrier(*irradiance_map_, 0, VK_ACCESS_SHADER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
  rhi_->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           {prev_pipeline_barrier});

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          g_pipeline_layouts_[DescriptorSetType::Compute], 0, 1,
                          &compute_descriptor_set_, 0, nullptr);
  vkCmdDispatch(command_buffer, kIrradianceMapSize / 32,
                kIrradianceMapSize / 32, 6);

  const auto post_pipeline_barrier = TextureMemoryBarrier(
      *irradiance_map_, VK_ACCESS_SHADER_WRITE_BIT, 0, VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  rhi_->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                           {post_pipeline_barrier});
  rhi_->ExecImmediateComputePassCommandBuffer(command_buffer);
  rhi_->DestroyPipeline(pipeline);
  rhi_->DestroyShaderModule(irradiance_map_shader);
}

void MainRenderPass::ComputeCookTorranceLut() {
  VkShaderModule lut_cs = rhi_->CreateShaderModule(kBrdfLutShaderFile);
  VkPipeline pipeline = rhi_->CreateComputePipeline(
      lut_cs, g_pipeline_layouts_[DescriptorSetType::Compute]);

  const VkDescriptorImageInfo out_texture = {
      VK_NULL_HANDLE, brdf_lut_->image_view, VK_IMAGE_LAYOUT_GENERAL};
  rhi_->UpdateImageDescriptorSet(compute_descriptor_set_, BindingsOutputTexture,
                                 VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                 {out_texture});
  VkCommandBuffer command_buffer = rhi_->BeginImmediateComputePassCommandBuffer();
  const auto lut_barrier =
      TextureMemoryBarrier(*brdf_lut_, 0, VK_ACCESS_SHADER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
  rhi_->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {lut_barrier});

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          g_pipeline_layouts_[DescriptorSetType::Compute], 0, 1,
                          &compute_descriptor_set_, 0, nullptr);
  vkCmdDispatch(command_buffer, kBrdfLutSize / 32, kBrdfLutSize / 32, 6);

  const auto post_lut_barrier = TextureMemoryBarrier(
      *brdf_lut_, VK_ACCESS_SHADER_WRITE_BIT, 0, VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  rhi_->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                           {post_lut_barrier});

  rhi_->ExecImmediateComputePassCommandBuffer(command_buffer);
  rhi_->DestroyPipeline(pipeline);
  rhi_->DestroyShaderModule(lut_cs);
}

}  // namespace peanut
