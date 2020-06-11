#pragma once

#include "hsm/details/idx.h"

#include <boost/any.hpp>

#include <array>
#include <functional>
#include <memory>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class T> auto& get(std::reference_wrapper<T> ref)
{
    return ref.get();
}

template <class Event> struct IDispatchTableEntry {
    virtual ~IDispatchTableEntry() = default;
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
class DispatchTableEntry final : public IDispatchTableEntry<Event> {
  public:
    DispatchTableEntry(
        Action action,
        Guard guard,
        const SourcePtr& source,
        const TargetPtr& target,
        OptionalDependency optionalDependency)
        : m_action(action)
        , m_guard(guard)
        , m_source(source)
        , m_target(target)
        , m_optionalDependency(optionalDependency)
    {
    }

    void executeAction(Event& event) override
    {
        // clang-format off
        bh::if_(
            is_no_action(m_action),
            [](auto&&...) {},
            [](auto& action, 
               auto& event, 
               const auto& source, 
               const auto& target, 
               const auto& optionalDependency) {
                bh::unpack(optionalDependency, [&action, &event, &source, &target](const auto&... optionalDependency){
                    action(event, *source, *target, get(optionalDependency)...);
                });
            })
        (m_action, event, m_source, m_target, m_optionalDependency);
        // clang-format on
    }

    bool executeGuard(Event& event) override
    {
        // clang-format off
        return bh::if_(
            is_no_guard(m_guard),
            [](auto&&...) { return true; },
            [](auto& guard,
               auto& event,
               const auto& source,
               const auto& target,
               const auto& optionalDependency) {
                return bh::unpack(
                    optionalDependency,
                    [&guard, &event, &source, &target](const auto&... optionalDependency) {
                        return guard(event, *source, *target, get(optionalDependency)...);
                    });
            })(m_guard, event, m_source, m_target, m_optionalDependency);
        // clang-format on
    }

  private:
    Action m_action;
    Guard m_guard;
    SourcePtr m_source;
    TargetPtr m_target;
    OptionalDependency m_optionalDependency;
};

constexpr auto make_transition = [](auto action,
                                    auto guard,
                                    auto eventTypeid,
                                    auto source,
                                    auto target,
                                    auto optionalDependency) {
    using Event = typename decltype(eventTypeid)::type;

    return std::make_unique<DispatchTableEntry<
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
    std::unique_ptr<IDispatchTableEntry<Event>> transition;
};

template <StateIdx NStates, class Event>
using DispatchArray = std::array<NextState<Event>, NStates>;

template <StateIdx NStates, class Event> struct DispatchTable {
    static DispatchArray<NStates, Event> table;
};

template <StateIdx NStates, class Event>
DispatchArray<NStates, Event> DispatchTable<NStates, Event>::table {};
}