#pragma once

// #include <volk.h>
#include <vulkan/vulkan.h>
#include <vector>

#include "runtime/core/context/runtime_context.h"
#include "runtime/functions/assets/asset_manager.h"
#include "runtime/functions/assets/mesh.h"
#include "runtime/functions/render/render_pass_base.h"
#include "runtime/functions/render/render_utils.h"
#include "runtime/functions/rhi/rhi.h"

namespace peanut {
enum DescriptorSetType : uint32_t {
  Uniforms = 0,
  Pbr,
  Skybox,
  ToneMap,
  Compute,
};

enum UniformsDescriptorSetBindName : uint32_t {
  BindingsTransformUniforms = 0,
  BindingsShadingUniforms = 1,
};

enum ComputDescriptorSetBindName : uint32_t {
  BindingsInputTexture = 0,
  BindingsOutputTexture = 1,
  BindingsOutputMipTail = 2,
};

enum AttachmentName : uint32_t {
  MainColorAttachment = 0,
  MainDepthStencilAttachment,
  SwapchainColorAttachment,
  ResolveColorAttachment,
};

class MainRenderPass : public RenderPassBase {
 public:
  MainRenderPass(){}
  virtual ~MainRenderPass() {}
  virtual void Initialize() override;
  virtual void DeInitialize() override;
  virtual void RenderTick(const ViewSettings& view,
                          const SceneSettings& scene) override;
  virtual void preparePassData() override;

 protected:
  void CreateUniformBuffer();
  void CreateFrameBuffer();
  void CreateRenderTarget();
  void CreateRenderTargetInternal(RenderTarget& target, uint32_t width,
                                  uint32_t height, uint32_t samples,
                                  VkFormat color_format, VkFormat depth_format);

  void SetupSamplers();
  void SetupRenderpass();
  void SetupComputDescriptorPool();
  void SetupComputDescriptorSets();
  void SetupUniformDescriptorSets();

  void SetupPBRPipeline();
  void SetupSkyboxPipeline();
  void SetupToneMapPipeline();

  void LoadAndProcessEnvironmentMap();
  void ComputeDiffuseIrradianceMap();
  void ComputeCookTorranceLut();

  template <typename T>
  UniformBufferAllocation AllocateSubStorageFromUniformBuffer(
      UniformBuffer& buffer) {
    return AllocateSubStorageFromUniformBuffer(buffer, sizeof(T));
  }
  UniformBufferAllocation AllocateSubStorageFromUniformBuffer(
      UniformBuffer& buffer, VkDeviceSize size);

 private:
  std::vector<VkDescriptorSetLayout> g_descriptor_layouts_;
  std::vector<VkPipelineLayout> g_pipeline_layouts_;
  std::vector<VkFramebuffer> g_frame_buffers_;
  VkRenderPass g_render_pass_;
  std::vector<RenderTarget> g_render_targets_;
  std::vector<RenderTarget> g_resolve_render_targets_;

  uint32_t render_samples_;

  // render pipelines
  VkPipeline pbr_pipeline_;
  VkPipeline tonemap_pipeline_;
  VkPipeline skybox_pipeline_;

  // samplers
  VkSampler comput_sampler_;
  VkSampler default_sampler_;
  VkSampler brdf_sampler_;

  // descriptor set
  VkDescriptorPool compute_descriptor_pool_;

  std::vector<VkDescriptorSet> tonemap_descriptor_sets_;
  std::vector<VkDescriptorSet> uniform_descriptor_sets_;
  VkDescriptorSet compute_descriptor_set_;
  VkDescriptorSet pbr_descriptor_set_;
  VkDescriptorSet skybox_descriptor_set_;

  // uniform buffer
  UniformBuffer uniform_buffer_;
  //
  std::vector<UniformBufferAllocation> transform_uniform_buffer_;
  std::vector<UniformBufferAllocation> shading_uniform_buffer_;

  // mipmaps
  uint32_t env_map_levels_;
  uint32_t display_width_;
  uint32_t display_height_;

  // pbr model assets
  // TODO: move to asset manager
  std::shared_ptr<TextureData> albedo_texture_;
  std::shared_ptr<TextureData> normal_texture_;
  std::shared_ptr<TextureData> metalness_texture_;
  std::shared_ptr<TextureData> roughness_texture_;

  // calculate pbr map from comput shader
  std::shared_ptr<TextureData> environment_map_;
  std::shared_ptr<TextureData> irradiance_map_;
  std::shared_ptr<TextureData> brdf_lut_;

  // mesh model
  std::shared_ptr<MeshBuffer> pbr_mesh_;
  std::shared_ptr<MeshBuffer> skybox_mesh_;

  static constexpr uint32_t kEnvMapSize = 1024;
  static constexpr uint32_t kIrradianceMapSize = 32;
  static constexpr uint32_t kBrdfLutSize = 256;
  static constexpr VkDeviceSize kUniformBufferSize = 64 * 1024;
  static constexpr uint8_t kNumDescriptorType = 5;

  // FIXME: use window size
  static constexpr uint32_t kDefaultDisplayWidth = 1024;
  static constexpr uint32_t kDefaultDisplayHeight = 1024;

  // default asset path
  // TODO: use reflector system to define the assets resource
  const std::string kPbrAlbedoTextureFile = "assets/textures/cerberus_A.png";
  const std::string kPbrNormalTextureFile = "assets/textures/cerberus_N.png";
  const std::string kMetalnessTextureFile = "assets/textures/cerberus_M.png";
  const std::string kRoughnessTextureFile = "assets/textures/cerberus_R.png";

  const std::string kEnvMapTextureFile = "assets/textures/environment.hdr";

  // shader resource
  const std::string kPbrVertexShaderFile = "assets/spirv/pbr_vs.spv";
  const std::string kPbrFragmentShaderFile = "assets/spirv/pbr_fs.spv";
  const std::string kTonemapVertexShaderFile = "assets/spirv/tonemap_vs.spv";
  const std::string kTonemapFragmentShaderFile =
      "assets/spirv/tonemap_fs.spv";
  const std::string kSkyboxVertexShaderFile = "assets/spirv/skybox_vs.spv";
  const std::string kSkyboxFragmentShaderFile = "assets/spirv/skybox_fs.spv";
  const std::string kEnvMapEquirectShaderFile =
      "assets/spirv/equirect2cube_cs.spv";
  const std::string kSpecularEnvMapShaderFile =
      "assets/spirv/spmap_cs.spv";
  const std::string kDiffuseIrradianceMapShaderFile =
      "assets/spirv/irmap_cs.spv";
  const std::string kBrdfLutShaderFile = "assets/spirv/spbrdf_cs.spv";

  // mesh model
  const std::string kPBRModelFile = "assets/mesh/cerberus.fbx";
  const std::string kSkyBoxModelFile = "./assets/mesh/skybox.obj";

  std::shared_ptr<RHI> rhi_;
};
}  // namespace peanut