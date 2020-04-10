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
        return [](auto event, auto /*source*/, auto /*target*/) { event.called->set_value(); };
    }
    constexpr auto on_exit()
    {
        return [](auto event, auto /*source*/, auto /*target*/) { event.called->set_value(); };
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
const auto g1 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto a1 = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::state<S1> {} + hsm::event<e1> {} / a1 =  hsm::state<S1> {}
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
    constexpr auto on_entry()
    {
        return [](auto event, auto /*source*/, auto /*target*/) { event.called->set_value(); };
    }
    constexpr auto on_exit()
    {
        return [](auto event, auto /*source*/, auto /*target*/) { event.called->set_value(); };
    }
};


struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::state<S1> {}       + hsm::event<e1> {} / a1 = hsm::state<S2> {},
            hsm::state<S2> {}       + hsm::event<e1> {} / a1 = hsm::state<S1> {},
            hsm::state<S1> {}       + hsm::event<e2> {} / a1 = hsm::state<SubState> {},
            hsm::state<SubState> {} + hsm::event<e2> {} / a1 = hsm::state<S1> {}
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};

}

class EntryExitActionsTests : public Test {
    protected:    
        hsm::sm<MainState> sm;
};

TEST_F(EntryExitActionsTests, should_call_entry_and_exit_action)
{
    auto entryActionCalled = std::make_shared<std::promise<void>>();
    auto exitActionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e1 { entryActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S2> {}));
    sm.process_event(e1 { exitActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));

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

    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e2{ entryActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e2 { exitActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));

    ASSERT_EQ(
        std::future_status::ready,
        entryActionCalled->get_future().wait_for(std::chrono::seconds(1)));
    ASSERT_EQ( // <-- fails in this assert
        std::future_status::ready,
        exitActionCalled->get_future().wait_for(std::chrono::seconds(1)));
}