#pragma once

#include <boost/hana.hpp>

#include <map>
#include <functional>

namespace hsm
{
namespace bh {
using namespace boost::hana;
};

template <class Transitions, class State>
auto make_action_map(Transitions transitions, State state ){
    auto events = collect_events_recursive(state);        
    auto actionTypeIds = collect_action_typeids_recursive(state);    

    auto toActionMap = [transitions, actionTypeIds](auto currentActionMap, auto event) {
        
        auto isEvent = [event](auto elem) {
            return boost::hana::equal(
                boost::hana::typeid_(boost::hana::at_c<1>(elem).getEvent()), boost::hana::typeid_(event));
        };

        auto filteredRows = boost::hana::filter(transitions, isEvent);

        std::map<int, std::function<void(decltype(event))>> actions;

        boost::hana::for_each(filteredRows, [&actions, actionTypeIds](auto row){
            auto action = boost::hana::at_c<3>(row);
            auto i =  bh::find(make_index_map(actionTypeIds), bh::typeid_(action)).value();
            actions[i] = action;
        });

        return boost::hana::append(
            currentActionMap, boost::hana::make_pair(boost::hana::typeid_(event), actions));
    };

    auto actionTuple = boost::hana::fold_left(events, boost::hana::make_tuple(), toActionMap);
    auto actionMap = boost::hana::to<boost::hana::map_tag>(actionTuple);

    return actionMap;    

}
}
