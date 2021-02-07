#pragma once

#include <boost/hana/for_each.hpp>

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }

    inline auto for_each_idx = [](const auto& list, const auto& closure) {
        std::size_t index = 0;
        bh::for_each(list, [closure, &index](const auto& elem) {
            closure(elem, index);
            index++;
        });
    };
}