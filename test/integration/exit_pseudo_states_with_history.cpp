#include "hsm/hsm.h"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace boost::hana;

namespace {

// States
// -- sub
struct S1 {
};
struct S2 {
};
struct SExit1 {
};
struct SExit2 {
};
// -- main
struct S3 {
};
struct S4 {
};
struct S5 {
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        using namespace hsm;

        // clang-format off
        return transition_table(
            * state<S1>                                                                           = state<SExit1>,
              state<S2>                  + event<e3>                                              = state<SExit2>
            );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        using namespace hsm;

        // clang-format off
        return transition_table(
            * state<S3>                                  + event<e1>                             = state<S4>,
              state<S4>                                  + event<e2>                             = state<SubState>,
              state<SubState>                            + event<e2>                             = history<SubState>,
              state<S5>                                                                          = state<S4>,
              hsm::exit<SubState, SExit1>                                                        = state<S5>,
              hsm::exit<SubState, SExit2>                                                        = state<S4>
            );
        // clang-format on
    }
};

}
class ExitPseudoStatesWithHistoryTests : public Test {
  protected:
     hsm::sm<MainState> sm;
};

TEST_F(ExitPseudoStatesWithHistoryTests, should_compile)
{
}
