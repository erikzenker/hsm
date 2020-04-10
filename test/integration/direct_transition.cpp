#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

// States
struct S1 {
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};

// Guards
const auto g1 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto a1 = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

using namespace ::testing;

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S1> {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};

struct SubState2 {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S1> {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            hsm::transition(hsm::state<S1>{}            , hsm::event<e1> {}, g1, a1, hsm::state<SubState>{}),
            hsm::transition(hsm::state<S1>{}            , hsm::event<e2> {}, g1, a1, hsm::state<SubState2>{}),
            hsm::transition(hsm::state<S1>{}            , hsm::event<e3> {}, g1, a1, hsm::direct<SubState, S1>{}),
            hsm::transition(hsm::direct<SubState, S1> {}, hsm::event<e2> {}, g1, a1, hsm::direct<SubState2, S1>{})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
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
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
}

TEST_F(DirectTransitionTests, should_transit_directly_between_substates)
{
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState2> {}, hsm::state<S1> {}));
}
