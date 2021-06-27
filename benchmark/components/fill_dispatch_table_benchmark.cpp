#include <iostream>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    auto rootState = state<ComplexStateMachine>;
    auto statesMap = make_states_map(rootState);
    auto dispatchTables = make_dispatch_tables(rootState);
    auto optionalDependencies = boost::hana::make_tuple();

    fill_dispatch_table_with_external_transitions(
        rootState, dispatchTables, statesMap, optionalDependencies);

    return 0;
}