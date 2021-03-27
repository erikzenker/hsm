#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>
#include <gtest/gtest.h>

namespace {

using namespace ::testing;
using namespace boost::hana;

// States
struct S1 {
};
struct S2 {
};

// Events
struct e1 {
};
struct e2 {
};

// Transition logging action
const auto log = [](auto event, auto source, auto target) {
    std::cout << experimental::print(typeid_(source)) << " + "
              << experimental::print(typeid_(event)) << " = "
              << experimental::print(typeid_(target)) << std::endl;
};

const auto logUnexpectedEvent = [](auto event, auto currentState) {
    std::cout << "Unexpected event: " << experimental::print(typeid_(currentState)) << " + "
              << experimental::print(typeid_(event)) << std::endl;
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> / log = hsm::state<S2>
            , hsm::state<S2> + hsm::event<e2> / log = hsm::state<S1>
        );
        // clang-format on
    }

    static constexpr auto on_unexpected_event()
    {
        return logUnexpectedEvent;
    }
};

}

class TransitionLoggingTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(TransitionLoggingTests, should_log_transition)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}

TEST_F(TransitionLoggingTests, should_log_unexpected_event)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
}