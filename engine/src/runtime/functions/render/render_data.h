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

struct TextureData
{
	Resource<VkImage> image;
	VkImageView image_view;
	int width;
	int height;
	int channels;
	int levels;
	void* pixels;
};
}
