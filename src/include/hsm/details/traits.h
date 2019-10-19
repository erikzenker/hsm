#pragma once

#include "pseudo_states.h"

#include <boost/hana.hpp>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    };

    auto has_transition_table = bh::is_valid([](auto&& state) -> decltype(state.make_transition_table()) { });
    auto has_initial_state = bh::is_valid([](auto&& state) -> decltype(state.initial_state()) { });
    auto has_entry_action = bh::is_valid([](auto&& state) -> decltype(state.on_entry()) { });
    auto has_exit_action = bh::is_valid([](auto&& state) -> decltype(state.on_exit()) { });

    auto is_exit_state = bh::is_valid([](auto&& state) -> decltype(state.isExitState) {});
    auto is_entry_state = bh::is_valid([](auto&& state) -> decltype(state.isEntryState) {});
    auto is_direct_state = bh::is_valid([](auto&& state) -> decltype(state.isDirectState) {});

    auto is_event = bh::is_valid([](auto&& event) -> decltype(event.typeid_) {});    

    auto const is_callable = [](auto&& callable, auto&& args) {
        return bh::unpack(args,
            bh::is_valid([&callable](auto&&... args)
                -> decltype(callable(args...))
            {})
        );
    };

}
