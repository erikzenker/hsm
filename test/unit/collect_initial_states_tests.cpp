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
class E1 {
};
class S1 {
};
class S2 {
};
class S3 {
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
            return hsm::transition_table(
                * hsm::state_t<S3> {} = hsm::state_t<S1> {}
            );
        // clang-format on
    }
};

struct S {
    static constexpr auto make_transition_table()
    {
        // clang-format off
            return hsm::transition_table(
                 * hsm::state_t<S1> {} = hsm::state_t<S1> {}
                ,  hsm::state_t<S1> {} = hsm::history_t<SubState> {}
                ,* hsm::state_t<S2> {} = hsm::state_t<S2> {}
            );
        // clang-format on
    }
};

constexpr auto stateIdx = [](auto parentState, auto state) {
    constexpr auto rootState = hsm::state_t<S> {};
    constexpr auto combinedStateTypeids = hsm::getCombinedStateTypeids(rootState);
    return hsm::getCombinedStateIdx(combinedStateTypeids, parentState, state);
};

constexpr auto parentIdx = [](auto parent) {
    constexpr auto rootState = hsm::state_t<S> {};
    return hsm::getParentStateIdx(rootState, parent);
};
}

TEST_F(CollectInitialStatesTests, should_collect_initial_states)
{
    constexpr auto collectedInitialStates
        = bh::transform(hsm::collect_initial_states(hsm::state_t<S> {}), bh::typeid_);

    ASSERT_EQ(bh::size_c<2>, bh::size(collectedInitialStates));
    ASSERT_EQ(bh::typeid_(hsm::state_t<S1> {}), bh::at_c<0>(collectedInitialStates));
    ASSERT_EQ(bh::typeid_(hsm::state_t<S2> {}), bh::at_c<1>(collectedInitialStates));
}

TEST_F(CollectInitialStatesTests, should_make_initial_state_map)
{
    constexpr auto initialStateMap = hsm::make_initial_state_map(hsm::state_t<S> {});

    ASSERT_EQ(bh::size_c<2>, bh::size(initialStateMap));

    constexpr auto initialStatesS
        = bh::find(initialStateMap, bh::typeid_(hsm::state_t<S> {})).value();

    ASSERT_EQ(bh::size_c<2>, bh::size(initialStatesS));
    ASSERT_EQ(
        bh::size_c<stateIdx(hsm::state_t<S> {}, hsm::state_t<S1> {})>, bh::at_c<0>(initialStatesS));
    ASSERT_EQ(
        bh::size_c<stateIdx(hsm::state_t<S> {}, hsm::state_t<S2> {})>, bh::at_c<1>(initialStatesS));

    constexpr auto initialStatesSubState
        = bh::find(initialStateMap, bh::typeid_(hsm::state_t<SubState> {})).value();

    ASSERT_EQ(bh::size_c<1>, bh::size(initialStatesSubState));
    ASSERT_EQ(
        bh::size_c<stateIdx(hsm::state_t<SubState> {}, hsm::state_t<S3> {})>,
        bh::at_c<0>(initialStatesSubState));
}

TEST_F(CollectInitialStatesTests, should_count_max_initial_states)
{
    ASSERT_EQ(bh::size_c<2>, hsm::maxInitialStates(hsm::state_t<S> {}));
}

TEST_F(CollectInitialStatesTests, should_fill_initial_state_table)
{
    std::array<std::vector<std::size_t>, hsm::maxInitialStates(hsm::state_t<S> {})>
        initialStateTable;

    hsm::fill_initial_state_table(hsm::state_t<S> {}, initialStateTable);
    ASSERT_EQ(2, initialStateTable.size());

    ASSERT_EQ(bh::size_c<2>, bh::size(hsm::collect_parent_state_typeids(hsm::state_t<S> {})));
    ASSERT_EQ(bh::size_c<0>, parentIdx(hsm::state_t<S> {}));
    ASSERT_EQ(bh::size_c<1>, parentIdx(hsm::state_t<SubState> {}));
    ASSERT_EQ(2, initialStateTable.at(parentIdx(hsm::state_t<S> {})).size());
    ASSERT_EQ(1, initialStateTable.at(parentIdx(hsm::state_t<SubState> {})).size());

    ASSERT_EQ(
        stateIdx(hsm::state_t<S> {}, hsm::state_t<S1> {}),
        initialStateTable.at(parentIdx(hsm::state_t<S> {})).at(0));
    ASSERT_EQ(
        stateIdx(hsm::state_t<S> {}, hsm::state_t<S2> {}),
        initialStateTable.at(parentIdx(hsm::state_t<S> {})).at(1));

    ASSERT_EQ(
        stateIdx(hsm::state_t<SubState> {}, hsm::state_t<S3> {}),
        initialStateTable.at(parentIdx(hsm::state_t<SubState> {})).at(0));
}