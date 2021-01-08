#include "hsm/details/dispatch_table.h"
#include "hsm/details/has_action.h"
#include "hsm/details/pseudo_states.h"
#include "hsm/details/state.h"
#include "hsm/details/transition.h"

#include <boost/hana/basic_tuple.hpp>

#include <gtest/gtest.h>

using namespace boost::hana;
using namespace ::testing;

namespace hsm::test {

class HasActionTests : public Test {
};

struct T {
};
struct P {
};
struct S {
};

TEST_F(HasActionTests, should_recognize_action)
{
    auto action = []() {};
    constexpr auto transition
        = details::transition(state_t<T> {}, "event", "guard", action, state_t<P> {});
    constexpr auto extendedTransition
        = details::extended_transition(hsm::state_t<S> {}, transition);

    static_assert(has_action(extendedTransition));
}
}