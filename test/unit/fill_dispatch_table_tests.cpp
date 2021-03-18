#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/make_dispatch_tables.h"
#include "hsm/details/make_states_map.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <array>
#include <future>
#include <memory>

using namespace ::testing;

namespace {
namespace bh = boost::hana;

class FillDispatchTableTests : public Test {
};

// States
struct S1 {
};
struct S2 {
};

// Events
struct e1 {
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::initial_t<S1>{}, hsm::event_t<e1>{}, hsm::noGuard {}, hsm::noAction {}, hsm::state_t<S2>{})
        );
        // clang-format on
    }
};
}

TEST_F(FillDispatchTableTests, should_will_dispatch_table_with_external_transitions)
{
    constexpr auto rootState = hsm::state_t<MainState> {};
    constexpr auto transitions = hsm::flatten_transition_table(rootState);
    auto statesMap = hsm::make_states_map(rootState);
    auto dispatchTables = hsm::make_dispatch_tables(rootState);

    hsm::fill_dispatch_table_with_transitions(
        rootState, dispatchTables, statesMap, bh::make_basic_tuple(), transitions);
}