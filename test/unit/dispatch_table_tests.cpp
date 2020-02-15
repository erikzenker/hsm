#include "hsm/details/dispatch_table.h"
#include "hsm/details/transition_table.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <array>
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

struct Defer {
    constexpr auto defer_events()
    {
        return hsm::defer(e1{});
    }    
};

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

TEST_F(DispatchTableTests, should_count_max_regions)
{
    ASSERT_EQ(bh::size_c<2>, hsm::maxInitialStates(MainState{}));
}

TEST_F(DispatchTableTests, should_make_region_map)
{
    auto map = hsm::make_initial_state_map(MainState{});
    ASSERT_EQ(bh::size_c<2>, bh::size(bh::find(map, bh::typeid_(MainState{})).value()));
    ASSERT_EQ(bh::size_c<1>, bh::size(bh::find(map, bh::typeid_(SubState{})).value()));
}

TEST_F(DispatchTableTests, should_count_regions)
{
    std::array<std::vector<std::size_t>, hsm::maxInitialStates(MainState {})> regions;
    hsm::fill_inital_state_table(MainState{}, regions);
    ASSERT_EQ(2, regions[0].size());
    ASSERT_EQ(1, regions[1].size());

    ASSERT_EQ(3, regions[0][0]);
    ASSERT_EQ(1, regions[0][1]);

    ASSERT_EQ(3, regions[1][0]);
}

TEST_F(DispatchTableTests, should_resolve_history_state)
{
    auto historyTransition = bh::make_tuple(0, 1, 2, 3, 4, hsm::History { S {} });
    auto noHistoryTransition = bh::make_tuple(0, 1, 2, 3, 4, S {});

    ASSERT_TRUE(hsm::resolveHistory(historyTransition));
    ASSERT_FALSE(hsm::resolveHistory(noHistoryTransition));
}