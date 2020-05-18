#include "hsm/hsm.h"

#include <boost/hana/experimental/printable.hpp>
#include <gtest/gtest.h>

using namespace ::testing;
using namespace boost::hana;

namespace {

// States
struct S1 {
};
struct S2 {
};
struct S3 {
};
struct S4 {
};
struct S5 {
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
struct e6 {
};

// Guards
const auto fail = [](auto /*event*/, auto /*source*/, auto /*target*/) { return false; };
const auto success = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto action = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

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
            * hsm::state<S1> {} + hsm::event<e1> {} = hsm::state<S2> {},
              hsm::state<S1> {} + hsm::event<e2> {} = hsm::state<S3> {},
              hsm::state<S1> {} + hsm::event<e4> {} = hsm::state<S4> {},
              hsm::state<S1> {} + hsm::event<e5> {} = hsm::state<S5> {}
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> {}          + hsm::event<e1> {}                    = hsm::entry<SubState, S3> {},
              hsm::state<S1> {}          + hsm::event<e2> {}                    = hsm::state<SubState> {},
              hsm::state<S1> {}          + hsm::event<e3> {} [fail]             = hsm::entry<SubState, S3> {},
              hsm::state<S1> {}          + hsm::event<e4> {} [fail]    / action = hsm::entry<SubState, S3> {},
              hsm::state<S1> {}          + hsm::event<e5> {}           / action = hsm::entry<SubState, S3> {},
              hsm::state<S1> {}          + hsm::event<e6> {}                    = hsm::state<S2> {},
              hsm::state<S2> {}                                                 = hsm::entry<SubState, S3> {},
              hsm::exit<SubState, S4> {} + hsm::event<e1> {}                    = hsm::state<S1> {},
              hsm::exit<SubState, S4> {} + hsm::event<e2> {} [success]          = hsm::state<S1> {},
              hsm::exit<SubState, S4> {} + hsm::event<e3> {} [success] / action = hsm::state<S1> {},
              hsm::exit<SubState, S4> {} + hsm::event<e4> {}           / action = hsm::state<S1> {},
              hsm::exit<SubState, S5> {}                                        = hsm::state<S1> {}
        );
        // clang-format on
    }
};

}

class EntryPseudoStatesTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(EntryPseudoStatesTests, should_entry_substate_on_pseudo_entry)
{
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S3> {}));
}

TEST_F(EntryPseudoStatesTests, should_entry_substate_on_pseudo_entry_with_anonymous_transition)
{
    sm.process_event(e6 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S3> {}));
}

TEST_F(EntryPseudoStatesTests, should_fail_to_entry_substate_on_pseudo_entry)
{
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<MainState> {}, hsm::state<S1> {}));
}

TEST_F(EntryPseudoStatesTests, should_fail_to_entry_substate_on_pseudo_entry_with_action)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<MainState> {}, hsm::state<S1> {}));
}

TEST_F(EntryPseudoStatesTests, should_entry_substate_on_pseudo_entry_with_action)
{
    sm.process_event(e5 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S3> {}));
}

class ExitPseudoStatesTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(ExitPseudoStatesTests, should_exit_subsubstate_from_pseudo_exit)
{
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}

TEST_F(ExitPseudoStatesTests, should_exit_subsubstate_from_pseudo_exit_with_action)
{
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e4 {});
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}

TEST_F(ExitPseudoStatesTests, should_exit_subsubstate_from_pseudo_exit_with_guard)
{
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e4 {});
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}

TEST_F(ExitPseudoStatesTests, should_exit_subsubstate_from_pseudo_exit_with_action_and_guard)
{
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e4 {});
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}

TEST_F(ExitPseudoStatesTests, should_exit_subsubstate_from_pseudo_exit_with_anonymous_transition)
{
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e5 {});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}