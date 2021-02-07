
#pragma once

#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/remove_duplicates.h"
#include "hsm/details/traits.h"

#include <boost/hana/at.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
const auto collectAction = [](auto transition) { return transition.action(); };
}

const auto collect_actions_recursive = [](auto state) {
    auto collectedActions = bh::transform(flatten_transition_table(state), collectAction);
    return remove_duplicate_types(collectedActions);
};

const auto collect_action_typeids_recursive
    = [](auto state) { return bh::transform(collect_actions_recursive(state), bh::typeid_); };

}