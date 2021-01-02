#pragma once

#include <tuple>

namespace hsm {

/*
 * Chain abitrary number of actions to be used in the single transition
 *
 * @param[in] actions actions that will be chained
 */
constexpr auto chain_actions = [](auto... actions) {
    return [=](auto&&... args) {
        return std::apply([&](auto... f) { (f(args...), ...); }, std::tie(actions...));
    };
};

/*
 * Shortcut for chain_actions
 */
constexpr auto chain = chain_actions;
}