#pragma once

#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/remove_duplicates.h"
#include "hsm/details/traits.h"

#include <boost/hana/front.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto collect_parent_state_typeids = [](auto state) {
    auto toParentStateTypeid = [](auto transition) { return bh::typeid_(transition.parent()); };

    auto transitions = flatten_transition_table(state);
    auto parentStateTypeids = bh::transform(transitions, toParentStateTypeid);
    return remove_duplicate_typeids(parentStateTypeids);
};
}