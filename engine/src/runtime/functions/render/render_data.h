#pragma once
#include <vulkan/vulkan.h>

namespace peanut {

template <class T>
struct Resource {
  T resource;
  VkDeviceMemory memory;
  VkDeviceSize allocation_size;
  uint32_t memory_type_index;
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

struct TextureData {
  Resource<VkImage> image;
  VkImageView image_view;
  uint32_t width;
  uint32_t height;
  uint32_t channels;
  uint32_t levels;
  uint32_t layers;
  void* pixels;
};

struct TextureMemoryBarrier {
  TextureMemoryBarrier(const TextureData& texture, VkAccessFlags srcAccessMask,
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

  TextureMemoryBarrier& AspectMask(VkImageAspectFlags aspectMask) {
    barrier.subresourceRange.aspectMask = aspectMask;
    return *this;
  }
  TextureMemoryBarrier& MipLevels(
      uint32_t baseMipLevel, uint32_t levelCount = VK_REMAINING_MIP_LEVELS) {
    barrier.subresourceRange.baseMipLevel = baseMipLevel;
    barrier.subresourceRange.levelCount = levelCount;
    return *this;
  }
  TextureMemoryBarrier& ArrayLayers(
      uint32_t baseArrayLayer,
      uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) {
    barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
    barrier.subresourceRange.layerCount = layerCount;
    return *this;
  }
};
}  // namespace peanut
