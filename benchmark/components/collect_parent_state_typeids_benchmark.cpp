#include <hsm/hsm.h>
#include <iostream>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    const auto a = collect_parent_state_typeids(ComplexStateMachine {});
    return boost::hana::size(a);
}