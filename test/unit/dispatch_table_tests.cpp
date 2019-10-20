#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <future>
#include <memory>

using namespace ::testing;

namespace {
namespace bh = boost::hana;

class DispatchTableTests : public Test {
};

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

// Guards
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto event) {};

struct S {

    constexpr auto on_entry()
    {
        return [](auto counter) { (*counter)++; };
    }
    constexpr auto on_exit()
    {
        return [](auto counter) { (*counter)++; };
    }
};

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
            hsm::transition(S2 {}, hsm::event<e1> {}, g1, a1, SubState {}),
            // Region 1
            hsm::transition(S3 {}, hsm::event<e1> {}, g1, a1, S4 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {}, S3 {});
    }
};

}

TEST_F(DispatchTableTests, should_resolve_action)
{
    auto counter = std::make_shared<int>(0);

    auto action = [](auto counter) { (*counter)++; };

    auto transition = bh::make_tuple(0, S {}, 2, 3, action, S {});

    auto resolvedAction = hsm::resolveAction(transition);

    resolvedAction(counter);

    ASSERT_EQ(3, *counter);
}

TEST_F(DispatchTableTests, should_count_max_regions)
{
    ASSERT_EQ(bh::size_c<2>, hsm::maxRegions(MainState{}));
}

TEST_F(DispatchTableTests, should_make_region_map)
{
    auto map = hsm::region_map(MainState{});
    ASSERT_EQ(bh::size_c<2>, bh::size(bh::find(map, bh::typeid_(MainState{})).value()));
    ASSERT_EQ(bh::size_c<1>, bh::size(bh::find(map, bh::typeid_(SubState{})).value()));
}

TEST_F(DispatchTableTests, should_count_regions)
{
    std::array<std::vector<std::size_t>, hsm::maxRegions(MainState{})> regions;
    hsm::make_region_map(MainState{}, regions);
    ASSERT_EQ(2, regions[0].size());
    ASSERT_EQ(1, regions[1].size());

    ASSERT_EQ(0, regions[0][0]);
    ASSERT_EQ(3, regions[0][1]);

    ASSERT_EQ(0, regions[1][0]);
}
