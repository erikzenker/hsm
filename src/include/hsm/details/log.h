#pragma once

#include <boost/hana/experimental/printable.hpp>
#include <iostream>

namespace hsm {

constexpr auto log = [](auto event, auto source, auto target) {
    std::cout << boost::hana::experimental::print(boost::hana::typeid_(source)) << " + "
              << boost::hana::experimental::print(boost::hana::typeid_(event)) << " = "
              << boost::hana::experimental::print(boost::hana::typeid_(target)) << std::endl;
};
}
