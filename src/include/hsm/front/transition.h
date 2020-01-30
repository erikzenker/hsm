#pragma once

#include <boost/hana.hpp>

namespace hsm {

struct noneEvent {
};

struct noAction {
};

struct noGuard {
};

template <class Source> class state;
template <class Event> struct event;

template <class Event, class Guard, class Action> class TransitionEGA {
  public:
    constexpr TransitionEGA(const Guard& guard, const Action& action)
        : guard(guard)
        , action(action)
    {
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

  public:
    const Guard guard;
};

template <class Event, class Action> class TransitionEA {
  public:
    constexpr TransitionEA(const Action& action)
        : action(action)
    {
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

    template <class Target> constexpr auto operator=(const state<Target>&)
    {
        return boost::hana::make_tuple(Source {}, Event {}, guard, action, Target {});
    }

  private:
    const Guard guard;
    const Action action;
};

template <class Source, class Event, class Guard> class TransitionSEG {
  public:
    constexpr TransitionSEG(const Guard& guard)
        : guard(guard)
    {
    }

    template <class Target> constexpr auto operator=(const state<Target>&)
    {
        return boost::hana::make_tuple(Source {}, Event {}, guard, noAction{}, Target {});
    }

  private:
    const Guard guard;
};

template <class Source, class Event, class Action> class TransitionSEA {
  public:
    constexpr TransitionSEA(const Action& action)
        : action(action)
    {
    }

    template <class Target> constexpr auto operator=(const state<Target>&)
    {
        return boost::hana::make_tuple(Source {}, Event {}, noGuard{}, action, Target {});
    }

  private:
    const Action action;
};

template <class Source, class Event> class TransitionSE {
  public:
    template <class Target> constexpr auto operator=(const state<Target>&)
    {
        return boost::hana::make_tuple(Source {}, Event {}, noGuard{}, noAction{}, Target {});
    }
};

template <class Source> class state {
  public:
    template <class Event> constexpr auto operator+(const event<Event>&)
    {
        return TransitionSE<Source, event<Event>> {};
    }

    template <class Event, class Guard>
    constexpr auto operator+(const TransitionEG<Event, Guard>& transition)
    {
        return TransitionSEG<Source, Event, Guard> { transition.guard };
    }

    template <class Event, class Action>
    constexpr auto operator+(const TransitionEA<Event, Action>& transition)
    {
        return TransitionSEA<Source, Event, Action> { transition.action };
    }

    template <class Event, class Guard, class Action>
    constexpr auto operator+(const TransitionEGA<Event, Guard, Action>& transition)
    {
        return TransitionSEGA<Source, Event, Guard, Action> { transition.guard, transition.action };
    }
};

template <class Event> struct event {
    static constexpr boost::hana::type<Event> typeid_ {};

    template <class Guard> constexpr auto operator[](const Guard& guard)
    {
        return TransitionEG<event<Event>, Guard> { guard };
    }

    template <class Action> constexpr auto operator/(const Action& guard)
    {
        return TransitionEA<event<Event>, Action> { guard };
    }
};

using none = event<noneEvent>;

}