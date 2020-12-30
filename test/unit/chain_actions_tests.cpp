#include "hsm/details/chain_actions.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <functional>
#include <future>

using namespace ::testing;

class ChainActionsTests : public Test {
};

TEST_F(ChainActionsTests, should_perfect_forward_args)
{
    auto action1 = [](bool& called) { called = true; };

    bool called = false;
    hsm::chain_actions(action1)(called);
    ASSERT_TRUE(called);
}