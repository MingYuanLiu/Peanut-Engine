#include "runtime/functions/rhi/vulkan/vulkan_rhi.h"

#include <algorithm>
#include <array>
#include <map>

#include "runtime/core/base/logger.h"

#define VKSUCCESS(x) ((x) == VK_SUCCESS)
#define VKFAILED(x) ((x) != VK_SUCCESS)

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

  SetupInstance();

  SetupPhysicalDevice();

  SetupLogicDevice();

  CreateSwapChain();

  CreateCommandPoolAndCommandBuffers();

  CreateDescriptorPool();

  CreateSyncPrimitives();

  InitializeFrameIndex();
}

void VulkanRHI::Shutdown() {
  // TODO
  vkDeviceWaitIdle(vk_device_);

  PEANUT_LOG_INFO("Destroy all vulkan resource");
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

  if (VKFAILED(vkCreateImage(vk_device_, &create_info, nullptr, &vkImage.resource))) {
      PEANUT_LOG_ERROR("Failed to create vulkan image");
      return vkImage;
  }

  VkMemoryRequirements requirements;
  vkGetImageMemoryRequirements(vk_device_, vkImage.resource, &requirements);

  VkMemoryAllocateInfo mem_alloc_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  mem_alloc_info.allocationSize = requirements.size;
  mem_alloc_info.memoryTypeIndex = 0; // TODO: ChooseMemoryType
  if (VKFAILED(vkAllocateMemory(vk_device_, &mem_alloc_info, nullptr, &vkImage.memory))) {
      vkDestroyImage(vk_device_, vkImage.resource, nullptr);
      PEANUT_LOG_FATAL("Failed to allocate memory to image");
  }
  if (VKFAILED(vkBindImageMemory(vk_device_, vkImage.resource, vkImage.memory, 0))) {
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
                                       uint32_t num_mip_levels, uint32_t layers) {
    VkImageViewCreateInfo view_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_info.image = image;
    view_info.viewType = (layers == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
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

void VulkanRHI::SetupInstance() {
  // app information
  VkApplicationInfo app_info;
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Peanut_Render";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Peanut_Engine";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_0;

  // create info
  VkInstanceCreateInfo instance_info;
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  // TODO: setup debug validation layer
  // TODO: add vulkan extension

  // create instance
  if (VKFAILED(vkCreateInstance(&instance_info, nullptr, &vk_instance_))) {
    PEANUT_LOG_FATAL("Failed to Create vulkan instance");
  }

  volkLoadInstance(vk_instance_);
}

void VulkanRHI::SetupPhysicalDevice() {
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
          "physical device (%d) not support required feature, skip it",
          physical_device_handle);
      continue;
    }

    if (!CheckPhysicalDeviceExtensionSupport(physical_device_handle,
                                             required_device_extensions_)) {
      PEANUT_LOG_WARN(
          "physical device (%d) not support required extension, skip it",
          physical_device_handle);
      continue;
    }

    if (!CheckPhysicalDeviceImageFormatSupport(physical_device_handle)) {
      PEANUT_LOG_WARN(
          "physical device (%d) not support required image format, skip it",
          physical_device_handle);
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
    if (reinterpret_cast<const VkBool32*>(&required_device_features)[i] ==
            VK_TRUE &&
        reinterpret_cast<const VkBool32*>(&features)[i] == VK_FALSE) {
      PEANUT_LOG_WARN("not support feature with index %d", i);
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
      vkEnumerateDeviceExtensionProperties(handle, nullptr, &extension_numbers,
                                           physical_device_extensions.data());
    }
  }

  // check device whether or not support all required extension
  bool required_extension_support = true;
  for (auto required_extension : required_device_extensions) {
    bool extension_found = false;
    for (auto extension : physical_device_extensions) {
      std::string extension_name = extension.extensionName;
      if (extension_name == required_extension) {
        PEANUT_LOG_INFO("find required extension %s", extension_name.data());
        extension_found = true;
        break;
      }
    }

    if (!extension_found) {
      PEANUT_LOG_WARN("Rquired extension %s not found",
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
        PEANUT_LOG_WARN("queue family index (%d) not support presentation mod",
                        index);
        continue;
      }
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
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::vector<uint32_t> queue_families = {
      physical_device_.queue_family_indices.graphics_family.value(),
      physical_device_.queue_family_indices.compute_family.value(),
      physical_device_.queue_family_indices.present_family.value()};

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

  volkLoadDevice(vk_device_);

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
      PEANUT_LOG_FATAL("Failed to create image view with index %d", i);
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

void VulkanRHI::CreateRenderTarget() {
  const VkFormat color_format = VK_FORMAT_R16G16B16A16_SFLOAT;
  const VkFormat depth_format = VK_FORMAT_D32_SFLOAT;

  auto QueryFormatMaxSamples = [this](VkFormat format,
                                      VkImageUsageFlags usage) -> uint32_t {
    VkImageFormatProperties properties;
    if (VKFAILED(vkGetPhysicalDeviceImageFormatProperties(
            physical_device_.physic_device_handle, format, VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL, usage, 0, &properties))) {
      PEANUT_LOG_ERROR("Failed to get image format properties");
      return 0;
    }

    for (VkSampleCountFlags max_sample_count = VK_SAMPLE_COUNT_64_BIT;
         max_sample_count > VK_SAMPLE_COUNT_1_BIT; max_sample_count >>= 1) {
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

  render_targets_.resize(frame_in_flight_numbers_);
  // TODO
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

  frame_submit_fences_.resize(frame_in_flight_numbers_);
  {
    VkFenceCreateInfo create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (VKFAILED(vkCreateFence(vk_device_, &create_info, nullptr,
                               &acquire_next_image_fence_))) {
      PEANUT_LOG_ERROR("Failed to create present fence");
    }
    for (int i = 0; i < frame_in_flight_numbers_; ++i) {
      if (VKFAILED(vkCreateFence(vk_device_, &create_info, nullptr,
                                 &frame_submit_fences_[i]))) {
        PEANUT_LOG_ERROR("Failed to create submit fence with index %d", i);
      }
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

}  // namespace peanut