#pragma once

#include "render_pass_base.h"

#include <array>
#include <map>

namespace peanut
{
	namespace RenderPipelineType
	{
		enum Type : uint8_t
		{
			MeshGbuffer = 0,
			DeferredLighting,
			ForwardLighting,
			Skybox,
			ColorGrading,
			// Axis,
			PipelineTypeCount
		};
	}
	
	namespace DescriptorLayoutType
	{
		enum Type : uint8_t
		{
			MeshGbuffer = 0,
			// SkeletalMeshGbuffer,
			// PerMaterial,
			DeferredLighting,
			ForwardLighting,
			Skybox,
			ColorGrading,
			// Axis,
			DescriptorLayoutTypeCount
		};
	}

	namespace AttachmentType
	{
		enum Type : uint8_t
		{
			GBufferA_Normal = 0, // normal
			GBufferB_Metallic_Roughness_Occlusion,	  // metallic, occlusion, roughness
			// todo: emissive color 
			GBufferC_BaseColor,	  // albedo (base color)
			DepthImage,
			BackupBuffer,
			// BackupBufferOdd,
			// BackupBufferEven,
			SwapChain,
			AttachmentTypeCount
		};
	}

	namespace SubpassType
	{
		enum Type : uint8_t
		{
			BasePass = 0,
			DeferredLightingPass,
			ForwardLightingPass,
			// ToneMappingPass,
			ColorGradingPass,
			// FXAAPass,
			// UIPass, // combine ui
			SubpassTypeCount
		};
	}


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
		void CreateFramebuffer() override;
		void ResizeSwapchainObject() override { /* todo: implement */ }

	protected:
		VkDescriptorSet CreateGbufferDescriptor();
		void CreateDeferredLightDescriptor();
		VkDescriptorSet CreateForwardLightDescriptor();
		void CreateSkyboxDescriptor();
		void CreateColorGradingDescriptor();

		void UpdateGbufferDescriptor(VkCommandBuffer command_buffer, const PbrMaterial& material_data, VkDescriptorSet dst_descriptor_set);
		void UpdateForwardLightDescriptor(VkCommandBuffer command_buffer, const PbrMaterial& material_data, VkDescriptorSet dst_descriptor_set);
		void UpdateDeferredLightDescriptor();
		void UpdateSkyboxDescriptor();
		void UpdateColorGradingDescriptor();
		
		void RenderMesh(VkCommandBuffer command_buffer, const std::shared_ptr<RenderData>& render_data, bool is_forward = false);
		void RenderDeferredLighting(VkCommandBuffer command_buffer, uint32_t current_frame_index);

		std::vector<std::shared_ptr<RenderData> > transparency_render_data_;

		// setup from render system
		LightingRenderData lighting_render_data_;
		std::optional<SkyboxRenderData> skybox_render_data_;
		std::optional<ColorGradingRenderData> color_grading_render_data_;
	
	private:
		std::optional<SubstorageUniformBuffer> lighting_data_uniform_buffer_;
		std::map<RenderPipelineType::Type, std::vector<VkPushConstantRange> > all_push_constant_range_;
	};
}
