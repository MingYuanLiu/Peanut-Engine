#include <gtest/gtest.h>
#include "runtime/engine.h"
#include "runtime/functions/assets/asset_manager.h"
#include "runtime/functions/render/render_data.h"
#include "runtime/core/log/peanut_log.h"

using namespace peanut;

TEST(EngineMainTest, BasicAssertions) {
  // PeanutEngine::GetInstance().Initliaze();
  // PeanutEngine::GetInstance().Run();
}

TEST(AssetLoad_Test, BasicAssertions) {
  peanut::LogSystem::init("./logs/log.txt", true);
  std::string test_asset_description_file_path = "assets_description.json";
  std::map<std::string, std::shared_ptr<PbrMaterial> > material_models;
  std::map<std::string, std::shared_ptr<MeshBuffer> > mesh_models;

  AssetsManager::GetInstance().LoadRenderObjectFromDescriptionFile(test_asset_description_file_path, material_models, mesh_models);
}

TEST(AssetLoad_FileNotExist_Test, BasicAssertions) {
  peanut::LogSystem::init("./logs/log.txt", true);
  std::string test_asset_description_file_path = "assets_description_not_exist.json";
  std::map<std::string, std::shared_ptr<PbrMaterial> > material_models;
  std::map<std::string, std::shared_ptr<MeshBuffer> > mesh_models;

  AssetsManager::GetInstance().LoadRenderObjectFromDescriptionFile(test_asset_description_file_path, material_models, mesh_models);
}