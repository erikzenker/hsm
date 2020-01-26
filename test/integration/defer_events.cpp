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

// Guards
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto) {};

using namespace ::testing;
using namespace boost::hana;


struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S2 {}, hsm::event<e2> {}, g1, a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }

    constexpr auto on_unexpected_event()
    {
        return [](auto) {
            assert(false);
            std::cout << "UNEXPECTED" << std::endl;
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
    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e2{});
    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e1{});
    ASSERT_TRUE(sm.is(S1 {}));
}