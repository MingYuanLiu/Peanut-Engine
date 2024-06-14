#include "render_pass_base.h"
#include "../render_utils.h"

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

	void IRenderPassBase::UpdatePushConstants(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout,
		const std::vector<const void*>& pcos, std::vector<VkPushConstantRange> push_constant_ranges)
	{
		assert(pcos.size() == push_constant_ranges.size());
		
		for (size_t i = 0; i < push_constant_ranges.size(); i++)
		{
			const VkPushConstantRange& push_constant_range = push_constant_ranges[i];
			vkCmdPushConstants(command_buffer, pipeline_layout, push_constant_range.stageFlags, 
				push_constant_range.offset, push_constant_range.size, pcos[i]);
		}
	}

	void IRenderPassBase::CreateUniformBuffer(uint32_t buffer_size)
	{
		std::shared_ptr<VulkanRHI> vulkan_rhi = std::static_pointer_cast<VulkanRHI>(rhi_.lock());
		if (!uniform_buffer_.has_value())
		{
			uniform_buffer_ = std::optional<UniformBuffer>();
		}

		uniform_buffer_->buffer = vulkan_rhi->CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		uniform_buffer_->capacity = buffer_size;
		uniform_buffer_->cursor = 0;
		vulkan_rhi->MapMemory(uniform_buffer_->buffer.memory, 0, VK_WHOLE_SIZE, 0,&uniform_buffer_->host_mem_ptr);
	}

	void IRenderPassBase::DestoryUniformBuffer()
	{
		std::shared_ptr<VulkanRHI> vulkan_rhi = std::static_pointer_cast<VulkanRHI>(rhi_.lock());
		if (uniform_buffer_->host_mem_ptr != nullptr &&
			uniform_buffer_->buffer.memory != VK_NULL_HANDLE)
		{
			vulkan_rhi->UnMapMemory(uniform_buffer_->buffer.memory);
		}

		vulkan_rhi->DestroyBuffer(uniform_buffer_->buffer);
	}

	template<typename T>
	SubstorageUniformBuffer IRenderPassBase::AllocateSubstorageFromUniformBuffer()
	{
		uint32_t buffer_size = sizeof(T);
		auto rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		const auto& properties = rhi->GetPhysicalDevice().properties;
		const VkDeviceSize min_alignment = properties.limits.minUniformBufferOffsetAlignment;
		const VkDeviceSize align_size = RenderUtils::RoundToPowerOfTwo(size, min_alignment);
		if (align_size > properties.limits.maxUniformBufferRange)
		{
			PEANUT_LOG_FATAL(
				"Request uniform buffer sub-allocation size exceeds "
				"maxUniformBufferRange of current physical device");
		}

		if (buffer.cursor + align_size > buffer.capacity)
		{
			PEANUT_LOG_FATAL("Failed to allocate with out-of-capacity unifor buffer");
		}

		SubstorageUniformBuffer substorage;
		substorage.descriptor_info.buffer = buffer.buffer.resource;
		substorage.descriptor_info.offset = buffer.cursor;
		substorage.descriptor_info.range = align_size;
		substorage.host_mem_ptr = reinterpret_cast<uint8_t*>(buffer.host_mem_ptr) + buffer.cursor;

		buffer.cursor += align_size;
		return substorage;
	}

	

} // namespace peanut
