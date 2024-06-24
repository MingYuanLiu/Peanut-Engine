#include "runtime/functions/assets/asset_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <direct.h>

#include "runtime/core/base/logger.h"
#include "runtime/core/context/runtime_context.h"
#include "runtime/functions/assets/mesh.h"
#include "runtime/functions/render/render_utils.h"

#include <json11.hpp>
#include <fstream>

namespace peanut 
{
std::shared_ptr<TextureData> AssetsManager::LoadTextureArrayData(const std::vector<std::string>& texture_files, VkFormat format)
{
    std::shared_ptr<TextureData> texture_data = std::make_shared<TextureData>();
    
    assert(texture_files.size() < 4);

    std::vector<std::pair<int, int> > texture_width_height;
    std::vector<void*> texture_pixels;

    auto exit_free = [&texture_pixels]() {
        for (void* loaded_pixel : texture_pixels)
        {
            stbi_image_free(loaded_pixel);
        }
    };

    for (const auto& texture_file : texture_files)
    {
        int width = 0, height = 0, channel = 0;
        auto pixels = stbi_load(texture_file.c_str(), &width, &height, &channel, STBI_rgb_alpha);
        if (!pixels)
        {
            PEANUT_LOG_ERROR("Failed to load texture file {0}", texture_file);
            exit_free();

            return nullptr;
        }

        texture_width_height.emplace_back(width, height);
        texture_pixels.push_back(pixels);
    }

    bool all_width_height_equal = true;
    int last_width = -1, last_height = -1;
    for (const auto& p : texture_width_height)
    {
        if (last_width == -1 && last_height == -1)
        {
            last_width = p.first;
            last_height = p.second;
        }
        else if (p.first != last_width || p.second != last_height)
        {
            all_width_height_equal = false;
        }
    }

    if (!all_width_height_equal)
    {
        PEANUT_LOG_ERROR("all array data size must be equal");
        exit_free();
        return nullptr;
    }

    texture_data->width = last_width;
    texture_data->height = last_height;
    texture_data->channels = 4;
    texture_data->levels = 1;
    texture_data->layers = 1;

    VkImageUsageFlags usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    // create image
    auto rhi = GlobalEngineContext::GetContext()->GetRenderSystem()->GetRHI();
    texture_data->image = rhi->CreateImage(texture_data->width, texture_data->height, 1, 1, 1, format, usage);
    texture_data->image_view = rhi->CreateImageView(texture_data->image.resource, format, VK_IMAGE_ASPECT_COLOR_BIT, 0,
                                                    VK_REMAINING_MIP_LEVELS, VK_REMAINING_ARRAY_LAYERS);

    // copy data to image
    size_t texture_pixel_size = texture_data->width * texture_data->height * texture_pixels.size();

    void* all_data_pixel_ptr = malloc(texture_pixel_size);
    size_t offset = texture_data->width * texture_data->height;
    for (size_t i = 0; i < texture_pixels.size(); ++i)
    {
        uint8_t* data_ptr = reinterpret_cast<uint8_t*>(all_data_pixel_ptr);
        memcpy(data_ptr + (i * offset), texture_pixels[i], offset);
    }

    Resource<VkBuffer> staging_buffer = rhi->CreateBuffer(texture_pixel_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    rhi->CopyMemToDevice(staging_buffer.memory, all_data_pixel_ptr, texture_pixel_size);

    VkCommandBuffer command_buffer = rhi->BeginImmediateComputePassCommandBuffer();

    const auto begin_barrier =
        TextureMemoryBarrier(*texture_data, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL).MipLevels(0, 1);

    rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, { begin_barrier });

    rhi->CmdCopyBufferToImage(command_buffer, staging_buffer, texture_data->image, texture_data->width, texture_data->height,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    rhi->ExecImmediateComputePassCommandBuffer(command_buffer);

    texture_data->pixels = all_data_pixel_ptr;

    // release staging buffer
    rhi->DestroyBuffer(staging_buffer);

    return texture_data;
}

std::shared_ptr<TextureData> AssetsManager::LoadTextureData(const std::string& texture_filepath, 
                                                            VkFormat format /*TODO: set a wapper format*/, int channels,
                                                            uint32_t levels) 
{
    std::shared_ptr<TextureData> texture_data = std::make_shared<TextureData>();
    int width = 0;
    int height = 0;
    int texture_channels = 0;
    bool is_hdr = false;
    if (stbi_is_hdr(texture_filepath.c_str()))
    {
        texture_data->pixels = stbi_loadf(texture_filepath.c_str(), &width, &height,
                                        &texture_channels, channels);
        is_hdr = true;
    } else 
    {
        texture_data->pixels = stbi_load(texture_filepath.c_str(), &width, &height,
                                        &texture_channels, channels);
    }

    texture_data->width = width;
    texture_data->height = height;
    texture_data->channels = channels;

    if (!texture_data->pixels) 
    {
        PEANUT_LOG_FATAL("Failed to read texture file {0}",
                        texture_filepath.c_str());
        return texture_data;
    }

    auto texture_width = texture_data->width;
    auto texture_height = texture_data->height;
    texture_data->levels = levels > 0 ? levels : RenderUtils::NumMipmapLevels(texture_width, texture_height);
    texture_data->layers = 1;

    VkImageUsageFlags usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (texture_data->levels > 1)
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // for mipmap generation

    auto rhi = GlobalEngineContext::GetContext()->GetRenderSystem()->GetRHI();
    texture_data->image = rhi->CreateImage(texture_width, texture_height, texture_data->layers,
                                            texture_data->levels, 1, format, usage);

    texture_data->image_view = rhi->CreateImageView(
        texture_data->image.resource, format, VK_IMAGE_ASPECT_COLOR_BIT, 0,
        VK_REMAINING_MIP_LEVELS, VK_REMAINING_ARRAY_LAYERS);

    size_t texture_pixel_size =
        texture_data->width * texture_data->height * texture_data->channels;

    if (is_hdr) texture_pixel_size = texture_pixel_size * sizeof(float);
    
    Resource<VkBuffer> staging_buffer =
        rhi->CreateBuffer(texture_pixel_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    rhi->CopyMemToDevice(staging_buffer.memory, texture_data->pixels,
                        texture_pixel_size);
    VkCommandBuffer command_buffer = rhi->BeginImmediateComputePassCommandBuffer();

    const auto begin_barrier =
        TextureMemoryBarrier(*texture_data, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL).MipLevels(0, 1);

    rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT, {begin_barrier});

    rhi->CmdCopyBufferToImage(command_buffer, staging_buffer, texture_data->image,
                            texture_data->width, texture_data->height,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkImageLayout final_base_mip_layout =
        (texture_data->levels > 1) ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    const auto end_barrier =
        TextureMemoryBarrier(*texture_data, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            final_base_mip_layout)
            .MipLevels(0, 1);

    rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {end_barrier});

    rhi->ExecImmediateComputePassCommandBuffer(command_buffer);
    rhi->DestroyBuffer(staging_buffer);
    if (texture_data->levels > 1) 
    {
        // TODO: generate mipmaps
        rhi->GenerateMipmaps(*texture_data);
    }

    return texture_data;
}

std::shared_ptr<MeshBuffer> AssetsManager::LoadMeshBuffer(const std::string& mesh_filepath) 
{
    std::shared_ptr<Mesh> mesh = Mesh::ReadFromFile(mesh_filepath);
    if (mesh.get() == nullptr) 
    {
        // FIXME: do not crash
        PEANUT_LOG_FATAL("Mesh can not be loaded");
        assert(mesh.get() != nullptr);
    }

    std::shared_ptr<MeshBuffer> mesh_buffer = std::make_shared<MeshBuffer>();
    mesh_buffer->num_elements = static_cast<uint32_t>(mesh->indexes().size()) * 3;
    const auto vertex_size = mesh->vertices().size() * sizeof(Mesh::Vertex);
    const auto index_size = mesh->indexes().size() * sizeof(Mesh::Index);

    const auto& rhi = GlobalEngineContext::GetContext()->GetRenderSystem()->GetRHI();

    // create mesh buffer
    mesh_buffer->vertex_buffer = rhi->CreateBuffer(
        vertex_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    mesh_buffer->index_buffer = rhi->CreateBuffer(
        index_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    bool using_staging_vertex_buffer = false;
    bool using_staging_index_buffer = false;

    Resource<VkBuffer> staging_vertex_buffer = mesh_buffer->vertex_buffer;
    if (rhi->MemoryTypeNeedsStaging(mesh_buffer->vertex_buffer.memory_type_index))
    {
        staging_vertex_buffer = rhi->CreateBuffer(
            mesh_buffer->vertex_buffer.allocation_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        
        using_staging_vertex_buffer = true;
    }

    Resource<VkBuffer> staging_index_buffer = mesh_buffer->index_buffer;
    if (rhi->MemoryTypeNeedsStaging(mesh_buffer->index_buffer.memory_type_index)) 
    {
        staging_index_buffer = rhi->CreateBuffer(
            mesh_buffer->vertex_buffer.allocation_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        
        using_staging_index_buffer = true;
    }

    rhi->CopyMemToDevice(staging_vertex_buffer.memory, mesh->vertices().data(), vertex_size);
    rhi->CopyMemToDevice(staging_index_buffer.memory, mesh->indexes().data(), index_size);

    // copy from staging buffer to mesh buffer
    if (using_staging_index_buffer || using_staging_vertex_buffer) 
    {
        VkCommandBuffer command_buffer = rhi->BeginImmediateComputePassCommandBuffer();
        if (using_staging_vertex_buffer) 
        {
            const VkBufferCopy region = {0, 0, vertex_size};
            vkCmdCopyBuffer(command_buffer, staging_vertex_buffer.resource,
                            mesh_buffer->vertex_buffer.resource, 1, &region);
        }

        if (using_staging_index_buffer) 
        {
            const VkBufferCopy index_copy_region = {0, 0, index_size};
            vkCmdCopyBuffer(command_buffer, staging_index_buffer.resource,
                            mesh_buffer->index_buffer.resource, 1,
                            &index_copy_region);
        }
        rhi->ExecImmediateComputePassCommandBuffer(command_buffer);
    }

    if (using_staging_index_buffer) 
    {
        rhi->DestroyBuffer(staging_index_buffer);
    }

    if (using_staging_vertex_buffer) 
    {
        rhi->DestroyBuffer(staging_vertex_buffer);
    }

    return mesh_buffer;
}

void AssetsManager::LoadRenderObjectFromDescriptionFile(const std::string& description_file,
                                                        std::map<std::string, std::shared_ptr<PbrMaterial> >& out_pbr_material_models,
                                                        std::map<std::string, std::shared_ptr<MeshBuffer> >& out_pbr_mesh_models) 
{
    std::string json_content = ReadJsonFile(description_file);
    if (json_content.empty()) 
    {
        PEANUT_LOG_WARN(
            "Render description is empty can not load any render objects");
        return;
    }

    std::string error;
    json11::Json render_objects_json = json11::Json::parse(json_content, error);
    if (!error.empty() || render_objects_json.is_null())
    {
        PEANUT_LOG_INFO("Failed to parse json, error: {0}, content: {1}", error,
                        json_content);
        return;
    }

    json11::Json::object render_objects =
        render_objects_json["render_objects"].object_items();
    std::map<std::string, PbrMaterial> pbr_material_models;
    std::map<std::string, MeshBuffer> pbr_mesh_models;
    for (auto pair : render_objects) 
    {
        std::string object_name = pair.first;
        json11::Json object_description = pair.second;

        // read mesh data
        std::string mesh_file = object_description["mesh"].string_value();
        std::shared_ptr<MeshBuffer> object_mesh = LoadMeshBuffer(mesh_file);
        out_pbr_mesh_models.insert(std::make_pair(object_name, object_mesh));

        // read material data
        json11::Json pbr_material_description = object_description["pbr_material"];
        std::string albedo_texture_file =
            pbr_material_description["albedo_texture"].string_value();
        std::string normal_texture_file =
            pbr_material_description["normal_texture"].string_value();
        std::string metallic_texture_file =
            pbr_material_description["metallic_texture"].string_value();
        std::string emissive_texture_file =
            pbr_material_description["emissive_texture"].string_value();

        std::shared_ptr<PbrMaterial> pbr_material = std::make_shared<PbrMaterial>();

        std::shared_ptr<TextureData> albedo_texture =
            LoadTextureData(albedo_texture_file, VK_FORMAT_R8G8B8A8_SRGB);
        std::shared_ptr<TextureData> normal_texture =
            LoadTextureData(normal_texture_file, VK_FORMAT_R8G8B8A8_UNORM);
        std::shared_ptr<TextureData> metallic_texture =
            LoadTextureData(metallic_texture_file, VK_FORMAT_R8_UNORM);
        std::shared_ptr<TextureData> emissive_texture =
            LoadTextureData(emissive_texture_file, VK_FORMAT_R8_UNORM);

        pbr_material->base_color_texture = albedo_texture;
        pbr_material->normal_texture = normal_texture;
        pbr_material->metallic_roughness_occlusion_texture = metallic_texture;
        pbr_material->emissive_texture = emissive_texture;

        out_pbr_material_models.insert(std::make_pair(object_name, pbr_material));

        PEANUT_LOG_INFO("Finished to read render object {0}", object_name);
    }
}

std::string AssetsManager::ReadJsonFile(const std::string& file_path) 
{
    std::ifstream file_read_stream(file_path.c_str(), std::ios::binary);
    if (!file_read_stream.is_open()) 
    {
        PEANUT_LOG_ERROR("Failed to open json file {0}", file_path);
        return "";
    }

    std::stringstream read_buffer;
    read_buffer << file_read_stream.rdbuf();
    std::string file_content(read_buffer.str());
    PEANUT_LOG_INFO("File content {0}", file_content);
    // file_read_in.read();
    file_read_stream.close();
    return file_content;
}

void AssetsManager::DestroyMeshBuffer(MeshBuffer& mesh_buffer) 
{
    const auto& rhi =
        GlobalEngineContext::GetContext()->GetRenderSystem()->GetRHI();
    rhi->DestroyBuffer(mesh_buffer.vertex_buffer);
    rhi->DestroyBuffer(mesh_buffer.index_buffer);
    mesh_buffer = {};
}
}  // namespace peanut
