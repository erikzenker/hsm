#include "hsm/details/collect_initial_states.h"
#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/idx.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_dsl.h"

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>
#include <gtest/gtest.h>

#include <array>

using namespace ::testing;
namespace bh = boost::hana;

class CollectInitialStatesTests : public Test {
};

namespace {
struct E1 {
};
struct Initial1 {
};
struct S1 {
};
struct S2 {
};
struct S3 {
};

struct SubState1 {
    static constexpr auto make_transition_table()
    {
        // clang-format off
            return hsm::transition_table(
                * hsm::state_t<S3> {} = hsm::state_t<Initial1> {}
            );
        // clang-format on
    }
};

struct Root1 {
    static constexpr auto make_transition_table()
    {
        // clang-format off
            return hsm::transition_table(
                 * hsm::state_t<Initial1> {} = hsm::state_t<Initial1> {}
                ,  hsm::state_t<Initial1> {} = hsm::history_t<SubState1> {}
                ,* hsm::state_t<S2> {} = hsm::state_t<S2> {}
            );
        // clang-format on
    }
};

struct Initial2 {
    static constexpr auto make_transition_table()
    {
        // clang-format off
            return hsm::transition_table(
                * hsm::state_t<S3> {} = hsm::state_t<Initial1> {}
            );
        // clang-format on
    }
    static constexpr auto make_internal_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            + (hsm::event<E1>)
        );
        // clang-format on
    }
};

struct Initial3 {
};

struct Root2 {
    static constexpr auto make_transition_table()
    {
        // clang-format off
            return hsm::transition_table(
                 * hsm::state_t<Initial2> {} = hsm::state_t<S1> {}
                 , hsm::state_t<Initial3> {} = hsm::state_t<S2> {}
            );
        // clang-format on
    }
};

constexpr auto stateIdx = [](auto parentState, auto state) {
    constexpr auto rootState = hsm::state_t<Root1> {};
    constexpr auto combinedStateTypeids = hsm::getCombinedStateTypeids(rootState);
    return hsm::getCombinedStateIdx(combinedStateTypeids, parentState, state);
};

constexpr auto parentIdx = [](auto parent) {
    constexpr auto rootState = hsm::state_t<Root1> {};
    return hsm::getParentStateIdx(rootState, parent);
};
}

TEST_F(CollectInitialStatesTests, should_collect_initial_states)
{
    constexpr auto collectedInitialStates
        = bh::transform(hsm::collect_initial_states(hsm::state_t<Root1> {}), bh::typeid_);

    ASSERT_EQ(bh::size_c<2>, bh::size(collectedInitialStates));
    ASSERT_EQ(bh::typeid_(hsm::state_t<Initial1> {}), bh::at_c<0>(collectedInitialStates));
    ASSERT_EQ(bh::typeid_(hsm::state_t<S2> {}), bh::at_c<1>(collectedInitialStates));
}

TEST_F(CollectInitialStatesTests, should_make_initial_state_map)
{
    constexpr auto initialStateMap = hsm::make_initial_state_map(hsm::state_t<Root1> {});

    ASSERT_EQ(bh::size_c<2>, bh::size(initialStateMap));

    constexpr auto initialStatesS
        = bh::find(initialStateMap, bh::typeid_(hsm::state_t<Root1> {})).value();

    ASSERT_EQ(bh::size_c<2>, bh::size(initialStatesS));
    ASSERT_EQ(
        bh::size_c<stateIdx(hsm::state_t<Root1> {}, hsm::state_t<Initial1> {})>,
        bh::at_c<0>(initialStatesS));
    ASSERT_EQ(
        bh::size_c<stateIdx(hsm::state_t<Root1> {}, hsm::state_t<S2> {})>,
        bh::at_c<1>(initialStatesS));

    constexpr auto initialStatesSubState
        = bh::find(initialStateMap, bh::typeid_(hsm::state_t<SubState1> {})).value();

    ASSERT_EQ(bh::size_c<1>, bh::size(initialStatesSubState));
    ASSERT_EQ(
        bh::size_c<stateIdx(hsm::state_t<SubState1> {}, hsm::state_t<S3> {})>,
        bh::at_c<0>(initialStatesSubState));
}

TEST_F(CollectInitialStatesTests, should_count_max_initial_states)
{
    ASSERT_EQ(bh::size_c<2>, hsm::maxInitialStates(hsm::state_t<Root1> {}));
}

TEST_F(CollectInitialStatesTests, should_fill_initial_state_table)
{
    std::array<std::vector<std::size_t>, hsm::maxInitialStates(hsm::state_t<Root1> {})>
        initialStateTable;

    hsm::fill_initial_state_table(hsm::state_t<Root1> {}, initialStateTable);
    ASSERT_EQ(2, initialStateTable.size());

    ASSERT_EQ(bh::size_c<2>, bh::size(hsm::collect_parent_state_typeids(hsm::state_t<Root1> {})));
    ASSERT_EQ(bh::size_c<0>, parentIdx(hsm::state_t<Root1> {}));
    ASSERT_EQ(bh::size_c<1>, parentIdx(hsm::state_t<SubState1> {}));
    ASSERT_EQ(2, initialStateTable.at(parentIdx(hsm::state_t<Root1> {})).size());
    ASSERT_EQ(1, initialStateTable.at(parentIdx(hsm::state_t<SubState1> {})).size());

    ASSERT_EQ(
        stateIdx(hsm::state_t<Root1> {}, hsm::state_t<Initial1> {}),
        initialStateTable.at(parentIdx(hsm::state_t<Root1> {})).at(0));
    ASSERT_EQ(
        stateIdx(hsm::state_t<Root1> {}, hsm::state_t<S2> {}),
        initialStateTable.at(parentIdx(hsm::state_t<Root1> {})).at(1));

    ASSERT_EQ(
        stateIdx(hsm::state_t<SubState1> {}, hsm::state_t<S3> {}),
        initialStateTable.at(parentIdx(hsm::state_t<SubState1> {})).at(0));
}

TEST_F(CollectInitialStatesTests, should_test_if_initial_states_have_transition_table)
{
    ASSERT_FALSE(hsm::has_transition_table(hsm::state_t<Initial1> {}));
    ASSERT_FALSE(hsm::has_transition_table(hsm::state_t<S3> {}));
    ASSERT_FALSE(hsm::at_least_one_initial_state_has_transition_table(hsm::state_t<Root1> {}));
    ASSERT_FALSE(
        hsm::at_least_one_initial_state_has_internal_transition_table(hsm::state_t<Root1> {}));

    ASSERT_TRUE(hsm::has_transition_table(hsm::state_t<Initial2> {}));
    ASSERT_FALSE(hsm::has_transition_table(hsm::state_t<Initial3> {}));
    ASSERT_TRUE(hsm::at_least_one_initial_state_has_transition_table(hsm::state_t<Root2> {}));
    ASSERT_TRUE(
        hsm::at_least_one_initial_state_has_internal_transition_table(hsm::state_t<Root2> {}));
}