#pragma once

#include <memory>
#include <string>
#include <map>

#include "runtime/functions/render/render_data.h"

namespace peanut 
{

class AssetsManager 
{
public:
	static AssetsManager& GetInstance() 
	{
		static AssetsManager AssetManager;
		return AssetManager;
	}

	/**
	* @brief Load pbr material texture data from local file
	*
	*/
	std::shared_ptr<TextureData> LoadTextureData(const std::string& texture_filepath,
						VkFormat format /*TODO: set a wapper format*/, int channels = 4, uint32_t levels = 0);

	/**
	* @brief load pbr mesh data from local file
	*
	* @param mesh_filepath
	* @return std::shared_ptr<MeshBuffer>
	*/
	std::shared_ptr<MeshBuffer> LoadMeshBuffer(const std::string& mesh_filepath);

	/**
	* @brief load pbr material data and mesh data from description file
	* the description about pbr model is about:
	* {
	*   "render_objects": {
	*      "object1": {
	*          "mesh": "",
	*          "pbr_material": {
	*            "albedo_texture": "",
	*            "normal_texture": "",
	*            "metallic_texture": "",
	*            "roughness_texture": ""
	*         }
	*      }
	*    }
	* }
	*
	* @param description_file
	* @param out_pbr_material_data_
	* @param out_pbr_mesh_data_
	*/
	void LoadRenderObjectFromDescriptionFile(const std::string& description_file,
		std::map<std::string, std::shared_ptr<PbrMaterial> >& out_pbr_material_models_,
		std::map<std::string, std::shared_ptr<MeshBuffer> >& out_pbr_mesh_models_);

	void DestroyMeshBuffer(MeshBuffer& mesh_buffer);

	AssetsManager(const AssetsManager&&) = delete;
	AssetsManager(const AssetsManager&) = delete;
	void operator=(const AssetsManager&) = delete;

private:
	std::string ReadJsonFile(const std::string& file_path);
	AssetsManager() = default;
	virtual ~AssetsManager() = default;
};
}  // namespace peanut