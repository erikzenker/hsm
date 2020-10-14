#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
    e3(int i)
        : i(i)
    {
    }
    int i;
};

// States
struct S1 {
    static constexpr auto defer_events()
    {
        return hsm::events<e2, e3>;
    }
};
struct S2 {
};

using namespace ::testing;
using namespace boost::hana;

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e3> = hsm::state<S2>
            , hsm::state<S2> + hsm::event<e2> = hsm::state<S1>
              
        );
        // clang-format on
    }
};

}

class DeferEventsTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(DeferEventsTests, should_defer_event)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
}

TEST_F(DeferEventsTests, should_defer_event_with_parameters)
{
    sm.process_event(e3 { 1 });
}