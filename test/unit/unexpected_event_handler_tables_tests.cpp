#include "hsm/details/fill_unexpected_event_handler_tables.h"
#include "hsm/details/make_states_map.h"
#include "hsm/details/make_unexpected_event_handler_tables.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

using namespace ::testing;

namespace hsm::tests {
namespace bh = boost::hana;

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
            hsm::transition(hsm::initial_t<S1>{}, hsm::event_t<e1>{}, hsm::noGuard {}, hsm::noAction {}, hsm::state_t<S2>{}),
            hsm::transition(hsm::initial_t<S2>{}, hsm::event_t<hsm::noneEvent>{}, hsm::noGuard {}, hsm::noAction {}, hsm::state_t<S2>{})
        );
        // clang-format on
    }
};

struct Dependency {
    std::size_t nTimesCalled = 0;
};

class UnexpectedEventHandlerTablesTests : public Test {
};

TEST(UnexpectedEventHandlerTablesTests, should_make_unexpected_event_handler_tables)
{
    auto unexpectedEventHandlerTables
        = hsm::make_unexpected_event_handler_tables(hsm::state_t<MainState> {});
    ASSERT_EQ(2u, bh::size(unexpectedEventHandlerTables));
    ASSERT_EQ(3u, unexpectedEventHandlerTables[bh::typeid_(e1 {})].size());
    ASSERT_EQ(3u, unexpectedEventHandlerTables[bh::typeid_(hsm::noneEvent {})].size());
}

TEST(UnexpectedEventHandlerTablesTests, should_fill_unexpected_event_handler_tables)
{
    auto constexpr unexpectedEventHandler =
        [](auto /*event*/, auto /*currentState*/, auto& dependency) { dependency.nTimesCalled++; };

    auto rootState = hsm::state_t<MainState> {};
    auto unexpectedEventHandlerTables = hsm::make_unexpected_event_handler_tables(rootState);
    auto statesMap = hsm::make_states_map(rootState);
    auto dependency = Dependency {};
    auto optionalDependency = bh::make_basic_tuple(std::ref(dependency));

    fill_unexpected_event_handler_tables(
        rootState,
        statesMap,
        unexpectedEventHandlerTables,
        unexpectedEventHandler,
        optionalDependency);

    auto event = e1 {};
    unexpectedEventHandlerTables[bh::typeid_(e1 {})][0]->executeHandler(event);

    ASSERT_EQ(1, dependency.nTimesCalled);
}
}