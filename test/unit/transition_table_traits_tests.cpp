#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/details/transition_table_traits.h"
#include "hsm/front/transition_dsl.h"

#include <gtest/gtest.h>

using namespace ::testing;

namespace hsm::test {

class TransitionTableTraitsTests : public Test {
};

struct S1 {
};
struct S2 {
};
struct S3 {
};
struct PS1 {
};
struct PS2 {
};
struct e1 {
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return transition_table(
            * state<S1> + event<e1>  = state<S2>
            , state<S2>              = state<S3>
            , * state<PS1> + event<e1> = state<PS2>
        );
        // clang-format on
    }
};

struct MainStateNoAnonymousTransition {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return transition_table(
            * state<S1> + event<e1>  = state<S2>
        );
        // clang-format on
    }
};

TEST_F(TransitionTableTraitsTests, should_recognize_anonymous_transitions)
{
    static_assert(has_anonymous_transition(state<MainState>));
}

TEST_F(TransitionTableTraitsTests, should_recognize_no_anonymous_transitions)
{
    static_assert(!has_anonymous_transition(state<MainStateNoAnonymousTransition>));
}
}