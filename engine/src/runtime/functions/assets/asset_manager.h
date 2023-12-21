#pragma once

#include <string>
#include <memory>

#include "runtime/functions/render/render_data.h"

namespace peanut {
	class AssetsManager {
	public:
		AssetsManager& GetInstance() {
			static AssetsManager AssetManager;
			return AssetManager;
		}

		std::shared_ptr<TextureData> LoadTextureData(const std::string& texture_filepath, VkFormat format /*TODO: set a wapper format*/);


		AssetsManager(const AssetsManager&&) = delete;
		AssetsManager(const AssetsManager&) = delete;
		void operator=(const AssetsManager&) = delete;
	private:
		AssetsManager() = default;
		virtual ~AssetsManager() = default;

		const int kDefaultDesiredChannels= 4;
	};
}