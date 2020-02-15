#include <iostream>
#include <hsm/hsm.h>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    fill_dispatch_table_with_external_transitions(ComplexStateMachine{});

    return 0;
}