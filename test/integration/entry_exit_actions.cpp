#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <future>

namespace {

using namespace ::testing;

// States
struct S1 {
};
struct S2 {
    static constexpr auto on_entry()
    {
        return [](auto event, auto /*source*/, auto /*target*/, auto& /*dep*/) {
            event.called->set_value();
        };
    }
    static constexpr auto on_exit()
    {
        return [](auto event, auto /*source*/, auto /*target*/, auto& /*dep*/) {
            event.called->set_value();
        };
    }
};
struct Exit {
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
struct e3 {
    e3(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e4 {
};

struct SubState {

    static constexpr auto on_entry()
    {
        return [](auto event, auto /*source*/, auto /*target*/, auto& /*dep*/) {
            event.called->set_value();
        };
    }

    static constexpr auto on_exit()
    {
        return [](auto event, auto /*source*/, auto /*target*/, auto& /*dep*/) {
            event.called->set_value();
        };
    }

    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> = hsm::state<S1>
        );
        // clang-format on
    }
};

struct SubState2 {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S2> + hsm::event<e1> = hsm::state<S2>
        );
        // clang-format on
    }
};

struct SubState3 {
    static constexpr auto on_exit()
    {
        return [](auto event, auto source, auto target, auto& dep) {
            std::cout << "on_exit" << std::endl;
            hsm::log(event, source, target, dep);
            dep.called.set_value();
        };
    }

    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e4> / hsm::log = hsm::state<Exit>
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>             + hsm::event<e1> = hsm::state<S2>
            , hsm::state<S2>             + hsm::event<e1> = hsm::state<S1>
            , hsm::state<S1>             + hsm::event<e2> = hsm::state<SubState>
            , hsm::state<S1>             + hsm::event<e3> = hsm::state<SubState2>
            , hsm::state<S1>             + hsm::event<e4> / hsm::log = hsm::state<SubState3>
            , hsm::state<SubState>       + hsm::event<e2> = hsm::state<S1>
            , hsm::exit<SubState3, Exit>                  / hsm::log = hsm::state<S1>
        );
        // clang-format on
    }
};

struct Dependency {
    std::promise<void> called;
};
}

class EntryExitActionsTests : public Test {
  protected:
    EntryExitActionsTests()
        : sm(dep)
    {
    }

    Dependency dep;
    hsm::sm<MainState, Dependency> sm;
};

TEST_F(EntryExitActionsTests, should_call_entry_and_exit_action)
{
    auto entryActionCalled = std::make_shared<std::promise<void>>();
    auto exitActionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e1 { entryActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S2>));
    sm.process_event(e1 { exitActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S1>));

    ASSERT_TRUE(
        std::future_status::ready
        == entryActionCalled->get_future().wait_for(std::chrono::seconds(1)));
    ASSERT_TRUE(
        std::future_status::ready
        == exitActionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(EntryExitActionsTests, should_call_entry_and_exit_action_of_substate)
{
    auto entryActionCalled = std::make_shared<std::promise<void>>();
    auto exitActionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e2{ entryActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S1>));
    sm.process_event(e2 { exitActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S1>));

    ASSERT_TRUE(
        std::future_status::ready
        == entryActionCalled->get_future().wait_for(std::chrono::seconds(1)));
    ASSERT_TRUE(
        std::future_status::ready
        == exitActionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(EntryExitActionsTests, should_call_entry_action_of_substate_initial_state)
{
    auto entryActionCalled = std::make_shared<std::promise<void>>();

    sm.process_event(e3 { entryActionCalled });
    ASSERT_TRUE(sm.is(hsm::state<SubState2>, hsm::state<S2>));

    ASSERT_TRUE(
        std::future_status::ready
        == entryActionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(EntryExitActionsTests, should_call_exit_action_on_pseudo_exit)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState3>, hsm::state<S1>));

    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));

    ASSERT_TRUE(
        std::future_status::ready == dep.called.get_future().wait_for(std::chrono::seconds(1)));
}