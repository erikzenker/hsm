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
const auto collectEvent = [](auto transition) { return bh::at_c<1>(transition).getEvent(); };
}

const auto collect_events_recursive = [](auto state) {
    auto collectedEvents = bh::transform(flatten_transition_table(state), collectEvent);
    return remove_duplicate_types(collectedEvents);
};

const auto collect_events_typeids_recursive
    = [](auto state) { return bh::transform(collect_events_recursive(state), bh::typeid_); };
}