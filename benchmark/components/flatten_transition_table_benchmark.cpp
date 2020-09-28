#include "ComplexStateMachine.h"

#include <iostream>


int main()
{
    flatten_transition_table(hsm::state<ComplexStateMachine>);

    return 0;
}