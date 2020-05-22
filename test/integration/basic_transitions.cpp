#include "hsm/hsm.h"
#include "hsm/details/transition_table.h"

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
};
struct S2 {
};
struct S3 {
};
struct S4 {
};

// Events
struct e1 {
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

// Actions
const auto log = [](auto event, auto source, auto target) {
    std::cout << experimental::print(typeid_(source)) << " + "
              << experimental::print(typeid_(event)) << " = "
              << experimental::print(typeid_(target)) << std::endl;
};

struct SubSubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> {} + hsm::event<e1> {} / log = hsm::state<S2> {}
        );
        // clang-format on
    }
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> {}          + hsm::event<e1> {} / log = hsm::state<S2> {},
              hsm::state<S1> {}          + hsm::event<e5> {} / log = hsm::state<S3> {},
              hsm::state<S2> {}          + hsm::event<e1> {} / log = hsm::state<SubSubState> {},
              hsm::state<SubSubState> {} + hsm::event<e2> {} / log = hsm::state<S1> {}
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            * hsm::state<S1> {}       + hsm::event<e1> {} / log = hsm::state<S2> {},
              hsm::state<S1> {}       + hsm::event<e2> {} / log = hsm::state<S3> {},
              hsm::state<S1> {}       + hsm::event<e4> {} / log = hsm::state<SubState> {},
              hsm::state<S1> {}       + hsm::event<e5> {} / log = hsm::state<S3> {},
              hsm::state<S1> {}       + hsm::event<e6> {} / log = hsm::state<S1> {},
              hsm::state<S2> {}       + hsm::event<e1> {} / log = hsm::state<S1> {},
              hsm::state<S2> {}       + hsm::event<e2> {} / log = hsm::state<S1> {},
              hsm::state<S2> {}       + hsm::event<e3> {} / log = hsm::state<S3> {},
              hsm::state<SubState> {} + hsm::event<e2> {} / log = hsm::state<S1> {},
              hsm::state<SubState> {} + hsm::event<e4> {} / log = hsm::state<SubState> {}
        );
        // clang-format on
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

class BasicTransitionTests : public Test {
    protected:    
        hsm::sm<MainState> sm;
};

TEST_F(BasicTransitionTests, should_start_in_initial_state)
{
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}

TEST_F(BasicTransitionTests, should_start_in_root_state)
{
    ASSERT_TRUE(sm.parent_is(hsm::state<MainState> {}));
}

TEST_F(BasicTransitionTests, should_process_event)
{
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));

    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S2> {}));
}

TEST_F(BasicTransitionTests, should_transit_into_SubState)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
}

TEST_F(BasicTransitionTests, should_transit_into_SubSubState)
{
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});

    ASSERT_TRUE(sm.is(hsm::state<SubSubState> {}, hsm::state<S1> {}));
}

TEST_F(BasicTransitionTests, should_transit_in_SubState_with_unique_event)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e5 {});

    ASSERT_FALSE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S3> {}));
}

TEST_F(BasicTransitionTests, should_exit_substate_on_event_in_parentstate)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));

    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<MainState> {}, hsm::state<S1> {}));
}

TEST_F(BasicTransitionTests, should_exit_subsubstate_on_event_in_parentstate)
{
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubSubState> {}, hsm::state<S1> {}));

    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
}

TEST_F(BasicTransitionTests, should_reentry_substate_on_initial_state)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S2> {}));
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.parent_is(hsm::state<SubState> {}));
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
}

TEST_F(BasicTransitionTests, should_self_transit)
{
    ASSERT_TRUE(sm.is(hsm::state<MainState> {}, hsm::state<S1> {}));
    sm.process_event(e6 {});
    ASSERT_TRUE(sm.is(hsm::state<MainState> {}, hsm::state<S1> {}));
    sm.process_event(e6 {});
    ASSERT_TRUE(sm.is(hsm::state<MainState> {}, hsm::state<S1> {}));
}