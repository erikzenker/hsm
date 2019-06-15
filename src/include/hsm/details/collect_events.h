#pragma once

#include "traits.h"
#include "remove_duplicates.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

template <class T> constexpr auto collect_sub_events(T&& state);

const auto collect_events_recursive = [](auto state) {
    auto collectedEvents = bh::fold_left(
        state.make_transition_table(), bh::make_tuple(), [](auto events, auto row) {
            return bh::concat(
                bh::append(events, bh::typeid_(bh::at_c<1>(row).getEvent())),
                collect_sub_events(bh::back(row)));
        });
    return remove_duplicate_typeids(collectedEvents);
};

template <class T> constexpr auto collect_sub_events(T&& state)
{
    return bh::if_(
        has_transition_table(state),
        [](auto& stateWithTransitionTable) {
            return collect_events_recursive(stateWithTransitionTable);
        },
        [](auto&) { return bh::make_tuple(); })(state);
};

const auto collect_events = [](auto state) {
    auto collectedEvents
        = bh::fold_left(state.make_transition_table(), bh::make_tuple(), [](auto events, auto row) {
              return bh::append(events, bh::typeid_(bh::at_c<1>(row).getEvent()));
          });
    return remove_duplicate_typeids(collectedEvents);
};
}