#include "runtime/functions/rhi/vulkan/vulkan_rhi.h"

#include <algorithm>
#include <array>
#include <map>
#include <iostream>
#include <set>

#include "runtime/core/base/logger.h"
#include "runtime/functions/render/render_utils.h"

#define PEANUT_XSTR(s) PEANUT_STR(s)
#define PEANUT_STR(s) #s

namespace peanut {
void VulkanRHI::Init(const std::shared_ptr<WindowSystem>& window_system) {
  native_window_ = (GLFWwindow*)window_system->GetNativeWindow();
  if (!native_window_) {
    PEANUT_LOG_ERROR("Glfw window must be created before init vulkan rhi");
    return;
  }

  required_device_features_.shaderStorageImageExtendedFormats = VK_TRUE;
  required_device_features_.samplerAnisotropy = VK_TRUE;
  window_width_ = window_system->GetWidth();
  window_height_ = window_system->GetHeight();

  char const* vk_layer_path = PEANUT_XSTR(PEANUT_VK_LAYER_PATH);
  SetEnvironmentVariableA("VK_LAYER_PATH", vk_layer_path);
  SetEnvironmentVariableA("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");

  SetupInstance();

  CreateWindowSurface();

  SetupPhysicalDevice();

  SetupLogicDevice();

  CreateSwapChain();

  CreateCommandPoolAndCommandBuffers();

  CreateDescriptorPool();

  CreateSyncPrimitives();

  // InitializeFrameIndex();

  totle_frame_count_ = 0;
  current_frame_index_ = 0;
}

void VulkanRHI::Shutdown() {
  PEANUT_LOG_INFO("Destroy all vulkan resource");
  vkDeviceWaitIdle(vk_device_);

  vkDestroyDescriptorPool(vk_device_, descriptor_pool_, nullptr);
  vkDestroyCommandPool(vk_device_, command_pool_, nullptr);
  vkDestroyFence(vk_device_, acquire_next_image_fence_, nullptr);
  vkDestroySwapchainKHR(vk_device_, swapchain_, nullptr);
  vkDestroySurfaceKHR(vk_instance_, window_surface_, nullptr);

  vkDestroyDevice(vk_device_, nullptr);
}

Resource<VkImage> VulkanRHI::CreateImage(uint32_t width, uint32_t height,
                                         uint32_t layers, uint32_t levels,
                                         uint32_t samples, VkFormat format,
                                         VkImageUsageFlags usage) {
  assert(width > 0 && height > 0);
  assert(levels > 0);
  assert(samples > 0 && samples < 64);

  Resource<VkImage> vkImage;

  VkImageCreateInfo create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  create_info.flags = (layers == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
  create_info.imageType = VK_IMAGE_TYPE_2D;
  create_info.format = format;
  create_info.extent = {width, height, 1};
  create_info.mipLevels = levels;
  create_info.arrayLayers = layers;
  create_info.samples = static_cast<VkSampleCountFlagBits>(samples);
  create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  create_info.usage = usage;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  if (VKFAILED(vkCreateImage(vk_device_, &create_info, nullptr,
                             &vkImage.resource))) {
    PEANUT_LOG_ERROR("Failed to create vulkan image");
    return vkImage;
  }

  VkMemoryRequirements requirements;
  vkGetImageMemoryRequirements(vk_device_, vkImage.resource, &requirements);

  VkMemoryAllocateInfo mem_alloc_info = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mem_alloc_info.allocationSize = requirements.size;
  mem_alloc_info.memoryTypeIndex =
      FindMemoryType(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (VKFAILED(vkAllocateMemory(vk_device_, &mem_alloc_info, nullptr,
                                &vkImage.memory))) {
    vkDestroyImage(vk_device_, vkImage.resource, nullptr);
    PEANUT_LOG_FATAL("Failed to allocate memory to image");
  }
  if (VKFAILED(
          vkBindImageMemory(vk_device_, vkImage.resource, vkImage.memory, 0))) {
    vkDestroyImage(vk_device_, vkImage.resource, nullptr);
    vkFreeMemory(vk_device_, vkImage.memory, nullptr);
    PEANUT_LOG_FATAL("Failed to bind memory to vulkan image");
  }
  vkImage.allocation_size = mem_alloc_info.allocationSize;
  vkImage.memory_type_index = mem_alloc_info.memoryTypeIndex;

  return vkImage;
}

VkImageView VulkanRHI::CreateImageView(VkImage image, VkFormat format,
                                       VkImageAspectFlags aspect_mask,
                                       uint32_t base_mip_level,
                                       uint32_t num_mip_levels,
                                       uint32_t layers) {
  VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_info.image = image;
  view_info.viewType =
      (layers == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = aspect_mask;
  view_info.subresourceRange.baseMipLevel = base_mip_level;
  view_info.subresourceRange.levelCount = num_mip_levels;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

  VkImageView view;
  if (VKFAILED(vkCreateImageView(vk_device_, &view_info, nullptr, &view))) {
    PEANUT_LOG_FATAL("Failed to create texture image view");
  }

  return view;
}

Resource<VkBuffer> VulkanRHI::CreateBuffer(VkDeviceSize size,
                                           VkBufferUsageFlags usage,
                                           VkMemoryPropertyFlags memoryFlags) {
  Resource<VkBuffer> buffer;

  VkBufferCreateInfo create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  create_info.size = size;
  create_info.usage = usage;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (VKFAILED(vkCreateBuffer(vk_device_, &create_info, nullptr,
                              &buffer.resource))) {
    PEANUT_LOG_FATAL("Failed to create buffer");
  }

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(vk_device_, buffer.resource,
                                &memory_requirements);

  VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocate_info.allocationSize = memory_requirements.size;
  allocate_info.memoryTypeIndex =
      FindMemoryType(memory_requirements, memoryFlags);
  if (VKFAILED(vkAllocateMemory(vk_device_, &allocate_info, nullptr,
                                &buffer.memory))) {
    PEANUT_LOG_FATAL("Failed to allocate device memory for buffer");
  }
  if (VKFAILED(
          vkBindBufferMemory(vk_device_, buffer.resource, buffer.memory, 0))) {
    PEANUT_LOG_FATAL("Failed to bind device memory to buffer");
  }

  buffer.allocation_size = allocate_info.allocationSize;
  buffer.memory_type_index = allocate_info.memoryTypeIndex;
  return buffer;
}

void VulkanRHI::DestroyBuffer(Resource<VkBuffer> buffer) {
  if (buffer.resource != VK_NULL_HANDLE) {
    vkDestroyBuffer(vk_device_, buffer.resource, nullptr);
  }
  if (buffer.memory != VK_NULL_HANDLE) {
    vkFreeMemory(vk_device_, buffer.memory, nullptr);
  }
}

void VulkanRHI::CopyMemToDevice(VkDeviceMemory memory, const void* data,
                                size_t size) {
  const VkMappedMemoryRange flush_range = {
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, memory, 0, VK_WHOLE_SIZE};

  void* mapped_device_memory;
  if (VKFAILED(vkMapMemory(vk_device_, memory, 0, VK_WHOLE_SIZE, 0,
                           &mapped_device_memory))) {
    PEANUT_LOG_FATAL("Failed to map device memory to host address space");
  }

  std::memcpy(mapped_device_memory, data, size);
  vkFlushMappedMemoryRanges(vk_device_, 1, &flush_range);
  vkUnmapMemory(vk_device_, memory);
}

VkCommandBuffer VulkanRHI::BeginImmediateCommandBuffer() {
  VkCommandBufferBeginInfo begin_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (VKFAILED(vkBeginCommandBuffer(command_buffers_[current_frame_index_],
                                    &begin_info))) {
    PEANUT_LOG_FATAL(
        "Failed to begin immediate command buffer (still in recording state?)");
  }
  return command_buffers_[current_frame_index_];
}

void VulkanRHI::CmdPipelineBarrier(
    VkCommandBuffer command_buffer, VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dst_stage_mask,
    const std::vector<TextureMemoryBarrier>& barriers) {
  vkCmdPipelineBarrier(
      command_buffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr,
      (uint32_t)barriers.size(),
      reinterpret_cast<const VkImageMemoryBarrier*>(barriers.data()));
}

void VulkanRHI::CmdCopyBufferToImage(VkCommandBuffer command_buffer,
                                     Resource<VkBuffer> buffer,
                                     Resource<VkImage> image,
                                     uint32_t image_width,
                                     uint32_t image_height,
                                     VkImageLayout layout) {
  VkBufferImageCopy copy_region = {};
  copy_region.bufferOffset = 0;
  copy_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  copy_region.imageExtent = {image_width, image_height, 1};
  vkCmdCopyBufferToImage(command_buffer, buffer.resource, image.resource,
                         layout, 1, &copy_region);
}

void VulkanRHI::ExecImmediateCommandBuffer(VkCommandBuffer command_buffer) {
  if (VKFAILED(vkEndCommandBuffer(command_buffer))) {
    PEANUT_LOG_FATAL("Failed to end immediate command buffer");
  }

  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;
  vkQueueSubmit(compute_queue_, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(compute_queue_);

  if (VKFAILED(vkResetCommandBuffer(
          command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT))) {
    PEANUT_LOG_FATAL("Failed to reset immediate command buffer");
  }
}

void VulkanRHI::GenerateMipmaps(const TextureData& texture) {
  assert(texture.levels > 1);

  auto command_buffer = BeginImmediateCommandBuffer();

  // Iterate through mip chain and consecutively blit from previous level to
  // next level with linear filtering.
  for (uint32_t level = 1, prev_level_width = texture.width,
                prev_level_height = texture.height;
       level < texture.levels;
       ++level, prev_level_width /= 2, prev_level_height /= 2) {
    const auto& pre_blit_barrier =
        TextureMemoryBarrier(texture, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            .MipLevels(level, 1);
    CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, {pre_blit_barrier});

    VkImageBlit region = {};
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 0,
                             texture.layers};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level, 0,
                             texture.layers};
    region.srcOffsets[1] = {(int32_t)prev_level_width,
                            (int32_t)prev_level_height, 1};
    region.dstOffsets[1] = {(int32_t)(prev_level_width / 2),
                            (int32_t)(prev_level_height / 2), 1};
    vkCmdBlitImage(command_buffer, texture.image.resource,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture.image.resource,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region,
                   VK_FILTER_LINEAR);

    const auto& post_blit_barrier = TextureMemoryBarrier(
        texture, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL).MipLevels(level, 1);
    CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, {post_blit_barrier});
  }
  // Transition whole mip chain to shader read only layout.
  {
    const auto barrier =
        TextureMemoryBarrier(texture, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {barrier});
  }

  ExecImmediateCommandBuffer(command_buffer);
}

void VulkanRHI::CreateSampler(VkSamplerCreateInfo* create_info,
                              VkSampler* out_sampler) {
  if (VKFAILED(
          vkCreateSampler(vk_device_, create_info, nullptr, out_sampler))) {
    PEANUT_LOG_FATAL("Failed to create sampler");
  }
}

VulkanPhysicalDevice VulkanRHI::GetPhysicalDevice() {
  return physical_device_;
}

void VulkanRHI::CreateDescriptorPool(VkDescriptorPoolCreateInfo* create_info,
                                     VkDescriptorPool* out_pool) {
  if (VKFAILED(
          vkCreateDescriptorPool(vk_device_, create_info, nullptr, out_pool))) {
    PEANUT_LOG_FATAL("Failed to create descriptor pool");
  }
}

VkDevice VulkanRHI::GetDevice() { return vk_device_; }

VkDescriptorSet VulkanRHI::AllocateDescriptor(VkDescriptorPool pool,
                                              VkDescriptorSetLayout layout) {
  VkDescriptorSet descriptorset;
  VkDescriptorSetAllocateInfo allocate_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocate_info.descriptorPool = pool;
  allocate_info.descriptorSetCount = 1;
  allocate_info.pSetLayouts = &layout;
  if (VKFAILED(vkAllocateDescriptorSets(vk_device_, &allocate_info,
                                        &descriptorset))) {
    PEANUT_LOG_FATAL("Failed to allocate descriptor sets");
  }
  return descriptorset;
}

VkDescriptorSet VulkanRHI::AllocateDescriptor(VkDescriptorSetLayout layout) {
  return AllocateDescriptor(descriptor_pool_, layout);
}

VkDescriptorSetLayout VulkanRHI::CreateDescriptorSetLayout(
    const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
  VkDescriptorSetLayout layout;
  VkDescriptorSetLayoutCreateInfo create_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  create_info.bindingCount = bindings.size();
  create_info.pBindings = bindings.data();
  if (VKFAILED(vkCreateDescriptorSetLayout(vk_device_, &create_info, nullptr,
                                           &layout))) {
    PEANUT_LOG_FATAL("Failed to create descriptorset layout");
  }
  return layout;
}

VkPipelineLayout VulkanRHI::CreatePipelineLayout(
    const std::vector<VkDescriptorSetLayout>& set_layout,
    const std::vector<VkPushConstantRange>& push_constants) {
  VkPipelineLayout layout;
  VkPipelineLayoutCreateInfo create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  create_info.setLayoutCount = set_layout.size();
  create_info.pSetLayouts = set_layout.data();
  create_info.pushConstantRangeCount = push_constants.size();
  create_info.pPushConstantRanges = push_constants.data();

  if (VKFAILED(
          vkCreatePipelineLayout(vk_device_, &create_info, nullptr, &layout))) {
    PEANUT_LOG_FATAL("Failed to create pipeline layout");
  }

  return layout;
}

void VulkanRHI::UpdateImageDescriptorSet(
    VkDescriptorSet descriptor_set, uint32_t dst_binding,
    VkDescriptorType descriptor_type,
    const std::vector<VkDescriptorImageInfo>& descriptors) {
  VkWriteDescriptorSet write_descriptor_set = {
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write_descriptor_set.dstSet = descriptor_set;
  write_descriptor_set.dstBinding = dst_binding;
  write_descriptor_set.descriptorType = descriptor_type;
  write_descriptor_set.descriptorCount =
      static_cast<uint32_t>(descriptors.size());
  write_descriptor_set.pImageInfo = descriptors.data();
  vkUpdateDescriptorSets(vk_device_, 1, &write_descriptor_set, 0, nullptr);
}

void VulkanRHI::UpdateBufferDescriptorSet(
    VkDescriptorSet descriptor_set, uint32_t dst_binding,
    VkDescriptorType descriptor_type,
    const std::vector<VkDescriptorBufferInfo>& descriptors) {
  VkWriteDescriptorSet write_descriptor_set = {
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write_descriptor_set.dstSet = descriptor_set;
  write_descriptor_set.dstBinding = dst_binding;
  write_descriptor_set.descriptorType = descriptor_type;
  write_descriptor_set.descriptorCount =
      static_cast<uint32_t>(descriptors.size());
  write_descriptor_set.pBufferInfo = descriptors.data();
  vkUpdateDescriptorSets(vk_device_, 1, &write_descriptor_set, 0, nullptr);
}

void VulkanRHI::CreateRenderPass(VkRenderPassCreateInfo* create_info,
                                 VkRenderPass* out_renderpass) {
  if (VKFAILED(vkCreateRenderPass(vk_device_, create_info, nullptr,
                                  out_renderpass))) {
    PEANUT_LOG_FATAL("Failed to create render pass");
  }
}

void VulkanRHI::GetPhysicalDeviceImageFormatProperties(
    VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkImageFormatProperties* out_properties) {
  if (VKFAILED(vkGetPhysicalDeviceImageFormatProperties(
          physical_device_.physic_device_handle, format, type, tiling, usage,
          flags, out_properties))) {
    PEANUT_LOG_FATAL("Failed to Get physical device image format properties");
  }
}

void VulkanRHI::CreateFrameBuffer(VkFramebufferCreateInfo* create_info,
                                  VkFramebuffer* out_framebuffer) {
  if (VKFAILED(vkCreateFramebuffer(vk_device_, create_info, nullptr,
                                   out_framebuffer))) {
    PEANUT_LOG_FATAL("Failed to create frame buffer");
  }
}

bool VulkanRHI::MemoryTypeNeedsStaging(uint32_t memory_type_index) {
  assert(memory_type_index <
         physical_device_.memory_properties.memoryTypeCount);
  const VkMemoryPropertyFlags flags =
      physical_device_.memory_properties.memoryTypes[memory_type_index]
          .propertyFlags;
  return (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0;
}

VkPipeline VulkanRHI::CreateGraphicsPipeline(
    VkRenderPass renderpass, uint32_t subpass, VkShaderModule vs_shader_module,
    VkShaderModule fs_shader_module, VkPipelineLayout pipeline_layout,
    const std::vector<VkVertexInputBindingDescription>* vertex_input_bindings,
    const std::vector<VkVertexInputAttributeDescription>* vertex_attributes,
    const VkPipelineMultisampleStateCreateInfo* multisample_state,
    const VkPipelineDepthStencilStateCreateInfo* depth_stencil_stat) {
  const VkViewport default_viewport = {
      0.0f, 0.0f, (float)window_width_, (float)window_height_, 0.0f, 1.0f};
  const VkRect2D default_scissor = {0, 0, window_width_, window_height_};
  const VkPipelineMultisampleStateCreateInfo default_multisample_state = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      nullptr,
      0,
      VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineColorBlendAttachmentState default_color_blend_attachment_state = {};
  default_color_blend_attachment_state.blendEnable = VK_TRUE;
  default_color_blend_attachment_state.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  const VkPipelineShaderStageCreateInfo shader_stage_create_info[] = {
      {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
       VK_SHADER_STAGE_VERTEX_BIT, vs_shader_module, "main", nullptr},
      {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
       VK_SHADER_STAGE_FRAGMENT_BIT, fs_shader_module, "main", nullptr}};

  VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  if (vertex_input_bindings) {
    vertex_input_state_create_info.vertexBindingDescriptionCount =
        static_cast<uint32_t>(vertex_input_bindings->size());
    vertex_input_state_create_info.pVertexBindingDescriptions =
        vertex_input_bindings->data();
  }
  if (vertex_attributes) {
    vertex_input_state_create_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(vertex_attributes->size());
    vertex_input_state_create_info.pVertexAttributeDescriptions =
        vertex_attributes->data();
  }

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  input_assembly_state_create_info.topology =
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewport_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewport_state_create_info.viewportCount = 1;
  viewport_state_create_info.scissorCount = 1;
  viewport_state_create_info.pViewports = &default_viewport;
  viewport_state_create_info.pScissors = &default_scissor;

  VkPipelineRasterizationStateCreateInfo rasteriz_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasteriz_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasteriz_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasteriz_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasteriz_state_create_info.lineWidth = 1.0f;

  const VkPipelineColorBlendAttachmentState attachment_states[] = {
      default_color_blend_attachment_state};

  VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  color_blend_state_create_info.attachmentCount = 1;
  color_blend_state_create_info.pAttachments = attachment_states;

  VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  graphics_pipeline_create_info.stageCount = 1;
  graphics_pipeline_create_info.pStages = shader_stage_create_info;
  graphics_pipeline_create_info.pVertexInputState =
      &vertex_input_state_create_info;
  graphics_pipeline_create_info.pInputAssemblyState =
      &input_assembly_state_create_info;
  graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
  graphics_pipeline_create_info.pRasterizationState =
      &rasteriz_state_create_info;
  graphics_pipeline_create_info.pColorBlendState =
      &color_blend_state_create_info;
  graphics_pipeline_create_info.pMultisampleState =
      multisample_state != nullptr ? multisample_state
                                   : &default_multisample_state;
  graphics_pipeline_create_info.pDepthStencilState = depth_stencil_stat;
  graphics_pipeline_create_info.layout = pipeline_layout;
  graphics_pipeline_create_info.renderPass = renderpass;
  graphics_pipeline_create_info.subpass = subpass;

  VkPipeline pipeline;
  if (VKFAILED(vkCreateGraphicsPipelines(vk_device_, VK_NULL_HANDLE, 1,
                                         &graphics_pipeline_create_info,
                                         nullptr, &pipeline))) {
    PEANUT_LOG_FATAL("Failed to create graphics render pipeline");
  }

  return pipeline;
}

VkShaderModule VulkanRHI::CreateShaderModule(
    const std::string& shader_file_path) {
  std::vector<char> shader_code;
  if (!RenderUtils::ReadBinaryFile(shader_file_path, shader_code)) {
    PEANUT_LOG_FATAL("Failed to read shader file {0}", shader_file_path.c_str());
  }

  VkShaderModuleCreateInfo create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  create_info.codeSize = shader_code.size();
  create_info.pCode = reinterpret_cast<uint32_t*>(&shader_code[0]);

  VkShaderModule shader_module = VK_NULL_HANDLE;
  if (VKFAILED(vkCreateShaderModule(vk_device_, &create_info, nullptr,
                                    &shader_module))) {
    PEANUT_LOG_FATAL("Failed to create shader module with file {0}",
                     shader_file_path);
  }

  return shader_module;
}

void VulkanRHI::DestroyShaderModule(VkShaderModule shader_module) {
  vkDestroyShaderModule(vk_device_, shader_module, nullptr);
}

std::shared_ptr<TextureData> VulkanRHI::CreateTexture(
    uint32_t width, uint32_t height, uint32_t layers, uint32_t levels,
    VkFormat format, VkImageUsageFlags additional_usage) {
  assert(width > 0 && height > 0);
  assert(layers > 0);

  std::shared_ptr<TextureData> texture = std::make_shared<TextureData>();
  texture->width = width;
  texture->height = height;
  texture->layers = layers;
  texture->levels =
      levels > 0 ? levels : RenderUtils::NumMipmapLevels(width, height);

  VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | additional_usage;
  if (texture->levels > 1) {
    usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // For mipmap generation
  }

  texture->image = CreateImage(width, height, layers, texture->levels, 1, format, usage);
  texture->image_view = CreateImageView(texture->image.resource, format,
                                        VK_IMAGE_ASPECT_COLOR_BIT, 0,
                                        VK_REMAINING_MIP_LEVELS, layers);

  return texture;
}

VkPipeline VulkanRHI::CreateComputePipeline(
    VkShaderModule cs_shader, VkPipelineLayout layout,
    const VkSpecializationInfo* specialize_info) {
  const VkPipelineShaderStageCreateInfo shader_stage = {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      nullptr,
      0,
      VK_SHADER_STAGE_COMPUTE_BIT,
      cs_shader,
      "main",
      specialize_info};
  VkComputePipelineCreateInfo create_info = {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
  create_info.stage = shader_stage;
  create_info.layout = layout;

  VkPipeline pipeline;
  if (VKFAILED(vkCreateComputePipelines(vk_device_, VK_NULL_HANDLE, 1,
                                        &create_info, nullptr, &pipeline))) {
    PEANUT_LOG_FATAL("Failed to create compute pipeline");
  }

  return pipeline;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void VulkanRHI::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

void VulkanRHI::SetupInstance() {
  // app information
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Peanut_Render";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Peanut_Engine";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_0;
  app_info.pNext = nullptr;

  std::vector<const char*> instance_layers;
  std::vector<const char*> instance_extensions;

  uint32_t glfw_required_extensions_num;
  const char** required_extensions = glfwGetRequiredInstanceExtensions(&glfw_required_extensions_num);
  if (glfw_required_extensions_num > 0) {
      instance_extensions = std::vector<const char*>{ required_extensions, required_extensions + glfw_required_extensions_num };
  }

#if _DEBUG
  instance_layers.push_back("VK_LAYER_KHRONOS_validation");
  instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

  // create info
  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  if (!instance_layers.empty()) {
      instance_info.enabledLayerCount = static_cast<uint32_t>(instance_layers.size());
      instance_info.ppEnabledLayerNames = &instance_layers[0];
  }
  if (!instance_extensions.empty()) {
      instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
      instance_info.ppEnabledExtensionNames = &instance_extensions[0]; 
  }

#ifdef _DEBUG
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
  PopulateDebugMessengerCreateInfo(debug_create_info);
  instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
#endif

  // create instance
  if (VKFAILED(vkCreateInstance(&instance_info, nullptr, &vk_instance_))) {
    PEANUT_LOG_FATAL("Failed to Create vulkan instance");
  }

  // volkLoadInstance(vk_instance_);
}

VkImageView VulkanRHI::CreateTextureView(
    const std::shared_ptr<TextureData>& texture, VkFormat format,
    VkImageAspectFlags aspect_mask, uint32_t base_mip_level,
    uint32_t num_mip_levels) {
  VkImageViewCreateInfo create_info = {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  create_info.image = texture->image.resource;
  create_info.viewType =
      (texture->layers == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = format;
  create_info.subresourceRange.aspectMask = aspect_mask;
  create_info.subresourceRange.baseMipLevel = base_mip_level;
  create_info.subresourceRange.levelCount = num_mip_levels;
  create_info.subresourceRange.baseArrayLayer = 0;
  create_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

  VkImageView view;
  if (VKFAILED(vkCreateImageView(vk_device_, &create_info, nullptr, &view))) {
    PEANUT_LOG_FATAL("Failed to create image view");
  }

  return view;
}

void VulkanRHI::PresentFrame() {
  VkResult result;

  VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain_;
  present_info.pImageIndices = &current_frame_index_;
  present_info.pResults = &result;
  if (VKFAILED(vkQueuePresentKHR(present_queue_, &present_info)) ||
      VKFAILED(result)) {
    PEANUT_LOG_FATAL("Failed to queue swapchain image presentation");
  }

  const VkFence fences[] = {acquire_next_image_fence_,
                            frame_submit_fences_[current_frame_index_]};
  const uint32_t fence_num_wait = 2;
  vkWaitForFences(vk_device_, fence_num_wait, fences, VK_TRUE, UINT64_MAX);
  vkResetFences(vk_device_, fence_num_wait, fences);

  if (totle_frame_count_ <= UINT16_MAX) {
    ++totle_frame_count_;
  }
}

void VulkanRHI::SetupPhysicalDevice() {
  assert(vk_instance_ != VK_NULL_HANDLE);

  uint32_t physical_device_count;
  // get device count
  vkEnumeratePhysicalDevices(vk_instance_, &physical_device_count, nullptr);
  if (physical_device_count == 0) {
    PEANUT_LOG_FATAL("Can not get any physical device");
    return;
  }

  std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
  vkEnumeratePhysicalDevices(vk_instance_, &physical_device_count,
                             physical_devices.data());

  VulkanPhysicalDevice suitable_physical_device =
      FindSuitablePhysicalDevice(physical_devices);
  if (suitable_physical_device.physic_device_handle == VK_NULL_HANDLE) {
    PEANUT_LOG_ERROR("Not exist suitable physical device");
    return;
  }

  physical_device_ = suitable_physical_device;

  QuerySurfaceCapabilities(physical_device_, window_surface_);
}

VulkanPhysicalDevice VulkanRHI::FindSuitablePhysicalDevice(
    const std::vector<VkPhysicalDevice>& physical_devices) {
  PEANUT_LOG_DEBUG(
      "find the suitable physical device from all %d physical devices",
      physical_devices.size());

  std::multimap<int32_t, VulkanPhysicalDevice, std::greater<int>>
      ranked_physical_devices;

  for (auto& physical_device_handle : physical_devices) {
    VulkanPhysicalDevice selected_device = {physical_device_handle};

    vkGetPhysicalDeviceProperties(physical_device_handle,
                                  &selected_device.properties);
    vkGetPhysicalDeviceMemoryProperties(physical_device_handle,
                                        &selected_device.memory_properties);
    vkGetPhysicalDeviceFeatures(physical_device_handle,
                                &selected_device.features);

    if (!CheckRequiredFeaturesSupport(required_device_features_,
                                      selected_device.features)) {
      PEANUT_LOG_WARN(
          "physical device (%llu) not support required feature, skip it",
          (uint64_t)physical_device_handle);
      continue;
    }

    if (!CheckPhysicalDeviceExtensionSupport(physical_device_handle,
                                             required_device_extensions_)) {
      PEANUT_LOG_WARN(
          "physical device (%llu) not support required extension, skip it",
          (uint64_t)physical_device_handle);
      continue;
    }

    if (!CheckPhysicalDeviceImageFormatSupport(physical_device_handle)) {
      PEANUT_LOG_WARN(
          "physical device (%llu) not support required image format, skip it",
          (uint64_t)physical_device_handle);
      continue;
    }

    // rank discrete device score higher
    uint32_t rank_score = 0;
    switch (selected_device.properties.deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        rank_score += 10;
        break;
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        rank_score += 1;
        break;
      default:
        break;
    }

    // find the queue families which supports compute and graphic
    FindPhysicalDeviceQueueFamily(physical_device_handle,
                                  selected_device.queue_family_indices);
    if (selected_device.queue_family_indices.present_family.has_value()) {
      ranked_physical_devices.insert(
          std::make_pair(rank_score, selected_device));
    }
  }

  // return the first physical device according to the rank score
  if (ranked_physical_devices.empty()) return VulkanPhysicalDevice{};
  return ranked_physical_devices.begin()->second;
}

bool VulkanRHI::CheckRequiredFeaturesSupport(
    const VkPhysicalDeviceFeatures& required_device_features,
    const VkPhysicalDeviceFeatures& features) {
  // check device whether or not support all required features
  bool required_features_supported = true;
  for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
       ++i) {
      VkBool32 required_feature = reinterpret_cast<const VkBool32*>(&required_device_features)[i];
      VkBool32 actual_support_feature = reinterpret_cast<const VkBool32*>(&features)[i];
    if (required_feature && !actual_support_feature) {
      PEANUT_LOG_WARN("not support feature with index {0}", i);
      required_features_supported = false;
      break;
    }
  }

  return required_features_supported;
}

bool VulkanRHI::CheckPhysicalDeviceExtensionSupport(
    VkPhysicalDevice handle,
    const std::vector<std::string>& required_device_extensions) {
  std::vector<VkExtensionProperties> physical_device_extensions;
  {
    uint32_t extension_numbers = 0;
    vkEnumerateDeviceExtensionProperties(handle, nullptr, &extension_numbers,
                                         nullptr);
    if (extension_numbers > 0) {
      physical_device_extensions.resize(extension_numbers);
      vkEnumerateDeviceExtensionProperties(handle, nullptr, &extension_numbers,
                                           physical_device_extensions.data());
    }
  }

  // check device whether or not support all required extension
  bool required_extension_support = true;
  for (const std::string& required_extension : required_device_extensions) {
    bool extension_found = false;
    for (const VkExtensionProperties& extension : physical_device_extensions) {
      std::string extension_name = extension.extensionName;
      if (extension_name == required_extension) {
        PEANUT_LOG_INFO("find required extension {0}", extension_name.data());
        extension_found = true;
        break;
      }
    }

    if (!extension_found) {
      PEANUT_LOG_WARN("Rquired extension {0} not found",
                      required_extension.data());
      required_extension_support = false;
      break;
    }
  }

  return required_extension_support;
}

bool VulkanRHI::CheckPhysicalDeviceImageFormatSupport(VkPhysicalDevice handle) {
  // TODO: check format properties

  return true;
}

void VulkanRHI::FindPhysicalDeviceQueueFamily(VkPhysicalDevice handle,
                                              QueueFamilyIndices& out_indices) {
  // Enumerate queue families and pick one with both graphics & compute
  // capability.
  std::vector<VkQueueFamilyProperties> queue_family_properties;
  uint32_t queue_family_properties_numbers = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      handle, &queue_family_properties_numbers, nullptr);
  if (queue_family_properties_numbers > 0) {
    queue_family_properties.resize(queue_family_properties_numbers);
    vkGetPhysicalDeviceQueueFamilyProperties(handle,
                                             &queue_family_properties_numbers,
                                             queue_family_properties.data());

    for (uint32_t index = 0; index < queue_family_properties_numbers; ++index) {
      const auto& property = queue_family_properties[index];

      // VK_QUEUE_TRANSFER_BIT is implied for graphics capable queue families.
      if (property.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) {
        out_indices.graphics_family = index;
      }

      if (property.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT) {
        out_indices.compute_family = index;
      }

      // check whether or not support WSI surface
      VkBool32 support_surface = false;
      if (VKFAILED(vkGetPhysicalDeviceSurfaceSupportKHR(
              handle, index, window_surface_, &support_surface))) {
        PEANUT_LOG_FATAL("Failed to query support surface");
        continue;
      }

      if (support_surface) {
        out_indices.present_family = index;
      }

      // check whether or not support presentation
      if (support_surface && (glfwGetPhysicalDevicePresentationSupport(
                                  vk_instance_, handle, index) != GLFW_TRUE)) {
        PEANUT_LOG_WARN("queue family index ({0}) not support presentation mod",
                        index);
        continue;
      }

      break;
    }
  }
}

void VulkanRHI::QuerySurfaceCapabilities(
    VulkanPhysicalDevice& in_physical_device, VkSurfaceKHR surface) {
  if (VKFAILED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          in_physical_device.physic_device_handle, surface,
          &in_physical_device.surface_capabilities))) {
    PEANUT_LOG_FATAL("Get surface capability failed");
    return;
  }

  bool has_surface_format = false;
  uint32_t surface_format_numbers = 0;
  if (VKSUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(
          in_physical_device.physic_device_handle, surface,
          &surface_format_numbers, nullptr)) &&
      surface_format_numbers > 0) {
    in_physical_device.surface_formats.resize(surface_format_numbers);
    if (VKSUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(
            in_physical_device.physic_device_handle, surface,
            &surface_format_numbers, &in_physical_device.surface_formats[0]))) {
      has_surface_format = true;
    }
  }

  if (!has_surface_format) {
    PEANUT_LOG_FATAL("Failed to get surface formats");
  }

  bool has_present_modes = false;
  uint32_t present_mode_numbers = 0;
  if (VKSUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(
          in_physical_device.physic_device_handle, surface,
          &present_mode_numbers, nullptr)) &&
      present_mode_numbers > 0) {
    in_physical_device.present_modes.resize(present_mode_numbers);
    if (VKSUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(
            in_physical_device.physic_device_handle, surface,
            &present_mode_numbers, &in_physical_device.present_modes[0]))) {
      has_present_modes = true;
    }
  }

  if (!has_present_modes) {
    PEANUT_LOG_FATAL("Failed to retrieve physical device support present mod");
  }
}

void VulkanRHI::CreateWindowSurface() {
  if (VKFAILED(glfwCreateWindowSurface(vk_instance_, native_window_, nullptr,
                                       &window_surface_))) {
    PEANUT_LOG_FATAL("Create vulkan window surface failed");
  }
}

void VulkanRHI::SetupLogicDevice() {
  assert(vk_instance_ != VK_NULL_HANDLE);
  assert(physical_device_.physic_device_handle != VK_NULL_HANDLE);
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> queue_families = {
      physical_device_.queue_family_indices.graphics_family.value(),
      //physical_device_.queue_family_indices.compute_family.value(),
      //physical_device_.queue_family_indices.present_family.value()
  };

  float queue_property = 1.0f;
  for (uint32_t queue_family : queue_families) {
    VkDeviceQueueCreateInfo queue_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_property;
    queue_create_infos.emplace_back(std::move(queue_create_info));
  }

  VkDeviceCreateInfo device_create_info = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  device_create_info.queueCreateInfoCount =
      static_cast<uint32_t>(queue_create_infos.size());
  device_create_info.pQueueCreateInfos = queue_create_infos.data();
  device_create_info.pEnabledFeatures = &required_device_features_;
  device_create_info.enabledExtensionCount =
      static_cast<uint32_t>(required_device_extensions_.size());
  const char* entesion_names = required_device_extensions_[0].c_str();
  device_create_info.ppEnabledExtensionNames = &entesion_names;

  if (VKFAILED(vkCreateDevice(physical_device_.physic_device_handle,
                              &device_create_info, nullptr, &vk_device_))) {
    PEANUT_LOG_FATAL("Create logical device failed");
  }

  // volkLoadDevice(vk_device_);

  vkGetDeviceQueue(
      vk_device_, physical_device_.queue_family_indices.graphics_family.value(),
      0, &graphics_queue_);
  vkGetDeviceQueue(vk_device_,
                   physical_device_.queue_family_indices.compute_family.value(),
                   0, &compute_queue_);
  vkGetDeviceQueue(vk_device_,
                   physical_device_.queue_family_indices.present_family.value(),
                   0, &present_queue_);
}

void VulkanRHI::CreateSwapChain() {
  VkPresentModeKHR present_mode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
  // not find in physical device present mode
  if (std::find(physical_device_.present_modes.begin(),
                physical_device_.present_modes.end(),
                present_mode) == physical_device_.present_modes.end()) {
    present_mode = physical_device_.present_modes[0];
  }

  // get image counts
  uint32_t image_counts =
      physical_device_.surface_capabilities.minImageCount + 1;
  if (physical_device_.surface_capabilities.maxImageCount > 0 &&
      image_counts > physical_device_.surface_capabilities.maxImageCount)
    image_counts = physical_device_.surface_capabilities.maxImageCount;

  VkSwapchainCreateInfoKHR swapchain_create_info = {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  swapchain_create_info.surface = window_surface_;
  swapchain_create_info.minImageCount = image_counts;
  swapchain_create_info.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  swapchain_create_info.imageExtent =
      physical_device_.surface_capabilities.currentExtent;
  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_create_info.preTransform =
      physical_device_.surface_capabilities.currentTransform;

  if (physical_device_.queue_family_indices.graphics_family !=
      physical_device_.queue_family_indices.present_family) {
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_create_info.queueFamilyIndexCount = 2;

    uint32_t queue_family_indices[] = {
        physical_device_.queue_family_indices.graphics_family.value(),
        physical_device_.queue_family_indices.present_family.value()};
    swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_create_info.clipped = VK_TRUE;
  swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
  if (VKFAILED(vkCreateSwapchainKHR(vk_device_, &swapchain_create_info, nullptr,
                                    &swapchain_))) {
    PEANUT_LOG_FATAL("Failed to create swapchain");
  }

  swapchain_images_.resize(image_counts);
  if (VKFAILED(vkGetSwapchainImagesKHR(vk_device_, swapchain_, &image_counts,
                                       &swapchain_images_[0]))) {
    PEANUT_LOG_FATAL("Failed to retrieve swapchain image handles");
  }

  swapchain_image_views_.resize(image_counts);
  for (int i = 0; i < image_counts; ++i) {
    VkImageViewCreateInfo view_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_create_info.image = swapchain_images_[i];
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.layerCount = 1;

    if (VKFAILED(vkCreateImageView(vk_device_, &view_create_info, nullptr,
                                   &swapchain_image_views_[i]))) {
      PEANUT_LOG_FATAL("Failed to create image view with index {0}", i);
    }
  }

  frame_in_flight_numbers_ = image_counts;
}

void VulkanRHI::CreateCommandPoolAndCommandBuffers() {
  VkCommandPoolCreateInfo create_info = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  create_info.queueFamilyIndex =
      physical_device_.queue_family_indices.graphics_family.value();
  if (VKFAILED(vkCreateCommandPool(vk_device_, &create_info, nullptr,
                                   &command_pool_))) {
    PEANUT_LOG_FATAL("Failed to create command pool");
  }

  command_buffers_.resize(frame_in_flight_numbers_);
  VkCommandBufferAllocateInfo allocate_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocate_info.commandPool = command_pool_;
  allocate_info.commandBufferCount = frame_in_flight_numbers_;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  if (VKFAILED(vkAllocateCommandBuffers(vk_device_, &allocate_info,
                                        &command_buffers_[0]))) {
    PEANUT_LOG_FATAL("Failed to create command buffer");
  }
}

void VulkanRHI::CreateDescriptorPool() {
  const std::array<VkDescriptorPoolSize, 3> pool_size = {
      {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 16},
       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16},
       {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 16}}};

  VkDescriptorPoolCreateInfo create_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  create_info.maxSets = 16;
  create_info.poolSizeCount = static_cast<uint32_t>(pool_size.size());
  create_info.pPoolSizes = pool_size.data();
  if (VKFAILED(vkCreateDescriptorPool(vk_device_, &create_info, nullptr,
                                      &descriptor_pool_))) {
    PEANUT_LOG_FATAL("Failed to create descriptor pools");
  }
}

template <>
void VulkanRHI::DestroyResource(Resource<VkBuffer>& buffer) {
  if (buffer.resource != VK_NULL_HANDLE) {
    vkDestroyBuffer(vk_device_, buffer.resource, nullptr);
  }

  if (buffer.memory != VK_NULL_HANDLE) {
    vkFreeMemory(vk_device_, buffer.memory, nullptr);
  }
}

template <>
void VulkanRHI::DestroyResource(Resource<VkImage>& image) {
  if (image.resource != VK_NULL_HANDLE) {
    vkDestroyImage(vk_device_, image.resource, nullptr);
  }

  if (image.memory != VK_NULL_HANDLE) {
    vkFreeMemory(vk_device_, image.memory, nullptr);
  }
}

void VulkanRHI::DestroyRenderTarget(RenderTarget& render_target) {
  DestroyResource(render_target.color_image);
  DestroyResource(render_target.depth_image);

  if (render_target.color_view != VK_NULL_HANDLE) {
    vkDestroyImageView(vk_device_, render_target.color_view, nullptr);
  }

  if (render_target.depth_view != VK_NULL_HANDLE) {
    vkDestroyImageView(vk_device_, render_target.depth_view, nullptr);
  }
}

void VulkanRHI::CreateSyncPrimitives() {
  // TODO: Create semaphore
  VkSemaphoreCreateInfo semaphore_create_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

  frame_submit_fences_.resize(frame_in_flight_numbers_);
  image_available_render_semaphores_.resize(frame_in_flight_numbers_);
  image_finish_render_semaphores_.resize(frame_in_flight_numbers_);

    VkFenceCreateInfo create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (VKFAILED(vkCreateFence(vk_device_, &create_info, nullptr,
                                &acquire_next_image_fence_))) {
        PEANUT_LOG_ERROR("Failed to create present fence");
    }
    for (int i = 0; i < frame_in_flight_numbers_; ++i) {
        if (VKFAILED(vkCreateFence(vk_device_, &create_info, nullptr,
                                    &frame_submit_fences_[i]))) {
            PEANUT_LOG_ERROR("Failed to create submit fence with index {0}", i);
        }

        if (VKFAILED(vkCreateSemaphore(vk_device_, &semaphore_create_info, nullptr, &image_available_render_semaphores_[i])) ||
        VKFAILED(vkCreateSemaphore(vk_device_, &semaphore_create_info, nullptr, &image_finish_render_semaphores_[i]))) {
            PEANUT_LOG_ERROR("Failed to create semaphore with index {0}", i);
        }
    }
}

void VulkanRHI::InitializeFrameIndex() {
  if (VKFAILED(vkAcquireNextImageKHR(vk_device_, swapchain_, UINT64_MAX,
                                     VK_NULL_HANDLE, acquire_next_image_fence_,
                                     &current_frame_index_))) {
    PEANUT_LOG_ERROR("Failed to acquire next image");
  }

  vkWaitForFences(vk_device_, 1, &acquire_next_image_fence_, VK_TRUE,
                  UINT64_MAX);
  vkResetFences(vk_device_, 1, &acquire_next_image_fence_);
}

uint32_t VulkanRHI::FindMemoryType(
    const VkMemoryRequirements& memory_requirements,
    VkMemoryPropertyFlags required_flag) {
  for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; ++i) {
    if (memory_requirements.memoryTypeBits & (1 << i)) {
      const auto& memory_type =
          physical_device_.memory_properties.memoryTypes[i];
      if ((memory_type.propertyFlags & required_flag) == required_flag) {
        return i;
      }
    }
  }

  return -1;
}

}  // namespace peanut