
#pragma once

#include "traits.h"

#include <boost/hana.hpp>

#include <iostream>

namespace hsm
{
    
namespace bh {
using namespace boost::hana;
};

/**
 * Calls the nth callable of a list of callables
 * 
 * index:       the index of the callabled to be called
 * callables:   a boost hana tuple of callables
 * args:        args for the nth callable
 */
template <class TCallables, class... TArgs>
auto call(std::size_t index, const TCallables& callables, TArgs&... args)
{
    // TODO: Is it possible to access the nth element of the callabes directly without
    // looping over them? Because by looping over them, every action call need to 
    // be compiled.
    bh::fold_left(callables, callables, [&callables, &index, &args...](const auto& remainingCallables, const auto& callable){
            auto currentIndex = bh::minus(bh::size(callables), bh::size(remainingCallables));

            if (index == currentIndex) {
                bh::if_(is_callable(callable, bh::make_tuple(args...)),
                    [](auto&  callable, TArgs&... args){callable(args...);},
                    [](auto&, TArgs&...){})(callable, args...);
            }   

            return bh::drop_front(remainingCallables);
        });
};

/**
 * Calls the nth callable of a list of callables return its bool result
 *
 * index:       the index of the callabled to be called
 * callables:   a boost hana tuple of callables
 * args:        args for the nth callable
 *
 */
template <class TCallables, class... TArgs>
bool call_guard(std::size_t index, const TCallables& callables, TArgs&... args)
{
    bool result = true;

    bh::fold_left(
        callables,
        callables,
        [&result, &callables, &index, &args...](
            const auto& remainingCallables, const auto& callable) {
            auto currentIndex = bh::minus(bh::size(callables), bh::size(remainingCallables));

            if (index == currentIndex) {
                bh::if_(
                    is_callable(callable, bh::make_tuple(args...)),
                    [&result](auto& callable, TArgs&... args) { result = callable(args...); },
                    [](auto&, TArgs&...) {})(callable, args...);
            }

            return bh::drop_front(remainingCallables);
        });
    return result;
};
}