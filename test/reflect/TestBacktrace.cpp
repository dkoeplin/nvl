#include <gtest/gtest.h>

#include <csignal>

#include "nvl/reflect/Backtrace.h"

namespace {

TEST(TestBacktrace, segfault) {
    nvl::register_signal_handlers();
    EXPECT_DEATH({ raise(SIGSEGV); }, ".*");
}

} // namespace
