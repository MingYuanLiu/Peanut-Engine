#include "env_map_compute_pass.h"
#include "../shader_manager.h"
#include "functions/assets/asset_manager.h"

namespace peanut
{
	void EnvironmentMapComputePass::Initialize(std::weak_ptr<RHI> rhi, const std::string& environment_map_url)
	{
		if (is_initialized)
		{
			return;
		}

		SetupComputeDescriptorPool();
		SetupComputeSampler();
		SetupDescriptorSetLayout();
		SetupComputePipelineLayout();

		rhi_ = rhi;
		environment_map_url_ = environment_map_url;
		
		is_initialized = true;
	}

	void EnvironmentMapComputePass::Dispatch()
	{
		if (is_dispatched)
		{
			return;
		}

		LoadEnvironmentMap();
		CreateFilteredTexture();
		CreateIrradianceMap();
		CreateBRDFLutTexture();

		is_dispatched = true;
	}

	void EnvironmentMapComputePass::Destroy()
	{
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);
		
		if (is_initialized)
        {
			// destroy common resource
			rhi->DestroyPipelineLayout(compute_pipeline_layout_);
			rhi->DestroySampler(&default_sampler_);
			rhi->DestroyDescriptorPool(&compute_descriptor_pool_);
			rhi->DestroyDescriptorSetLayout(&default_descriptor_layout_);

			compute_pipeline_layout_ = VK_NULL_HANDLE;
			default_sampler_ = VK_NULL_HANDLE;
			compute_descriptor_pool_ = VK_NULL_HANDLE;
			default_descriptor_layout_ = VK_NULL_HANDLE;

			is_initialized = false;
        }

