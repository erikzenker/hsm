#pragma once

#include "hsm/details/event.h"
#include "hsm/details/pseudo_states.h"
#include "hsm/details/state.h"
#include "hsm/details/transition.h"

#include <boost/hana/bool.hpp>
#include <boost/hana/tuple.hpp>

namespace hsm {

template <class Event, class Guard, class Action> class TransitionEGA {
  public:
    constexpr TransitionEGA(const Guard& guard, const Action& action)
        : guard(guard)
        , action(action)
    {
    }

    constexpr auto operator+()
    {
        return details::internal_transition(Event {}, guard, action);
    }

  public:
    const Guard guard;
    const Action action;
};

template <class Event, class Guard> class TransitionEG {
  public:
    constexpr TransitionEG(const Guard& guard)
        : guard(guard)
    {
    }

    template <class Action> constexpr auto operator/(const Action& action)
    {
        return TransitionEGA<Event, Guard, Action> { guard, action };
    }

    constexpr auto operator+()
    {
        return details::internal_transition(Event {}, guard, noAction {});
    }

  public:
    const Guard guard;
};

template <class Event, class Action> class TransitionEA {
  public:
    constexpr TransitionEA(const Action& action)
        : action(action)
    {
    }

    constexpr auto operator+()
    {
        return details::internal_transition(Event {}, noGuard {}, action);
    }

  public:
    const Action action;
};

template <class Source, class Event, class Guard, class Action> class TransitionSEGA {
  public:
    constexpr TransitionSEGA(const Guard& guard, const Action& action)
        : guard(guard)
        , action(action)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(state_t<Source> {}, Event {}, guard, action, target);
    }

    const Guard guard;
    const Action action;
};

template <class Source, class Event, class Guard> class TransitionSEG {
  public:
    constexpr TransitionSEG(const Guard& guard)
        : guard(guard)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(state_t<Source> {}, Event {}, guard, noAction {}, target);
    }

    const Guard guard;
};

template <class Source, class Event, class Action> class TransitionSEA {
  public:
    constexpr TransitionSEA(const Action& action)
        : action(action)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(state_t<Source> {}, Event {}, noGuard {}, action, target);
    }

    const Action action;
};

template <class Source, class Event> class TransitionSE {
  public:
    
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(state_t<Source> {}, Event {}, noGuard {}, noAction {}, target);
    }
};

template <class Source, class Action> class TransitionSA {
  public:
    constexpr TransitionSA(const Action& action)
        : action(action)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state_t<Source> {}, event_t<noneEvent> {}, noGuard {}, action, target);
    }

  private:
    const Action action;
};

template <class Source, class Guard, class Action> class TransitionSGA {
  public:
    constexpr TransitionSGA(const Guard& guard, const Action& action)
        : guard(guard)
        , action(action)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state_t<Source> {}, event_t<noneEvent> {}, guard, action, target);
    }

  private:
    const Guard guard;
    const Action action;
};

template <class Source, class Guard> class TransitionSG {
  public:
    constexpr TransitionSG(const Guard& guard)
        : guard(guard)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state_t<Source> {}, event_t<noneEvent> {}, guard, noAction {}, target);
    }

    template <class Action> constexpr auto operator/(const Action& action)
    {
        return TransitionSGA<Source, Guard, Action> { guard, action };
    }

  private:
    const Guard guard;
};

}