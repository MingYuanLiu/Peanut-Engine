#include <gtest/gtest.h>
#include "runtime/engine.h"
using namespace peanut;

TEST(EngineMainTest, BasicAssertions) {
  PeanutEngine::GetInstance().Initliaze();
  PeanutEngine::GetInstance().Run();
}