#pragma once

#include "../front/transition.h"
#include "index_map.h"
#include "collect_states.h"
#include "collect_guards.h"
#include "collect_parent_states.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

using Idx = std::uint16_t;
using StateIdx = Idx;
using EventIdx = Idx;
using ActionIdx = Idx;
using GuardIdx = Idx;

// TODO: replace these free standing functions by type alias
template <typename... Args> constexpr auto transition_table(Args... args)
{
    return bh::make_tuple(args...);
}

template <typename... Args> constexpr auto row(Args... args)
{
    return bh::make_tuple(args...);
}

template <typename... Args> constexpr auto transition(Args... args)
{
    return bh::make_tuple(args...);
}

template <typename... Args> constexpr auto initial(Args... args)
{
    return bh::make_tuple(args...);
}

template <typename... Args> constexpr auto defer(Args... args)
{
    return bh::make_tuple(args...);
}

constexpr auto getIdx = [](auto map, auto type) -> Idx { return bh::find(map, type).value(); };

constexpr auto getSrcParent = [](auto transition) { return bh::at_c<0>(transition); };

constexpr auto getSrc = [](auto transition) { return bh::at_c<1>(transition); };

constexpr auto getEvent = [](auto transition) { return bh::at_c<2>(transition); };

constexpr auto getGuard = [](auto transition) { return bh::at_c<3>(transition); };

constexpr auto getAction = [](auto transition) { return bh::at_c<4>(transition); };

constexpr auto getDst = [](auto transition) { return bh::at_c<5>(transition); };

constexpr auto getParentStateIdx = [](auto rootState, auto parentState) {
    return getIdx(make_index_map(collect_parent_state_typeids(rootState)), bh::typeid_(parentState));
};

constexpr auto getStateIdx = [](auto rootState, auto state) {
    return getIdx(make_index_map(collect_state_typeids_recursive(rootState)), bh::typeid_(state));
};

constexpr auto getCombinedStateIdx = [](auto rootState, auto parentState, auto state) {
    auto stateTypeids = collect_parent_state_typeids(rootState);
    auto parentStateTypeids = collect_state_typeids_recursive(rootState);
    auto stateCartesianProduct = bh::cartesian_product(bh::make_tuple(stateTypeids, parentStateTypeids));
    auto combinedTypeids = bh::transform(stateCartesianProduct, bh::typeid_);
    auto combinedStateTypeid = bh::typeid_(bh::make_tuple(bh::typeid_(parentState), bh::typeid_(state)));

    return getIdx(make_index_map(combinedTypeids), combinedStateTypeid);
};

auto calcCombinedStateIdx = [](std::size_t nStates, Idx parentStateIdx, Idx stateIdx) -> Idx {
    return (parentStateIdx * nStates) + stateIdx;
};

auto calcParentStateIdx
    = [](std::size_t nStates, Idx combinedState) -> Idx { return combinedState / nStates; };

auto calcStateIdx
    = [](std::size_t nStates, Idx combinedState) -> Idx { return combinedState % nStates; };

constexpr auto getEventIdx = [](auto rootState, auto event) {
    auto takeWrappedEvent = [](auto event) { return event.typeid_; };
    auto takeEvent = [](auto event) { return bh::typeid_(event); };
    auto eventId = bh::if_(is_event(event), takeWrappedEvent, takeEvent)(event);

    return getIdx(make_index_map(collect_event_typeids_recursive(rootState)), eventId);
};

constexpr auto getActionIdx = [](auto rootState, auto action) {
    return getIdx(make_index_map(collect_action_typeids_recursive(rootState)), bh::typeid_(action));
};

constexpr auto getGuardIdx = [](auto rootState, auto guard) {
    return getIdx(make_index_map(collect_guard_typeids_recursive(rootState)), bh::typeid_(guard));
};

const auto is_anonymous_transition
    = [](auto transition) { return bh::typeid_(getEvent(transition)) == bh::typeid_(none {}); };

const auto get_unexpected_event_handler = [](auto rootState) {
    return bh::if_(
        has_unexpected_event_handler(rootState),
        [](auto rootState) { return rootState.on_unexpected_event(); },
        [](auto) { return [](auto /*event*/) {}; })(rootState);
};
}