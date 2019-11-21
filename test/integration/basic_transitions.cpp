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
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto event) {};

using namespace ::testing;
using namespace boost::hana;

struct SubSubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e5> {}, g1, a1, S3 {}),
            hsm::transition(S2 {}, hsm::event<e1> {}, g1, a1, SubSubState {}),
            hsm::transition(SubSubState {}, hsm::event<e2> {}, g1, a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e2> {}, g1, a1, S3 {}),
            hsm::transition(S1 {}, hsm::event<e4> {}, g1, a1, SubState {}),
            hsm::transition(S1 {}, hsm::event<e5> {}, g1, a1, S3 {}),
            hsm::transition(S2 {}, hsm::event<e1> {}, g1, a1, S1 {}),
            hsm::transition(S2 {}, hsm::event<e2> {}, g1, a1, S1 {}),
            hsm::transition(S2 {}, hsm::event<e3> {}, g1, a1, S3 {}),
            hsm::transition(SubState {}, hsm::event<e2> {}, g1, a1, S1 {}),
            hsm::transition(SubState {}, hsm::event<e4> {}, g1, a1, SubState {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

}

class BasicTransitionTests : public Test {
    protected:    
        hsm::Sm<MainState> sm;
};

TEST_F(BasicTransitionTests, should_start_in_initial_state)
{
    ASSERT_TRUE(sm.is(S1 {}));
}

TEST_F(BasicTransitionTests, should_start_in_root_state)
{
    ASSERT_TRUE(sm.is(MainState {}, S1 {}));
}

TEST_F(BasicTransitionTests, should_process_event)
{
    ASSERT_TRUE(sm.is(S1 {}));

    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(S2 {}));
}

TEST_F(BasicTransitionTests, should_transit_into_SubState)
{
    sm.process_event(e4 {});

    ASSERT_TRUE(sm.is(SubState {}, S1 {}));
}

TEST_F(BasicTransitionTests, should_transit_into_SubSubState)
{
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});

    ASSERT_TRUE(sm.is(SubSubState {}, S1 {}));
}

TEST_F(BasicTransitionTests, should_transit_in_SubState_with_unique_event)
{
    sm.process_event(e4 {});
    sm.process_event(e5 {});

    ASSERT_TRUE(sm.is(SubState {}, S3 {}));
}

TEST_F(BasicTransitionTests, should_exit_substate_on_event_in_parentstate)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(SubState {}, S1 {}));

    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(MainState {}, S1 {}));
}

TEST_F(BasicTransitionTests, should_exit_subsubstate_on_event_in_parentstate)
{
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubSubState {}, S1 {}));

    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(SubState {}, S1 {}));
}

TEST_F(BasicTransitionTests, should_reentry_substate_on_initial_state)
{
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubState {}, S2 {}));
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(SubState {}, S1 {}));
}

TEST_F(BasicTransitionTests, should_process_alot_event)
{

    ASSERT_TRUE(sm.is(S1 {}));

    for (int i = 0; i < 1000000; i++) {
        sm.process_event(e1 {});
    }
}