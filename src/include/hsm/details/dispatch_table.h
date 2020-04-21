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

struct ITransition {
    virtual void executeAction(boost::any event) = 0;
    virtual bool executeGuard(boost::any event) = 0;
};

template <
    class Action,
    class Guard,
    class Source,
    class Target,
    class Event,
    class OptionalDependency>
class Transition final : public ITransition {
  public:
    Transition(Action action, Guard guard, OptionalDependency optionalDependency)
        : action(action)
        , guard(guard)
        , source(Source {})
        , target(Target {})
        , optionalDependency(optionalDependency)
    {
    }

    void executeAction(boost::any event) override
    {
        auto args = bh::concat(
            bh::make_tuple(boost::any_cast<Event>(event), source, target), optionalDependency);

        // clang-format off
        bh::if_(
            is_no_action(action),
            [](auto&&...) {},
            [](auto&& action, auto&& args) {
                bh::unpack(args, action);
            })
        (action, args);
        // clang-format on
    }

    bool executeGuard(boost::any event) override
    {
        auto args = bh::concat(
            bh::make_tuple(boost::any_cast<Event>(event), source, target), optionalDependency);

        // clang-format off
        return bh::if_(
            is_no_guard(guard),
            [](auto&&...) { return true; },
            [](auto&& guard, auto&& args) {
                return bh::unpack(args, guard);
            })
        (guard, args);
        // clang-format on
    }

  private:
    Action action;
    Guard guard;
    Source source;
    Target target;
    OptionalDependency optionalDependency;
};

auto make_transition = [](auto action,
                          auto guard,
                          auto eventTypeid,
                          auto source,
                          auto target,
                          auto optionalDependency) {
    using Event = typename decltype(eventTypeid)::type;
    return std::make_shared<Transition<
        decltype(action),
        decltype(guard),
        typename decltype(source)::type,
        typename decltype(target)::type,
        Event,
        decltype(optionalDependency)>>(action, guard, optionalDependency);
};

template <class Event> struct NextState {
    StateIdx combinedState;
    bool history;
    bool defer;
    bool valid = false;
    std::shared_ptr<ITransition> transition;
};

template <StateIdx NStates, class Event>
using DispatchArray = std::array<NextState<Event>, NStates>;

template <StateIdx NStates, class Event> struct DispatchTable {
    static DispatchArray<NStates, Event> table;
};

template <StateIdx NStates, class Event>
DispatchArray<NStates, Event> DispatchTable<NStates, Event>::table {};
}