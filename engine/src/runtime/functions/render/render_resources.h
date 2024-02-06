#pragma once
#include "runtime/functions/render/render_data.h"
#include <memory>
#include <map>
namespace peanut {

/**
 * @brief Manage the render resources data during rendering, including mesh,
 * texture, material and all the things about rendering.
 *
 */
class RenderResources {
 private:
  std::map<int, PbrMaterial> pbr_materials_;
  std::map<int, MeshBuffer> meshes_;
};
}  // namespace peanut
