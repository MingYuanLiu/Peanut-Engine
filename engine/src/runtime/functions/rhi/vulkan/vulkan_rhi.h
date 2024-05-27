#pragma once
// #include <volk.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "runtime/functions/rhi/rhi.h"
#include "runtime/functions/render/render_data.h"

namespace peanut {

class VulkanRHI : public RHI 
{
public:
    VulkanRHI() = default;
    virtual ~VulkanRHI() {}
    virtual void Init(const std::shared_ptr<WindowSystem>& window_system) override;
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

    virtual void DestroyImage(Resource<VkImage> image) 
    {
        if (image.resource != VK_NULL_HANDLE)
            vkDestroyImage(vk_device_, image.resource, nullptr);

        if (image.memory != VK_NULL_HANDLE)
            vkFreeMemory(vk_device_, image.memory, nullptr);
    }

    virtual void DestroyImageView(VkImageView image_view) 
    {
        if (image_view != VK_NULL_HANDLE)
            vkDestroyImageView(vk_device_, image_view, nullptr);
    }

    virtual Resource<VkBuffer> CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags) override;

    virtual void DestroyBuffer(Resource<VkBuffer> buffer) override;

    virtual std::shared_ptr<TextureData> CreateTexture(uint32_t width, uint32_t height, uint32_t layers, uint32_t levels,
        VkFormat format, VkImageUsageFlags additional_usage) override;

    VkImageView CreateTextureView(const std::shared_ptr<TextureData>& texture,
                                VkFormat format, VkImageAspectFlags aspect_mask,
                                uint32_t base_mip_level,
                                uint32_t num_mip_levels);

    virtual void DestroyTexture(std::shared_ptr<TextureData>& texture) override 
    {
        if (texture->image_view != VK_NULL_HANDLE)
            vkDestroyImageView(vk_device_, texture->image_view, nullptr);
        DestroyImage(texture->image);
    }

    virtual void CopyMemToDevice(VkDeviceMemory memory, const void* data, size_t size) override;

    virtual VkCommandBuffer BeginImmediateCommandBuffer() override;

    void CmdPipelineBarrier(VkCommandBuffer command_buffer, VkPipelineStageFlags src_stage_mask,
        VkPipelineStageFlags dst_stage_mask,const std::vector<TextureMemoryBarrier>& barriers) override;

    virtual void CmdCopyBufferToImage(VkCommandBuffer command_buffer,
                                    Resource<VkBuffer> buffer,
                                    Resource<VkImage> image,
                                    uint32_t image_width, uint32_t image_height,
                                    VkImageLayout layout);

    virtual void ExecImmediateCommandBuffer(
        VkCommandBuffer command_buffer) override;

    virtual void GenerateMipmaps(const TextureData& texture) override;

    virtual void CreateSampler(VkSamplerCreateInfo* create_info,
                                VkSampler* out_sampler) override;

    virtual void DestroySampler(VkSampler* sampler) override 
    {
        vkDestroySampler(vk_device_, *sampler, nullptr);
    }

    // fixme: do not copy whole structure
    virtual VulkanPhysicalDevice GetPhysicalDevice() override;

    virtual void CreateDescriptorPool(VkDescriptorPoolCreateInfo* create_info,
                                    VkDescriptorPool* out_pool);

    virtual VkDevice GetDevice();

    virtual VkDescriptorSet AllocateDescriptor(VkDescriptorPool pool, VkDescriptorSetLayout layout) override;

    virtual VkDescriptorSet AllocateDescriptor(VkDescriptorSetLayout layout) override;

    virtual VkDescriptorSetLayout CreateDescriptorSetLayout(
        const std::vector<VkDescriptorSetLayoutBinding>& bindings) override;

    virtual VkPipelineLayout CreatePipelineLayout(
        const std::vector<VkDescriptorSetLayout>& set_layout,
        const std::vector<VkPushConstantRange>& push_constants) override;

    virtual uint32_t GetNumberFrames() override
    {
        return frame_in_flight_numbers_;
    }
    virtual void MapMemory(VkDeviceMemory memory, VkDeviceSize offset,
                            VkDeviceSize size, VkMemoryMapFlags flags,
                            void** ppdata) override 
    {
        if (VKFAILED(
                vkMapMemory(vk_device_, memory, offset, size, flags, ppdata))) {
            PEANUT_LOG_FATAL("Failed to Map memory");
        }
    }
    virtual void UnMapMemory(VkDeviceMemory memory) override 
    {
        vkUnmapMemory(vk_device_, memory);
    }
    virtual void UpdateImageDescriptorSet(
        VkDescriptorSet descriptor_set, uint32_t dst_binding,
        VkDescriptorType descriptor_type,
        const std::vector<VkDescriptorImageInfo>& descriptors) override;

    virtual void UpdateBufferDescriptorSet(
        VkDescriptorSet descriptor_set, uint32_t dst_binding,
        VkDescriptorType descriptor_type,
        const std::vector<VkDescriptorBufferInfo>& descriptors) override;

    virtual void CreateRenderPass(VkRenderPassCreateInfo* create_info,
                                VkRenderPass* out_renderpass) override;

    virtual void DestroyRenderPass(VkRenderPass& renderpass) override 
    {
        vkDestroyRenderPass(vk_device_, renderpass, nullptr);
    }

