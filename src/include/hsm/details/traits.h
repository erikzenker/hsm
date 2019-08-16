#pragma once

#include "pseudo_states.h"

#include <boost/hana.hpp>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    };

    auto has_transition_table = bh::is_valid([](auto&& state) -> decltype(state.make_transition_table()) { });

    auto is_exit_state = bh::is_valid([](auto&& state) -> decltype(state.isExitState) {});
    auto is_entry_state = bh::is_valid([](auto&& state) -> decltype(state.isEntryState) {});

    auto is_event = bh::is_valid([](auto&& event) -> decltype(event.getEvent()) {});    

    auto const is_callable = [](auto&& callable, auto&& args) {
        return bh::unpack(args,
            bh::is_valid([&callable](auto&&... args)
                -> decltype(callable(args...))
            {})
        );
    };

}
