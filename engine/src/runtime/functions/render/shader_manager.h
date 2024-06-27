#pragma once

#include <mutex>
#include "runtime/functions/rhi/vulkan/vulkan_rhi.h"
#include <unordered_map>

namespace peanut
{
	class ShaderManager
	{
	public:
		static ShaderManager& Get()
		{
			std::call_once(s_Flag, &ShaderManager::InitSingleton);
			return *s_Instance;
		}

		void Init();

		VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo(std::weak_ptr<RHI> rhi, const std::string& shader_name, VkShaderStageFlagBits stage);
		VkShaderModule GetShaderModule(std::weak_ptr<RHI> rhi, const std::string& shader_name);

	private:
		std::unordered_map<std::string, std::string> shader_files_name_to_path;
		std::unordered_map<std::string, VkShaderModule> loaded_shader_modules;

	private:
		ShaderManager() {}

		ShaderManager(const ShaderManager&) = delete;
		ShaderManager& operator=(const ShaderManager&) = delete;

		bool CompileEngineShaders();

		ShaderManager* InitSingleton()
		{
			return new ShaderManager();
		}

		static ShaderManager* s_Instance;
		static std::once_flag s_Flag;
	};

} // namespace peanut