    virtual void GetPhysicalDeviceImageFormatProperties( VkFormat format, VkImageType type, VkImageTiling tiling,
        VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* out_properties) override;

    virtual void CreateFrameBuffer(VkFramebufferCreateInfo* create_info, VkFramebuffer* out_framebuffer) override;

    virtual void DestroyFrameBuffer(VkFramebuffer& framebuffer) override 
    {
        vkDestroyFramebuffer(vk_device_, framebuffer, nullptr);
    }

    virtual bool MemoryTypeNeedsStaging(uint32_t memory_type_index) override;

    virtual VkPipeline CreateGraphicsPipeline(VkRenderPass renderpass, uint32_t subpass,
        VkShaderModule vs_shader_module, VkShaderModule fs_shader_module, VkPipelineLayout pipeline_layout,
        const std::vector<VkVertexInputBindingDescription>* vertex_input_bindings,
        const std::vector<VkVertexInputAttributeDescription>* vertex_attributes,
        const VkPipelineMultisampleStateCreateInfo* multisample_state,
        const VkPipelineDepthStencilStateCreateInfo* depth_stencil_stat) override;

    virtual VkPipeline CreateComputePipeline(
        VkShaderModule cs_shader, VkPipelineLayout layout,
        const VkSpecializationInfo* specialize_info = nullptr);

    virtual void DestroyPipelineLayout(VkPipelineLayout& pipeline_layout) override 
    {
        vkDestroyPipelineLayout(vk_device_, pipeline_layout, nullptr);
    }

    virtual void DestroyPipeline(VkPipeline pipeline) override
    {
        vkDestroyPipeline(vk_device_, pipeline, nullptr);
    }

    virtual VkShaderModule CreateShaderModule(const std::string& shader_file_path) override;

    virtual void DestroyShaderModule(VkShaderModule shader_module) override;

    virtual void QueueSubmit(uint32_t submit_count, uint32_t current_frame_index, VkSubmitInfo* submit_info) override
    {
        vkQueueSubmit(present_queue_, submit_count, submit_info, frame_submit_fences_[current_frame_index]);
    }

    virtual void PresentFrame() override;

    virtual void AcquireNextImage()
    {
        // if (VKFAILED(vkAcquireNextImageKHR(vk_device_, swapchain_, UINT64_MAX,
        //     image_available_render_semaphores_[current_frame_index_],
        //     acquire_next_image_fence_, &current_frame_index_))) {
        //     PEANUT_LOG_FATAL("Failed to acquire next swapchain image");
        // }
    }

    uint32_t GetCurrentFrameIndex() { return current_frame_index_; }
    uint32_t GetRenderSamples() { return render_samples_; }
    const std::vector<VkImageView>& GetSwapchainImageView()
    {
        return swapchain_image_views_;
    }

    VkImage GetCurrentSwapchainImage()
    {
        return swapchain_images_[current_frame_index_];
    }

    uint32_t GetDisplayWidth() { return window_width_; }
    uint32_t GetDisplayHeight() { return window_height_; }

    VkFormat GetDepthImageFormat() { return depth_image_format_; }

protected:
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info);
    void CreateWindowSurface();
    void SetupInstance();
    void SetupPhysicalDevice();
    void SetupLogicDevice();
    void CreateSwapChain();
    void CreateCommandPoolAndCommandBuffers();
    void CreateDescriptorPool();
    void CreateSyncPrimitives();
    void InitializeFrameIndex();
    uint32_t FindMemoryType(const VkMemoryRequirements& memory_requirements, VkMemoryPropertyFlags required_flag);

    template <class T>
    void DestroyResource(Resource<T>& resource);
    void DestroyRenderTarget(RenderTarget& render_target);

    VulkanPhysicalDevice FindSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);

    bool CheckRequiredFeaturesSupport(const VkPhysicalDeviceFeatures& required_device_features, const VkPhysicalDeviceFeatures& features);

    bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice handle, const std::vector<std::string>& required_device_extensions);

    bool CheckPhysicalDeviceImageFormatSupport(VkPhysicalDevice handle);

    void FindPhysicalDeviceQueueFamily(VkPhysicalDevice handle, QueueFamilyIndices& out_indices);

    void QuerySurfaceCapabilities(VulkanPhysicalDevice& in_physical_device, VkSurfaceKHR surface);

    VkFormat FindSuitableDepthFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags feature_flag);

private:
    // the physical device must contains these externsions
    const std::vector<std::string> required_device_extensions_ = {"VK_KHR_swapchain"};

    // the physical device must contains the required features
    VkPhysicalDeviceFeatures required_device_features_ = {};

    private:
    GLFWwindow* native_window_;
    VkSurfaceKHR window_surface_;
    uint32_t window_width_;
    uint32_t window_height_;
    uint32_t current_frame_index_;
    uint64_t totle_frame_count_;

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

    VkFormat depth_image_format_;

    VkCommandPool command_pool_;
    std::vector<VkCommandBuffer> command_buffers_;

    // std::vector<VkSemaphore> image_available_render_semaphores_;
    // std::vector<VkSemaphore> image_finish_render_semaphores_;

    VkFence acquire_next_image_fence_;
    std::vector<VkFence> frame_submit_fences_;

    VkDescriptorPool descriptor_pool_;
    uint32_t render_samples_;
};
}  // namespace peanut