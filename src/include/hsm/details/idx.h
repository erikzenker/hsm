#pragma once

#include "hsm/details/collect_guards.h"
#include "hsm/details/collect_parent_states.h"
#include "hsm/details/collect_states.h"
#include "hsm/details/event.h"
#include "hsm/details/index_map.h"

#include <boost/hana/at.hpp>
#include <boost/hana/cartesian_product.hpp>
#include <boost/hana/find.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/type.hpp>

#include <cstdint>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

using Idx = std::uint64_t;
using StateIdx = Idx;
using EventIdx = Idx;
using ActionIdx = Idx;
using GuardIdx = Idx;

constexpr auto getIdx = [](auto map, auto type) -> Idx { return bh::find(map, type).value(); };

constexpr auto getParentStateIdx = [](auto rootState, auto parentState) {
    return index_of(collect_parent_state_typeids(rootState), bh::typeid_(parentState));
};

constexpr auto getStateIdx = [](auto rootState, auto state) {
    return index_of(collect_state_typeids_recursive(rootState), bh::typeid_(state));
};

template <class State> constexpr auto getCombinedStateTypeids(const State& rootState)
{
    auto parentStateTypeids = collect_parent_state_typeids(rootState);
    auto stateTypeids = collect_state_typeids_recursive(rootState);
    auto stateCartesianProduct
        = bh::cartesian_product(bh::make_basic_tuple(parentStateTypeids, stateTypeids));
    return bh::transform(stateCartesianProduct, bh::typeid_);
}

template <class ParentState, class State>
constexpr auto getCombinedStateTypeid(const ParentState& parentState, const State& state)
{
    return bh::typeid_(bh::make_basic_tuple(bh::typeid_(parentState), bh::typeid_(state)));
}

template <class StateTypeids, class ParentState, class State>
constexpr auto
getCombinedStateIdx(StateTypeids combinedStateTypids, ParentState parentState, State state)
{
    return index_of(combinedStateTypids, getCombinedStateTypeid(parentState, state));
}

constexpr auto calcCombinedStateIdx
    = [](std::size_t nStates, Idx parentStateIdx, Idx stateIdx) -> Idx {
    return (parentStateIdx * nStates) + stateIdx;
};

constexpr auto calcParentStateIdx
    = [](std::size_t nStates, Idx combinedState) -> Idx { return combinedState / nStates; };

constexpr auto calcStateIdx
    = [](std::size_t nStates, Idx combinedState) -> Idx { return combinedState % nStates; };

constexpr auto getEventIdx = [](auto rootState, auto event) {
    auto takeWrappedEvent = [](auto event) { return event.typeid_; };
    auto takeEvent = [](auto event) { return bh::typeid_(event); };
    auto eventId = bh::if_(is_event(event), takeWrappedEvent, takeEvent)(event);

    return index_of(collect_event_typeids_recursive(rootState), eventId);
};

constexpr auto getActionIdx = [](auto rootState, auto action) {
    return index_of(collect_action_typeids_recursive(rootState), bh::typeid_(action));
};

constexpr auto getGuardIdx = [](auto rootState, auto guard) {
    return index_of(collect_guard_typeids_recursive(rootState), bh::typeid_(guard));
};

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