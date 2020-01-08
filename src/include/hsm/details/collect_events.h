#pragma once

#include "flatten_transition_table.h"
#include "remove_duplicates.h"
#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
constexpr auto collectEventTypeids = [](auto transition) { return bh::at_c<2>(transition).typeid_; };
constexpr auto collectEvents = [](auto transition) {
    using Event = typename decltype(bh::at_c<2>(transition).typeid_)::type;
    return bh::tuple_t<Event>;
};
}

constexpr auto collect_event_typeids_recursive
    = [](auto state) { 
        return remove_duplicate_typeids(bh::transform(flatten_transition_table(state), collectEventTypeids));    
};

constexpr auto collect_events_recursive = [](auto state) {
    return remove_duplicate_types(
        bh::flatten(bh::transform(flatten_transition_table(state), collectEvents)));
};
}