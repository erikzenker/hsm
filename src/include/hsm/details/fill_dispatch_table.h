#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/collect_initial_states.h"
#include "hsm/details/dispatch_table.h"
#include "hsm/details/flatten_internal_transition_table.h"
#include "hsm/details/for_each_idx.h"
#include "hsm/details/idx.h"
#include "hsm/details/switch.h"
#include "hsm/details/to_pairs.h"

#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/find.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
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

constexpr auto resolveDst = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dstTypeid){
        return lazy_switch_(
            // TODO: make multi region capable    
            case_(bh::make_lazy(has_transition_table(dstTypeid)), [](auto&& submachine, auto&&) { return bh::at_c<0>(collect_initial_states(submachine));}),
            case_(bh::make_lazy(is_entry_state(dstTypeid)),       [](auto&& entry, auto&&) { return unwrap_typeid(entry).get_state(); }),
            case_(bh::make_lazy(is_direct_state(dstTypeid)),      [](auto&& direct, auto&&) { return unwrap_typeid(direct).get_state(); }),
            // TODO: make multi region capable 
            case_(bh::make_lazy(is_history_state(dstTypeid)),     [](auto&&, auto&& history) { return bh::at_c<0>(collect_initial_states(history.get_parent_state()));}),
            case_(bh::make_lazy((otherwise())),                   [](auto&& dstTypeid, auto&&) { return dstTypeid; }))
            (dstTypeid, unwrap_typeid(dstTypeid));
    },
    transition.target());
    // clang-format on
};

constexpr auto resolveDstParent = [](auto transition) {
    // clang-format off
    return bh::apply([](auto&& dstTypeid, auto&& transition){
        return lazy_switch_(
            case_(bh::make_lazy(has_transition_table(dstTypeid)), [](auto&& submachine, auto&&) { return submachine; }),
            case_(bh::make_lazy(is_entry_state(dstTypeid)),       [](auto&& entry, auto&&) { return unwrap_typeid(entry).get_parent_state(); }),
            case_(bh::make_lazy(is_direct_state(dstTypeid)),      [](auto&& direct, auto&&) { return unwrap_typeid(direct).get_parent_state(); }),
            case_(bh::make_lazy(is_history_state(dstTypeid)),     [](auto&& history, auto&&) { return unwrap_typeid(history).get_parent_state(); }),
            case_(bh::make_lazy(otherwise()),                     [](auto&&, auto&& transition) { return transition.parent(); }))
            (dstTypeid, transition);
    },
    transition.target(), transition);
    // clang-format on
};

constexpr auto resolveSrc = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src){
        return lazy_switch_(
            case_(bh::make_lazy(is_initial_state(src)), [](auto&& initial) { return unwrap_typeid(initial).get_state(); }),    
            case_(bh::make_lazy(is_exit_state(src)),    [](auto&& exit) { return unwrap_typeid(exit).get_state(); }),
            case_(bh::make_lazy(is_direct_state(src)),  [](auto&& direct) { return unwrap_typeid(direct).get_state(); }),
            case_(bh::make_lazy(otherwise()),           [](auto&& state) { return state; }))
            (src);    
    },
    transition.source());
    // clang-format on
};

constexpr auto resolveSrcParent = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src, auto&& transition){
        return lazy_switch_(
            case_(bh::make_lazy(is_exit_state(src)),   [](auto&& exit, auto&&) { return unwrap_typeid(exit).get_parent_state(); }),
            case_(bh::make_lazy(is_direct_state(src)), [](auto&& direct, auto&&) { return unwrap_typeid(direct).get_parent_state(); }),
            case_(bh::make_lazy(otherwise()),          [](auto&&, auto&& transition) { return transition.parent(); }))
            (src, transition);
    },
    transition.source(), transition);
    // clang-format on
};

constexpr auto makeInvalidGuard
    = [](auto dispatchTable) { return decltype(dispatchTable[0].guard) {}; };

constexpr auto makeInvalidAction
    = [](auto dispatchTable) { return decltype(dispatchTable[0].action) {}; };

constexpr auto resolveEntryAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dst){
        return bh::if_(has_entry_action(dst)
            , [](auto&& dst) { return dst.on_entry(); }
            , [](auto&&) { return [](auto...) {}; })
            (unwrap_typeid(dst));    
    },
    transition.target());
    // clang-format on
};

constexpr auto resolveExitAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src){
        return bh::if_(has_exit_action(src)
            , [](auto&& src) { return src.on_exit(); }
            , [](auto&&) { return [](auto...) {}; })
            (unwrap_typeid(src));
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
            entryAction(resolveEntryAction(transition))](auto&&... params) {
        exitAction(params...);
        action(params...);
        entryAction(params...);
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

                          auto parentState2 = bh::find(statesMap, bh::typeid_(parentState)).value();
                          auto target2 = bh::find(statesMap, bh::typeid_(target)).value();

                          dispatchTable[from] = { to, history, defer, valid, make_transition(action, guard, eventTypeid, parentState2, target2, optionalDependency)};
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

constexpr auto fill_dispatch_table_with_transitions = [](
    auto rootState, auto&& statesMap, auto&& optionalDependency, auto transitions)
{
    auto eventTypeids = collect_event_typeids_recursive_with_transitions(transitions);
    constexpr auto combinedStateTypeids = getCombinedStateTypeids(rootState);
    constexpr StateIdx states = nStates(rootState) * nParentStates(rootState);

    bh::for_each(eventTypeids, [&](auto eventTypeid) {
        using Event = typename decltype(eventTypeid)::type;

        auto filteredTransitions = filter_transitions(transitions, eventTypeid);
        auto& dispatchTable = DispatchTable<states, Event>::table;

        bh::for_each(filteredTransitions, [&](auto transition) {
            addDispatchTableEntry(combinedStateTypeids, transition, dispatchTable, eventTypeid, statesMap, optionalDependency);
            addDispatchTableEntryOfSubMachineExits(combinedStateTypeids, transition, dispatchTable, eventTypeid, statesMap, optionalDependency);
        });
    });

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
    const auto combinedStateTypeids = getCombinedStateTypeids(rootState);
    const auto transitions = getDeferingTransitions(rootState);
    constexpr StateIdx states = nStates(rootState) * nParentStates(rootState);

    bh::for_each(transitions, [&](auto transition) {
        const auto deferredEvents = unwrap_typeid(resolveExtentedInitialState(transition)).defer_events();

        bh::for_each(deferredEvents, [&](auto event) {
            using Event = decltype(event);

            auto& dispatchTable = DispatchTable<states, Event>::table;
            const auto from = getCombinedStateIdx(
                combinedStateTypeids, resolveSrcParent(transition), resolveSrc(transition));
            dispatchTable[from].defer = true;
        });
    });
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