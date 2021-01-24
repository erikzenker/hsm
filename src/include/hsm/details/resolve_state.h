#pragma once

#include "hsm/details/has_action.h"
#include "hsm/details/traits.h"
#include <boost/hana/functional/apply.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Transition> constexpr auto resolveSubStateParent(Transition transition)
{
    auto constexpr target = transition.target();
    if constexpr (is_entry_state(target)) {
        return get_parent_state(target);
    } else if constexpr (is_direct_state(target)) {
        return get_parent_state(target);
    } else if constexpr (is_history_state(target)) {
        return get_parent_state(target);
    } else {
        return target;
    }
}

template <class Transition> constexpr auto resolveDst(Transition transition)
{
    auto constexpr dst = transition.target();

    if constexpr (has_transition_table(dst)) {
        return bh::at_c<0>(collect_initial_states(dst));
    } else if constexpr (is_entry_state(dst)) {
        return get_state(dst);
    } else if constexpr (is_direct_state(dst)) {
        return get_state(dst);
    } else if constexpr (is_history_state(dst)) {
        return bh::at_c<0>(collect_initial_states(get_parent_state(dst)));
    } else if constexpr (is_initial_state(dst)) {
        return get_state(dst);
    } else {
        return dst;
    }
}

template <class Transition> constexpr auto resolveDstParent(Transition transition)
{
    auto constexpr target = transition.target();

    if constexpr (has_transition_table(target)) {
        return target;
    } else if constexpr (is_entry_state(target)) {
        return get_parent_state(target);
    } else if constexpr (is_direct_state(target)) {
        return get_parent_state(target);
    } else if constexpr (is_history_state(target)) {
        return get_parent_state(target);
    } else {
        return transition.parent();
    }
}

template <class Transition> constexpr auto resolveSrc(Transition transition)
{
    auto constexpr src = transition.source();

    if constexpr (is_initial_state(src)) {
        return get_state(src);
    } else if constexpr (is_exit_state(src)) {
        return get_state(src);
    } else if constexpr (is_direct_state(src)) {
        return get_state(src);
    } else {
        return src;
    }
}

template <class Transition> constexpr auto resolveSrcParent(Transition transition)
{
    auto constexpr src = transition.source();

    if constexpr (is_exit_state(src)) {
        return get_parent_state(src);
    } else if constexpr (is_direct_state(src)) {
        return get_parent_state(src);
    } else {
        return transition.parent();
    }
}

template <class Transition> constexpr auto resolveEntryAction(Transition transition)
{
    if constexpr (transition.internal()) {
        return [](auto&&...) {};
    } else if constexpr (has_entry_action(transition.target())) {
        return get_entry_action(transition.target());
    } else {
        return [](auto&&...) {};
    }
}

template <class Transition> constexpr auto resolveInitialStateEntryAction(Transition transition)
{
    if constexpr (has_substate_initial_state_entry_action(transition.target())) {
        return get_entry_action(bh::at_c<0>(collect_initial_states(transition.target())));
    } else {
        return [](auto&&...) {};
    }
}

template <class Transition> constexpr auto resolveExitAction(Transition transition)
{
    if constexpr (transition.internal()) {
        return [](auto&&...) {};
    } else if constexpr (has_exit_action(transition.source())) {
        return get_exit_action(transition.source());
    } else {
        return [](auto&&...) {};
    }
}

template <class Transition> constexpr auto resolveNoAction(Transition transition)
{
    const auto isNoAction = is_no_action(transition.action());

    if constexpr (isNoAction) {
        return [](auto&&...) {};
    } else {
        return transition.action();
    }
}

template <class Transition> constexpr auto resolveEntryExitAction(Transition transition)
{
    return [exitAction(resolveExitAction(transition)),
            action(resolveNoAction(transition)),
            entryAction(resolveEntryAction(transition)),
            initialStateEntryAction(resolveInitialStateEntryAction(transition))](auto&&... params) {
        exitAction(std::forward<decltype(params)>(params)...);
        action(std::forward<decltype(params)>(params)...);
        entryAction(std::forward<decltype(params)>(params)...);
        initialStateEntryAction(std::forward<decltype(params)>(params)...);
    };
}

template <class Transition> constexpr auto resolveAction(Transition transition)
{
    const auto hasAction = has_action(transition);

    if constexpr (hasAction) {
        return resolveEntryExitAction(transition);
    } else {
        return transition.action();
    }
}

template <class Transition> constexpr auto resolveHistory(Transition transition)
{
    if constexpr (is_history_state(transition.target())) {
        return true;
    } else {
        return false;
    }
}
}