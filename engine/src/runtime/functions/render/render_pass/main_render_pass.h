#pragma once

#include "render_pass_base.h"

#include <array>

namespace peanut
{
	enum RenderPipelineType : uint8_t
	{
		MeshGbuffer = 0,
		DeferredLighting,
		ForwardLighting,
		Skybox,
		// Axis,
		PipelineTypeCount
	};

	enum DescriptorLayoutType : uint8_t
	{
		MeshGbuffer = 0,
		// SkeletalMeshGbuffer,
		// PerMaterial,
		DeferredLighting,
		ForwardLighting,
		Skybox,
		// Axis,
		DescriptorLayoutTypeCount
	};

	enum AttachmentType : uint8_t
	{
		GBufferA_Normal = 0, // normal
		GBufferB_Metallic_Occlusion_Roughness,	  // metallic, occlusion, roughness
		GBufferC_BaseColor,	  // albedo (base color)
		DepthImage,
		BackupBufferOdd,
		BackupBufferEven,
		AttachmentTypeCount
	};

	enum SubpassType : uint8_t
	{
		BasePass = 0,
		DeferredLightingPass,
		ForwardLightingPass,
		ToneMappingPass,
		ColorGradingPass,
		// FXAAPass,
		// UIPass, // combine ui
		SubpassTypeCount
	};

	struct MainPassInitInfo : PassInitInfo
	{

	};

	class MainRenderPass : public IRenderPassBase
	{
	public:

	public:
		MainRenderPass() = default;
		virtual ~MainRenderPass() {}

		void Initialize(PassInitInfo* init_info) override;
		void DeInitialize() override;
		void Render() override;

		void CreateRenderPass() override;
		void CreateRenderTargets() override;
		void CreateDescriptorSetLayouts() override;
		void CreateDescriptorSets() override;
		void CreatePipelineLayouts() override;
		void CreatePipelines() override;

	private:
		std::vector<RenderDescriptorSet> render_descriptors_;
		std::vector<RenderPipeline> render_pipelines_;

		std::optional<VkRenderPass> render_pass_;
		std::optional<RenderPassTarget> render_target_;


		std::weak_ptr<RHI> rhi_;
	};
}