#include <hsm/hsm.h>
#include <iostream>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    const auto a = collect_state_typeids_recursive(ComplexStateMachine {});
    return boost::hana::size(a);
}