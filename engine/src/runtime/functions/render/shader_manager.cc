#include "shader_manager.h"

namespace peanut
{
	ShaderManager* ShaderManager::s_Instance = nullptr;
	std::once_flag ShaderManager::s_Flag = {};

	VkPipelineShaderStageCreateInfo ShaderManager::GetShaderStageCreateInfo(std::weak_ptr<RHI> rhi, const std::string& shader_name, VkShaderStageFlagBits stage)
	{
		if (shader_files_name_to_path.find(shader_name) == shader_files_name_to_path.end())
		{
			PEANUT_LOG_ERROR("Failed to find shader file: {0}", shader_name);
			return {};
		}

		VkShaderModule shader_module;
		if (loaded_shader_modules.find(shader_name) != loaded_shader_modules.end())
		{
			shader_module = loaded_shader_modules[shader_name];
		}
		else
		{
			shader_module = rhi.lock()->CreateShaderModule(shader_files_name_to_path[shader_name]);
			if (shader_module == VK_NULL_HANDLE)
			{
				PEANUT_LOG_ERROR("Failed to create shader module: {0}", shader_name);
				return {};
			}

			loaded_shader_modules[shader_name] = shader_module;
		}

		VkPipelineShaderStageCreateInfo shader_stage_info = {};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = stage;
		shader_stage_info.module = shader_module;
		shader_stage_info.pName = "main";

		return shader_stage_info;
	}

	VkShaderModule ShaderManager::GetShaderModule(std::weak_ptr<RHI> rhi, const std::string& shader_name)
	{
		VkShaderModule shader_module = VK_NULL_HANDLE;
		if (loaded_shader_modules.find(shader_name) != loaded_shader_modules.end())
		{
			shader_module = loaded_shader_modules[shader_name];
		}
		else
		{
			shader_module = rhi.lock()->CreateShaderModule(shader_files_name_to_path[shader_name]);
			if (shader_module == VK_NULL_HANDLE)
			{
				PEANUT_LOG_ERROR("Failed to create shader module: {0}", shader_name);
				return {};
			}

			loaded_shader_modules[shader_name] = shader_module;
		}

		return shader_module;
	}

	bool ShaderManager::CompileEngineShaders()
	{

	}
}
