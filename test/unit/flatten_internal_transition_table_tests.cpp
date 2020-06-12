#include "hsm/details/flatten_internal_transition_table.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;
using namespace hsm;

namespace {

struct T {
    constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

struct P {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(
            hsm::transition(hsm::state<T> {}, "event", "guard", "action", hsm::state<T> {}));
    }

    constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

struct S {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(
            hsm::transition(hsm::state<T> {}, "event", "guard", "action", hsm::state<P> {}));
    }

    constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

class FlattenInternalTransitionTableTests : public Test {
};
}

TEST_F(FlattenInternalTransitionTableTests, should_flatten_internal_transition_table)
{
    auto flattenInternalTransitionTable = flatten_internal_transition_table(hsm::state<S> {});

    ASSERT_EQ(boost::hana::size_c<9>, boost::hana::size(flattenInternalTransitionTable));

    // T{} State
    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).target());

    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<1>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<1>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<1>(flattenInternalTransitionTable).target());
    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<2>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<2>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<2>(flattenInternalTransitionTable).target());

    // P{} State
    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<3>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<3>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<3>(flattenInternalTransitionTable).target());

    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<4>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<4>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<4>(flattenInternalTransitionTable).target());

    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<5>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<5>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<5>(flattenInternalTransitionTable).target());

    // S{} State
    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<6>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<6>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<T> {} == boost::hana::at_c<6>(flattenInternalTransitionTable).target());

    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<7>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<7>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<P> {} == boost::hana::at_c<7>(flattenInternalTransitionTable).target());

    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<8>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<8>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(hsm::state<S> {} == boost::hana::at_c<8>(flattenInternalTransitionTable).target());
}