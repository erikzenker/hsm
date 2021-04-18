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

template <class T> auto get(std::reference_wrapper<T> ref) -> auto&
{
    return ref.get();
}

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
template <class Event> struct IDispatchTableEntry {
    virtual ~IDispatchTableEntry() = default;
    virtual void executeAction(Event& event) = 0;
    virtual auto executeGuard(Event& event) -> bool = 0;
};

template <
    bool Internal,
    class Action,
    class Guard,
    class SourcePtr,
    class TargetPtr,
    class Event,
    class OptionalDependency>
class DispatchTableEntry final : public IDispatchTableEntry<Event> {
  public:
    constexpr DispatchTableEntry(
        Action action,
        Guard guard,
        SourcePtr source,
        TargetPtr target,
        OptionalDependency optionalDependency)
        : m_action(action)
        , m_guard(guard)
        , m_source(std::move(source))
        , m_target(std::move(target))
        , m_optionalDependency(optionalDependency)
    {
    }

    void executeAction(Event& event) override
    {
        // clang-format off
        if constexpr(is_action<Action>()){
            [](auto& action, 
               auto& event, 
               const auto& source, 
               auto& target, 
               const auto& optionalDependency) {
                bh::unpack(optionalDependency, 
                    [&action, &event, &source, &target](const auto&... optionalDependency){
                    using Target = typename TargetPtr::element_type::element_type;
                    (void) target;
                    if constexpr(is_default_constructable(bh::type_c<Target>)){
                        if constexpr (Internal){
                            action(event, **source, **source, get(optionalDependency)...);
                        }
                        else 
                        {
                            action(event, **source, **target, get(optionalDependency)...);
                        }
                    }
                    else {
                        action(event, **source, target, get(optionalDependency)...);
                    }
                });
            }(m_action, event, m_source, m_target, m_optionalDependency);
        }
        // clang-format on
    }

    auto executeGuard(Event& event) -> bool override
    {
        // clang-format off
        if constexpr(is_guard<Guard>()){
            return [](
               auto& guard,
               auto& event,
               const auto& source,
               const auto& target,
               const auto& optionalDependency) {
                return bh::unpack(
                    optionalDependency,
                    [&guard, &event, &source, &target](const auto&... optionalDependency) {
                        (void) target;    
                        if constexpr (Internal)
                        {
                            return guard(event, **source, **source, get(optionalDependency)...);
                        }
                        else
                        {
                            return guard(event, **source, **target, get(optionalDependency)...);
                        }
                        
                    });
            }(m_guard, event, m_source, m_target, m_optionalDependency);
        }
        else {
            return true;
        }
        // clang-format on
    }

  private:
    Action m_action;
    Guard m_guard;
    SourcePtr m_source;
    TargetPtr m_target;
    OptionalDependency m_optionalDependency;
};

template <
    class Transition,
    class Action,
    class Guard,
    class EventTypeid,
    class Source,
    class Target,
    class Dependency>
constexpr auto make_transition(
    Transition transition,
    Action action,
    Guard guard,
    EventTypeid eventTypeid,
    Source source,
    Target target,
    Dependency optionalDependency)
{
    using Event = typename decltype(eventTypeid)::type;

    return std::make_unique<DispatchTableEntry<
        transition.internal(),
        Action,
        Guard,
        Source,
        Target,
        Event,
        decltype(optionalDependency)>>(action, guard, source, target, optionalDependency);
}

template <class Event> struct NextState {
    StateIdx combinedState {};
    bool history {};
    bool defer {};
    bool valid = false;
    bool internal = false;
    std::unique_ptr<IDispatchTableEntry<Event>> transition;
};
}