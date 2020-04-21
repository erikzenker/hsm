#include "hsm/details/fill_dispatch_table.h"
#include <iostream>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    fill_dispatch_table_with_external_transitions(
        state<ComplexStateMachine> {}, boost::hana::make_tuple());

    return 0;
}