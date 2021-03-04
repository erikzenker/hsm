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
constexpr auto collectEventTypeids = [](auto transition) { return transition.event().typeid_; };
constexpr auto collectEvents = [](auto transition) {
    using Event = typename decltype(transition.event().typeid_)::type;
    return bh::tuple_t<Event>;
};
}

template <class State> constexpr auto collect_event_typeids_recursive(State state)
{
    return remove_duplicates(bh::transform(
        bh::concat(flatten_transition_table(state), flatten_internal_transition_table(state)),
        collectEventTypeids));
}

template <class TransitionTuple>
constexpr auto collect_event_typeids_recursive_from_transitions(TransitionTuple transitions)
{
    return remove_duplicates(bh::transform(transitions, collectEventTypeids));
}

template <class State> constexpr auto collect_events_recursive(State state)
{
    return remove_duplicate_types(bh::flatten(bh::transform(
        bh::concat(flatten_transition_table(state), flatten_internal_transition_table(state)),
        collectEvents)));
}
}