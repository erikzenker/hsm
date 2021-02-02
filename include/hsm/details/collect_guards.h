
#pragma once

#include "hsm/details/remove_duplicates.h"
#include "hsm/details/traits.h"
#include "hsm/details/flatten_transition_table.h"

#include <boost/hana/at.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
const auto collectGuard = [](auto transition) { return bh::at_c<3>(transition); };
}

const auto collect_guards_recursive = [](auto state) {
    auto collectedGuards = bh::transform(flatten_transition_table(state), collectGuard);
    return remove_duplicate_types(collectedGuards);
};

const auto collect_guard_typeids_recursive = [](auto state) {
    return bh::transform(collect_guards_recursive(state), bh::typeid_);
};
}