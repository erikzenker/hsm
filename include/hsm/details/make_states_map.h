#pragma once

#include "hsm/details/collect_states.h"
#include "hsm/details/to_pairs.h"

#include <boost/hana/append.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/zip.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto to_map = [](auto tupleOfPairs) {
    return bh::unpack(tupleOfPairs, [](auto... pairs) { return bh::make_map(pairs...); });
};

/***
 * Returns a map from typeid(state<State>) -> State
 * of all states found recursive under parentState
 *
 */
template <class State> constexpr auto make_states_map(State&& parentState)
{
    auto collectedStateTypeids = collect_state_typeids_recursive(parentState);
    auto collectedStatesPtr
        = bh::transform(collect_states_recursive(parentState), unwrap_typeid_to_shared_ptr);
    return to_map(to_pairs(bh::zip(collectedStateTypeids, collectedStatesPtr)));
}
}