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

// Events
struct e1 {
    e1(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e2 {
};
struct e3 {
};
struct e4 {
};

// Guards
const auto g2 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return false; };
const auto g3 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto a2 = [](auto event, auto /*source*/, auto /*target*/) { event.called->set_value(); };

using namespace ::testing;
using namespace boost::hana;

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> / a2 = hsm::state<S1>
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> /  a2 = hsm::state<S1>,
              hsm::state<S1> + hsm::event<e2>       = hsm::state<SubState>,
              hsm::state<S1> + hsm::event<e3>  [g2] = hsm::state<S2>,
              hsm::state<S1> + hsm::event<e4>  [g3] = hsm::state<S2>
        );
        // clang-format on
    }
};

}

class GuardsActionsTests : public Test {
    protected:    
        hsm::sm<MainState> sm;
};



TEST_F(GuardsActionsTests, should_call_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    sm.process_event(e1 { actionCalled });

    ASSERT_EQ(
        std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(GuardsActionsTests, should_call_substate_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    sm.process_event(e2 {});
    sm.process_event(e1 { actionCalled });

    ASSERT_EQ(
        std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(GuardsActionsTests, should_block_transition_guard)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
}

TEST_F(GuardsActionsTests, should_not_block_transition_by_guard)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}