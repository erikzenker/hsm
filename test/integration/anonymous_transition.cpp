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
const auto g1 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto a1 = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

using namespace ::testing;

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S2> {}),    
            hsm::transition(hsm::state<S2> {}, hsm::none {}     , g1, a1, hsm::state<S3> {})
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};
}

class AnonymousTransitionTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(AnonymousTransitionTests, should_transit_with_anonymous_transition)
{
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S3> {}));
}