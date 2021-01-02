#pragma once

#include "hsm/details/flatten_transition_table.h"

#include <boost/hana/filter.hpp>
#include <boost/hana/size.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

const auto is_anonymous_transition
    = [](auto transition) { return bh::typeid_(transition.event()) == bh::typeid_(none {}); };

const auto is_history_transition
    = [](auto transition) { return is_history_state(transition.target()); };

template <class State> constexpr auto has_anonymous_transition(State rootState)
{
    auto transitions = flatten_transition_table(rootState);
    auto anonymousTransition = bh::filter(transitions, is_anonymous_transition);
    return bh::size(anonymousTransition);
}

template <class State> constexpr auto has_history(State rootState)
{
    auto transitions = flatten_transition_table(rootState);
    auto historyTransitions = bh::filter(transitions, is_history_transition);
    return bh::size(historyTransitions);
}
}