#pragma once
#include <memory>

#include "runtime/functions/render/render_data.h"
#include "runtime/functions/window/window_system.h"

#ifndef VKSUCCESS
#define VKSUCCESS(x) ((x) == VK_SUCCESS)
#endif  // !VKSUCCESS
#ifndef VKFAILED
#define VKFAILED(x) ((x) != VK_SUCCESS)
#endif  // !VKFAILED

namespace peanut {
class RHI {
 public:
  virtual ~RHI() {}
  virtual void Init(const std::shared_ptr<WindowSystem>& window_system) = 0;
  virtual void Shutdown() = 0;

  // TODO: set vulkan irrelevant
  virtual Resource<VkImage> CreateImage(uint32_t width, uint32_t height, uint32_t layers, uint32_t levels,
                                        uint32_t samples, VkFormat format, VkImageUsageFlags usage) = 0;

  virtual VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask,
                                      uint32_t base_mip_level, uint32_t num_mip_levels, uint32_t layers) = 0;

  virtual void DestroyImage(Resource<VkImage> image) = 0;

  virtual void DestroyImageView(VkImageView image_view) = 0;

  virtual Resource<VkBuffer> CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags) = 0;

  virtual std::shared_ptr<TextureData> CreateTexture(
      uint32_t width, uint32_t height, uint32_t layers, uint32_t levels,
      VkFormat format, VkImageUsageFlags additional_usage) = 0;

  virtual VkImageView CreateTextureView(
      const std::shared_ptr<TextureData>& texture, VkFormat format,
      VkImageAspectFlags aspect_mask, uint32_t base_mip_level,
      uint32_t num_mip_levels) = 0;

  virtual void DestroyTexture(std::shared_ptr<TextureData>& texture) = 0;

  virtual void CopyMemToDevice(VkDeviceMemory memory, const void* data,
                               size_t size) = 0;

  virtual VkCommandBuffer BeginImmediateCommandBuffer() = 0;

  virtual void CmdPipelineBarrier(
      VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
      VkPipelineStageFlags dstStageMask,
      const std::vector<TextureMemoryBarrier>& barriers) = 0;

  virtual void CmdCopyBufferToImage(VkCommandBuffer command_buffer,
                                    Resource<VkBuffer> buffer,
                                    Resource<VkImage> image,
                                    uint32_t image_width, uint32_t image_height,
                                    VkImageLayout layout) = 0;

  virtual void ExecImmediateCommandBuffer(VkCommandBuffer command_buffer) = 0;

  virtual void DestroyBuffer(Resource<VkBuffer> buffer) = 0;

  virtual void GenerateMipmaps(const TextureData& texture) = 0;

  virtual void CreateSampler(VkSamplerCreateInfo* create_info,
                             VkSampler* out_sampler) = 0;

  virtual void DestroySampler(VkSampler* sampler) = 0;

  virtual VulkanPhysicalDevice GetPhysicalDevice() = 0;

  virtual void CreateDescriptorPool(VkDescriptorPoolCreateInfo* create_info,
                                    VkDescriptorPool* out_pool) = 0;

  virtual VkDevice GetDevice() = 0;

  virtual VkDescriptorSet AllocateDescriptor(VkDescriptorPool pool,
                                             VkDescriptorSetLayout layout) = 0;

  // use internal descriptor pool
  virtual VkDescriptorSet AllocateDescriptor(VkDescriptorSetLayout) = 0;

  virtual void UpdateImageDescriptorSet(
      VkDescriptorSet descriptor_set, uint32_t dst_binding,
      VkDescriptorType descriptor_type,
      const std::vector<VkDescriptorImageInfo>& descriptors) = 0;

  virtual void UpdateBufferDescriptorSet(
      VkDescriptorSet descriptor_set, uint32_t dst_binding,
      VkDescriptorType descriptor_type,
      const std::vector<VkDescriptorBufferInfo>& descriptors) = 0;

  virtual VkDescriptorSetLayout CreateDescriptorSetLayout(
      const std::vector<VkDescriptorSetLayoutBinding>& bindings) = 0;

  virtual VkPipelineLayout CreatePipelineLayout(
      const std::vector<VkDescriptorSetLayout>& set_layout,
      const std::vector<VkPushConstantRange>& push_constants) = 0;

  virtual void DestroyPipelineLayout(VkPipelineLayout& pipeline_layout) = 0;

  virtual uint32_t GetNumberFrames() = 0;

  virtual void MapMemory(VkDeviceMemory memory, VkDeviceSize offset,
                         VkDeviceSize size, VkMemoryMapFlags flags,
                         void** ppdata) = 0;
  virtual void UnMapMemory(VkDeviceMemory memory) = 0;
  // render pass
  virtual void CreateRenderPass(VkRenderPassCreateInfo* create_info,
                                VkRenderPass* out_renderpass) = 0;

  virtual void DestroyRenderPass(VkRenderPass& renderpass) = 0;

  // pipeline
  virtual VkPipeline CreateGraphicsPipeline(
      VkRenderPass renderpass, uint32_t subpass,
      VkShaderModule vs_shader_module, VkShaderModule fs_shader_module,
      VkPipelineLayout pipeline_layout,
      const std::vector<VkVertexInputBindingDescription>*
          vertex_input_bindings = nullptr,
      const std::vector<VkVertexInputAttributeDescription>* vertex_attributes =
          nullptr,
      const VkPipelineMultisampleStateCreateInfo* multisample_state = nullptr,
      const VkPipelineDepthStencilStateCreateInfo* depth_stencil_stat =
          nullptr) = 0;

  virtual VkPipeline CreateComputePipeline(
      VkShaderModule cs_shader, VkPipelineLayout layout,
      const VkSpecializationInfo* specialize_info = nullptr) = 0;

  virtual void DestroyPipeline(VkPipeline) = 0;

  virtual void GetPhysicalDeviceImageFormatProperties(
      VkFormat format, VkImageType type, VkImageTiling tiling,
      VkImageUsageFlags usage, VkImageCreateFlags flags,
      VkImageFormatProperties* out_properties) = 0;

  virtual void CreateFrameBuffer(VkFramebufferCreateInfo* create_info,
                                 VkFramebuffer* out_framebuffer) = 0;

  virtual void DestroyFrameBuffer(VkFramebuffer& framebuffer) = 0;

  // memory whether or not can support host visible type
  virtual bool MemoryTypeNeedsStaging(uint32_t memory_type_index) = 0;

  // shader module
  virtual VkShaderModule CreateShaderModule(
      const std::string& shader_file_path) = 0;

  virtual void DestroyShaderModule(VkShaderModule shader_module) = 0;

  virtual void QueueSubmit(uint32_t submit_count, uint32_t current_frame_index,
                           VkSubmitInfo* submit_info) = 0;
  virtual void PresentFrame() = 0;

  virtual void AcquireNextImage() = 0;
};
}  // namespace peanut