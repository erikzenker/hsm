#include "hsm/details/fill_dispatch_table.h"
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
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S2> {})
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S2> {}),
            hsm::transition(hsm::state<S2> {}, hsm::event<e1> {}, g1, a1, hsm::state<SubState> {}),
            // Region 1
            hsm::transition(hsm::state<S3> {}, hsm::event<e1> {}, g1, a1, hsm::state<S4> {})
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {}, hsm::state<S3> {});
    }
};

}

TEST_F(DispatchTableTests, should_count_max_regions)
{
    ASSERT_EQ(bh::size_c<2>, hsm::maxInitialStates(hsm::state<MainState> {}));
}

TEST_F(DispatchTableTests, should_make_region_map)
{
    auto map = hsm::make_initial_state_map(hsm::state<MainState> {});
    ASSERT_EQ(
        bh::size_c<2>, bh::size(bh::find(map, bh::typeid_(hsm::state<MainState> {})).value()));
    ASSERT_EQ(bh::size_c<1>, bh::size(bh::find(map, bh::typeid_(hsm::state<SubState> {})).value()));
}

TEST_F(DispatchTableTests, should_count_regions)
{
    std::array<std::vector<std::size_t>, hsm::maxInitialStates(hsm::state<MainState> {})> regions;
    hsm::fill_initial_state_table(hsm::state<MainState> {}, regions);
    ASSERT_EQ(2, regions[0].size());
    ASSERT_EQ(1, regions[1].size());

    ASSERT_EQ(0, regions[0][0]);
    ASSERT_EQ(3, regions[0][1]);

    ASSERT_EQ(0, regions[1][0]);
}

TEST(ResolveHistoryTests, should_resolve_history_state)
{
    constexpr auto historyTransition = bh::make_tuple(0, 1, 2, 3, 4, hsm::history<S> {});
    constexpr auto noHistoryTransition = bh::make_tuple(0, 1, 2, 3, 4, hsm::state<S> {});

    ASSERT_TRUE(hsm::resolveHistory(historyTransition));
    ASSERT_FALSE(hsm::resolveHistory(noHistoryTransition));
}

TEST(ResolveDestinationTests, should_resolve_destination)
{
    auto dst = hsm::state<S1> {};
    auto transition = bh::make_tuple(0, 1, 2, 3, 4, dst);
    ASSERT_TRUE(dst == hsm::resolveDst(transition));
}

TEST(ResolveDestinationTests, should_resolve_submachine_destination)
{
    auto transition = bh::make_tuple(0, 1, 2, 3, 4, hsm::state<SubState> {});
    ASSERT_TRUE(hsm::state<S1> {} == hsm::resolveDst(transition));
}

TEST(ResolveParentDestinationTests, should_resolve_destination_parent)
{
    auto dst = hsm::state<S1> {};
    auto srcParent = hsm::state<S2> {};
    auto transition = bh::make_tuple(srcParent, 1, 2, 3, 4, dst);
    ASSERT_TRUE(srcParent == hsm::resolveDstParent(transition));
}

TEST(ResolveParentDestinationTests, should_resolve_submachine_destination_parent)
{
    auto dst = hsm::state<SubState> {};
    auto transition = bh::make_tuple(0, 1, 2, 3, 4, dst);
    ASSERT_TRUE(dst == hsm::resolveDstParent(transition));
}

TEST(ResolveSourceTests, should_resolve_source)
{
    auto src = hsm::state<S1> {};
    auto transition = bh::make_tuple(0, src, 2, 3, 4, 5);
    ASSERT_TRUE(src == hsm::resolveSrc(transition));
}

TEST(ResolveSourceParentTests, should_resolve_source)
{
    auto srcParent = hsm::state<S2> {};
    auto transition = bh::make_tuple(srcParent, hsm::state<S1> {}, 2, 3, 4, 5);
    ASSERT_TRUE(srcParent == hsm::resolveSrcParent(transition));
}
