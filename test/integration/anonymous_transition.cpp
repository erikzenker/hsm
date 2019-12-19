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
};

// Guards
const auto g1 = [](auto /*event*/) { return true; };

// Actions
const auto a1 = [](auto /*event*/) {};

using namespace ::testing;

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),    
            hsm::transition(S2 {}, hsm::none {}, g1, a1, S3 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

}

class AnonymousTransitionTests : public Test {
  protected:
    hsm::Sm<MainState> sm;
};

TEST_F(AnonymousTransitionTests, should_transit_with_anonymous_transition)
{
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(S3 {}));
}