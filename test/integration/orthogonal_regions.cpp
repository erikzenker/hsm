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

// Events
struct e1 {
};
struct e2 {
};

// Guards
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto event) {};

using namespace ::testing;

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {})
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
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e2> {}, g1, a1, S4 {}),
            hsm::transition(S2 {}, hsm::event<e1> {}, g1, a1, SubState {}),
            hsm::transition(S4 {}, hsm::none {}, g1, a1, S1 {}),
            // Region 1
            hsm::transition(S3 {}, hsm::event<e1> {}, g1, a1, S4 {}),
            hsm::transition(S3 {}, hsm::event<e2> {}, g1, a1, S5 {}),
            hsm::transition(S5 {}, hsm::none {}, g1, a1, S1 {})
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

// TODO: move to unit tests
TEST_F(OrthogonalRegionsTests, should_count_max_regions)
{
    ASSERT_EQ(boost::hana::size_c<2>, hsm::maxRegions(MainState{}));
}

// TODO: move to unit tests
TEST_F(OrthogonalRegionsTests, should_make_region_map)
{
    auto map = hsm::region_map(MainState{});
    ASSERT_EQ(boost::hana::size_c<2>, boost::hana::size(boost::hana::find(map, boost::hana::typeid_(MainState{})).value()));
    ASSERT_EQ(boost::hana::size_c<1>, boost::hana::size(boost::hana::find(map, boost::hana::typeid_(SubState{})).value()));
}

// TODO: move to unit tests
TEST_F(OrthogonalRegionsTests, should_count_regions)
{
    std::array<std::vector<std::size_t>, hsm::maxRegions(MainState{})> regions;
    hsm::make_region_map(MainState{}, regions);
    ASSERT_EQ(2, regions[0].size());
    ASSERT_EQ(1, regions[1].size());

    ASSERT_EQ(0, regions[0][0]);
    ASSERT_EQ(4, regions[0][1]);

    ASSERT_EQ(0, regions[1][0]);
}

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