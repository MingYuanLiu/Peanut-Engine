#include <Windows.h>
#include <gtest/gtest.h>

#include <memory>

#include "runtime/core/context/runtime_context.h"
#include "runtime/core/event/window_event.h"
#include "runtime/core/log/peanut_log.h"
#include "runtime/functions/window/window_system.h"

using namespace peanut;

// TEST(WindowSystemTest, BasicAssertions) {
//   // peanut::LogSystem::init("./logs/log.txt", true);

//   WindowCreateInfo create_info(1280, 720, "WindowSystemTest");
//   std::shared_ptr<WindowSystem> window_system_ptr =
//       std::make_shared<WindowSystem>();

//   window_system_ptr->Initialize(create_info);
//   bool window_should_close = false;

//   window_system_ptr->PushEventCallback([&window_should_close](Event& e) {
//     EventDispatcher dispatcher(e);
//     dispatcher.Dispatch<WindowCloseEvent>(
//         [&window_should_close](WindowCloseEvent& e) -> bool {
//           PEANUT_LOG_INFO(e.ToString());
//           window_should_close = true;
//           return true;
//         });
//   });

//   while (!window_should_close) {
//     window_system_ptr->OnUpdate();

//     Sleep(100);
//   }
// }

// TEST(RuntimeContextTest, BasicAssertions) {
//   peanut::GlobalRuntimeContext::GetContext()->SetupSubSystems();

//   bool window_should_close = false;
//   auto window_system =
//       peanut::GlobalRuntimeContext::GetContext()->window_system_;
//   window_system->PushEventCallback([&window_should_close](Event& e) {
//     EventDispatcher dispatcher(e);
//     dispatcher.Dispatch<WindowCloseEvent>(
//         [&window_should_close](WindowCloseEvent& e) -> bool {
//           PEANUT_LOG_INFO(e.ToString());
//           window_should_close = true;
//           return true;
//         });
//   });

//   while (!window_should_close) {
//     window_system->OnUpdate();

//     Sleep(100);
//   }
// }