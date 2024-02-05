#include <vector>

namespace Peanut {

/**
 * @brief The real entity in the render scene
 *
 */
struct RenderEntity {};

/**
 * @brief The render world
 *
 */
struct World {
  std::vector<RenderEntity> render_entities_;
};

/**
 * @brief The main render scene of the engine
 *
 */
class RenderScene {
 public:
 private:
};
}  // namespace Peanut