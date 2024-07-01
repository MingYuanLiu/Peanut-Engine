#include "shader_manager.h"

#include "functions/file/file_helper.h"

namespace peanut
{
	ShaderManager* ShaderManager::s_instance_ = nullptr;
	std::once_flag ShaderManager::s_flag_ = {};
	std::string s_engine_shader_path_ = ENGINE_SHADER_SRC_PATH;

	void ShaderManager::Init()
	{
		CompileEngineShaders();
	}

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
		// 搜寻引擎内的shader文件
		const std::vector<std::string> valid_shader_file_extensions = { ".vert", ".frag", ".comp" };
		if (!FileHelper::IsDirectoryExist(s_engine_shader_path_))
		{
			PEANUT_LOG_WARN("Engine shader path: {0} not exist.", s_engine_shader_path_);
		}

		std::vector<std::string> shader_file_paths;
		FileHelper::TranverseDirectory(s_engine_shader_path_, false,
			[&valid_shader_file_extensions, &shader_file_paths](const std::string& file_path, bool is_dir)
			{
				if (is_dir)
                {
                    return true;
                }

				const std::string extension_name = FileHelper::GetExtension(file_path);

				if (std::find(valid_shader_file_extensions.begin(),
					valid_shader_file_extensions.end(), extension_name) != valid_shader_file_extensions.end())
                {
					shader_file_paths.push_back(file_path);
                }
				
				return true;
			});
		const std::string shader_include_path = FileHelper::CombinePath(ENGINE_SHADER_SRC_PATH, "include");
		
		auto compile_shader_code_func = [&](const std::string& shader_filepath)
		{
			const std::string file_name = FileHelper::GetFileName(shader_filepath);
			if (shader_files_name_to_path.find(file_name) != shader_files_name_to_path.end())
			{
				shader_files_name_to_path.erase(file_name);
			}

			// 生成目标shader code文件路径
			
			// 调用glsl程序编译shader文件
			const std::string compile_shader_command;
			// 执行命令并返回结果

			// 保存编译后的shader文件路径
		};
		

		// 记录编译后的shader文件路径
	}
}
