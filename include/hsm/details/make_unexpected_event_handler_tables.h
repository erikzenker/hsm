#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/dispatch_table.h"
#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/to_map.h"

#include <boost/hana/pair.hpp>
#include <boost/hana/transform.hpp>

#include <memory>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
template <class Event> struct IUnexpectedEventHandler {
    virtual ~IUnexpectedEventHandler() = default;
    virtual void executeHandler(Event& event) = 0;
};

template <class Handler, class CurrentStatePtr, class Event, class OptionalDependency>
class UnexpectedEventHandler final : public IUnexpectedEventHandler<Event> {
  public:
    UnexpectedEventHandler(
        Handler handler, CurrentStatePtr currentState, OptionalDependency optionalDependency)
        : m_handler(handler)
        , m_currentState(std::move(currentState))
        , m_optionalDependency(optionalDependency)
    {
    }

    void executeHandler(Event& event) override
    {
        bh::unpack(m_optionalDependency, [this, &event](const auto&... optionalDependency) {
            m_handler(event, **m_currentState, get(optionalDependency)...);
        });
    }

  private:
    Handler m_handler;
    CurrentStatePtr m_currentState;
    OptionalDependency m_optionalDependency;
};

template <class RootState> constexpr auto make_unexpected_event_handler_tables(RootState rootState)
{
    return to_map([rootState](auto eventTypeids) {
        return bh::transform(eventTypeids, [rootState](auto eventTypeid) {
            return [eventTypeid](auto states) {
                using Event = typename decltype(eventTypeid)::type;
                return bh::make_pair(
                    eventTypeid,
                    std::array<std::unique_ptr<IUnexpectedEventHandler<Event>>, states>());
            }(nStates(rootState) * nParentStates(rootState));
        });
    }(collect_event_typeids_recursive(rootState)));
}
}