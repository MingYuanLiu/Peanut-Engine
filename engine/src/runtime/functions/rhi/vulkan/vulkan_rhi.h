#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <volk.h>

#include <optional>
#include <vector>

#include "runtime/functions/rhi/rhi.h"

namespace peanut {
struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  std::optional<uint32_t> compute_family;

  bool isComplete() {
    return graphics_family.has_value() && present_family.has_value() &&
           compute_family.has_value();
    ;
  }
};

struct VulkanPhysicalDevice {
  VkPhysicalDevice physic_device_handle;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memory_properties;
  VkPhysicalDeviceFeatures features;
  VkSurfaceCapabilitiesKHR surface_capabilities;
  std::vector<VkSurfaceFormatKHR> surface_formats;
  std::vector<VkPresentModeKHR> present_modes;
  QueueFamilyIndices queue_family_indices;
};

class VulkanRHI : public RHI {
 public:
  virtual void Init(
      const std::shared_ptr<WindowSystem>& window_system) override;
  virtual void Shutdown() override;
  virtual Resource<VkImage> CreateImage(uint32_t width, uint32_t height,
                                        uint32_t layers, uint32_t levels,
                                        uint32_t samples, VkFormat format,
                                        VkImageUsageFlags usage) override;
  virtual VkImageView CreateImageView(VkImage image, VkFormat format,
                                      VkImageAspectFlags aspect_mask,
                                      uint32_t base_mip_level,
                                      uint32_t num_mip_levels,
                                      uint32_t layers) override;

  virtual Resource<VkBuffer> CreateBuffer(
      VkDeviceSize size, VkBufferUsageFlags usage,
      VkMemoryPropertyFlags memoryFlags) override;

  virtual void CopyMemToDevice(VkDeviceMemory memory, const void* data,
                               size_t size) override;

  virtual VkCommandBuffer BeginImmediateCommandBuffer() override;

  void CmdPipelineBarrier(
      VkCommandBuffer command_buffer, VkPipelineStageFlags src_stage_mask,
      VkPipelineStageFlags dst_stage_mask,
      const std::vector<TextureMemoryBarrier>& barriers) override;

  virtual void CmdCopyBufferToImage(VkCommandBuffer command_buffer,
                                    Resource<VkBuffer> buffer,
                                    Resource<VkImage> image,
                                    uint32_t image_width, uint32_t image_height,
                                    VkImageLayout layout);

  virtual void ExecImmediateCommandBuffer(
      VkCommandBuffer command_buffer) override;

  virtual void DestoryBuffer(Resource<VkBuffer> buffer) override;

  virtual void GenerateMipmaps(const TextureData& texture) override;

  uint32_t GetCurrentFrameIndex() { return current_frame_index_; }

  void CreateWindowSurface();
  void SetupInstance();
  void SetupPhysicalDevice();
  void SetupLogicDevice();
  void CreateSwapChain();
  void CreateCommandPoolAndCommandBuffers();
  void CreateDescriptorPool();
  void CreateSyncPrimitives();
  void InitializeFrameIndex();
  uint32_t FindMemoryType(const VkMemoryRequirements& memory_requirements,
                          VkMemoryPropertyFlags required_flag);

  // fixme: do create render target in render pipeline or render pass
  void CreateRenderTarget();

  template <class T>
  void DestroyResource(Resource<T>& resource);
  void DestroyRenderTarget(RenderTarget& render_target);

 protected:
  VulkanPhysicalDevice FindSuitablePhysicalDevice(
      const std::vector<VkPhysicalDevice>& physical_devices);

  bool CheckRequiredFeaturesSupport(
      const VkPhysicalDeviceFeatures& required_device_features,
      const VkPhysicalDeviceFeatures& features);

  bool CheckPhysicalDeviceExtensionSupport(
      VkPhysicalDevice handle,
      const std::vector<std::string>& required_device_extensions);

  bool CheckPhysicalDeviceImageFormatSupport(VkPhysicalDevice handle);

  void FindPhysicalDeviceQueueFamily(VkPhysicalDevice handle,
                                     QueueFamilyIndices& out_indices);

  void QuerySurfaceCapabilities(VulkanPhysicalDevice& in_physical_device,
                                VkSurfaceKHR surface);

 private:
  // the physical device must contains these externsions
  const std::vector<std::string> required_device_extensions_ = {
      "VK_KHR_swapchain"};

  // the physical device must contains the required features
  VkPhysicalDeviceFeatures required_device_features_ = {};

 private:
  GLFWwindow* native_window_;
  VkSurfaceKHR window_surface_;
  uint32_t window_width_;
  uint32_t window_height_;
  uint32_t current_frame_index_;

  VulkanPhysicalDevice physical_device_;

  VkInstance vk_instance_;
  VkDevice vk_device_;

  VkQueue present_queue_;
  VkQueue graphics_queue_;
  VkQueue compute_queue_;

  VkSwapchainKHR swapchain_;
  uint32_t frame_in_flight_numbers_;
  std::vector<VkImage> swapchain_images_;
  std::vector<VkImageView> swapchain_image_views_;
  std::vector<VkFramebuffer> swapchain_frame_buffers_;

  VkCommandPool command_pool_;
  std::vector<VkCommandBuffer> command_buffers_;
  std::vector<RenderTarget> render_targets_;

  VkFence acquire_next_image_fence_;
  std::vector<VkFence> frame_submit_fences_;

  VkDescriptorPool descriptor_pool_;
  uint32_t render_samples_;
};
}  // namespace peanut