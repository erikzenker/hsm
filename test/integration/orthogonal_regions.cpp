#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

namespace {

namespace bh {
using namespace boost::hana;
}

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
};
struct S6 {
};
struct S7 {
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

// Guards
const auto trueGuard = [](auto) { return true; };
const auto falseGuard = [](auto) { return false; };

// Actions
const auto a1 = [](auto event) {};

using namespace ::testing;

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(S1 {}, hsm::event<e1> {}, trueGuard, a1, S2 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(S1 {}, hsm::event<e1> {}, trueGuard , a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e2> {}, trueGuard , a1, S4 {}),
            // Guard tests
            hsm::transition(S1 {}, hsm::event<e3> {}, falseGuard, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e4> {}, trueGuard , a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e5> {}, falseGuard, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e6> {}, trueGuard , a1, S6 {}),
            hsm::transition(S6 {}, hsm::none {}     , falseGuard, a1, S1 {}),

            hsm::transition(S2 {}, hsm::event<e1> {}, trueGuard , a1, SubState {}),
            hsm::transition(S4 {}, hsm::none {}     , trueGuard , a1, S1 {}),
            // Region 1
            hsm::transition(S3 {}, hsm::event<e1> {}, trueGuard , a1, S4 {}),
            hsm::transition(S3 {}, hsm::event<e2> {}, trueGuard , a1, S5 {}),
            // Guard tests            
            hsm::transition(S3 {}, hsm::event<e3> {}, falseGuard, a1, S2 {}),
            hsm::transition(S3 {}, hsm::event<e4> {}, falseGuard, a1, S2 {}),
            hsm::transition(S3 {}, hsm::event<e5> {}, trueGuard , a1, S2 {}),
            hsm::transition(S3 {}, hsm::event<e6> {}, trueGuard , a1, S7 {}),
            hsm::transition(S7 {}, hsm::none {}     , trueGuard , a1, S1 {}),

            hsm::transition(S5 {}, hsm::none {}     , trueGuard , a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {}, S3 {});
    }
};

}

class OrthogonalRegionsTests : public Test {
    protected:    
        hsm::Sm<MainState> sm;
};

TEST_F(OrthogonalRegionsTests, should_start_in_region_initial_states)
{
    ASSERT_TRUE(sm.is(0, MainState{}, S1 {}));
    ASSERT_TRUE(sm.is(1, MainState{}, S3 {}));
}

TEST_F(OrthogonalRegionsTests, should_transit_in_all_regions)
{
    sm.process_event(e1{});
    ASSERT_TRUE(sm.is(0, MainState{}, S2 {}));
    ASSERT_TRUE(sm.is(1, MainState{}, S4 {}));
}

TEST_F(OrthogonalRegionsTests, should_anonymous_transit_in_all_regions)
{
    sm.process_event(e2{});
    ASSERT_TRUE(sm.is(0, MainState{}, S1 {}));
    ASSERT_TRUE(sm.is(1, MainState{}, S1 {}));
}

TEST_F(OrthogonalRegionsTests, should_guard_in_all_regions)
{
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(0, MainState {}, S1 {}));
    ASSERT_TRUE(sm.is(1, MainState {}, S3 {}));
}

TEST_F(OrthogonalRegionsTests, should_guard_in_a_single_regions)
{
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(0, MainState {}, S2 {}));
    ASSERT_TRUE(sm.is(1, MainState {}, S3 {}));
}

TEST_F(OrthogonalRegionsTests, should_guard_in_a_single_regions_inverse)
{
    sm.process_event(e5 {});
    ASSERT_TRUE(sm.is(0, MainState {}, S1 {}));
    ASSERT_TRUE(sm.is(1, MainState {}, S2 {}));
}

TEST_F(OrthogonalRegionsTests, should_anonymous_guard_in_a_single_regions)
{
    sm.process_event(e6 {});
    ASSERT_TRUE(sm.is(0, MainState {}, S6 {}));
    ASSERT_TRUE(sm.is(1, MainState {}, S1 {}));
}