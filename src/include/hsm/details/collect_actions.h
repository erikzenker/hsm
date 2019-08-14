
#pragma once

#include "flatten_transition_table.h"
#include "remove_duplicates.h"
#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

namespace {
const auto collectAction = [](auto transition) { return bh::at_c<3>(transition); };
}

const auto collect_actions_recursive = [](auto state) {
    auto collectedActions = bh::transform(flatten_transition_table(state), collectAction);
    return remove_duplicate_types(collectedActions);
};

const auto collect_action_typeids_recursive
    = [](auto state) { return bh::transform(collect_actions_recursive(state), bh::typeid_); };

}