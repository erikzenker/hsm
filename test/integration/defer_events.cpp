#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

// Events
struct e1 {
    static const int name = 1;
};
struct e2 {
    static const int name = 2;
};

// States
struct S1 {
    constexpr auto defer_events()
    {
        return hsm::defer(e2 {});
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
            hsm::state<S1> {} + hsm::event<e1> {} = hsm::state<S2> {},
            hsm::state<S2> {} + hsm::event<e2> {} = hsm::state<S1> {}
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }

    constexpr auto on_unexpected_event()
    {
        return [](auto event) {
            throw std::runtime_error(
                std::string("unexpected event ") + experimental::print(typeid_(event)));
        };
    }
};

}

class DeferEventsTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(DeferEventsTests, should_defer_event)
{
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e2{});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
    sm.process_event(e1{});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}