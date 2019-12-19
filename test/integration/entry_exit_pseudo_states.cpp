#include "hsm/hsm.h"

#include <gtest/gtest.h>

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

// Guards
const auto g1 = [](auto /*event*/) { return true; };

// Actions
const auto a1 = [](auto /*event*/) {};

using namespace ::testing;

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e2> {}, g1, a1, S3 {})
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
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, hsm::Entry {SubState{}, S3{}}),
            hsm::transition(S1 {}, hsm::event<e2> {}, g1, a1, SubState{}),                        
            hsm::transition(hsm::Exit { SubState {}, S2 {} }, hsm::none {}, g1, a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

}

class EntryExitPseudoStatesTests : public Test {
    protected:    
        hsm::Sm<MainState> sm;
};

TEST_F(EntryExitPseudoStatesTests, should_entry_substate_on_pseudo_entry)
{
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubState {}, S3 {}));
}

TEST_F(EntryExitPseudoStatesTests, should_exit_subsubstate_from_pseudo_exit)
{
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(SubState {}, S1 {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(S1 {}));
}