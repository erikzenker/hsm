#include "hsm/hsm.h"

#include <gtest/gtest.h>

namespace {

using namespace ::testing;

// Events
struct e2 {
};
struct e1 {
};

// States
struct S1 {
};
struct S2 {
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
                * hsm::state<S1> + hsm::event<e2>  = hsm::state<S1>
              , * hsm::state<S2> + hsm::event<e2>  = hsm::state<S2>
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
                * hsm::state<S1> + hsm::event<e1> = hsm::state<SubState>
        );
        // clang-format on
    }
};
}

TEST(ReproducerTests2, shouldEnterSubStateWithMultipleRegions)
{
    hsm::sm<MainState> sm;

    sm.process_event(e1 {});
}