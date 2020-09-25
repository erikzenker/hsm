#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

using namespace ::testing;
using namespace boost::hana;

// States
struct S1 {
    std::string name = "S1";
};
struct S2 {
    S2(std::string name)
        : name(name)
    {
    }
    const std::string name;
};

// Events
struct e1 {
    std::string name = "e1";
};
struct e2 {
};
struct e3 {
};
struct e4 {
    std::string name = "e4";
};

/*
 * A state factory is functor that constructs a non default constructible target state
 *
 * @param[in] event event of the transition
 * @param[in] source source state of the transition
 *
 * @return unique_ptr of the target state
 */
const auto stateFactory
    = [](auto event, auto source) { return std::make_unique<S2>(event.name + source.name); };

const auto alternativeStateFactory = [](auto event, auto source) {
    return std::make_unique<S2>(std::string { "static name" } + event.name + source.name);
};

const auto log = [](auto event, auto source, auto target) {
    std::cout << experimental::print(typeid_(source)) << ".name=" << source.name << " + "
              << experimental::print(typeid_(event)) << " = "
              << experimental::print(typeid_(target)) << ".name=" << target.name << std::endl;
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Transitions with a non default constructible target need to create the state with
            // the create_state action and provide a state factory functor
            * hsm::state<S1> + hsm::event<e1> / hsm::create_state(stateFactory)            = hsm::state<S2>
            , hsm::state<S1> + hsm::event<e4> / hsm::create_state(alternativeStateFactory) = hsm::state<S2> 
            // If no create_state action is used the target state should already be created
            , hsm::state<S1> + hsm::event<e3>                                              = hsm::state<S2>
            // If you want add an action to the transition you need to encapsulate the action
            // into the reuse_state action
            , hsm::state<S1> + hsm::event<e2> / hsm::reuse_state(log)                      = hsm::state<S2>
            , hsm::state<S2> + hsm::event<e2> / log                                        = hsm::state<S1>
        );
        // clang-format on
    }

    static constexpr auto on_unexpected_event()
    {
        return [](auto event) {
            throw std::runtime_error(
                std::string("unexpected event ") + experimental::print(typeid_(event)));
        };
    }
};
}

class CustomTargetsTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(CustomTargetsTests, should_construct_state_on_transition)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}

TEST_F(CustomTargetsTests, should_construct_state_on_transition_with_alternative_state_factory)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}

TEST_F(CustomTargetsTests, should_reuse_constructed_state)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e1 {});
    sm.process_event(e2 {});
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}