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
			std::call_once(s_flag_, &ShaderManager::InitSingleton);
			return *s_instance_;
		}

		void Init();
		~ShaderManager()
		{
			if (s_instance_)
            {
                delete s_instance_;
                s_instance_ = nullptr;
            }
		}

		ShaderManager(const ShaderManager&) = delete;
		ShaderManager& operator=(const ShaderManager&) = delete;
		ShaderManager(ShaderManager&&) = delete;
        ShaderManager& operator=(ShaderManager&&) = delete;

		VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo(std::weak_ptr<RHI> rhi, const std::string& shader_name, VkShaderStageFlagBits stage);
		VkShaderModule GetShaderModule(std::weak_ptr<RHI> rhi, const std::string& shader_name);

	private:
		std::unordered_map<std::string, std::string> shader_files_name_to_path;
		std::unordered_map<std::string, VkShaderModule> loaded_shader_modules;

	private:
		ShaderManager() = default;

		bool CompileEngineShaders();

		static ShaderManager* InitSingleton()
		{
			return new ShaderManager();
		}

		static ShaderManager* s_instance_;
		static std::once_flag s_flag_;
		static std::string s_engine_shader_path_;
	};

} // namespace peanut