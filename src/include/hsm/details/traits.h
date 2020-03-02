#pragma once

#include "../front/transition.h"
#include "pseudo_states.h"

#include <boost/hana.hpp>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    }

    auto has_transition_table = bh::is_valid([](auto&& state) -> decltype(state.make_transition_table()) { });
    auto has_internal_transition_table
        = bh::is_valid([](auto&& state) -> decltype(state.make_internal_transition_table()) {});
    auto has_initial_state = bh::is_valid([](auto&& state) -> decltype(state.initial_state()) { });
    auto has_entry_action = bh::is_valid([](auto&& state) -> decltype(state.on_entry()) { });
    auto has_exit_action = bh::is_valid([](auto&& state) -> decltype(state.on_exit()) { });
    auto has_unexpected_event_handler
        = bh::is_valid([](auto&& state) -> decltype(state.on_unexpected_event()) {});
    auto has_deferred_events = bh::is_valid([](auto&& state) -> decltype(state.defer_events()) {});

    auto constexpr is_exit_state = [](auto type) {
        return bh::equal(
            bh::bool_c<std::is_base_of<ExitPseudoState, decltype(type)>::value>, bh::true_c);
    };
    auto constexpr is_entry_state = [](auto type) {
        return bh::equal(
            bh::bool_c<std::is_base_of<EntryPseudoState, decltype(type)>::value>, bh::true_c);
    };
    auto constexpr is_direct_state = [](auto type) {
        return bh::equal(
            bh::bool_c<std::is_base_of<DirectPseudoState, decltype(type)>::value>, bh::true_c);
    };
    auto constexpr is_history_state = [](auto type) {
        return bh::equal(
            bh::bool_c<std::is_base_of<HistoryPseudoState, decltype(type)>::value>, bh::true_c);
    };
    auto constexpr is_no_action
        = [](auto action) { return bh::equal(bh::typeid_(action), bh::typeid_(noAction {})); };
    auto constexpr is_no_guard
        = [](auto guard) { return bh::equal(bh::typeid_(guard), bh::typeid_(noGuard {})); };

    auto is_event = bh::is_valid([](auto&& event) -> decltype(event.typeid_) {});

    auto const is_callable = [](auto&& callable, auto&& args) {
        return bh::unpack(args,
            bh::is_valid([&callable](auto&&... args)
                -> decltype(callable(args...))
            {})
        );
    };

    auto const contains_dependency = [](const auto& parameters) { return bh::size(parameters); };
    }