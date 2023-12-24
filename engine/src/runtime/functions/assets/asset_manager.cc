#include "runtime/functions/assets/asset_manager.h"

#include <stb_image.h>

#include "runtime/core/base/logger.h"
#include "runtime/core/context/runtime_context.h"
#include "runtime/functions/render/render_utils.h"

namespace peanut {
std::shared_ptr<TextureData> AssetsManager::LoadTextureData(
    const std::string& texture_filepath,
    VkFormat format /*TODO: set a wapper format*/) {
  std::shared_ptr<TextureData> texture_data = std::make_shared<TextureData>();
  int width;
  int height;
  int channels;
  if (stbi_is_hdr(texture_filepath.c_str())) {
    texture_data->pixels = stbi_loadf(texture_filepath.c_str(), &width, &height,
                                      &channels, kDefaultDesiredChannels);
  } else {
    texture_data->pixels = stbi_load(texture_filepath.c_str(), &width, &height,
                                     &channels, kDefaultDesiredChannels);
  }

  texture_data->width = width;
  texture_data->height = height;
  texture_data->channels = channels;

  if (!texture_data->pixels) {
    PEANUT_LOG_FATAL("Failed to read texture file %s",
                     texture_filepath.c_str());
  }

  auto texture_width = texture_data->width;
  auto texture_height = texture_data->height;
  texture_data->levels =
      RenderUtils::NumMipmapLevels(texture_width, texture_height);
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

  const size_t texture_pixel_size =
      texture_data->width * texture_data->height * texture_data->channels;
  Resource<VkBuffer> staging_buffer =
      rhi->CreateBuffer(texture_pixel_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  rhi->CopyMemToDevice(staging_buffer.memory, texture_data->pixels,
                       texture_pixel_size);
  VkCommandBuffer command_buffer = rhi->BeginImmediateCommandBuffer();
  {
    const auto barrier =
        TextureMemoryBarrier(*texture_data, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            .MipLevels(0, 1);
    ;
    rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT, {barrier});
  }
  rhi->CmdCopyBufferToImage(command_buffer, staging_buffer, texture_data->image,
                            texture_data->width, texture_data->height,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  {
    const VkImageLayout final_base_mip_layout =
        (texture_data->levels > 1) ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                   : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    const auto barrier = TextureMemoryBarrier(
        *texture_data, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, final_base_mip_layout);
    rhi->CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {barrier});
  }

  rhi->ExecImmediateCommandBuffer(command_buffer);
  rhi->DestoryBuffer(staging_buffer);
  if (texture_data->levels > 1) {
    // TODO: generate mipmaps
    rhi->GenerateMipmaps(*texture_data);
  }

  return texture_data;
}
}  // namespace peanut
