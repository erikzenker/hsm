#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/collect_initial_states.h"
#include "hsm/details/dispatch_table.h"
#include "hsm/details/flatten_internal_transition_table.h"
#include "hsm/details/for_each_idx.h"
#include "hsm/details/has_action.h"
#include "hsm/details/idx.h"
#include "hsm/details/resolve_state.h"
#include "hsm/details/switch.h"
#include "hsm/details/to_pairs.h"

#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/find.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/functional/capture.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/lazy.hpp>
#include <boost/hana/length.hpp>
#include <boost/hana/mult.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto nParentStates
    = [](auto rootState) { return bh::length(collect_parent_state_typeids(rootState)); };
constexpr auto nStates
    = [](auto rootState) { return bh::length(collect_state_typeids_recursive(rootState)); };
constexpr auto nEvents
    = [](auto rootState) { return bh::length(collect_event_typeids_recursive(rootState)); };

constexpr auto hasRegions
    = [](auto rootState) { return bh::equal(bh::size_c<1>, maxInitialStates(rootState)); };

constexpr auto makeInvalidGuard
    = [](auto dispatchTable) { return decltype(dispatchTable[0].guard) {}; };

constexpr auto makeInvalidAction
    = [](auto dispatchTable) { return decltype(dispatchTable[0].action) {}; };

constexpr auto resolveEntryAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dst){
        return bh::if_(has_entry_action(dst)
            , [](auto&& dst) { return get_entry_action(dst); }
            , [](auto&&) { return [](auto...) {}; })
            (dst);    
    },
    transition.target());
    // clang-format on
};

constexpr auto resolveInitialStateEntryAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& target){
        return bh::if_(has_substate_initial_state_entry_action(target)
            , [](auto&& target) { return get_entry_action(bh::at_c<0>(collect_initial_states(target)));}
            , [](auto&&) { return [](auto...) {}; })
            (target);    
    },
    transition.target());
    // clang-format on
};

constexpr auto resolveExitAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src){
        return bh::if_(has_exit_action(src)
            , [](auto&& src) { return get_exit_action(src); }
            , [](auto&&) { return [](auto...) {}; })
            (src);
    },
    transition.source());
    // clang-format on
};

constexpr auto resolveNoAction = [](auto&& transition) {
    return bh::if_(
        is_no_action(transition.action()),
        [](auto&&) { return [](auto&&...) {}; },
        [](auto&& transition) { return transition.action(); })(transition);
};

constexpr auto resolveEntryExitAction = [](auto&& transition) {
    return [exitAction(resolveExitAction(transition)),
            action(resolveNoAction(transition)),
            entryAction(resolveEntryAction(transition)),
            initialStateEntryAction(resolveInitialStateEntryAction(transition))](auto&&... params) {
        exitAction(params...);
        action(params...);
        entryAction(params...);
        initialStateEntryAction(params...);
    };
};

constexpr auto resolveAction = [](auto&& transition) {
    // clang-format off
    return bh::if_(
        has_action(transition),
        [](auto&& transition) { return resolveEntryExitAction(transition);},
        [](auto&& transition) { return transition.action(); })
        (transition);
    // clang-format on
};

constexpr auto resolveHistory = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dst){
        return bh::if_(is_history_state(dst)
            , [](auto&&) { return true; }
            , [](auto&&) { return false; })
            (dst);
    }, 
    transition.target());
    // clang-format on                   
};

constexpr auto addDispatchTableEntry = [](auto&& combinedStateTypids, auto&& transition, auto& dispatchTable, auto eventTypeid, auto&& statesMap, auto optionalDependency) {
          const auto source = resolveSrc(transition);
          const auto target = resolveDst(transition);
          const auto from = getCombinedStateIdx(combinedStateTypids, resolveSrcParent(transition), source);
          const auto guard = transition.guard();
          const auto action = resolveAction(transition);
          const auto to = getCombinedStateIdx(combinedStateTypids, resolveDstParent(transition), target);
          const auto history = resolveHistory(transition);
          const auto defer = false;
          const auto valid = true;

          auto mappedSource = bh::find(statesMap, bh::typeid_(source)).value();
          auto mappedTarget = bh::find(statesMap, bh::typeid_(target)).value();

          dispatchTable[from] = { to, history, defer, valid, make_transition(action, guard, eventTypeid, mappedSource, mappedTarget, optionalDependency)};
      };

