
#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;
using namespace hsm;

namespace {

  struct T {
  };

    struct P{
        auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(T {}, "event", "guard", "action", T {}));
        }

    };

    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(T {}, "event", "guard", "action", P {}));
        }
    };

class FlattenTransitionTableTests : public Test {
protected:
};
}

TEST_F(FlattenTransitionTableTests, should_flatten_with_parent_state)
{
    auto flattenTransitionTable = hsm::flatten_transition_table(S {});

    ASSERT_EQ(boost::hana::size_c<2>, boost::hana::size(flattenTransitionTable));
    ASSERT_EQ(
        boost::hana::size_c<6>, boost::hana::size(boost::hana::at_c<0>(flattenTransitionTable)));
    ASSERT_EQ(
        boost::hana::size_c<6>, boost::hana::size(boost::hana::at_c<1>(flattenTransitionTable)));
    ASSERT_EQ(
        boost::hana::typeid_(S {}),
        boost::hana::typeid_(boost::hana::front(boost::hana::at_c<0>(flattenTransitionTable))));
    ASSERT_EQ(
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(boost::hana::front(boost::hana::at_c<1>(flattenTransitionTable))));
}