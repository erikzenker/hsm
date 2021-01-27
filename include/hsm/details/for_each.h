#pragma once

#include <boost/hana/unpack.hpp>




namespace hsm {

namespace bh {
using namespace boost::hana;
}

    auto constexpr for_each = [](auto sequence, auto closure){
        bh::unpack(sequence, [closure](auto... elements){
            ((void)closure(elements),...);    
        });
    };
}