#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;

class TraitsTests : public Test {
};

class S1 {
};
class S2 {
};

TEST_F(TraitsTests, should_recognize_exit_state)
{
    auto exit = hsm::Exit(S1 {}, S2 {});

    boost::hana::if_(
        hsm::is_exit_state(exit),
        [](auto exit) { ASSERT_TRUE(true); },
        [&](auto) { ASSERT_TRUE(false); })(exit);
}