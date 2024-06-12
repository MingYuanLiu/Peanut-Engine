#include "render_pass_base.h"

namespace peanut
{
	void IRenderPassBase::Initialize(PassInitInfo* init_info)
	{
		rhi_ = init_info->rhi_;

		CreateRenderTargets();
		CreateDescriptorSetLayouts();
	}

	void IRenderPassBase::CreatePipelineCache()
	{
		auto rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		VkPipelineCacheCreateInfo pipeline_cache_ci { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
		pipeline_cache_ = rhi->CreatePipelineCache(&pipeline_cache_ci);

		// pipeline create info
		input_assembly_state_ci.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_ci.primitiveRestartEnable = VK_FALSE;

		rasterization_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_ci.depthClampEnable = VK_FALSE;
		rasterization_state_ci.rasterizerDiscardEnable = VK_FALSE;
		rasterization_state_ci.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_state_ci.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_state_ci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterization_state_ci.depthBiasEnable = VK_FALSE;
		rasterization_state_ci.lineWidth = 1.0f;
		
		multisample_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_ci.sampleShadingEnable = VK_TRUE;
		multisample_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_ci.minSampleShading = 0.2f;

		depth_stencil_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_ci.depthTestEnable = VK_TRUE;
		depth_stencil_state_ci.depthWriteEnable = VK_TRUE;
		depth_stencil_state_ci.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_ci.depthBoundsTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment_state {};
		color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment_state.blendEnable = VK_FALSE;
		color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
		color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
		color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

		color_blend_attachment_states.push_back(color_blend_attachment_state);

		color_blend_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_ci.attachmentCount = static_cast<uint32_t>(color_blend_attachment_states.size());
		color_blend_state_ci.pAttachments = color_blend_attachment_states.data();

		viewport_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_ci.viewportCount = 1;
		viewport_state_ci.scissorCount = 1;
		viewport_state_ci.pViewports = nullptr;
		viewport_state_ci.pScissors = nullptr;

		dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_ci.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state_ci.pDynamicStates = dynamic_states.data();

		pipeline_create_info_.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info_.pInputAssemblyState = &input_assembly_state_ci;
		pipeline_create_info_.pMultisampleState = &multisample_state_ci;
		pipeline_create_info_.pRasterizationState = &rasterization_state_ci;
		pipeline_create_info_.pDepthStencilState = &depth_stencil_state_ci;
		pipeline_create_info_.pColorBlendState = &color_blend_state_ci;
		pipeline_create_info_.pViewportState = &viewport_state_ci;
		pipeline_create_info_.pDynamicState = &dynamic_state_ci;

	}

} // namespace peanut
