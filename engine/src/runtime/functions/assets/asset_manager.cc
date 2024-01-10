#include "runtime/functions/assets/asset_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <direct.h>

#include "runtime/core/base/logger.h"
#include "runtime/core/context/runtime_context.h"
#include "runtime/functions/assets/mesh.h"
#include "runtime/functions/render/render_utils.h"

namespace peanut {
std::shared_ptr<TextureData> AssetsManager::LoadTextureData(
    const std::string& texture_filepath,
    VkFormat format /*TODO: set a wapper format*/, int channels,
    uint32_t levels) {
  std::shared_ptr<TextureData> texture_data = std::make_shared<TextureData>();
  int width = 0;
  int height = 0;
  int texture_channels = 0;
  bool is_hdr = false;
  if (stbi_is_hdr(texture_filepath.c_str())) {
    texture_data->pixels = stbi_loadf(texture_filepath.c_str(), &width, &height,
                                      &texture_channels, channels);
    is_hdr = true;
  } else {
    texture_data->pixels = stbi_load(texture_filepath.c_str(), &width, &height,
                                     &texture_channels, channels);
  }

  texture_data->width = width;
  texture_data->height = height;
  texture_data->channels = channels;

  if (!texture_data->pixels) {
    PEANUT_LOG_FATAL("Failed to read texture file {0}",
                     texture_filepath.c_str());
  }

  auto texture_width = texture_data->width;
  auto texture_height = texture_data->height;
  texture_data->levels =
      levels > 0 ? levels
                 : RenderUtils::NumMipmapLevels(texture_width, texture_height);
  texture_data->layers = 1;

  VkImageUsageFlags usage =
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if (texture_data->levels > 1)
    usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // for mipmap generation

  auto rhi = GlobalRuntimeContext::GetContext()->GetRenderSystem()->GetRHI();
  texture_data->image =
      rhi->CreateImage(texture_width, texture_height, texture_data->layers,
                       texture_data->levels, 1, format, usage);

  texture_data->image_view = rhi->CreateImageView(
      texture_data->image.resource, format, VK_IMAGE_ASPECT_COLOR_BIT, 0,
      VK_REMAINING_MIP_LEVELS, 1);

  size_t texture_pixel_size =
      texture_data->width * texture_data->height * texture_data->channels;
  if (is_hdr) texture_pixel_size = texture_pixel_size * sizeof(float);
  Resource<VkBuffer> staging_buffer =
      rhi->CreateBuffer(texture_pixel_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  rhi->CopyMemToDevice(staging_buffer.memory, texture_data->pixels,
                       texture_pixel_size);
  VkCommandBuffer command_buffer = rhi->BeginImmediateCommandBuffer();

  const auto begin_barrier = TextureMemoryBarrier(*texture_data, 0, 
      VK_ACCESS_TRANSFER_WRITE_BIT,VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL).MipLevels(0, 1);

  rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT, { begin_barrier });

  rhi->CmdCopyBufferToImage(command_buffer, staging_buffer, texture_data->image,
                            texture_data->width, texture_data->height,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  const VkImageLayout final_base_mip_layout =
        (texture_data->levels > 1) ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                   : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  const auto end_barrier = TextureMemoryBarrier(
        *texture_data, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, final_base_mip_layout).MipLevels(0, 1);

  rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { end_barrier });

  rhi->ExecImmediateCommandBuffer(command_buffer);
  rhi->DestroyBuffer(staging_buffer);
  if (texture_data->levels > 1) {
    // TODO: generate mipmaps
    rhi->GenerateMipmaps(*texture_data);
  }

  return texture_data;
}

std::shared_ptr<MeshBuffer> AssetsManager::LoadMeshBuffer(
    const std::string& mesh_filepath) {

  std::shared_ptr<Mesh> mesh = Mesh::ReadFromFile(mesh_filepath);
  if (mesh.get() == nullptr) {
    // FIXME: do not crash
    PEANUT_LOG_FATAL("Mesh can not be loaded");
    assert(mesh.get() != nullptr);
  }
  
  std::shared_ptr<MeshBuffer> mesh_buffer = std::make_shared<MeshBuffer>();
  mesh_buffer->num_elements = static_cast<uint32_t>(mesh->faces().size()) * 3;
  const auto vertex_size = mesh->vertices().size() * sizeof(Mesh::Vertex);
  const auto index_size = mesh->faces().size() * sizeof(Mesh::Face);

  const auto& rhi =
      GlobalRuntimeContext::GetContext()->GetRenderSystem()->GetRHI();

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

  Resource<VkBuffer> staging_vertext_buffer = mesh_buffer->vertex_buffer;
  if (rhi->MemoryTypeNeedsStaging(
          mesh_buffer->vertex_buffer.memory_type_index)) {
    staging_vertext_buffer = rhi->CreateBuffer(
        mesh_buffer->vertex_buffer.allocation_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    using_staging_vertex_buffer = true;
  }

  Resource<VkBuffer> staging_index_buffer = mesh_buffer->index_buffer;
  if (rhi->MemoryTypeNeedsStaging(
          mesh_buffer->index_buffer.memory_type_index)) {
    staging_index_buffer = rhi->CreateBuffer(
        mesh_buffer->vertex_buffer.allocation_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    using_staging_index_buffer = true;
  }

  rhi->CopyMemToDevice(staging_vertext_buffer.memory, mesh->vertices().data(),
                       vertex_size);
  rhi->CopyMemToDevice(staging_index_buffer.memory, mesh->faces().data(),
                       index_size);

  // copy from staging buffer to mesh buffer
  if (using_staging_index_buffer || using_staging_vertex_buffer) {
    VkCommandBuffer command_buffer = rhi->BeginImmediateCommandBuffer();
    if (using_staging_vertex_buffer) {
      const VkBufferCopy region = {0, 0, vertex_size};
      vkCmdCopyBuffer(command_buffer, staging_vertext_buffer.resource,
                      mesh_buffer->vertex_buffer.resource, 1, &region);
    }

    if (using_staging_index_buffer) {
      const VkBufferCopy index_copy_region = {0, 0, index_size};
      vkCmdCopyBuffer(command_buffer, staging_index_buffer.resource,
                      mesh_buffer->index_buffer.resource, 1,
                      &index_copy_region);
    }
    rhi->ExecImmediateCommandBuffer(command_buffer);
  }

  if (using_staging_index_buffer) {
    rhi->DestroyBuffer(staging_index_buffer);
  }

  if (using_staging_vertex_buffer) {
    rhi->DestroyBuffer(staging_vertext_buffer);
  }

  return mesh_buffer;
}
}  // namespace peanut
