#pragma once

#include "transition_table.h"

#include <boost/any.hpp>

#include <array>
#include <functional>
#include <memory>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Event> struct ITransition {
    virtual ~ITransition() = default;
    virtual void executeAction(Event& event) = 0;
    virtual bool executeGuard(Event& event) = 0;
};

template <
    class Action,
    class Guard,
    class SourcePtr,
    class TargetPtr,
    class Event,
    class OptionalDependency>
class Transition final : public ITransition<Event> {
  public:
    Transition(
        Action action,
        Guard guard,
        SourcePtr source,
        TargetPtr target,
        OptionalDependency optionalDependency)
        : action(action)
        , guard(guard)
        , source(source)
        , target(target)
        , optionalDependency(optionalDependency)
    {
    }

    void executeAction(Event& event) override
    {
        // clang-format off
        bh::if_(
            is_no_action(action),
            [](auto&&...) {},
            [](auto&& action, auto&& event, auto&& source, auto&& target, auto&& optionalDependency) {
                bh::unpack(optionalDependency, [&action, &event, &source, &target](auto... optionalDependency){
                    action(event, *source, *target, optionalDependency...);
                });
            })
        (action, event, source, target, optionalDependency);
        // clang-format on
    }

    bool executeGuard(Event& event) override
    {
        // clang-format off
        return bh::if_(
            is_no_guard(guard),
            [](auto&&...) { return true; },
            [](auto&& guard,
               auto&& event,
               auto&& source,
               auto&& target,
               auto&& optionalDependency) {
                return bh::unpack(
                    optionalDependency,
                    [&guard, &event, &source, &target](auto... optionalDependency) {
                        return guard(event, *source, *target, optionalDependency...);
                    });
            })(guard, event, source, target, optionalDependency);
        // clang-format on
    }

  private:
    Action action;
    Guard guard;
    SourcePtr source;
    TargetPtr target;
    OptionalDependency optionalDependency;
};

auto make_transition = [](auto action,
                          auto guard,
                          auto eventTypeid,
                          auto source,
                          auto target,
                          auto optionalDependency) {
    using Event = typename decltype(eventTypeid)::type;

    return std::make_unique<Transition<
        decltype(action),
        decltype(guard),
        decltype(source),
        decltype(target),
        Event,
        decltype(optionalDependency)>>(action, guard, source, target, optionalDependency);
};

template <class Event> struct NextState {
    StateIdx combinedState;
    bool history;
    bool defer;
    bool valid = false;
    std::unique_ptr<ITransition<Event>> transition;
};

template <StateIdx NStates, class Event>
using DispatchArray = std::array<NextState<Event>, NStates>;

template <StateIdx NStates, class Event> struct DispatchTable {
    static DispatchArray<NStates, Event> table;
};

template <StateIdx NStates, class Event>
DispatchArray<NStates, Event> DispatchTable<NStates, Event>::table {};
}