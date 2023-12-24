#include <memory>

#include "runtime/functions/render/render_data.h"
#include "runtime/functions/window/window_system.h"

namespace peanut {
class RHI {
 public:
  virtual void Init(const std::shared_ptr<WindowSystem>& window_system) = 0;
  virtual void Shutdown() = 0;

  virtual Resource<VkImage> CreateImage(uint32_t width, uint32_t height,
                                        uint32_t layers, uint32_t levels,
                                        uint32_t samples, VkFormat format,
                                        VkImageUsageFlags usage) = 0;
  virtual VkImageView CreateImageView(VkImage image, VkFormat format,
                                      VkImageAspectFlags aspect_mask,
                                      uint32_t base_mip_level,
                                      uint32_t num_mip_levels,
                                      uint32_t layers) = 0;
  virtual Resource<VkBuffer> CreateBuffer(
      VkDeviceSize size, VkBufferUsageFlags usage,
      VkMemoryPropertyFlags memoryFlags) = 0;
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
  virtual void DestoryBuffer(Resource<VkBuffer> buffer) = 0;
  virtual void GenerateMipmaps(const TextureData& texture) = 0;
};
}  // namespace peanut