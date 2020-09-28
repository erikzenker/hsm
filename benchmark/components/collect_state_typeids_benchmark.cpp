#include <iostream>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    const auto a = collect_state_typeids_recursive(hsm::state_t<ComplexStateMachine> {});
    return boost::hana::size(a);
}