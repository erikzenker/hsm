#include "hsm/hsm.h"
#include <boost/hana/experimental/printable.hpp>

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

// Events
struct e1 {
};
struct defered1 {
};
struct defered2 {
    defered2(int i)
        : i(i)
    {
    }
    int i;
};
struct e4 {
};

// States
struct S1 {
    static constexpr auto defer_events()
    {
        return hsm::events<defered1, defered2>;
    }
};
struct S2 {
};

struct PS1 {
};
struct PS2 {
};

using namespace ::testing;
using namespace boost::hana;

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>  + hsm::event<e1>       / hsm::log = hsm::state<S2>
            , hsm::state<S1>  + hsm::event<defered2> / hsm::log = hsm::state<S2>
            , hsm::state<S2>  + hsm::event<defered1> / hsm::log = hsm::state<S1>
            ,*hsm::state<PS1> + hsm::event<e4>       / hsm::log = hsm::state<PS2>              
        );
        // clang-format on
    }

    static constexpr auto on_unexpected_event()
    {
        return [](auto event, auto /*currentState*/) {
            std::cout << "unexpected event: "
                      << boost::hana::experimental::print(boost::hana::typeid_(event));
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
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(defered1 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
}

TEST_F(DeferEventsTests, should_defer_event_with_parameters)
{
    sm.process_event(defered2 { 1 });
}