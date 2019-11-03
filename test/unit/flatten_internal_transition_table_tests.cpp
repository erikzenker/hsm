
#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;
using namespace hsm;

namespace {

struct T {
    auto make_internal_transition_table()
    {
        return boost::hana::make_tuple(boost::hana::make_tuple("event", "guard", "action"));
    }
};

struct P {
    auto make_transition_table()
    {
        return boost::hana::make_tuple(
            boost::hana::make_tuple(T {}, "event", "guard", "action", T {}));
    }

    auto make_internal_transition_table()
    {
        return boost::hana::make_tuple(boost::hana::make_tuple("event", "guard", "action"));
    }
};

struct S {
    auto make_transition_table()
    {
        return boost::hana::make_tuple(
            boost::hana::make_tuple(T {}, "event", "guard", "action", P {}));
    }

    auto make_internal_transition_table()
    {
        return boost::hana::make_tuple(boost::hana::make_tuple("event", "guard", "action"));
    }
};

class FlattenInternalTransitionTableTests : public Test {
};
}

TEST_F(FlattenInternalTransitionTableTests, should_flatten_internal_transition_table)
{
    auto flattenInternalTransitionTable = flatten_internal_transition_table(S {});

    ASSERT_EQ(boost::hana::size_c<9>, boost::hana::size(flattenInternalTransitionTable));

    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<0>(flattenInternalTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<1>(flattenInternalTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<2>(flattenInternalTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<3>(flattenInternalTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<4>(flattenInternalTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<5>(flattenInternalTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<6>(flattenInternalTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<7>(flattenInternalTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, bh::size(boost::hana::at_c<8>(flattenInternalTransitionTable)));

    // T{} State
    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<0>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<0>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<0>(flattenInternalTransitionTable))));

    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<1>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<1>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<1>(flattenInternalTransitionTable))));

    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<2>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<2>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<2>(flattenInternalTransitionTable))));

    // P{} State
    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<3>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<3>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<3>(flattenInternalTransitionTable))));

    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<4>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<4>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<4>(flattenInternalTransitionTable))));

    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<5>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<5>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<5>(flattenInternalTransitionTable))));

    // S{} State
    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<6>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<6>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(T {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<6>(flattenInternalTransitionTable))));

    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<7>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<7>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<7>(flattenInternalTransitionTable))));

    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<0>(boost::hana::at_c<8>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<1>(boost::hana::at_c<8>(flattenInternalTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(
            boost::hana::at_c<5>(boost::hana::at_c<8>(flattenInternalTransitionTable))));
}