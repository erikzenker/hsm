#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <future>

// States
struct S1 {
};
struct S2 {
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};

// Guards
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto event) {};

using namespace ::testing;
using namespace boost::hana;

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            hsm::transition(S1 {}, hsm::event<e3> {}, g1, a1, SubState {}),
            hsm::transition(S1 {}, hsm::event<e2> {}, g1, a1, S2 {}),
            hsm::transition(S2 {}, hsm::event<e2> {}, g1, a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

class FailingTests : public Test {
    protected:    
        hsm::Sm<MainState> sm;
};

TEST_F(FailingTests, fails1)
{
    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(S2 {}));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(S1 {}));
}