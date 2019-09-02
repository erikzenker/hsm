
#pragma once

#include "flatten_transition_table.h"

#include <boost/hana.hpp>

#include <map>
#include <functional>

namespace hsm
{
namespace bh {
using namespace boost::hana;
};

template <class State> auto make_action_map(State state)
{
    auto transitions = flatten_sub_transition_table2(state);
    auto eventTypeids = collect_events_typeids_recursive(state);        
    auto actionTypeIds = collect_action_typeids_recursive(state);    

    auto toActionMap = [transitions, actionTypeIds](auto currentActionMap, auto eventTypeid) {
        
        auto isEvent = [eventTypeid](auto elem) {
            return boost::hana::equal(boost::hana::at_c<2>(elem).typeid_, eventTypeid);
        };

        auto filteredRows = boost::hana::filter(transitions, isEvent);

        using EventType = typename decltype(eventTypeid)::type;

        std::map<int, std::function<void(EventType)>> actions;

        boost::hana::for_each(filteredRows, [&actions, actionTypeIds](auto row){
            auto action = boost::hana::at_c<4>(row);
            auto i =  bh::find(make_index_map(actionTypeIds), bh::typeid_(action)).value();
            actions[i] = action;
        });

        return boost::hana::append(
            currentActionMap, boost::hana::make_pair(eventTypeid, actions));
    };

    auto actionTuple = boost::hana::fold_left(eventTypeids, boost::hana::make_tuple(), toActionMap);
    auto actionMap = boost::hana::to<boost::hana::map_tag>(actionTuple);

    return actionMap;
}

template <class State> auto make_guard_map(State state)
{
    auto transitions = flatten_sub_transition_table2(state);
    auto eventTypeids = collect_events_typeids_recursive(state);
    auto guardTypeIds = collect_guard_typeids_recursive(state);

    auto toGuardMap = [transitions, guardTypeIds](auto currentGuardMap, auto eventTypeid) {
        auto isEvent = [eventTypeid](auto elem) {
            return boost::hana::equal(
                boost::hana::at_c<2>(elem).typeid_,
                eventTypeid);
        };

        auto filteredRows = boost::hana::filter(transitions, isEvent);

        using EventType = typename decltype(eventTypeid)::type;

        std::map<int, std::function<bool(EventType)>> guards;

        boost::hana::for_each(filteredRows, [&guards, guardTypeIds](auto row) {
            auto guard = boost::hana::at_c<3>(row);
            auto i = bh::find(make_index_map(guardTypeIds), bh::typeid_(guard)).value();
            guards[i] = guard;
        });

        return boost::hana::append(
            currentGuardMap, boost::hana::make_pair(eventTypeid, guards));
    };

    auto guardTuple = boost::hana::fold_left(eventTypeids, boost::hana::make_tuple(), toGuardMap);
    auto guardMap = boost::hana::to<boost::hana::map_tag>(guardTuple);

    return guardMap;
}
}
