#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

// States
struct S1 {
};
struct S2 {
};
struct S3 {
};
struct S4 {
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};
struct e4 {
};
struct e5 {
};

// Guards
const auto fail = [](auto /*event*/, auto /*source*/, auto /*target*/) { return false; };

// Actions
const auto action = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

using namespace ::testing;

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> = hsm::state<S1>,
              hsm::state<S1> + hsm::event<e3> = hsm::state<S2>,
              hsm::state<S1> + hsm::event<e4> = hsm::state<S3>,
              hsm::state<S1> + hsm::event<e5> = hsm::state<S4>
        );
        // clang-format on
    }
};

struct SubState2 {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> = hsm::state<S1>
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>             + hsm::event<e1>                  = hsm::state<SubState>,
              hsm::state<S1>             + hsm::event<e2>                  = hsm::state<SubState2>,
              hsm::state<S1>             + hsm::event<e3>                  = hsm::direct<SubState, S1>,
              hsm::direct<SubState, S1>  + hsm::event<e2>                  = hsm::direct<SubState2, S1>,
              hsm::direct<SubState, S2>                                     = hsm::direct<SubState2, S2>,
              hsm::direct<SubState, S3>  + hsm::event<e2>  [fail]          = hsm::direct<SubState2, S3>,
              hsm::direct<SubState, S4>  + hsm::event<e2>         / action = hsm::direct<SubState2, S4>,
              hsm::direct<SubState, S4>  + hsm::event<e2>  [fail] / action = hsm::direct<SubState2, S4>
        );
        // clang-format on
    }
};

}

class DirectTransitionTests : public Test {
    protected:    
        hsm::sm<MainState> sm;
};

TEST_F(DirectTransitionTests, should_transit_directly_into_substate)
{
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S1>));
}

TEST_F(DirectTransitionTests, should_transit_directly_between_substates)
{
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S1>));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState2>, hsm::state<S1>));
}

TEST_F(DirectTransitionTests, should_transit_anonymous_directly_between_substates)
{
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S1>));
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState2>, hsm::state<S2>));
}

TEST_F(DirectTransitionTests, should_transit_directly_between_substates_with_guard)
{
    sm.process_event(e1 {});
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S3>));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S3>));
}

TEST_F(DirectTransitionTests, should_transit_directly_between_substates_with_action)
{
    sm.process_event(e1 {});
    sm.process_event(e5 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S4>));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState2>, hsm::state<S4>));
}