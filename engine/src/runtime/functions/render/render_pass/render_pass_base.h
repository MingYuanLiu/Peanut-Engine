#pragma once

#include "../render_data.h"
#include "runtime/functions/rhi/vulkan/vulkan_rhi.h"
#include <memory>

namespace peanut
{
    struct PassInitInfo
    {
        std::weak_ptr<RHI> rhi_;
    };

    class IRenderPassBase
    {
    public:
        virtual ~IRenderPassBase() {}

        virtual void Initialize(PassInitInfo* init_info);
        virtual void DeInitialize() = 0;
        virtual void Render() = 0;

        virtual void CreateRenderPass() = 0;
        virtual void CreateDescriptorSetLayouts() = 0;
        virtual void CreateDescriptorSets() = 0;
        virtual void CreatePipelineLayouts() = 0;
        virtual void CreatePipelines() = 0;
        virtual void CreateRenderTargets() = 0;
        virtual void CreateFramebuffer() = 0;
        virtual void ResizeSwapchainObject() = 0;
        virtual void UpdatePushConstants(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, 
                                         const std::vector<const void*>& pcos, std::vector<VkPushConstantRange> push_constant_ranges);
        virtual void CreatePipelineCache();
        
        void PrepareRenderPassData(const std::vector<std::shared_ptr<RenderData> >& render_data) { render_data_ = render_data; }

        void SetViewSettings(const ViewSettings& view_settings) { view_settings_ = view_settings; }
        void SetSceneSettings(const SceneSettings& scene_settings) { scene_settings_ = scene_settings; }

        template<typename T>
        SubstorageUniformBuffer AllocateSubstorageFromUniformBuffer();

    protected:
        void CreateUniformBuffer(uint32_t buffer_size);
        void DestoryUniformBuffer();

    protected:
        std::weak_ptr<RHI> rhi_;

        ViewSettings view_settings_;
        SceneSettings scene_settings_;

        std::optional<UniformBuffer> uniform_buffer_;
        std::optional<RenderPassTarget> render_target_;
        std::optional<VkRenderPass> render_pass_;

        std::vector<RenderDescriptorSet> render_descriptors_;
        std::vector<RenderPipeline> render_pipelines_;
        std::vector<VkFramebuffer> render_pass_framebuffer_;
        std::vector<std::shared_ptr<RenderData> > render_data_;

        // pipeline data
        VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
        VkGraphicsPipelineCreateInfo pipeline_create_info_{};
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_ci{};
        VkPipelineRasterizationStateCreateInfo rasterization_state_ci{};
        VkPipelineMultisampleStateCreateInfo multisample_state_ci{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_ci{};
        VkPipelineViewportStateCreateInfo viewport_state_ci{};
        VkPipelineColorBlendStateCreateInfo color_blend_state_ci{};
        std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states{};
        VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
        std::vector<VkDynamicState> dynamic_states{};
    };
}  // namespace peanut