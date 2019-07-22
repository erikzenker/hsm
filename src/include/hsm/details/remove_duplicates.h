#pragma once

#include <boost/hana.hpp>
#include <iostream>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

const auto remove_duplicate_typeids
    = [](auto tuple) { return bh::to<bh::tuple_tag>(bh::to<bh::set_tag>(tuple)); };


const auto remove_duplicate_types
    = [](auto tuple) { 
        auto result = bh::fold_left(tuple, bh::make_pair(bh::make_tuple(), bh::make_set()), [](auto accu, auto element){
            auto elements = bh::first(accu);
            auto typeids = bh::second(accu);
            return bh::if_(bh::contains(typeids, bh::typeid_(element)),
                [](auto elements, auto typeids, auto element){
                    return bh::make_pair(elements, typeids);
                },
                [](auto elements, auto typeids, auto element){
                    auto newElements = bh::append(elements, element);
                    auto newTypeids = bh::insert(typeids, bh::typeid_(element));
                    return bh::make_pair(newElements, newTypeids);
                })(elements, typeids, element);
        });

        return bh::first(result);
    };

}