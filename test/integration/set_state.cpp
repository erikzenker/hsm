#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>
#include <gtest/gtest.h>

namespace {

using namespace ::testing;
using namespace boost::hana;

// States
struct S1 {
};
struct S2 {
};
struct S3 {
};

// Events
struct e1 {
};
struct e2 {
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>           + hsm::event<e1> = hsm::state<S2>
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            * hsm::state<S1>       + hsm::event<e1> = hsm::state<S2>
            , hsm::state<S1>       + hsm::event<e1> = hsm::state<SubState>
        );
        // clang-format on
    }
};

}

class SetStateTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(SetStateTests, should_set_state)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.set(hsm::state<S2>);
    ASSERT_TRUE(sm.is(hsm::state<S2>));
}

TEST_F(SetStateTests, should_set_to_substate)
{
    ASSERT_TRUE(sm.is(hsm::state<S1>));
    sm.set(hsm::state<SubState>, hsm::state<S2>);
    ASSERT_TRUE(sm.is(hsm::state<SubState>, hsm::state<S2>));
}

TEST_F(SetStateTests, should_fail_to_set_invalid_region)
{
    ASSERT_THROW(sm.set(1, hsm::state<SubState>, hsm::state<S1>), std::out_of_range);
}

//
// Should fail to compile since state S3 is not part of the transition table
//
// TEST_F(SetStateTests, should_fail_to_compile)
// {
//     sm.set(hsm::state<S3>);
// }

//
// Should fail to compile since parent state S3 is not part of the transition table
//
// TEST_F(SetStateTests, should_fail_to_compile)
// {
//     sm.set(hsm::state<S3>, hsm::state<S1>);
// }