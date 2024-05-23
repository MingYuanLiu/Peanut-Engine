#pragma once

#include "render_pass_base.h"

namespace peanut
{
	enum RenderPipelineType : uint8_t
	{
		MeshGBuffer = 0,
		DeferredLighting,
		MeshLighting,
		Skybox,
		Axis,
		PipelineTypeCount
	};

	enum DescriptorLayoutType : uint8_t
	{
		MeshGlobal = 0,
		PerMesh,
		PerMaterial,
		DeferredLighting,
		Skybox,
		Axis,
		DescriptorLayoutTypeCount
	};

	enum AttachmentType : uint8_t
	{
		GBufferA_Normal = 0, // normal
		GBufferB_Metallic_Occlusion_Roughness,	  // metallic, occlusion, roughness
		GBufferC_BaseColor,	  // albedo (base color)
		EmissiveColor,
		DethImage,
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


		std::weak_ptr<RHI> vulkan_rhi_;
	};
}