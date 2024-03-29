#pragma once
// #include <volk.h>
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace peanut {

template <class T>
struct Resource {
  T resource;
  VkDeviceMemory memory;
  VkDeviceSize allocation_size;
  uint32_t memory_type_index;
};

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

struct RenderTarget {
  Resource<VkImage> color_image;
  Resource<VkImage> depth_image;
  VkImageView color_view;
  VkImageView depth_view;
  VkFormat color_format;
  VkFormat depth_format;
  uint32_t width, height;
  uint32_t samples;
};

struct MeshBuffer {
  Resource<VkBuffer> vertex_buffer;
  Resource<VkBuffer> index_buffer;
  uint32_t num_elements;
};

struct TextureData {
  Resource<VkImage> image;
  VkImageView image_view;
  uint32_t width;
  uint32_t height;
  uint32_t channels;
  uint32_t levels;
  uint32_t layers;
  void *pixels;
};

struct TextureMemoryBarrier {
  TextureMemoryBarrier(const TextureData &texture, VkAccessFlags srcAccessMask,
                       VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                       VkImageLayout newLayout) {
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.image.resource;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
  }
  operator VkImageMemoryBarrier() const { return barrier; }
  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};

  TextureMemoryBarrier &AspectMask(VkImageAspectFlags aspectMask) {
    barrier.subresourceRange.aspectMask = aspectMask;
    return *this;
  }
  TextureMemoryBarrier &MipLevels(
      uint32_t baseMipLevel, uint32_t levelCount = VK_REMAINING_MIP_LEVELS) {
    barrier.subresourceRange.baseMipLevel = baseMipLevel;
    barrier.subresourceRange.levelCount = levelCount;
    return *this;
  }
  TextureMemoryBarrier &ArrayLayers(
      uint32_t baseArrayLayer,
      uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) {
    barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
    barrier.subresourceRange.layerCount = layerCount;
    return *this;
  }
};

struct UniformBuffer {
  Resource<VkBuffer> buffer;
  VkDeviceSize capacity;
  VkDeviceSize cursor;
  void *host_mem_ptr;
};

struct UniformBufferAllocation {
  VkDescriptorBufferInfo descriptor_info;
  void *host_mem_ptr;

  template <typename T>
  T *as() const {
    return reinterpret_cast<T *>(host_mem_ptr);
  }
};

struct SpecularFilterPushConstants {
  uint32_t level;
  float roughness;
};

struct TransformUniforms {
  glm::mat4 viewProjectionMatrix;
  glm::mat4 skyProjectionMatrix;
  glm::mat4 sceneRotationMatrix;
};

struct ViewSettings {
  float pitch = 0.0f;
  float yaw = 0.0f;
  float distance = 0.0f;
  float fov = 0.0f;
};

struct SceneSettings {
  float pitch = 0.0f;
  float yaw = 0.0f;

  static const int kNumLights = 3;
  struct Light {
    glm::vec3 direction;
    glm::vec3 radiance;
    bool enabled = false;
  } lights[kNumLights];
};

struct ShadingUniforms {
  struct {
    glm::vec4 direction;
    glm::vec4 radiance;
  } lights[SceneSettings::kNumLights];
  glm::vec4 eye_position;
};

}  // namespace peanut
