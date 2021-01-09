#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>
#include <boost/hof.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace hsm::test {

// States
struct S1 {
};
struct S2 {
};

// Events
struct e1 {
    e1(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e2 {
};
struct e3 {
};
struct e4 {
};
struct e5 {
};
struct e6 {
};
struct e7 {
};
struct e8 {
};

template <class Event, class Source, class Target>
auto print_transition(Event event, Source source, Target target, std::string context)
{
    std::cout << boost::hana::experimental::print(boost::hana::typeid_(source)) << " + "
              << boost::hana::experimental::print(boost::hana::typeid_(event)) << " = "
              << boost::hana::experimental::print(boost::hana::typeid_(target)) << "(" << context
              << ")" << std::endl;
}

// Guards
const auto g2 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return false; };
const auto g3 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto a2 = [](auto event, auto /*source*/, auto /*target*/) { event.called->set_value(); };

struct Functor {
    template <class Event, class Source, class Target>
    auto operator()(Event event, Source source, Target target) const
    {
        print_transition(event, source, target, "Functor based action");
    }
};

template <class Event, class Source, class Target>
auto free_function(Event event, Source source, Target target)
{
    print_transition(event, source, target, "Function based action");
}

auto lambda = [](auto event, auto source, auto target) {
    print_transition(event, source, target, "Lambda based action");
};

using namespace ::testing;
using namespace boost::hana;

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> / a2 = hsm::state<S1>
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> + hsm::event<e1> /  a2 = hsm::state<S1>
            , hsm::state<S1> + hsm::event<e2>       = hsm::state<SubState>
            , hsm::state<S1> + hsm::event<e3>  [g2] = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e4>  [g3] = hsm::state<S2>
            // The following transitions show different possibilities to provide actions
            , hsm::state<S1> + hsm::event<e5> / Functor{} = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e6> / BOOST_HOF_LIFT(free_function) = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e7> / lambda  = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e8> / BOOST_HOF_LIFT(MainState::member_function) = hsm::state<S2>
        );
        // clang-format on
    }

  private:
    template <class Event, class Source, class Target>
    static constexpr auto member_function(Event event, Source source, Target target)
    {
        print_transition(event, source, target, "Member function based action");
    }
};

class GuardsActionsTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(GuardsActionsTests, should_call_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    sm.process_event(e1 { actionCalled });

    ASSERT_EQ(
        std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(GuardsActionsTests, should_call_substate_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    sm.process_event(e2 {});
    sm.process_event(e1 { actionCalled });

    ASSERT_EQ(
        std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(GuardsActionsTests, should_block_transition_guard)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
}

TEST_F(GuardsActionsTests, should_not_block_transition_by_guard)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}

TEST_F(GuardsActionsTests, should_use_functor)
{
    sm.process_event(e5 {});
}

TEST_F(GuardsActionsTests, should_use_free_function)
{
    sm.process_event(e6 {});
}

TEST_F(GuardsActionsTests, should_use_lambda_function)
{
    sm.process_event(e7 {});
}

TEST_F(GuardsActionsTests, should_use_member_functoin)
{
    sm.process_event(e8 {});
}
}