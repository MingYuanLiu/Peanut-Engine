#include <Windows.h>
#include <gtest/gtest.h>

#include <memory>

#include "runtime/core/log/peanut_log.h"
#include "runtime/functions/window/window_system.h"

using namespace peanut;
TEST(WindowSystemTest, BasicAssertions) {
  // peanut::LogSystem::init("./logs/log.txt", true);

  WindowCreateInfo create_info(1280, 720, "WindowSystemTest");
  std::shared_ptr<WindowSystem> window_system_ptr =
      std::make_shared<WindowSystem>();

  window_system_ptr->Initialize(create_info);

  while (true) {
    window_system_ptr->OnUpdate();

    Sleep(1000);
  }
}