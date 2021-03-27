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
struct S3 {
};

// Events
struct e1 {
    bool called = false;
};
struct e2 {
    bool called = false;
};
struct e3 {
    bool called = false;
};

// Guards
constexpr auto alwaysFalse = [](auto...) { return false; };

using namespace ::testing;
using namespace boost::hana;

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1>              = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e3>              = hsm::state<S3> 
            , hsm::state<S2> + hsm::event<e2>              = hsm::state<S1>
            , hsm::state<S3> + hsm::event<e2> [alwaysFalse]= hsm::state<S1>
        );
        // clang-format on
    }

    static constexpr auto on_unexpected_event()
    {
        return [](auto& event, auto /*currentState*/) { event.called = true; };
    }
};

}

class UnexpectedEventHandler : public Test {
  protected:
    hsm::sm<MainState> sm;
    hsm::sm<MainState> sm2;
};

TEST_F(UnexpectedEventHandler, should_call_unexpected_event_handler)
{
    auto event = e2 {};
    sm.process_event(event);

    ASSERT_TRUE(event.called);
}

TEST_F(UnexpectedEventHandler, should_not_call_unexpected_event_handler_when_guard_fails)
{

    sm.process_event(e3 {});

    auto event = e2 {};
    sm.process_event(event);

    ASSERT_FALSE(event.called);
}