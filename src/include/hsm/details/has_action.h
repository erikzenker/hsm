#pragma once

#include "hsm/details/collect_initial_states.h"
#include "hsm/details/traits.h"

#include <boost/hana/bool.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/or.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto has_substate_initial_state_entry_action = [](auto&& target) {
    return bh::if_(
        has_transition_table(target),
        [](auto&& target) { return has_entry_action(bh::at_c<0>(collect_initial_states(target))); },
        [](auto&&) { return bh::false_c; })(target);
};

constexpr auto has_action = [](auto&& transition) {
    return bh::or_(
        bh::not_(is_no_action(transition.action())),
        has_entry_action(transition.target()),
        has_substate_initial_state_entry_action(transition.target()),
        has_exit_action(transition.source()));
};
}