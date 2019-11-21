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
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {})
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
            hsm::transition(S1 {}      , hsm::event<e1> {}, g1, a1, SubState {}),
            //hsm::transition(S2 {}      , hsm::none {}     , g1, a1, hsm::History{SubState{}}),
            hsm::transition(SubState {}, hsm::event<e3> {}, g1, a1, S2{}),
            hsm::transition(SubState {}, hsm::event<e2> {}, g1, a1, hsm::History{SubState{}})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};
}

class HistoryPseudoStateTests : public Test {
  protected:
    hsm::Sm<MainState> sm;
};

TEST_F(HistoryPseudoStateTests, should_reentry_substate_in_history_state)
{
    ASSERT_TRUE(sm.is(MainState {}, S1 {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubState {}, S1 {}));
    sm.process_event(e1 {});
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(SubState {}, S2 {}));
}