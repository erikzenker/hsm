#include <hsm/hsm.h>
#include <iostream>

#include "ComplexStateMachine.h"

#include <boost/hana/experimental/printable.hpp>

using namespace hsm;
using namespace boost::hana;

constexpr auto isZero = [](auto&& element) { return equal(element, int_c<0>); };

int main()
{
    auto tuple = to_tuple(make_range(int_c<0>, int_c<50>));

    for_each(tuple, [](auto&& element) {
        switch_(
            case_(
                equal(element, int_c<0>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(
                equal(element, int_c<1>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(otherwise(), [](auto) { std::cout << "otherwise" << std::endl; }))(element);

        switch_(
            case_(
                equal(element, int_c<2>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(
                equal(element, int_c<3>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(otherwise(), [](auto) { std::cout << "otherwise" << std::endl; }))(element);

        switch_(
            case_(
                equal(element, int_c<4>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(
                equal(element, int_c<5>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(
                equal(element, int_c<6>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(
                equal(element, int_c<7>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(otherwise(), [](auto) { std::cout << "otherwise" << std::endl; }))(element);

        switch_(
            case_(
                equal(element, int_c<8>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(
                equal(element, int_c<9>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(
                equal(element, int_c<10>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(
                equal(element, int_c<11>),
                [](auto&& element) { std::cout << experimental::print(element) << std::endl; }),
            case_(otherwise(), [](auto) { std::cout << "otherwise" << std::endl; }))(element);
    });
}