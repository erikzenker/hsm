#pragma once

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/unpack.hpp>

namespace hsm {

/*
 * Chain abitrary number of actions to be used in the single transition
 *
 * @param[in] actions actions that will be chained
 */
constexpr auto chain_actions = [](auto... actions) {
    constexpr auto actionsTpl = boost::hana::make_basic_tuple(actions...);

    return [actions(actionsTpl)](auto... args) {
        auto argsTpl = boost::hana::make_basic_tuple(args...);

        boost::hana::for_each(actions, [args(argsTpl)](auto action) {
            boost::hana::unpack(args, [action(action)](auto... args) { action(args...); });
        });
    };
};

/*
 * Shortcut for chain_actions
 */
constexpr auto chain = chain_actions;
}