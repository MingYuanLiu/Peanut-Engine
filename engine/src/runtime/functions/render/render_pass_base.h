#pragma once

namespace peanut {
class RenderPassBase
{
public:
    virtual ~RenderPassBase() {}
    virtual void Initialize() = 0;
    virtual void DeInitialize() = 0;
    virtual void preparePassData() = 0;
    virtual void RenderTick(const ViewSettings& view,
                            const SceneSettings& scene) = 0;
};
}  // namespace peanut