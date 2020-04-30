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
struct S4 {
};
struct S5 {
};
struct S6 {
};
struct S7 {
};
struct S8 {
};
struct S9 {
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};
struct e4 {
};

// Guards
const auto guard = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto action = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

using namespace ::testing;

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::state<S1> {} + hsm::event<e1> {}                = hsm::state<S2> {},
            hsm::state<S1> {} + hsm::event<e2> {}                = hsm::state<S4> {},
            hsm::state<S1> {} + hsm::event<e3> {}                = hsm::state<S6> {},
            hsm::state<S1> {} + hsm::event<e4> {}                = hsm::state<S8> {},
            hsm::state<S2> {}                                    = hsm::state<S3> {},
            hsm::state<S4> {}                           / action = hsm::state<S5> {},
            hsm::state<S6> {}                   [guard]          = hsm::state<S7> {},
            hsm::state<S8> {}                   [guard] / action = hsm::state<S9> {}
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

TEST_F(AnonymousTransitionTests, should_transit_with_anonymous_transition_and_action)
{
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<S5> {}));
}

TEST_F(AnonymousTransitionTests, should_transit_with_anonymous_transition_and_guard)
{
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<S7> {}));
}

TEST_F(AnonymousTransitionTests, should_transit_with_anonymous_transition_and_guard_and_action)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<S9> {}));
}