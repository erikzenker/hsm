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
using namespace boost::hana;

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //   Target      <=   Source           + Event            [Guard] / Action
            hsm::state<S2>{} <= * hsm::state<S1>{} + hsm::event<e1>{},
            hsm::state<S3>{} <=   hsm::state<S1>{} + hsm::event<e4>{},
            hsm::state<S2>{} <=   hsm::state<S1>{} + hsm::event<e2>{} [guard],
            hsm::state<S2>{} <=   hsm::state<S1>{} + hsm::event<e3>{} [guard] / action,
            hsm::state<S2>{} <=   hsm::state<S1>{} + hsm::event<e3>{}         / action,
            hsm::state<S1>{} <=   hsm::state<S3>{}
        );
        // clang-format on
    }
};
}

class TransitionPrefixDslTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(TransitionPrefixDslTests, should_use_transition_dsl)
{
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S2> {}));
}

TEST_F(TransitionPrefixDslTests, should_use_transition_dsl_with_anonymous_event)
{
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}

TEST_F(TransitionPrefixDslTests, should_use_transition_dsl_with_guard)
{
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<S2> {}));
}

TEST_F(TransitionPrefixDslTests, should_use_transition_dsl_with_guard_and_action)
{
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<S2> {}));
}