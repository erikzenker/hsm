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
    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

struct P {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(
            hsm::transition(hsm::state_t<T> {}, "event", "guard", "action", hsm::state_t<T> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

struct S {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(
            hsm::transition(hsm::state_t<T> {}, "event", "guard", "action", hsm::state_t<P> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

class FlattenInternalTransitionTableTests : public Test {
};
}

TEST_F(FlattenInternalTransitionTableTests, should_flatten_internal_transition_table)
{
    auto flattenInternalTransitionTable = flatten_internal_transition_table(hsm::state_t<S> {});

    ASSERT_EQ(boost::hana::size_c<3>, boost::hana::size(flattenInternalTransitionTable));

    // S{} State
    ASSERT_TRUE(
        hsm::state_t<P> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(
        hsm::state_t<T> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(
        hsm::state_t<T> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).target());

    ASSERT_TRUE(
        hsm::state_t<S> {} == boost::hana::at_c<1>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(
        hsm::state_t<T> {} == boost::hana::at_c<1>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(
        hsm::state_t<T> {} == boost::hana::at_c<1>(flattenInternalTransitionTable).target());

    // P{} State
    ASSERT_TRUE(
        hsm::state_t<S> {} == boost::hana::at_c<2>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(
        hsm::state_t<P> {} == boost::hana::at_c<2>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(
        hsm::state_t<P> {} == boost::hana::at_c<2>(flattenInternalTransitionTable).target());
}