		if (is_dispatched)
		{
			// destroy textures
			rhi->DestroyTexture(prefiltered_texture_);
			rhi->DestroyTexture(environment_map_);
			rhi->DestroyTexture(brdf_lut_texture_);
			rhi->DestroyTexture(env_irradiance_map_);

			prefiltered_texture_= nullptr;
			environment_map_= nullptr;
			brdf_lut_texture_= nullptr;
			env_irradiance_map_= nullptr;

			is_dispatched = false;
		}
	}
	
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

		const std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings = 
		{
			{INPUT_TEXTURE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, &default_sampler_},
			{OUTPUT_TEXTURE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
			{OUTPUT_MIP_TAILS, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, environment_map_mip_levels_ - 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
		};

		default_descriptor_layout_ = rhi->CreateDescriptorSetLayout(descriptor_set_layout_bindings);
	}

	void EnvironmentMapComputePass::SetupComputePipelineLayout()
	{
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		const std::vector<VkDescriptorSetLayout> pipeline_descriptor_layouts = { default_descriptor_layout_ };
		const std::vector<VkPushConstantRange> pipeline_push_const = { {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(SpecularFilterPushConstants)} };

		compute_pipeline_layout_ = rhi->CreatePipelineLayout(pipeline_descriptor_layouts, pipeline_push_const);
	}

	void EnvironmentMapComputePass::LoadEnvironmentMap()
	{
		if (environment_map_ != nullptr)
		{
			PEANUT_LOG_INFO("environment map already loaded");
			return;
		}
		
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		VkDescriptorSet general_descriptor_set = rhi->AllocateDescriptor(default_descriptor_layout_);

		VkShaderModule equirect2cube_cs = ShaderManager::Get().GetShaderModule(rhi_, "equirect2cube.comp");
		VkPipeline compute_pipeline = rhi->CreateComputePipeline(equirect2cube_cs, compute_pipeline_layout_);

		if (environment_map_url_.empty())
		{
			PEANUT_LOG_WARN("environment map url is empty can not load environment map");
			return;
		}

		auto env_texture_original = AssetsManager::GetInstance().LoadTextureData(environment_map_url_, VK_FORMAT_R32G32B32A32_SFLOAT, 4, 1);
		if (env_texture_original == nullptr)
        {
            PEANUT_LOG_WARN("load environment map texture data failed");
            return;
        }

		uint32_t env_map_width = env_texture_original->width;
		uint32_t env_map_height = env_texture_original->height;

		// create the loaded environment map
		prefiltered_texture_ = rhi->CreateTexture(env_map_width, env_map_height, 6, 0,
			VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		
		environment_map_ = rhi->CreateTexture(env_map_width, env_map_height,
			6, 0, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

		// update input image descriptor
		const VkDescriptorImageInfo input_image_descriptor = { default_sampler_, env_texture_original->image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        rhi->UpdateImageDescriptorSet(general_descriptor_set, INPUT_TEXTURE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { input_image_descriptor });

		// update output image descriptor
		const VkDescriptorImageInfo out_cube_image_descriptor = {default_sampler_, environment_map_->image_view, VK_IMAGE_LAYOUT_GENERAL};
        rhi->UpdateImageDescriptorSet(general_descriptor_set, OUTPUT_TEXTURE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, { out_cube_image_descriptor });
		
		// transition image layout
		const auto pre_dispatch_barrier = TextureMemoryBarrier(*environment_map_, 0,
			VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)
			.MipLevels(0, environment_map_->levels);

		const auto post_dispatch_barrier = TextureMemoryBarrier(*environment_map_,
			VK_ACCESS_SHADER_WRITE_BIT, 0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			.MipLevels(0, environment_map_->levels);

		VkCommandBuffer command_buffer = rhi->BeginImmediateComputePassCommandBuffer();
		{
			rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {pre_dispatch_barrier});
			
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
			
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout_, 0,
				1, &general_descriptor_set, 0, nullptr);
			
            vkCmdDispatch(command_buffer, env_map_width / 32, env_map_height / 32, 6);
			
            rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT, {post_dispatch_barrier});
		}
		rhi->ExecImmediateComputePassCommandBuffer(command_buffer);

		// generate mipmap
		rhi->GenerateMipmaps(*environment_map_);
		
		rhi->DestroyTexture(env_texture_original);
	}

	void EnvironmentMapComputePass::CreateIrradianceMap()
	{
		// create irradiance texture
		if (env_irradiance_map_ != nullptr)
		{
			PEANUT_LOG_INFO("irradiance map already loaded");
			return;
		}

		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);
		
		env_irradiance_map_ = rhi->CreateTexture(kDefaultIrradianceMapSize, kDefaultIrradianceMapSize, 6, 1,
			VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT);

		VkShaderModule irradiance_cs = ShaderManager::Get().GetShaderModule(rhi_, "irradiance_env_map.comp");
		VkPipeline irradiance_pipeline = rhi->CreateComputePipeline(irradiance_cs, compute_pipeline_layout_);
		VkDescriptorSet irradiance_ds = rhi->AllocateDescriptor(default_descriptor_layout_);

		// update image descriptor
		const VkDescriptorImageInfo input_texture = {environment_map_->image_sampler, environment_map_->image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        rhi->UpdateImageDescriptorSet(irradiance_ds, INPUT_TEXTURE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { input_texture });

		const VkDescriptorImageInfo output_irradiance_map = {default_sampler_, env_irradiance_map_->image_view, VK_IMAGE_LAYOUT_GENERAL};
		rhi->UpdateImageDescriptorSet(irradiance_ds, OUTPUT_TEXTURE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, { output_irradiance_map });
		VkCommandBuffer command_buffer = rhi->BeginImmediateComputePassCommandBuffer();
        {
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, irradiance_pipeline);
            
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout_, 0,
                1, &irradiance_ds, 0, nullptr);
            
            vkCmdDispatch(command_buffer, env_irradiance_map_->width / 32, env_irradiance_map_->height / 32, 6);
        }
        rhi->ExecImmediateComputePassCommandBuffer(command_buffer);

		// release
		rhi->DestroyPipeline(irradiance_pipeline);
		
	}
	
	void EnvironmentMapComputePass::CreateFilteredTexture()
	{
		if (prefiltered_texture_ != nullptr)
		{
			PEANUT_LOG_INFO("prefiltered map already loaded");
            return;
        }
		
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);
		
		VkDescriptorSet prefiltered_descriptor_set = rhi->AllocateDescriptor(default_descriptor_layout_);
		
		VkCommandBuffer command_buffer = rhi->BeginImmediateComputePassCommandBuffer();
		// copy mipmap image to environment map
		// fixme: maybe not need to copy
		{
			const std::vector<TextureMemoryBarrier> pre_copy_barriers =
			{
				TextureMemoryBarrier(*environment_map_, 0, VK_ACCESS_TRANSFER_READ_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL).MipLevels(0, environment_map_->levels),
				TextureMemoryBarrier(*prefiltered_texture_, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL).MipLevels(0, prefiltered_texture_->levels),
			};

			const std::vector<TextureMemoryBarrier> post_copy_barriers =
			{
				TextureMemoryBarrier(*environment_map_, VK_ACCESS_TRANSFER_READ_BIT, 0,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).MipLevels(0, environment_map_->levels),
			    TextureMemoryBarrier(*prefiltered_texture_, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL).MipLevels(0, prefiltered_texture_->levels),
            };

			// do copy
			rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, pre_copy_barriers);

			const uint32_t env_map_width = environment_map_->width;
			const uint32_t env_map_height = environment_map_->height;
			
			VkImageCopy copy_region = {};
			copy_region.extent = {env_map_width, env_map_height, 1};
			copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_region.srcSubresource.layerCount = environment_map_->layers;
			copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_region.dstSubresource.layerCount = prefiltered_texture_->layers;
			vkCmdCopyImage(command_buffer, environment_map_->image.resource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                prefiltered_texture_->image.resource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

			rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, post_copy_barriers);
		}
		
		std::vector<VkImageView> prefilter_out_tail_image_views;
		std::vector<VkDescriptorImageInfo> prefilter_out_tail_image_descriptor;
		
		// compute prefilter texture
		// create compute pipeline
		VkShaderModule prefilter_cs = ShaderManager::Get().GetShaderModule(rhi_, "prefiltered_env_map.comp");
		const VkSpecializationMapEntry map_entry = {0, 0, sizeof(uint32_t)};
		uint32_t mip_levels = environment_map_->levels - 1;
		const VkSpecializationInfo specialization_info = {1, &map_entry, sizeof(uint32_t), &mip_levels};
		VkPipeline prefilter_pipeline = rhi->CreateComputePipeline(prefilter_cs, compute_pipeline_layout_, &specialization_info);
		
		const VkDescriptorImageInfo prefiltered_input_texture =
		{
			environment_map_->image_sampler, environment_map_->image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		rhi->UpdateImageDescriptorSet(prefiltered_descriptor_set, INPUT_TEXTURE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { prefiltered_input_texture });

		for (uint32_t level = 1; level < prefiltered_texture_->levels; ++level)
		{
			prefilter_out_tail_image_views.push_back(
				rhi->CreateTextureView(prefiltered_texture_, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, level, 1));
			prefilter_out_tail_image_descriptor.push_back(
                { default_sampler_, prefilter_out_tail_image_views[level - 1], VK_IMAGE_LAYOUT_GENERAL });

			rhi->UpdateImageDescriptorSet(prefiltered_descriptor_set, OUTPUT_MIP_TAILS, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, { prefilter_out_tail_image_descriptor[level - 1] });
		}
		
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, prefilter_pipeline);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout_, 0,
            1, &prefiltered_descriptor_set, 0, nullptr);

		const float delta_roughness = 1.0f / std::max(static_cast<float>(mip_levels), 1.0f);
		for (uint32_t level = 1; level < prefiltered_texture_->levels; ++level)
		{
			uint32_t mip_size = prefiltered_texture_->width >> level;
			const uint32_t num_groups = std::max(1.0f, static_cast<float>(mip_size) / 32.0f);
			const SpecularFilterPushConstants push_constant = { level - 1,  static_cast<float>(level) * delta_roughness };
			vkCmdPushConstants(command_buffer, compute_pipeline_layout_, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(SpecularFilterPushConstants), &push_constant);
			vkCmdDispatch(command_buffer, num_groups, num_groups, 6);
		}

		rhi->ExecImmediateComputePassCommandBuffer(command_buffer);

		// release
		for (auto& image_view : prefilter_out_tail_image_views)
        {
            rhi->DestroyImageView(image_view);
        }

		rhi->DestroyPipeline(prefilter_pipeline);
	}

	void EnvironmentMapComputePass::CreateBRDFLutTexture()
	{
		std::shared_ptr<RHI> rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		VkShaderModule brdf_lut_cs = ShaderManager::Get().GetShaderModule(rhi_, "brdf_lut.comp");
		VkPipeline brdf_lut_pipeline = rhi->CreateComputePipeline(brdf_lut_cs, compute_pipeline_layout_);
		VkDescriptorSet brdf_lut_ds = rhi->AllocateDescriptor(default_descriptor_layout_);

		// create brdf lut texture
		brdf_lut_texture_ = rhi->CreateTexture(kDefaultBRDFLutSize, kDefaultBRDFLutSize, 1, 1,
			VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT);

		// update image descriptor
        const VkDescriptorImageInfo output_brdf_lut = {default_sampler_, brdf_lut_texture_->image_view, VK_IMAGE_LAYOUT_GENERAL};
        rhi->UpdateImageDescriptorSet(brdf_lut_ds, OUTPUT_TEXTURE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, { output_brdf_lut });

		VkCommandBuffer command_buffer = rhi->BeginImmediateComputePassCommandBuffer();
		{
			const auto lut_transition_barrier = TextureMemoryBarrier(*brdf_lut_texture_, 0, VK_ACCESS_SHADER_WRITE_BIT,
						 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
			rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {lut_transition_barrier});
			
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, brdf_lut_pipeline);
            
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout_, 0,
				1, &brdf_lut_ds, 0, nullptr);

			vkCmdDispatch(command_buffer, kDefaultBRDFLutSize / 32, kDefaultBRDFLutSize / 32, 6);
		}
		rhi->ExecImmediateComputePassCommandBuffer(command_buffer);
		rhi->DestroyPipeline(brdf_lut_pipeline);
	}
}
