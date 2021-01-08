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
    auto exit = hsm::state_t<hsm::Exit<S1, S2>> {};

    boost::hana::if_(
        hsm::is_exit_state(exit),
        [](auto /*exit*/) { ASSERT_TRUE(true); },
        [&](auto) { ASSERT_TRUE(false); })(exit);
}

TEST_F(TraitsTests, should_recognize_transition_table)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::has_transition_table(hsm::state_t<S1> {}),
        []() { return true; },
        []() { return false; })();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_not_recognize_transition_table)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::has_transition_table(hsm::state_t<S2> {}),
        []() { return true; },
        []() { return false; })();
    ASSERT_FALSE(result);
}

TEST_F(TraitsTests, should_recognize_internal_transition_table)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::has_internal_transition_table(hsm::state_t<S1> {}),
        []() { return true; },
        []() { return false; })();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_on_entry_function)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::has_entry_action(hsm::state_t<S1> {}),
        []() { return true; },
        []() { return false; })();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_on_exit_function)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::has_exit_action(hsm::state_t<S1> {}), []() { return true; }, []() { return false; })();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_history_state)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::is_history_state(hsm::state_t<hsm::History<S1>> {}),
        []() { return true; },
        []() { return false; })();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_initial_state)
{
    namespace bh = boost::hana;

    constexpr auto result = bh::if_(
        hsm::is_initial_state(hsm::state_t<hsm::Initial<S1>> {}),
        []() { return true; },
        []() { return false; })();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_defered_events)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::has_deferred_events(hsm::state_t<S1> {}),
        []() { return true; },
        []() { return false; })();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_no_action)
{
    namespace bh = boost::hana;

    constexpr auto result = bh::if_(
        hsm::is_no_action(hsm::noAction {}), []() { return true; }, []() { return false; })();
    static_assert(result);
}

TEST_F(TraitsTests, should_recognize_substate_initial_state_entry_action)
{
    namespace bh = boost::hana;

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

    auto result = bh::if_(
        hsm::has_substate_initial_state_entry_action(hsm::state_t<SubState> {}),
        []() { return true; },
        []() { return false; })();
    ASSERT_TRUE(result);
}