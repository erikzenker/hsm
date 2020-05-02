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
    virtual void executeAction(boost::any eventRef) = 0;
    virtual bool executeGuard(boost::any eventRef) = 0;
};

template <
    class Action,
    class Guard,
    class SourcePtr,
    class TargetPtr,
    class Event,
    class OptionalDependency>
class Transition final : public ITransition {
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

    void executeAction(boost::any eventRef) override
    {
        auto args = bh::concat(
            bh::make_tuple(
                boost::any_cast<std::reference_wrapper<Event>>(eventRef), source, target),
            optionalDependency);

        // clang-format off
        bh::if_(
            is_no_action(action),
            [](auto&&...) {},
            [](auto&& action, auto&& args) {
                bh::unpack(args, [action](auto event, auto source, auto target, auto... optionalDependency){
                    action(event.get(), *source, *target, optionalDependency...);
                });
            })
        (action, args);
        // clang-format on
    }

    bool executeGuard(boost::any eventRef) override
    {
        auto args = bh::concat(
            bh::make_tuple(
                boost::any_cast<std::reference_wrapper<Event>>(eventRef), source, target),
            optionalDependency);

        // clang-format off
        return bh::if_(
            is_no_guard(guard),
            [](auto&&...) { return true; },
            [](auto&& guard, auto&& args) {
                return bh::unpack(args, [guard](auto event, auto source, auto target, auto... optionalDependency){
                    return guard(event.get(), *source, *target, optionalDependency...);
                });
            })
        (guard, args);
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

    return std::make_shared<Transition<
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