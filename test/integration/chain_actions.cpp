#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <future>
#include <memory>

using namespace ::testing;

namespace {
struct S1 {
};
struct S2 {
};
struct e1 {
    std::shared_ptr<std::promise<void>> a1Called;
    std::shared_ptr<std::promise<void>> a2Called;
};

constexpr auto a1 = [](auto event, auto...) { event.a1Called->set_value(); };
constexpr auto a2 = [](auto event, auto...) { event.a2Called->set_value(); };

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> / hsm::chain_actions(a1, a2) = hsm::state<S2>
            , hsm::state<S2> + hsm::event<e1> / hsm::chain(a1, a2)         = hsm::state<S1>
        );
        // clang-format on
    }
};
}

class ComposeActionTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(ComposeActionTests, should_call_chained_actions)
{
    auto a1Called = std::make_shared<std::promise<void>>();
    auto a2Called = std::make_shared<std::promise<void>>();

    sm.process_event(e1 { a1Called, a2Called });

    ASSERT_TRUE(
        std::future_status::ready == a1Called->get_future().wait_for(std::chrono::seconds(1)));
    ASSERT_TRUE(
        std::future_status::ready == a2Called->get_future().wait_for(std::chrono::seconds(1)));
}
