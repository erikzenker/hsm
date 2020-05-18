#pragma once

#include "hsm/details/pseudo_states.h"

#include <boost/hana/bool.hpp>
#include <boost/hana/tuple.hpp>

namespace hsm {

struct noneEvent {
};

struct noAction {
};

struct noGuard {
};

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

    template <class Target> constexpr auto operator=(const Target& target)
    {
        return boost::hana::make_tuple(state<Source> {}, Event {}, guard, action, target);
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

    template <class Target> constexpr auto operator=(const Target& target)
    {
        return boost::hana::make_tuple(state<Source> {}, Event {}, guard, noAction {}, target);
    }

    const Guard guard;
};

template <class Source, class Event, class Action> class TransitionSEA {
  public:
    constexpr TransitionSEA(const Action& action)
        : action(action)
    {
    }

    template <class Target> constexpr auto operator=(const Target& target)
    {
        return boost::hana::make_tuple(state<Source> {}, Event {}, noGuard {}, action, target);
    }

    const Action action;
};

template <class Source, class Event> class TransitionSE {
  public:
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return boost::hana::make_tuple(state<Source> {}, Event {}, noGuard {}, noAction {}, target);
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

template <class Source, class Action> class TransitionSA {
  public:
    constexpr TransitionSA(const Action& action)
        : action(action)
    {
    }

    template <class Target> constexpr auto operator=(const Target& target)
    {
        return boost::hana::make_tuple(
            state<Source> {}, event<noneEvent> {}, noGuard {}, action, target);
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

    template <class Target> constexpr auto operator=(const Target& target)
    {
        return boost::hana::make_tuple(
            state<Source> {}, event<noneEvent> {}, guard, action, target);
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

    template <class Target> constexpr auto operator=(const Target& target)
    {
        return boost::hana::make_tuple(
            state<Source> {}, event<noneEvent> {}, guard, noAction {}, target);
    }

    template <class Action> constexpr auto operator/(const Action& action)
    {
        return TransitionSGA<Source, Guard, Action> { guard, action };
    }

  private:
    const Guard guard;
};

template <class Type> struct StateBase {
    using type = Type;

    template <class Event> constexpr auto operator+(const event<Event>&)
    {
        return TransitionSE<Type, event<Event>> {};
    }

    template <class Event, class Guard>
    constexpr auto operator+(const TransitionEG<Event, Guard>& transition)
    {
        return TransitionSEG<Type, Event, Guard> { transition.guard };
    }

    template <class Event, class Action>
    constexpr auto operator+(const TransitionEA<Event, Action>& transition)
    {
        return TransitionSEA<Type, Event, Action> { transition.action };
    }

    template <class Event, class Guard, class Action>
    constexpr auto operator+(const TransitionEGA<Event, Guard, Action>& transition)
    {
        return TransitionSEGA<Type, Event, Guard, Action> { transition.guard, transition.action };
    }

    template <class Action> constexpr auto operator/(const Action& action)
    {
        return TransitionSA<Type, Action> { action };
    }

    template <class Guard> constexpr auto operator[](const Guard& guard)
    {
        return TransitionSG<Type, Guard> { guard };
    }

    template <class Target> constexpr auto operator=(const Target& target)
    {
        return boost::hana::make_tuple(
            state<Type> {}, event<noneEvent> {}, noGuard {}, noAction {}, target);
    }

    template <class Source, class Event>
    constexpr auto operator<=(const TransitionSE<Source, Event>& transition)
    {
        return boost::hana::make_tuple(
            state<Source> {}, Event {}, noGuard {}, noAction {}, state<Type> {});
    }

    template <class Source, class Event, class Guard>
    constexpr auto operator<=(const TransitionSEG<Source, Event, Guard>& transition)
    {
        return boost::hana::make_tuple(
            state<Source> {}, Event {}, transition.guard, noAction {}, state<Type> {});
    }

    template <class Source, class Event, class Action>
    constexpr auto operator<=(const TransitionSEA<Source, Event, Action>& transition)
    {
        return boost::hana::make_tuple(
            state<Source> {}, Event {}, noGuard {}, transition.action, state<Type> {});
    }

    template <class Source, class Event, class Guard, class Action>
    constexpr auto operator<=(const TransitionSEGA<Source, Event, Guard, Action>& transition)
    {
        return boost::hana::make_tuple(
            state<Source> {}, Event {}, transition.guard, transition.action, state<Type> {});
    }

    template <class Source> constexpr auto operator<=(const state<Source>& source)
    {
        return boost::hana::make_tuple(
            source, event<noneEvent> {}, noGuard {}, noAction {}, state<Type> {});
    }

    template <class OtherState> bool operator==(OtherState)
    {
        return boost::hana::equal(
            boost::hana::type_c<typename OtherState::type>, boost::hana::type_c<Type>);
    }
};

template <class Source> struct istate;

template <class Source> struct state : public StateBase<Source> {
    using StateBase<Source>::operator=;

    constexpr auto operator*()
    {
        return istate<Source> {};
    }
};

template <class Source> struct istate : public StateBase<Initial<state<Source>>> {
    using StateBase<Initial<state<Source>>>::operator=;
};
template <class Parent, class State>
struct direct : public StateBase<Direct<state<Parent>, state<State>>> {
    using StateBase<Direct<state<Parent>, state<State>>>::operator=;
};
template <class Parent, class State>
struct entry : public StateBase<Entry<state<Parent>, state<State>>> {
    using StateBase<Entry<state<Parent>, state<State>>>::operator=;
};
template <class Parent, class State>
struct exit : public StateBase<Exit<state<Parent>, state<State>>> {
    using StateBase<Exit<state<Parent>, state<State>>>::operator=;
};
template <class Parent> struct history : public StateBase<History<state<Parent>>> {
    using StateBase<History<state<Parent>>>::operator=;
};

using none = event<noneEvent>;

}