#pragma once

#include <volk.h>
#include <vulkan/vulkan.h>

namespace peanut {
class RenderPass {
 public:
  virtual void Initialize();
};
}  // namespace peanut