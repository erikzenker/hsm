#include <iostream>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    const auto a = collect_events_recursive(state_t<ComplexStateMachine> {});

    return boost::hana::size(a);
}