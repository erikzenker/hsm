#pragma once

#include "flatten_transition_table.h"
#include "remove_duplicates.h"
#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

const auto collect_parent_states = [](auto state) {
    auto toParentState = [](auto transition) { return bh::typeid_(bh::front(transition)); };

    auto transitions = flatten_transition_table(state);
    auto collectedParentStates = bh::transform(transitions, toParentState);
    return remove_duplicate_typeids(collectedParentStates);
};
}