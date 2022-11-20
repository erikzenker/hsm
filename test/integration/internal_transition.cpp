

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
    static constexpr auto on_entry()
    {
        return [](auto event, auto...) { (void)event.name; };
    }

    static constexpr auto on_exit()
    {
        return [](auto event, auto...) { (void)event.name; };
    }
};
struct S3 {
};

// Events
struct e1 {
    std::string name = "e1";
};
struct e2 {
    e2(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
    std::string name = "e2";
};
struct e3 {
};
struct e4 {
    e4(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e5 {
};
struct e6 {
    std::string name = "e2";
};
struct e7 {
    std::string name = "e7";
};
struct e8 {
};
struct e9 {
};

// Guards
const auto fail = [](auto /*event*/, auto /*source*/, auto /*target*/) { return false; };

// Actions
const auto action = [](auto event, auto /*source*/, auto /*target*/) { event.called->set_value(); };

const auto error = [](auto /*event*/, auto /*source*/, auto /*target*/) { EXPECT_TRUE(false); };

using namespace ::testing;
using namespace boost::hana;

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e5> = hsm::state<S3>
        );
        // clang-format on
    }

    static constexpr auto make_internal_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            + (hsm::event<e1>),
            + (hsm::event<e8> / hsm::log)
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e7> = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e3> = hsm::state<SubState>
            , hsm::state<S2> + hsm::event<e6> = hsm::state<S1>
        );
        // clang-format on
    }

    static constexpr auto make_internal_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            + (hsm::event<e1> ),
            + (hsm::event<e2> / action),
            + (hsm::event<e5> [fail] / hsm::log),
            + (hsm::event<e4> [fail] / action),
            + (hsm::event<e8> / hsm::chain(hsm::log, error)),
            + (hsm::event<e9>)
        );
        // clang-format on
    }
};
}

class InternalTransitionTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(InternalTransitionTests, should_overwrite_internal_by_inner_external_transition)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}

TEST_F(InternalTransitionTests, should_overwrite_internal_by_inner_transition_in_substate)
{
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S1>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S2>));
}

TEST_F(InternalTransitionTests, should_call_action_on_internal_transition)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e2 { actionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S1>));

    ASSERT_TRUE(
        std::future_status::ready == actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(InternalTransitionTests, should_guard_internal_transition)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e5 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
}

TEST_F(InternalTransitionTests, should_guard_internal_transition_with_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e4 { actionCalled });
    ASSERT_TRUE(sm.is(hsm::state<S1>));

    ASSERT_TRUE(
        std::future_status::timeout
        == actionCalled->get_future().wait_for(std::chrono::milliseconds(1)));
}

TEST_F(
    InternalTransitionTests, should_not_overwrite_substate_transition_by_parent_internal_transition)
{
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S1>));
    sm.process_event(e5 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S3>));
}

TEST_F(
    InternalTransitionTests,
    should_not_overwrite_substate_internal_transition_by_parent_internal_transition)
{
    sm.process_event(e3 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S2>));
    sm.process_event(e8 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S2>));
}

TEST_F(InternalTransitionTests, should_not_transit_when_internal_transition_on_parent_state)
{
    sm.process_event(e3 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S2>));
    sm.process_event(e9 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S2>));
}

TEST_F(InternalTransitionTests, should_not_transit_to_initial_state)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e7 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}
