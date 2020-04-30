#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/make_states_map.h"
#include <iostream>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    auto statesMap = make_states_map(state<ComplexStateMachine>{});

    fill_dispatch_table_with_external_transitions(state<ComplexStateMachine> {}, statesMap, boost::hana::make_tuple());

    return 0;
}