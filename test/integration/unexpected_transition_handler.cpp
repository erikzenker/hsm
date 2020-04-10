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

// Events
struct e1 {
};
struct e2 {
    e2(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }

    std::shared_ptr<std::promise<void>> called;
};

using namespace ::testing;
using namespace boost::hana;

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::state<S1> {} + hsm::event<e1> {} = hsm::state<S2> {},
            hsm::state<S2> {} + hsm::event<e2> {} = hsm::state<S1> {}
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }

    constexpr auto on_unexpected_event()
    {
        return [](auto event) { event.called->set_value(); };
    }
};

}

class NoTransitionHandlerTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(NoTransitionHandlerTests, should_call_no_transition_handler)
{
    auto called = std::make_shared<std::promise<void>>();
    sm.process_event(e2 { called });

    ASSERT_EQ(std::future_status::ready, called->get_future().wait_for(std::chrono::seconds(1)));
}