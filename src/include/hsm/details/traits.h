#pragma once

#include "hsm/details/pseudo_states.h"

#include <boost/hana/bool.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/functional/compose.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/or.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/type.hpp>

#include <memory>
#include <utility>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace details {
constexpr auto has_internal_transition_table
    = bh::is_valid([](auto&& state) -> decltype(state.make_internal_transition_table()) {});

constexpr auto has_entry_action = bh::is_valid([](auto&& state) -> decltype(state.on_entry()) {});

constexpr auto has_exit_action = bh::is_valid([](auto&& state) -> decltype(state.on_exit()) {});

constexpr auto has_unexpected_event_handler
    = bh::is_valid([](auto&& state) -> decltype(state.on_unexpected_event()) {});

constexpr auto is_exit_state = [](auto type) {
    return bh::equal(
        bh::bool_c<std::is_base_of<ExitPseudoState, decltype(type)>::value>, bh::true_c);
};

constexpr auto has_deferred_events
    = bh::is_valid([](auto&& state) -> decltype(state.defer_events()) {});

constexpr auto is_entry_state = [](auto type) {
    return bh::equal(
        bh::bool_c<std::is_base_of<EntryPseudoState, decltype(type)>::value>, bh::true_c);
};

constexpr auto is_direct_state = [](auto type) {
    return bh::equal(
        bh::bool_c<std::is_base_of<DirectPseudoState, decltype(type)>::value>, bh::true_c);
};

constexpr auto is_history_state = [](auto type) {
    return bh::equal(
        bh::bool_c<std::is_base_of<HistoryPseudoState, decltype(type)>::value>, bh::true_c);
};
constexpr auto is_initial_state = [](auto type) {
    return bh::equal(
        bh::bool_c<std::is_base_of<InitialPseudoState, decltype(type)>::value>, bh::true_c);
};
}

constexpr auto unwrap_typeid = [](auto typeid_) { return typename decltype(typeid_)::type {}; };
constexpr auto unwrap_typeid_to_shared_ptr
    = [](auto typeid_) { return std::make_shared<typename decltype(typeid_)::type>(); };

constexpr auto make_transition_table = [](auto t) {
    return decltype(std::declval<typename decltype(t)::type>().make_transition_table())();
};

constexpr auto make_transition_table2
    = [](auto state) { return decltype(state)::type::make_transition_table(); };

constexpr auto has_transition_table = bh::is_valid(
    [](auto stateTypeid) -> decltype(std::declval<typename decltype(stateTypeid)::type>()
                                         .make_transition_table()) {});

constexpr auto has_internal_transition_table
    = bh::compose(details::has_internal_transition_table, unwrap_typeid);

constexpr auto has_entry_action = bh::compose(details::has_entry_action, unwrap_typeid);

constexpr auto has_exit_action = bh::compose(details::has_exit_action, unwrap_typeid);

constexpr auto has_unexpected_event_handler
    = bh::compose(details::has_unexpected_event_handler, unwrap_typeid);

constexpr auto has_deferred_events = bh::compose(details::has_deferred_events, unwrap_typeid);

constexpr auto is_exit_state = bh::compose(details::is_exit_state, unwrap_typeid);
constexpr auto is_entry_state = bh::compose(details::is_entry_state, unwrap_typeid);
constexpr auto is_direct_state = bh::compose(details::is_direct_state, unwrap_typeid);
constexpr auto is_history_state = bh::compose(details::is_history_state, unwrap_typeid);
// constexpr auto is_initial_state = bh::compose(details::is_initial_state, unwrap_typeid);
constexpr auto is_initial_state = [](auto typeid_) {
    return bh::equal(
        bh::bool_c<std::is_base_of<InitialPseudoState, typename decltype(typeid_)::type>::value>,
        bh::true_c);
};

constexpr auto is_no_action
    = [](auto action) { return bh::equal(bh::typeid_(action), bh::typeid_(noAction {})); };

constexpr auto is_no_guard
    = [](auto guard) { return bh::equal(bh::typeid_(guard), bh::typeid_(noGuard {})); };

constexpr auto is_event = bh::is_valid([](auto&& event) -> decltype(event.typeid_) {});

constexpr auto contains_dependency = [](const auto& parameters) { return bh::size(parameters); };

constexpr auto has_action = [](auto&& transition) {
    return bh::or_(
        bh::not_(is_no_action(transition.action())),
        has_entry_action(transition.target()),
        has_exit_action(transition.source()));
};

constexpr auto has_no_action = [](auto&& transition) {
    return bh::and_(
        is_no_action(transition.action()), bh::not_(has_entry_action(transition.target())));
};
}