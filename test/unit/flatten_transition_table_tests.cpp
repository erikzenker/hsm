
#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition.h"

#include <gtest/gtest.h>

#include <boost/hana/at.hpp>
#include <boost/hana/front.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

using namespace ::testing;
using namespace hsm;
using namespace boost::hana;

namespace {

struct event {
};
struct guard {
};
struct action {
};

struct T {
    // Non constexpr data member
    // std::string data = "42";
};

    struct P{
        static constexpr auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(state<T> {}, event {}, guard {}, action {}, state<T> {}));
        }

        // Non constexpr data member
        // std::string data = "42";
    };

    struct S {
        static constexpr auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(state<T> {}, event {}, guard {}, action {}, state<P> {}));
        }

        // Non constexpr data member
        // std::string data = "42";
    };

class FlattenTransitionTableTests : public Test {
};
}

TEST_F(FlattenTransitionTableTests, should_flatten_with_parent_state)
{
    constexpr auto flattenTransitionTable = flatten_transition_table(state<S> {});

    ASSERT_EQ(size_c<2>, size(flattenTransitionTable));
    ASSERT_EQ(size_c<6>, size(at_c<0>(flattenTransitionTable)));
    ASSERT_EQ(size_c<6>, size(at_c<1>(flattenTransitionTable)));
    // ASSERT_TRUE(state<S> {} == front(at_c<0>(flattenTransitionTable)));
    // ASSERT_TRUE(state<P> {} == front(at_c<1>(flattenTransitionTable)));
}