#include "runtime/engine.h"

#include "runtime/core/context/runtime_context.h"
#include "runtime/core/event/event.h"

namespace peanut {
void PeanutEngine::Initliaze() {
  GlobalRuntimeContext::GetContext()->SetupSubSystems();
}

void PeanutEngine::Shutdown() {
  GlobalRuntimeContext::GetContext()->DestroySubSystems();
}

bool PeanutEngine::HandleWindowCloseEvent(WindowCloseEvent& e) {
  PEANUT_LOG_INFO("Get window close event ({0}), stop engine",
                  e.ToString().c_str());
  IsShutdown = true;
  return true;
}

void PeanutEngine::Run() {
  const auto& window_system =
      GlobalRuntimeContext::GetContext()->GetWindowSystem();
  const auto& render_system =
      GlobalRuntimeContext::GetContext()->GetRenderSystem();

  window_system->PushEventCallback([this](Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(
        BIND_EVENT_FN(PeanutEngine::HandleWindowCloseEvent));
  });

  while (!IsShutdown) {
    render_system->Tick();
    window_system->OnUpdate();
  }
}
}  // namespace peanut
