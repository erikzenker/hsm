#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/dispatch_table.h"
#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/make_unexpected_event_handler_tables.h"
#include "hsm/details/to_map.h"
#include "hsm/details/utils/dunique_ptr.h"

#include <boost/hana/pair.hpp>
#include <boost/hana/transform.hpp>

#include <memory>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Handler, class CurrentStatePtr, class EventTypeid, class OptionalDependency>
constexpr auto make_unexpected_event_handler(
    Handler handler,
    CurrentStatePtr currentStatePtr,
    EventTypeid eventTypeid,
    OptionalDependency optionalDependency)
{
    using Event = typename decltype(eventTypeid)::type;

    return hsm::details::utils::dunique_ptr<IUnexpectedEventHandler<Event>>(
        new UnexpectedEventHandler<Handler, CurrentStatePtr, Event, OptionalDependency>(
            handler, currentStatePtr, optionalDependency));
}

template <
    class RootState,
    class StatesMap,
    class UnexpectedEventHandlerTables,
    class UnexpectedEventHandler,
    class OptionalDependency>
constexpr auto fill_unexpected_event_handler_tables(
    RootState rootState,
    StatesMap statesMap,
    UnexpectedEventHandlerTables& unexpectedEventHandlerTables,
    UnexpectedEventHandler unexpectedEventHandler,
    OptionalDependency optionalDependency)
{
    [=, &unexpectedEventHandlerTables](auto eventTypeids) {
        bh::for_each(eventTypeids, [=, &unexpectedEventHandlerTables](auto eventTypeid) {
            bh::for_each(
                collect_parent_state_typeids(rootState),
                [=, &unexpectedEventHandlerTables](auto parentStateTypeid) {
                    bh::for_each(
                        collect_state_typeids_recursive(rootState),
                        [=, &unexpectedEventHandlerTables](auto stateTypeid) {
                            [=, &unexpectedEventHandlerTables](
                                auto combinedStateTypeids, auto mappedCurrentState) {
                                [=, &unexpectedEventHandlerTables](auto combinedStateId) {
                                    unexpectedEventHandlerTables[eventTypeid][combinedStateId]
                                        = make_unexpected_event_handler(
                                            unexpectedEventHandler,
                                            mappedCurrentState,
                                            eventTypeid,
                                            optionalDependency);
                                }(getCombinedStateIdx(
                                    combinedStateTypeids, parentStateTypeid, stateTypeid));
                            }(getCombinedStateTypeids(rootState),
                              bh::find(statesMap, stateTypeid).value());
                        });
                });
        });
    }(collect_event_typeids_recursive(rootState));
}
}