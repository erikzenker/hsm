#pragma once

#include "flatten_transition_table.h"
#include "remove_duplicates.h"
#include "traits.h"

#include <boost/hana/front.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto collect_parent_state_typeids = [](auto state) {
    auto toParentState = [](auto transition) { return bh::typeid_(bh::front(transition)); };

    auto transitions = flatten_transition_table(state);
    auto collectedParentStates = bh::transform(transitions, toParentState);
    return remove_duplicate_typeids(collectedParentStates);
};
}