#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/dispatch_table.h"
#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/to_map.h"

#include <boost/hana/pair.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class RootState> constexpr auto make_dispatch_tables(RootState rootState)
{
    return to_map([rootState](auto eventTypeids) {
        return bh::transform(eventTypeids, [rootState](auto eventTypeid) {
            return [eventTypeid](auto states) {
                using Event = typename decltype(eventTypeid)::type;
                return bh::make_pair(eventTypeid, std::array<NextState<Event>, states>());
            }(nStates(rootState) * nParentStates(rootState));
        });
    }(collect_event_typeids_recursive(rootState)));
}
}