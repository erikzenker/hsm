#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <future>

namespace {

using namespace ::testing;

// States
struct S1 {
};
struct S2 {
    constexpr auto on_entry()
    {
        return [](auto event) { event.called->set_value(); };
    }
    constexpr auto on_exit()
    {
        return [](auto event) { event.called->set_value(); };
    }
};

// Events
struct e1 {
    e1(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e2 {
    e2(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};

// Guards
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto event) {};

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
    constexpr auto on_entry()
    {
        return [](auto event) { event.called->set_value(); };
    }
    constexpr auto on_exit()
    {
        return [](auto event) { event.called->set_value(); };
    }
};


struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S2 {}, hsm::event<e1> {}, g1, a1, S1 {}),
            hsm::transition(S1 {}, hsm::event<e2> {}, g1, a1, SubState {}),
            hsm::transition(SubState {}, hsm::event<e2> {}, g1, a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

}

class EntryExitActionsTests : public Test {
    protected:    
        hsm::Sm<MainState> sm;
};

TEST_F(EntryExitActionsTests, should_call_entry_and_exit_action)
{
    auto entryActionCalled = std::make_shared<std::promise<void>>();
    auto exitActionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e1 { entryActionCalled });
    ASSERT_TRUE(sm.is(S2 {}));
    sm.process_event(e1 { exitActionCalled });
    ASSERT_TRUE(sm.is(S1 {}));

    ASSERT_EQ(
        std::future_status::ready,
        entryActionCalled->get_future().wait_for(std::chrono::seconds(1)));
    ASSERT_EQ(
        std::future_status::ready,
        exitActionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

// TODO: No exit actions of substates are generated
TEST_F(EntryExitActionsTests, DISABLED_should_call_entry_and_exit_action_of_substate)
{
    auto entryActionCalled = std::make_shared<std::promise<void>>();
    auto exitActionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e2{ entryActionCalled });
    ASSERT_TRUE(sm.is(SubState{}, S1 {}));
    sm.process_event(e2 { exitActionCalled });
    ASSERT_TRUE(sm.is(S1 {}));

    ASSERT_EQ(
        std::future_status::ready,
        entryActionCalled->get_future().wait_for(std::chrono::seconds(1)));
    ASSERT_EQ( // <-- fails in this assert
        std::future_status::ready,
        exitActionCalled->get_future().wait_for(std::chrono::seconds(1)));
}