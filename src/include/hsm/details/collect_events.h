#pragma once

#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

template <class T> constexpr auto collect_sub_events(T&& state);

const auto collect_event_recursive = [](auto state) {
    return bh::to<bh::tuple_tag>(bh::to<bh::set_tag>(
        bh::fold_left(state.make_transition_table(), bh::make_tuple(), [](auto events, auto row) {
            return bh::concat(
                bh::append(events, bh::typeid_(bh::at_c<1>(row))),
                collect_sub_events(bh::back(row)));
        })));
};

template <class T> constexpr auto collect_sub_events(T&& state)
{
    return bh::if_(
        has_transition_table(state),
        [](auto& stateWithTransitionTable) {
            return collect_event_recursive(stateWithTransitionTable);
        },
        [](auto&) { return bh::make_tuple(); })(state);
};

const auto collect_events = [](auto state) {
    return bh::to<bh::tuple_tag>(bh::to<bh::set_tag>(
        bh::fold_left(state.make_transition_table(), bh::make_tuple(), [](auto events, auto row) {
            return bh::append(events, bh::typeid_(bh::at_c<1>(row)));
        })));
};
}