#include "env_map_compute_pass.h"
#include "../shader_manager.h"

namespace peanut
{
	void EnvironmentMapComputePass::SetupComputeDescriptorPool()
	{
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		const std::vector<VkDescriptorPoolSize> pool_size = 
		{
			{{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
			{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, environment_map_mip_levels_}} 
		};

		VkDescriptorPoolCreateInfo create_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		create_info.maxSets = 2;
		create_info.poolSizeCount = (uint32_t)pool_size.size();
		create_info.pPoolSizes = pool_size.data();
		rhi->CreateDescriptorPool(&create_info, &compute_descriptor_pool_);
	}

	void EnvironmentMapComputePass::SetupComputeSampler()
	{
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		VkSamplerCreateInfo create_info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		create_info.minFilter = VK_FILTER_LINEAR;
		create_info.magFilter = VK_FILTER_LINEAR;
		create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		rhi->CreateSampler(&create_info, &default_sampler_);
	}

	void EnvironmentMapComputePass::SetupDescriptorSetLayout()
	{
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		const std::vector<VkDescriptorSetLayoutBinding> descriptorset_layout_bindings = 
		{
			{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, &default_sampler_},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
			{2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, environment_map_mip_levels_ - 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
		};

		default_descriptor_layout_ = rhi->CreateDescriptorSetLayout(descriptorset_layout_bindings);
	}

	void EnvironmentMapComputePass::SetupComputePipelineLayout()
	{
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		const std::vector<VkDescriptorSetLayout> pipeline_descriptor_layouts = { default_descriptor_layout_ };
		const std::vector<VkPushConstantRange> pipeline_push_const = { {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(SpecularFilterPushConstants)} };

		compute_pipeline_layout_ = rhi->CreatePipelineLayout(pipeline_descriptor_layouts, pipeline_push_const);
	}

	void EnvironmentMapComputePass::CreateComputePipeline()
	{

	}

	void EnvironmentMapComputePass::LoadEnvironmentMap()
	{
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		VkDescriptorSet general_descriptor_set = rhi->AllocateDescriptor(default_descriptor_layout_);

		VkShaderModule equirect2cube_cs = ShaderManager::Get().GetShaderModule(rhi_, "equirect2cube.comp");
		VkPipeline compute_pipeline = rhi->CreateComputePipeline(equirect2cube_cs, compute_pipeline_layout_);



	}
}