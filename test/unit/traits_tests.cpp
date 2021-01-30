#include "hsm/details/has_action.h"
#include "hsm/details/state.h"
#include "hsm/details/traits.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;

namespace {
class TraitsTests : public Test {
};

struct e1 {
};

struct S1 {
    constexpr auto on_entry(){
    }

    constexpr auto on_exit(){
    }

    static constexpr auto make_transition_table()
    {
    }

    constexpr auto make_internal_transition_table()
    {
        return 0;
    }

    constexpr auto defer_events()
    {
        return hsm::events<e1>;
    }
};

struct S2 {
};
}

TEST_F(TraitsTests, should_recognize_exit_state)
{
    constexpr auto exit = hsm::state_t<hsm::Exit<S1, S2>> {};
    static_assert(hsm::is_exit_state(exit));
}

TEST_F(TraitsTests, should_recognize_transition_table)
{
    static_assert(hsm::has_transition_table(hsm::state_t<S1> {}));
}

TEST_F(TraitsTests, should_not_recognize_transition_table)
{
    static_assert(!hsm::has_transition_table(hsm::state_t<S2> {}));
}

TEST_F(TraitsTests, should_recognize_internal_transition_table)
{
    static_assert(hsm::has_internal_transition_table(hsm::state_t<S1> {}));
}

TEST_F(TraitsTests, should_recognize_on_entry_function)
{
    static_assert(hsm::has_entry_action(hsm::state_t<S1> {}));
}

TEST_F(TraitsTests, should_recognize_on_exit_function)
{
    static_assert(hsm::has_exit_action(hsm::state_t<S1> {}));
}

TEST_F(TraitsTests, should_recognize_history_state)
{
    static_assert(hsm::is_history_state(hsm::state_t<hsm::History<S1>> {}));
}

TEST_F(TraitsTests, should_recognize_initial_state)
{
    static_assert(hsm::is_initial_state(hsm::state_t<hsm::Initial<S1>> {}));
}

TEST_F(TraitsTests, should_recognize_defered_events)
{
    static_assert(hsm::has_deferred_events(hsm::state_t<S1> {}));
}

TEST_F(TraitsTests, should_recognize_no_action)
{
    static_assert(hsm::is_no_action(hsm::noAction {}));
}

TEST_F(TraitsTests, should_recognize_substate_initial_state_entry_action)
{
    struct SubState {
        static auto constexpr make_transition_table()
        {
            return hsm::transition_table(hsm::transition(
                hsm::initial_t<S1> {},
                hsm::event_t<e1> {},
                hsm::noGuard {},
                hsm::noAction {},
                hsm::state_t<S1> {}));
        }
    };

    static_assert(hsm::has_substate_initial_state_entry_action(hsm::state_t<SubState> {}));
}