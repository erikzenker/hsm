#pragma once

#include <boost/hana.hpp>

namespace hsm {

namespace {
constexpr auto defaultGuard = [](auto...) { return true; };
constexpr auto defaultAction = [](auto...) {};
}

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
    Guard guard;
    Action action;
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
    Guard guard;
};

template <class Source, class Event, class Guard, class Action> class TransitionSEGA {
    const Guard guard;
    const Action action;

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
};

template <class Source, class Event, class Guard> class TransitionSEG {
    const Guard guard;

  public:
    constexpr TransitionSEG(const Guard& guard)
        : guard(guard)
    {
    }

    template <class Target> constexpr auto operator=(const state<Target>&)
    {
        return boost::hana::make_tuple(Source {}, Event {}, guard, defaultAction, Target {});
    }
};

template <class Source, class Event> class TransitionSE {
  public:
    template <class Target> constexpr auto operator=(const state<Target>&)
    {
        return boost::hana::make_tuple(Source {}, Event {}, defaultGuard, defaultAction, Target {});
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
};

struct noneEvent {
};

using none = event<noneEvent>;
}