#pragma once

#include "hsm/details/flatten_internal_transition_table.h"
#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/remove_duplicates.h"
#include "hsm/details/traits.h"

#include <boost/hana/at.hpp>
#include <boost/hana/concat.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/tuple.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
constexpr auto collectEventTypeids
    = [](auto transition) { return bh::at_c<2>(transition).typeid_; };
constexpr auto collectEvents = [](auto transition) {
    using Event = typename decltype(bh::at_c<2>(transition).typeid_)::type;
    return bh::tuple_t<Event>;
};
}

constexpr auto collect_event_typeids_recursive = [](auto state) {
    return remove_duplicate_typeids(bh::transform(
        bh::concat(flatten_transition_table(state), flatten_internal_transition_table(state)),
        collectEventTypeids));
};

constexpr auto collect_event_typeids_recursive_with_transitions = [](auto transitions) {
    return remove_duplicate_typeids(bh::transform(transitions, collectEventTypeids));
};

constexpr auto collect_events_recursive = [](auto state) {
    return remove_duplicate_types(bh::flatten(bh::transform(
        bh::concat(flatten_transition_table(state), flatten_internal_transition_table(state)),
        collectEvents)));
};
}