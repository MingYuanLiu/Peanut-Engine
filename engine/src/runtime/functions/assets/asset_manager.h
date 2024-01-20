#pragma once

#include <memory>
#include <string>

#include "runtime/functions/render/render_data.h"

namespace peanut {

class AssetsManager {
 public:
  static AssetsManager& GetInstance() {
    static AssetsManager AssetManager;
    return AssetManager;
  }

  std::shared_ptr<TextureData> LoadTextureData(
      const std::string& texture_filepath,
      VkFormat format /*TODO: set a wapper format*/, int channels = 4,
      uint32_t levels = 0);

  std::shared_ptr<MeshBuffer> LoadMeshBuffer(const std::string& mesh_filepath);

  void DestroyMeshBuffer(MeshBuffer& mesh_buffer);

  AssetsManager(const AssetsManager&&) = delete;
  AssetsManager(const AssetsManager&) = delete;
  void operator=(const AssetsManager&) = delete;

 private:
  AssetsManager() = default;
  virtual ~AssetsManager() = default;
};
}  // namespace peanut