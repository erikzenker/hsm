#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

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
const auto g1 = []() {};

// Actions
const auto a1 = []() {};

using namespace ::testing;
using namespace boost::hana;

struct SubSubState {
    constexpr auto make_transition_table()
    {
        return hsm::transition_table(hsm::transition(S1 {}, e1 {}, g1, a1, S2 {}));
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

struct SubState {
    constexpr auto make_transition_table()
    {
        return hsm::transition_table(
            hsm::transition(S4 {}, e1 {}, g1, a1, S2 {}),
            hsm::transition(S4 {}, e5 {}, g1, a1, S3 {}),
            hsm::transition(S2 {}, e1 {}, g1, a1, SubSubState {}),
            hsm::transition(SubSubState {}, e2 {}, g1, a1, S4 {}),
            hsm::transition(hsm::Exit { SubSubState {}, S2 {} }, hsm::none {}, g1, a1, S4 {}));
    }

    constexpr auto initial_state()
    {
        return S4 {};
    }
};

struct MainState {
    constexpr auto make_transition_table()
    {
        return hsm::transition_table(
            hsm::transition(S1 {}, e1 {}, g1, a1, S2 {}),
            hsm::transition(S1 {}, e2 {}, g1, a1, S3 {}),
            hsm::transition(S1 {}, e4 {}, g1, a1, SubState {}),
            hsm::transition(S3 {}, hsm::none {}, g1, a1, S1 {}),
            hsm::transition(S1 {}, e5 {}, g1, a1, S3 {}),
            hsm::transition(S2 {}, e1 {}, g1, a1, S1 {}),
            hsm::transition(S2 {}, e2 {}, g1, a1, S1 {}),
            hsm::transition(S2 {}, e3 {}, g1, a1, S3 {}),
            hsm::transition(SubState {}, e2 {}, g1, a1, S1 {}));
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

class HsmTests : public Test {
    hsm::Sm<MainState> sm;
};

TEST_F(HsmTests, should_start_in_initial_state)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1 {}));
}

TEST_F(HsmTests, should_start_in_root_state)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(MainState {}, S1 {}));
}

TEST_F(HsmTests, should_process_event)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1 {}));

    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(S2 {}));
}

TEST_F(HsmTests, should_throw_on_unexpected_event)
{
    hsm::Sm<MainState> sm;
    EXPECT_THROW(sm.process_event(e3 {}), std::exception);
}

TEST_F(HsmTests, should_transit_into_SubState)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});

    ASSERT_TRUE(sm.is(SubState {}, S4 {}));
}

TEST_F(HsmTests, should_transit_into_SubSubState)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});

    ASSERT_TRUE(sm.is(SubSubState {}, S1 {}));
}

TEST_F(HsmTests, should_transit_in_SubState_with_unique_event)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e5 {});

    ASSERT_TRUE(sm.is(SubState {}, S3 {}));
}

TEST_F(HsmTests, should_exit_substate_on_event_in_parentstate)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(SubState {}, S4 {}));

    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(MainState {}, S1 {}));
}

TEST_F(HsmTests, should_exit_subsubstate_on_event_in_parentstate)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubSubState {}, S1 {}));

    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(SubState {}, S4 {}));
}

TEST_F(HsmTests, should_exit_subsubstate_from_pseudo_exit)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubSubState {}, S1 {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubState {}, S4 {}));
}

TEST_F(HsmTests, should_transit_with_anonymous_transition)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e5 {});
    ASSERT_TRUE(sm.is(S1 {}));
}

TEST_F(HsmTests, should_process_alot_event)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1 {}));

    for (int i = 0; i < 1000000; i++) {
        sm.process_event(e1 {});
    }
}