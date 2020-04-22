#include "hsm/hsm.h"

#include <boost/hana/experimental/printable.hpp>
#include <gtest/gtest.h>

using namespace ::testing;
using namespace boost::hana;

namespace {

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

// Guards
const auto g1 = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
const auto a1 = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

const auto log = [](auto event, auto source, auto target) {
    std::cout << experimental::print(typeid_(source)) << " + "
              << experimental::print(typeid_(event)) << " = "
              << experimental::print(typeid_(target)) << std::endl;
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::state<S1> {}, hsm::event<e1> {}, g1, a1, hsm::state<S2> {}),
            hsm::transition(hsm::state<S1> {}, hsm::event<e2> {}, g1, a1, hsm::state<S3> {})
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::state<S1> {}         , hsm::event<e1> {}, g1, log, hsm::entry<SubState, S3> {}),
            hsm::transition(hsm::state<S1> {}         , hsm::event<e2> {}, g1, a1, hsm::state<SubState> {}),                        
            hsm::transition(hsm::exit<SubState, S2> {}, hsm::none {}     , g1, a1, hsm::state<S1> {})
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};

}

class EntryExitPseudoStatesTests : public Test {
    protected:    
        hsm::sm<MainState> sm;
};

TEST_F(EntryExitPseudoStatesTests, should_entry_substate_on_pseudo_entry)
{
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.parent_is(hsm::state<SubState> {}));
    ASSERT_TRUE(sm.is(hsm::state<S3> {}));
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S3> {}));
}

TEST_F(EntryExitPseudoStatesTests, should_exit_subsubstate_from_pseudo_exit)
{
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState> {}, hsm::state<S1> {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));
}