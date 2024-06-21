#pragma once

#include <optional>
#include <string>
#include "../render_data.h"
#include "functions/rhi/vulkan/vulkan_rhi.h"

namespace peanut
{
	/**
	* @brief pre-compute environment irradiance map, filtered texture and brdf lut texture using compute shader pass
	* 
	* normally we should load these pre-computed textures from disk
	* so, we should provide some interface to save these textures to disk file
	*/
	class EnvironmentMapComputePass
	{
		enum DescriptorBinding : uint8_t
		{
			INPUT_TEXTURE = 0,
			OUTPUT_TEXTURE,
			OUTPUT_MIP_TAILS
		};
		
	public:
		EnvironmentMapComputePass() = default;
		~EnvironmentMapComputePass() = default;

		void Initialize(std::weak_ptr<RHI> rhi, const std::string& environment_map_url);

		/**
		 * load environment map and generate ibl textures
		 */
		void Dispatch();

		/**
		 * destroy all resources
		 */
		void Destroy();

		std::shared_ptr<TextureData> GetEnvironmentMap()
		{
			return environment_map_;
		}
		
		std::shared_ptr<TextureData> GetIrradianceMap()
		{
			return env_irradiance_map_;
		}
		
		std::shared_ptr<TextureData> GetFilteredTexture()
		{
			return prefiltered_texture_;
		}
		
		std::shared_ptr<TextureData> GetBRDFLutTexture()
		{
			return brdf_lut_texture_;
		}

		void FillSkyboxRenderData(SkyboxRenderData& skybox_render_data)
		{
			skybox_render_data.skybox_env_texture = *environment_map_;
		}
		
		void FillIblTextureResource(IblLightTexture& ibl_light_texture)
		{
			ibl_light_texture.ibl_prefilter_texture = *prefiltered_texture_;
			ibl_light_texture.ibl_irradiance_texture = *env_irradiance_map_;
			ibl_light_texture.brdf_lut_texture = *brdf_lut_texture_;
		}

		EnvironmentMapComputePass(const EnvironmentMapComputePass&) = delete;
		EnvironmentMapComputePass(EnvironmentMapComputePass&&) = delete;
		EnvironmentMapComputePass& operator=(const EnvironmentMapComputePass&) = delete;

	private:
		void SetupComputeDescriptorPool();
		void SetupComputeSampler();

		void SetupDescriptorSetLayout();
		void SetupComputePipelineLayout();

		// load environment map hdr image and generate mipmap images
		void LoadEnvironmentMap();

		void CreateIrradianceMap();
		void CreateFilteredTexture();
		void CreateBRDFLutTexture();

		bool is_dispatched = false;
		bool is_initialized = false;

		uint32_t environment_map_mip_levels_; // mipmap levels
		std::string environment_map_url_;

		std::weak_ptr<RHI> rhi_;

		std::shared_ptr<TextureData> environment_map_;
		std::shared_ptr<TextureData> env_irradiance_map_;
		std::shared_ptr<TextureData> prefiltered_texture_;
		std::shared_ptr<TextureData> brdf_lut_texture_;

		VkDescriptorPool compute_descriptor_pool_;
		VkSampler default_sampler_;
		VkDescriptorSetLayout default_descriptor_layout_;
		VkPipelineLayout compute_pipeline_layout_;

		static constexpr uint32_t kDefaultIrradianceMapSize = 32;
		static constexpr  uint32_t kDefaultBRDFLutSize = 128;
	};
}
