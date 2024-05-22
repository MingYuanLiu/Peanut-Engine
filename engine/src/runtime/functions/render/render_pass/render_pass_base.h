#pragma once

#include "../render_data.h"
#include "runtime/functions/rhi/vulkan/vulkan_rhi.h"

namespace peanut
{
    class IRenderPassBase
    {
    public:
        virtual ~IRenderPassBase() {}

        virtual void Initialize() = 0;
        virtual void DeInitialize() = 0;
        virtual void Render() = 0;

        virtual void CreateRenderPass() = 0;
        virtual void CreateDescriptorSetLayouts() = 0;
        virtual void CreateDescriptorSets() = 0;
        virtual void CreatePipelineLayouts() = 0;
        virtual void CreatePipelines() = 0;
        virtual void CreateRenderTargets() = 0;

        void PrepareRenderPassData(const std::vector<std::shared_ptr<RenderData> >& render_data) { render_data_ = render_data; }

        void SetViewSettings(const ViewSettings& view_settings) { view_settings_ = view_settings; }
        void SetSceneSettings(const SceneSettings& scene_settings) { scene_settings_ = scene_settings; }

    private:
        ViewSettings view_settings_;
        SceneSettings scene_settings_;

        std::vector<std::shared_ptr<RenderData> > render_data_;
    };
}  // namespace peanut