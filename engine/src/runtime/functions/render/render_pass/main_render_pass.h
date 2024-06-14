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
			BackupBufferOdd,
			BackupBufferEven,
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
			ToneMappingPass,
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
		void CreateForwardLightDescriptor();
		void CreateSkyboxDescriptor();

		void UpdateGbufferDescriptor(VkCommandBuffer command_buffer, const PbrMaterial& material_data, VkDescriptorSet dst_descriptor_set);
		void UpdateDeferredLightDescriptor();
		void UpdateForwardLightDescriptor();
		void UpdateSkyboxDescriptor();
		void UpdateLightingUniformbuffer();
		
		void RenderMesh(VkCommandBuffer command_buffer, const std::shared_ptr<RenderData>& render_data, bool IsForward = false);
		void RenderDeferredLighting(VkCommandBuffer command_buffer, uint32_t current_frame_index);

		std::vector<LightingRenderData> lighting_render_data_;
		std::optional<SkyboxRenderData> skybox_render_data_;
	
	private:
		std::optional<SubstorageUniformBuffer> lighting_data_uniform_buffer_;
		std::map<RenderPipelineType::Type, std::vector<VkPushConstantRange> > all_push_constant_range_;
	};
}
