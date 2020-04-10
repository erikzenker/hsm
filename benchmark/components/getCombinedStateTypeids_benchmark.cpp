#include <iostream>
#include <hsm/hsm.h>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    auto a = getCombinedStateTypeids(hsm::state<ComplexStateMachine> {});
    auto b = getCombinedStateTypeids(hsm::state<ComplexStateMachine> {});

    std::cout << boost::hana::size(a) << boost::hana::size(b) << std::endl;

    return 0;
}