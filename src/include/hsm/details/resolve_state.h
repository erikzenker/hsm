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

template <class Transition> constexpr auto resolveEntryAction(Transition&& transition)
{
    // clang-format off
    return bh::apply([](auto&& dst){
        return bh::if_(has_entry_action(dst)
            , [](auto&& dst) { return get_entry_action(dst); }
            , [](auto&&) { return [](auto...) {}; })
            (dst);    
    },
    transition.target());
    // clang-format on
}

template <class Transition> constexpr auto resolveInitialStateEntryAction(Transition&& transition)
{
    // clang-format off
    return bh::apply([](auto&& target){
        return bh::if_(has_substate_initial_state_entry_action(target)
            , [](auto&& target) { return get_entry_action(bh::at_c<0>(collect_initial_states(target)));}
            , [](auto&&) { return [](auto...) {}; })
            (target);    
    },
    transition.target());
    // clang-format on
}

template <class Transition> constexpr auto resolveExitAction(Transition&& transition)
{
    // clang-format off
    return bh::apply([](auto&& src){
        return bh::if_(has_exit_action(src)
            , [](auto&& src) { return get_exit_action(src); }
            , [](auto&&) { return [](auto...) {}; })
            (src);
    },
    transition.source());
    // clang-format on
}

template <class Transition> constexpr auto resolveNoAction(Transition&& transition)
{
    return bh::if_(
        is_no_action(transition.action()),
        [](auto&&) { return [](auto&&...) {}; },
        [](auto&& transition) { return transition.action(); })(transition);
}

template <class Transition> constexpr auto resolveEntryExitAction(Transition&& transition)
{
    return [exitAction(resolveExitAction(transition)),
            action(resolveNoAction(transition)),
            entryAction(resolveEntryAction(transition)),
            initialStateEntryAction(resolveInitialStateEntryAction(transition))](auto&&... params) {
        exitAction(params...);
        action(params...);
        entryAction(params...);
        initialStateEntryAction(params...);
    };
}

template <class Transition> constexpr auto resolveAction(Transition&& transition)
{
    // clang-format off
    return bh::if_(
        has_action(transition),
        [](auto&& transition) { return resolveEntryExitAction(transition);},
        [](auto&& transition) { return transition.action(); })
        (transition);
    // clang-format on
}

template <class Transition> constexpr auto resolveHistory(Transition&& transition)
{
    // clang-format off
    return bh::apply([](auto&& dst){
        return bh::if_(is_history_state(dst)
            , [](auto&&) { return true; }
            , [](auto&&) { return false; })
            (dst);
    }, 
    transition.target());
    // clang-format on                   
}
}