#include <iostream>

#include <boost/hana.hpp>

#include <hsm/details/switch.h>

#include "ComplexStateMachine.h"

using namespace hsm;

class SuperType {
};

template <class Specific> class SubType : SuperType {
};

auto constexpr isInt = [](auto type) { return bh::equal(bh::typeid_(type), bh::typeid_(int {})); };
auto constexpr isChar
    = [](auto type) { return bh::equal(bh::typeid_(type), bh::typeid_(char {})); };
auto constexpr isFloat
    = [](auto type) { return bh::equal(bh::typeid_(type), bh::typeid_(char {})); };
auto constexpr isSuperType = [](auto type) {
    return bh::equal(bh::bool_c<std::is_base_of<SuperType, decltype(type)>::value>, bh::true_c);
};

int main()
{

    auto tuple = bh::make_tuple(
        s1 {},
        s2 {},
        s3 {},
        s4 {},
        s5 {},
        s6 {},
        s7 {},
        s8 {},
        s9 {},
        s11 {},
        s12 {},
        s13 {},
        s14 {},
        s15 {},
        s16 {},
        s17 {},
        s18 {},
        s19 {},
        s21 {},
        s22 {},
        s23 {},
        s24 {},
        s25 {},
        s26 {},
        s27 {},
        s28 {},
        s29 {},
        s31 {},
        s32 {},
        s33 {},
        s34 {},
        s35 {},
        s36 {},
        s37 {},
        s38 {},
        s39 {},
        s41 {},
        s42 {},
        s43 {},
        s44 {},
        s45 {},
        s46 {},
        s47 {},
        s48 {},
        s49 {});

    bh::for_each(tuple, [](auto element) {
        switch_(
            case_(isInt, [](auto) { std::cout << "Int" << std::endl; }),
            case_(isChar, [](auto) { std::cout << "Char" << std::endl; }),
            case_(isFloat, [](auto) { std::cout << "Float" << std::endl; }),
            case_(isSuperType, [](auto) { std::cout << "isSuperType" << std::endl; }),
            case_(
                has_transition_table,
                [](auto) { std::cout << "has transition table" << std::endl; }),
            case_(
                has_unexpected_event_handler,
                [](auto) { std::cout << "unexpected_event" << std::endl; }),
            case_(
                has_deferred_events, [](auto) { std::cout << "has deferred events" << std::endl; }),
            case_(otherwise, [](auto) { std::cout << "Unknown type" << std::endl; }))(element);
    });

    return 0;
}