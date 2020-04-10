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
const auto g1 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto a1 = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

using namespace ::testing;
using namespace boost::hana;

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S2> {})
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
            hsm::transition(hsm::state<S1> {}       , hsm::event<e1> {} , g1, a1,  hsm::state<SubState> {}),
            hsm::transition(hsm::state<SubState> {} , hsm::event<e3> {} , g1, a1,  hsm::state<S2> {}),
            hsm::transition(hsm::state<SubState> {} , hsm::event<e2> {} , g1, a1,  hsm::history<SubState> {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};
}

class HistoryPseudoStateTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(HistoryPseudoStateTests, should_reentry_substate_in_history_state)
{
    ASSERT_TRUE(sm.is(hsm::state<MainState> {}, hsm::state<S1> {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S2> {}));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S2> {}));
}