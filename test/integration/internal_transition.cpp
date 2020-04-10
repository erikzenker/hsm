

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
};
struct e2 {
    e2(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e3 {
};

// Guards
const auto g1 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto a1 = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto a2 = [](auto event, auto /*source*/, auto /*target*/) { event.called->set_value(); };

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

    constexpr auto make_internal_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::event<e1> {}, g1, a1)
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
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S2> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e2> {}, g1, a1, hsm::state<S2> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e3> {}, g1, a1, hsm::state<SubState> {})
        );
        // clang-format on
    }

    constexpr auto make_internal_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::event<e1> {}, g1, a1),
            hsm::transition(hsm::event<e2> {}, g1, a2)
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};
}

class InternalTransitionTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(InternalTransitionTests, should_overwrite_external_by_internal_transition)
{
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}

TEST_F(InternalTransitionTests, should_overwrite_external_by_internal_transition_in_substate)
{
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
}

TEST_F(InternalTransitionTests, should_call_action_on_internal_transition)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e2 { actionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));

    ASSERT_EQ(
        std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}
