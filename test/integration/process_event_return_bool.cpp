#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

using namespace ::testing;
using namespace boost::hana;

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};
struct e4 {
};

// States
struct S1 {
};
struct S2 {
    static constexpr auto defer_events()
    {
        return hsm::events<e4>;
    }
};

// Actions
const auto log = [](auto event, auto source, auto target) {
    std::cout << experimental::print(typeid_(source)) << " + "
              << experimental::print(typeid_(event)) << " = "
              << experimental::print(typeid_(target)) << std::endl;
};

const auto fail = [](auto /*event*/, auto /*source*/, auto /*target*/) { return false; };

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            * hsm::state<S1>       + hsm::event<e1> / log = hsm::state<S2>
            , hsm::state<S2>       + hsm::event<e2> / log = hsm::state<S1>
            , hsm::state<S2>       + hsm::event<e4> / log = hsm::state<S1>
            , hsm::state<S1>       + hsm::event<e3> [fail] / log = hsm::state<S2>
        );
        // clang-format on
    }
};

}

class ProcessEventReturnBoolTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(ProcessEventReturnBoolTests, should_return_true_on_transition)
{
    ASSERT_TRUE(sm.process_event(e1 {}));
}

TEST_F(ProcessEventReturnBoolTests, should_return_false_on_no_transition)
{
    ASSERT_FALSE(sm.process_event(e2 {}));
}

TEST_F(ProcessEventReturnBoolTests, should_return_false_on_guarded_transition)
{
    ASSERT_FALSE(sm.process_event(e3 {}));
}

TEST_F(ProcessEventReturnBoolTests, should_return_false_on_defered_event)
{
    sm.process_event(e1 {});
    ASSERT_FALSE(sm.process_event(e4 {}));
}