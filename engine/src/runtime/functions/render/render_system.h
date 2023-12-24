#pragma once

#include "runtime/functions/render/render_pass.h"
#include "runtime/functions/rhi/rhi.h"

namespace peanut {
class RenderSystem {
 public:
  void Initialize();
  void DeInitialize();

  std::shared_ptr<RHI> GetRHI() { return rhi_; };

  std::shared_ptr<RHI> rhi_;
  std::shared_ptr<RenderPass> main_render_pass_;
};
}  // namespace peanut