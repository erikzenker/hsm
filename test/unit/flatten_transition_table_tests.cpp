#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <gtest/gtest.h>

#include <boost/hana/at.hpp>
#include <boost/hana/experimental/printable.hpp>
#include <boost/hana/front.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

#include <array>

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
    std::string data = "42";
};

    struct P{
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(hsm::transition(
                state_t<T> {}, hsm::event_t<event> {}, guard {}, action {}, state_t<T> {}));
        }

        // Non constexpr data member
        std::string data = "42";
    };

    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(hsm::transition(
                state_t<T> {}, hsm::event_t<event> {}, guard {}, action {}, state_t<P> {}));
        }

        // Non constexpr data member
        std::string data = "42";
    };

class FlattenTransitionTableTests : public Test {
};
}

TEST_F(FlattenTransitionTableTests, should_flatten_with_parent_state)
{
    constexpr auto transitions = flatten_transition_table(state_t<S> {});

    ASSERT_EQ(size_c<2>, size(transitions));
}