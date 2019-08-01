
#pragma once

#include "remove_duplicates.h"
#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

template <class T> constexpr auto collect_sub_guard_typeids(T&& state);

const auto collect_guard_typeids_recursive = [](auto state) {
    auto collectedGuards
        = bh::fold_left(state.make_transition_table(), bh::make_tuple(), [](auto guards, auto row) {
              return bh::concat(
                  bh::append(guards, bh::typeid_(bh::at_c<2>(row))),
                  collect_sub_guard_typeids(bh::back(row)));
          });
    return remove_duplicate_typeids(collectedGuards);
};

template <class T> constexpr auto collect_sub_guard_typeids(T&& state)
{
    return bh::if_(
        has_transition_table(state),
        [](auto& stateWithTransitionTable) {
            return collect_guard_typeids_recursive(stateWithTransitionTable);
        },
        [](auto&) { return bh::make_tuple(); })(state);
};

template <class T> constexpr auto collect_sub_guards(T&& state);

const auto collect_guards_recursive = [](auto state) {
    auto collectedGuards
        = bh::fold_left(state.make_transition_table(), bh::make_tuple(), [](auto guards, auto row) {
              return bh::concat(
                  bh::append(guards, bh::at_c<2>(row)), collect_sub_guards(bh::back(row)));
          });
    return remove_duplicate_types(collectedGuards);
};

template <class T> constexpr auto collect_sub_guards(T&& state)
{
    return bh::if_(
        has_transition_table(state),
        [](auto& stateWithTransitionTable) {
            return collect_guards_recursive(stateWithTransitionTable);
        },
        [](auto&) { return bh::make_tuple(); })(state);
};

}