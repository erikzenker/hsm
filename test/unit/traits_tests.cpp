#include "hsm/details/traits.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;

class TraitsTests : public Test {
};

struct S1 {
    constexpr auto on_entry(){
    }

    constexpr auto on_exit(){
    }

    constexpr auto make_transition_table(){
    }

    constexpr auto make_internal_transition_table()
    {
    }
};

struct S2 {
};

TEST_F(TraitsTests, should_recognize_exit_state)
{
    auto exit = hsm::Exit(S1 {}, S2 {});

    boost::hana::if_(
        hsm::is_exit_state(exit),
        [](auto /*exit*/) { ASSERT_TRUE(true); },
        [&](auto) { ASSERT_TRUE(false); })(exit);
}

TEST_F(TraitsTests, should_recognize_transition_table)
{
    namespace bh = boost::hana;

    auto result = bh::if_(hsm::has_transition_table(S1{}),
                    [](){ return true;},
                    [](){ return false;})();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_internal_transition_table)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::has_internal_transition_table(S1 {}), []() { return true; }, []() { return false; })();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_on_entry_function)
{
    namespace bh = boost::hana;

    auto result = bh::if_(hsm::has_entry_action(S1{}),
                    [](){ return true;},
                    [](){ return false;})();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_on_exit_function)
{
    namespace bh = boost::hana;

    auto result = bh::if_(hsm::has_exit_action(S1{}),
                    [](){ return true;},
                    [](){ return false;})();
    ASSERT_TRUE(result);
}

TEST_F(TraitsTests, should_recognize_history_state)
{
    namespace bh = boost::hana;

    auto result = bh::if_(
        hsm::is_history_state(hsm::History { S1 {} }),
        []() { return true; },
        []() { return false; })();
    ASSERT_TRUE(result);
}
