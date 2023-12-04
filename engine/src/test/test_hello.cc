#include <gtest/gtest.h>

#include "runtime/core/base/logger.h"
#include "runtime/core/log/peanut_log.h"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

TEST(LogTest, BasicAssertions) {
  Peanut::LogSystem::init("./logs/log.txt", true);
  PEANUT_LOG_INFO("hello world!");
}