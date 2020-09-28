#include "hsm/details/collect_initial_states.h"
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

struct S {
    static constexpr auto make_transition_table()
    {
        // clang-format off
            return hsm::transition_table(
                * hsm::state_t<S1> {} = hsm::state_t<S1> {},
                * hsm::state_t<S2> {} = hsm::state_t<S2> {}
            );
        // clang-format on
    }
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

    ASSERT_EQ(bh::size_c<1>, bh::size(initialStateMap));

    constexpr auto initialStates
        = bh::find(initialStateMap, bh::typeid_(hsm::state_t<S> {})).value();

    ASSERT_EQ(bh::size_c<2>, bh::size(initialStates));
    ASSERT_EQ(bh::size_c<0>, bh::at_c<0>(initialStates));
    ASSERT_EQ(bh::size_c<1>, bh::at_c<1>(initialStates));
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

    ASSERT_EQ(2, initialStateTable[0].size());

    ASSERT_EQ(0, initialStateTable[0][0]);
    ASSERT_EQ(1, initialStateTable[0][1]);
}