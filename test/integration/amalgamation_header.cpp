#include "hsm/hsm_gen.h"

#include <gtest/gtest.h>

namespace {

// States
struct S1 {
};

// Events
struct e1 {
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> = hsm::state<S1>
        );
        // clang-format on
    }
};

}

using namespace ::testing;
class AmalgamationHeaderTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(AmalgamationHeaderTests, should_start_in_initial_state)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
}