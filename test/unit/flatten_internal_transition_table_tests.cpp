#include "hsm/details/flatten_internal_transition_table.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>

using namespace ::testing;
using namespace hsm;

namespace {

struct PlainState { };

struct InternalOnly {
    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

struct X {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(hsm::transition(
            hsm::state_t<InternalOnly> {},
            "event",
            "guard",
            "action",
            hsm::state_t<InternalOnly> {}));
    }
};

struct Y {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(hsm::transition(
            hsm::state_t<PlainState> {}, "event", "guard", "action", hsm::state_t<PlainState> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

struct Z {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(hsm::transition(
            hsm::state_t<InternalOnly> {},
            "event",
            "guard",
            "action",
            hsm::state_t<InternalOnly> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

struct P {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(hsm::transition(
            hsm::state_t<InternalOnly> {}, "event", "guard", "action", hsm::state_t<Z> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition("event", "guard", "action"));
    }
};

class FlattenInternalTransitionTableTests : public Test {
};
}

TEST_F(FlattenInternalTransitionTableTests, should_flatten_root_internal_transition_table)
{
    auto flattenInternalTransitionTable = flatten_internal_transition_table(hsm::state_t<Y> {});
    ASSERT_EQ(boost::hana::size_c<1>, boost::hana::size(flattenInternalTransitionTable));

    ASSERT_TRUE(
        hsm::state_t<Y> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(
        hsm::state_t<PlainState> {}
        == boost::hana::at_c<0>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(
        hsm::state_t<PlainState> {}
        == boost::hana::at_c<0>(flattenInternalTransitionTable).target());
}

TEST_F(FlattenInternalTransitionTableTests, should_flatten_substate_internal_transition_table)
{
    auto flattenInternalTransitionTable = flatten_internal_transition_table(hsm::state_t<X> {});
    ASSERT_EQ(boost::hana::size_c<1>, boost::hana::size(flattenInternalTransitionTable));

    ASSERT_TRUE(
        hsm::state_t<X> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(
        hsm::state_t<InternalOnly> {}
        == boost::hana::at_c<0>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(
        hsm::state_t<InternalOnly> {}
        == boost::hana::at_c<0>(flattenInternalTransitionTable).target());
}

TEST_F(
    FlattenInternalTransitionTableTests, should_flatten_root_and_substate_internal_transition_table)
{
    auto flattenInternalTransitionTable = flatten_internal_transition_table(hsm::state_t<Z> {});
    ASSERT_EQ(boost::hana::size_c<1>, boost::hana::size(flattenInternalTransitionTable));

    ASSERT_TRUE(
        hsm::state_t<Z> {} == boost::hana::at_c<0>(flattenInternalTransitionTable).parent());
    ASSERT_TRUE(
        hsm::state_t<InternalOnly> {}
        == boost::hana::at_c<0>(flattenInternalTransitionTable).source());
    ASSERT_TRUE(
        hsm::state_t<InternalOnly> {}
        == boost::hana::at_c<0>(flattenInternalTransitionTable).target());
}

TEST_F(FlattenInternalTransitionTableTests, should_flaten_internal_transition_table)
{
    auto flattenInternalTransitionTable = flatten_internal_transition_table(hsm::state_t<P> {});

    ASSERT_EQ(boost::hana::size_c<3>, boost::hana::size(flattenInternalTransitionTable));
}