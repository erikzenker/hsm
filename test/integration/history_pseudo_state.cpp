#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>
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

using namespace ::testing;
using namespace boost::hana;

const auto log = [](auto event, auto source, auto target) {
    std::cout << experimental::print(typeid_(source)) << " + "
              << experimental::print(typeid_(event)) << " = "
              << experimental::print(typeid_(target)) << std::endl;
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> / log = hsm::state<S2>
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
           * hsm::state<S1>       + hsm::event<e1> / log = hsm::state<SubState>
           , hsm::state<S1>       + hsm::event<e2> / log = hsm::history<SubState>
           , hsm::state<SubState> + hsm::event<e3> / log = hsm::state<S2>
           , hsm::state<SubState> + hsm::event<e2> / log = hsm::history<SubState>
        );
        // clang-format on
    }
};
}

class HistoryPseudoStateTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(HistoryPseudoStateTests, should_reentry_substate_in_history_state)
{
    ASSERT_TRUE(sm.is(hsm::state<MainState>, hsm::state<S1>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S1>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S2>));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S2>));
}

TEST_F(HistoryPseudoStateTests, should_enter_history_substate_first_time_on_initial_state)
{
    ASSERT_TRUE(sm.is(hsm::state<MainState>, hsm::state<S1>));
    sm.process_event(e2 {});
    ASSERT_FALSE(sm.is(hsm::state<MainState>, hsm::state<S1>));
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S1>));
}