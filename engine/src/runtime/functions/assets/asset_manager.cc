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
  if (stbi_is_hdr(texture_filepath.c_str())) {
    texture_data->pixels = stbi_loadf(
        texture_filepath.c_str(), &texture_data->width, &texture_data->height,
        &texture_data->channels, kDefaultDesiredChannels);
  } else {
    texture_data->pixels = stbi_load(
        texture_filepath.c_str(), &texture_data->width, &texture_data->height,
        &texture_data->channels, kDefaultDesiredChannels);
  }

  if (!texture_data->pixels) {
    PEANUT_LOG_FATAL("Failed to read texture file %s",
                     texture_filepath.c_str());
  }

  auto texture_width = texture_data->width;
  auto texture_height = texture_data->height;
  texture_data->levels =
      RenderUtils::NumMipmapLevels(texture_width, texture_height);

  VkImageUsageFlags usage =
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if (texture_data->levels > 1)
    usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // for mipmap generation

  auto rhi = GlobalRuntimeContext::GetContext()->GetRenderSystem()->GetRHI();
  texture_data->image = rhi->CreateImage(
      texture_width, texture_height, 1, texture_data->levels, 1, format, usage);

  texture_data->image_view = rhi->CreateImageView(texture_data->image.resource, format, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 1);

  // TODO: copy data to image

  return texture_data;
}
}
}  // namespace peanut
