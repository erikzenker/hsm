#include "hsm/details/fill_dispatch_table.h"
#include "hsm/front/transition_dsl.h"

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
        return hsm::events(e1{});
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
            * hsm::state<S1> {} + hsm::event<e1> {} = hsm::state<S2> {}
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            * hsm::state<S1> {} + hsm::event<e1> {} = hsm::state<S2> {},
              hsm::state<S2> {} + hsm::event<e1> {} = hsm::state<SubState> {},
            // Region 1
            * hsm::state<S3> {} + hsm::event<e1> {} = hsm::state<S4> {}
        );
        // clang-format on
    }
};
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