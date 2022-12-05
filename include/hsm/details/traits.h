#pragma once

#include "hsm/details/pseudo_states.h"

#include <boost/hana/bool.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/functional/compose.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/or.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

#include <functional>
#include <memory>
#include <utility>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace details {
constexpr auto has_internal_transition_table
    = bh::is_valid([](auto&& state) -> decltype(state.make_internal_transition_table()) {});

}

constexpr auto is_default_constructable
    = bh::is_valid([](auto typeid_) -> decltype(typename decltype(typeid_)::type()) {});

constexpr auto is_custom_target_action
    = bh::is_valid([](auto action, auto... args) -> decltype(action(args...)) {});

auto const is_callable = [](auto&& callable, auto&& args) {
    return bh::unpack(args, bh::is_valid([](auto&&... args) -> decltype(callable(args...)) {}));
};

constexpr auto get_parent_state = [](auto state) {
    return decltype(std::declval<typename decltype(state)::type>().get_parent_state())();
};

constexpr auto get_state = [](auto state) {
    return decltype(std::declval<typename decltype(state)::type>().get_state())();
};

constexpr auto unwrap_typeid = [](auto typeid_) { return typename decltype(typeid_)::type {}; };

constexpr auto unwrap_typeid_to_shared_ptr = [](auto typeid_) {
    using UnwrappedType = typename decltype(typeid_)::type;
    if constexpr (is_default_constructable(typeid_)) {
        return std::make_shared<std::unique_ptr<UnwrappedType>>(std::make_unique<UnwrappedType>());
    } else {
        return std::make_shared<std::unique_ptr<UnwrappedType>>(nullptr);
    }
};

constexpr auto make_transition_table = [](auto t) {
    return decltype(std::declval<typename decltype(t)::type>().make_transition_table())();
};

constexpr auto make_internal_transition_table
    = [](auto state) { return decltype(state)::type::make_internal_transition_table(); };

constexpr auto make_transition_table2
    = [](auto state) { return decltype(state)::type::make_transition_table(); };

constexpr auto has_transition_table = bh::is_valid(
    [](auto stateTypeid) -> decltype(std::declval<typename decltype(stateTypeid)::type>()
                                         .make_transition_table()) {});

constexpr auto has_internal_transition_table = bh::is_valid(
    [](auto stateTypeid) -> decltype(std::declval<typename decltype(stateTypeid)::type>()
                                         .make_internal_transition_table()) {});

constexpr auto has_entry_action = bh::is_valid(
    [](auto stateTypeid) -> decltype(
                             std::declval<typename decltype(stateTypeid)::type>().on_entry()) {});

constexpr auto has_exit_action = bh::is_valid(
    [](auto stateTypeid) -> decltype(
                             std::declval<typename decltype(stateTypeid)::type>().on_exit()) {});

constexpr auto has_unexpected_event_handler = bh::is_valid(
    [](auto stateTypeid)
        -> decltype(std::declval<typename decltype(stateTypeid)::type>().on_unexpected_event()) {});

constexpr auto has_deferred_events = bh::is_valid(
    [](auto stateTypeid)
        -> decltype(std::declval<typename decltype(stateTypeid)::type>().defer_events()) {});

constexpr auto is_exit_state = [](auto stateTypeid) {
    return bh::equal(
        bh::bool_c<std::is_base_of<ExitPseudoState, typename decltype(stateTypeid)::type>::value>,
        bh::true_c);
};

constexpr auto is_entry_state = [](auto stateTypeid) {
    return bh::equal(
        bh::bool_c<std::is_base_of<EntryPseudoState, typename decltype(stateTypeid)::type>::value>,
        bh::true_c);
};

constexpr auto is_direct_state = [](auto stateTypeid) {
    return bh::equal(
        bh::bool_c<std::is_base_of<DirectPseudoState, typename decltype(stateTypeid)::type>::value>,
        bh::true_c);
};

constexpr auto is_history_state = [](auto stateTypeid) {
    return bh::equal(
        bh::bool_c<
            std::is_base_of<HistoryPseudoState, typename decltype(stateTypeid)::type>::value>,
        bh::true_c);
};

constexpr auto is_initial_state = [](auto typeid_) {
    return bh::equal(
        bh::bool_c<std::is_base_of<InitialPseudoState, typename decltype(typeid_)::type>::value>,
        bh::true_c);
};

constexpr auto is_no_action
    = [](auto action) { return bh::equal(bh::typeid_(action), bh::typeid_(noAction {})); };

template <class Action> constexpr auto is_action() -> decltype(auto)
{
    return bh::not_equal(bh::type_c<Action>, bh::typeid_(noAction {}));
}

constexpr auto is_no_guard
    = [](auto guard) { return bh::equal(bh::typeid_(guard), bh::typeid_(noGuard {})); };

template <class Guard> constexpr auto is_guard() -> decltype(auto)
{
    return bh::not_equal(bh::type_c<Guard>, bh::typeid_(noGuard {}));
}

constexpr auto is_event = bh::is_valid([](auto&& event) -> decltype(event.typeid_) {});

constexpr auto get_entry_action
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::on_entry(); };
constexpr auto get_exit_action
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::on_exit(); };
constexpr auto get_defer_events
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::defer_events(); };
const auto get_unexpected_event_handler = [](auto rootState) {
    if constexpr (has_unexpected_event_handler(rootState)) {
        return decltype(rootState)::type::on_unexpected_event();
    } else {
        return [](auto&&...) {};
    }
};
}