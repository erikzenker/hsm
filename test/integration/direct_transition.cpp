#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

// States
struct S1 {
};
struct S2 {
};
struct S3 {
};
struct S4 {
};
struct S5 {
    constexpr auto on_entry()
    {
        return [](auto event) { event.called->set_value(); };
    }
    constexpr auto on_exit()
    {
        return [](auto event) { event.called->set_value(); };
    }
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};
struct e4 {
    e4()
        : called(std::make_shared<std::promise<void>>())
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e5 {
};
struct e6 {
    e6(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e7 {
};
struct e8 {
};
struct e9 {
};
struct e10 {
    e10(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e11 {
    e11(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e12 {
};
struct e13 {
};

// Guards
const auto g1 = [](auto) { return true; };
const auto g2 = [](auto) { return false; };
const auto g3 = [](auto) { return true; };

// Actions
const auto a1 = [](auto event) {};
const auto a2 = [](auto event) { event.called->set_value(); };

using namespace ::testing;
using namespace boost::hana;

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

struct SubState2 {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            hsm::transition(S1{}, hsm::event<e1> {}, g1, a1, SubState{}),
            hsm::transition(S1{}, hsm::event<e2> {}, g1, a1, SubState2{}),
            hsm::transition(S1{}, hsm::event<e3> {}, g1, a1, hsm::Direct{SubState{}, S1{}}),
            hsm::transition(hsm::Direct{SubState{}, S1{}}, hsm::event<e2> {}, g1, a1, hsm::Direct{SubState2{}, S1{}}));
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

class DirectTransitionTests : public Test {
    protected:    
        hsm::Sm<MainState> sm;
};

TEST_F(DirectTransitionTests, should_transit_directly_into_substate)
{
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(SubState {}, S1 {}));
}

TEST_F(DirectTransitionTests, should_transit_directly_between_substates)
{

    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubState {}, S1 {}));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(SubState2 {}, S1 {}));
}
