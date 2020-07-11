#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

// States
struct S1 {
};
struct S2 {
};
struct S3 {
};
struct S4 {
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};
struct e4 {
};
struct e5 {
};
struct e6 {
};
struct e7 {
};
struct e8 {
};
struct e9 {
};

// Guards
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto /*event*/) {};

using namespace ::testing;
using namespace boost::hana;

struct SubSubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(hsm::transition(*hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S1> {}));
        // clang-format on
    }
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<SubSubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e2> {}, g1, a1, hsm::state<SubSubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e3> {}, g1, a1, hsm::state<SubSubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e4> {}, g1, a1, hsm::state<SubSubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e5> {}, g1, a1, hsm::state<SubSubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e6> {}, g1, a1, hsm::state<SubSubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e7> {}, g1, a1, hsm::state<SubSubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e8> {}, g1, a1, hsm::state<SubSubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e9> {}, g1, a1, hsm::state<SubSubState> {}));
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            hsm::transition(*hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<SubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e2> {}, g1, a1, hsm::state<SubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e3> {}, g1, a1, hsm::state<SubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e4> {}, g1, a1, hsm::state<SubState> {}),            
            hsm::transition(hsm::state<S1> {}, hsm::event<e5> {}, g1, a1, hsm::state<SubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e6> {}, g1, a1, hsm::state<SubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e7> {}, g1, a1, hsm::state<SubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e8> {}, g1, a1, hsm::state<SubState> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e9> {}, g1, a1, hsm::state<SubState> {})                                                            
            );
        // clang-format on
    }
};

class HsmTests : public Test {
};

TEST_F(HsmTests, should_not_compile_with_a_lot_of_memory_consumption)
{
    hsm::collect_state_typeids_recursive(hsm::state<MainState> {});
}