const auto addDispatchTableEntryOfSubMachineExits
    = [](auto&& combinedStateTypids, auto&& transition, auto& dispatchTable, auto&& eventTypeid, auto&& statesMap, auto optionalDependency) {
          bh::if_(
              has_transition_table(transition.source()),
              [&](auto parentState) {
                  auto states = collect_child_state_typeids(parentState);

                  bh::for_each(
                      states, [&](auto state) {
                          const auto target = resolveDst(transition);                          
                          const auto from = getCombinedStateIdx(combinedStateTypids, parentState, state);
                          const auto guard = transition.guard();
                          const auto action = resolveAction(transition);
                          const auto to = getCombinedStateIdx(
                              combinedStateTypids, resolveDstParent(transition), target);
                          const auto history = resolveHistory(transition);
                          const auto defer = false;
                          const auto valid = true;

                          auto mappedParentState = bh::find(statesMap, bh::typeid_(parentState)).value();
                          auto mappedTarget = bh::find(statesMap, bh::typeid_(target)).value();

                          dispatchTable[from] = { to, history, defer, valid, make_transition(action, guard, eventTypeid, mappedParentState, mappedTarget, optionalDependency)};
                      });
              },
              [](auto) {})(transition.source());
      };

constexpr auto filter_transitions = [](auto transitions, auto eventTypeid) {
    auto isEvent = [eventTypeid](auto transition) {
        return bh::equal(transition.event().typeid_, eventTypeid);
    };

    return bh::filter(transitions, isEvent);
};


constexpr auto fill_dispatch_table_for_filtered_transitions = [](auto rootState, auto&& statesMap, auto&& optionalDependency, auto eventTypeid, auto transition){
    using Event = typename decltype(eventTypeid)::type;

    constexpr auto combinedStateTypeids = getCombinedStateTypeids(rootState);
    constexpr StateIdx states = nStates(rootState) * nParentStates(rootState);    
    auto& dispatchTable = DispatchTable<states, Event>::table;

    addDispatchTableEntry(combinedStateTypeids, transition, dispatchTable, eventTypeid, statesMap, optionalDependency);
    addDispatchTableEntryOfSubMachineExits(combinedStateTypeids, transition, dispatchTable, eventTypeid, statesMap, optionalDependency);
};

constexpr auto fill_dispatch_table_for_event = [](auto rootState, auto&& statesMap, auto&& optionalDependency, auto transitions, auto eventTypeid){
    
    auto filteredTransitions = filter_transitions(transitions, eventTypeid);

    bh::for_each(filteredTransitions, bh::capture(rootState, statesMap, optionalDependency, eventTypeid)(fill_dispatch_table_for_filtered_transitions));
};

constexpr auto fill_dispatch_table_with_transitions = [](
    auto rootState, auto&& statesMap, auto&& optionalDependency, auto transitions)
{
    auto eventTypeids = collect_event_typeids_recursive_with_transitions(transitions);

    bh::for_each(eventTypeids, bh::capture(rootState, statesMap, optionalDependency, transitions)(fill_dispatch_table_for_event));

};

constexpr auto getDeferingTransitions = [](auto rootState) {
    constexpr auto transitionHasDeferedEvents
        = [](auto transition) { return has_deferred_events(resolveExtentedInitialState(transition)); };

    constexpr auto transitions = flatten_transition_table(rootState);
    return bh::filter(transitions, transitionHasDeferedEvents);
};

constexpr auto hasDeferedEvents
    = [](auto rootState) { return bh::size(getDeferingTransitions(rootState)); };




template <class RootState, class OptionalDependency>
constexpr auto
fill_dispatch_table_with_deferred_events(RootState rootState, OptionalDependency /*optionalDependency*/)
{
    const auto transitions = getDeferingTransitions(rootState);
    bh::for_each(transitions, bh::capture(rootState)([](auto rootState, auto transition){
        const auto deferredEvents = get_defer_events(resolveExtentedInitialState(transition));
        bh::for_each(deferredEvents, bh::capture(rootState, transition)([](auto rootState, auto transition, auto event){
            using Event = typename decltype(event)::type;
            const auto combinedStateTypeids = getCombinedStateTypeids(rootState);
            constexpr StateIdx states = nStates(rootState) * nParentStates(rootState);
            auto& dispatchTable = DispatchTable<states, Event>::table;
            const auto from = getCombinedStateIdx(
                combinedStateTypeids, resolveSrcParent(transition), resolveSrc(transition));
            dispatchTable[from].defer = true;
        }));
    }));
}

template <class RootState, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_external_transitions(
    const RootState& rootState, StatesMap&& statesMap, OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions(
        rootState, statesMap, optionalDependecy, flatten_transition_table(rootState));
}

template <class RootState, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_internal_transitions(
    const RootState& rootState, StatesMap&& statesMap, OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions(
        rootState, statesMap, optionalDependecy, flatten_internal_transition_table(rootState));
}
}