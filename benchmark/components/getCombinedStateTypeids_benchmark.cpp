#include <iostream>
#include <hsm/hsm.h>

#include "ComplexStateMachine.h"

using namespace hsm;

int main()
{
    auto a = getCombinedStateTypeids(ComplexStateMachine{});
    auto b = getCombinedStateTypeids(ComplexStateMachine{});

    std::cout << boost::hana::size(a) << boost::hana::size(b) << std::endl;

    return 0;
}