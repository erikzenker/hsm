#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;
using namespace hsm;

class FlattenTransitionTableTests : public Test {
protected:
    struct P{
        auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(1, "event", "guard", "action", 1));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(0, "event", "guard", "action", P{}));
        }
    };
};

TEST_F(FlattenTransitionTableTests, should_make_action_map)
{
    auto flattenTransitionTable = hsm::flatten_transition_table(FlattenTransitionTableTests::S{});

    ASSERT_EQ(boost::hana::size_c<2>, boost::hana::size(flattenTransitionTable));
    ASSERT_EQ(0, boost::hana::front(boost::hana::at_c<0>(flattenTransitionTable)));
    ASSERT_EQ(1, boost::hana::front(boost::hana::at_c<1>(flattenTransitionTable)));
}