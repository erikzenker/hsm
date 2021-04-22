#include <iostream>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    auto rootState = state<ComplexStateMachine>;
    auto statesMap = make_states_map(rootState);
    auto unexpectedEventHandlerTables = make_unexpected_event_handler_tables(rootState);
    auto unexpectedEventHandler = [](auto&...) {};
    auto optionalDependency = boost::hana::make_tuple();

    fill_unexpected_event_handler_tables(
        state<ComplexStateMachine>,
        statesMap,
        unexpectedEventHandlerTables,
        unexpectedEventHandler,
        optionalDependency);

    return 0;
}