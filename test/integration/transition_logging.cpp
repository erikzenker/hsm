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

// Transition logging action
const auto log = [](auto event, auto source, auto target) {
    std::cout << experimental::print(typeid_(source)) << " + "
              << experimental::print(typeid_(event)) << " = "
              << experimental::print(typeid_(target)) << std::endl;
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> / log = hsm::state<S2>
        );
        // clang-format on
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
}