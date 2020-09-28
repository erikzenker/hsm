#pragma once

#include "hsm/details/event.h"
#include "hsm/details/forwards.h"
#include "hsm/details/transition.h"

namespace hsm {

template <class Type> struct StateBase {
    using type = Type;

    template <class Event> constexpr auto operator+(const event_t<Event>&)
    {
        return TransitionSE<Type, event_t<Event>> {};
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

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state_t<Type> {}, event_t<noneEvent> {}, noGuard {}, noAction {}, target);
    }

    template <class Source, class Event>
    constexpr auto operator<=(const TransitionSE<Source, Event>&)
    {
        return details::transition(
            state_t<Source> {}, Event {}, noGuard {}, noAction {}, state_t<Type> {});
    }

    template <class Source, class Event, class Guard>
    constexpr auto operator<=(const TransitionSEG<Source, Event, Guard>& transitionSeg)
    {
        return details::transition(
            state_t<Source> {}, Event {}, transitionSeg.guard, noAction {}, state_t<Type> {});
    }

    template <class Source, class Event, class Action>
    constexpr auto operator<=(const TransitionSEA<Source, Event, Action>& transitionSea)
    {
        return details::transition(
            state_t<Source> {}, Event {}, noGuard {}, transitionSea.action, state_t<Type> {});
    }

    template <class Source, class Event, class Guard, class Action>
    constexpr auto operator<=(const TransitionSEGA<Source, Event, Guard, Action>& transitionSega)
    {
        return details::transition(
            state_t<Source> {},
            Event {},
            transitionSega.guard,
            transitionSega.action,
            state_t<Type> {});
    }

    template <class Source> constexpr auto operator<=(const state_t<Source>& source)
    {
        return details::transition(
            source, event_t<noneEvent> {}, noGuard {}, noAction {}, state_t<Type> {});
    }

    template <class OtherState> auto operator==(OtherState) -> bool
    {
        return boost::hana::equal(
            boost::hana::type_c<typename OtherState::type>, boost::hana::type_c<Type>);
    }
};

template <class Source> struct state_t : public StateBase<Source> {
    using StateBase<Source>::operator=;

    constexpr auto operator*()
    {
        return initial_t<Source> {};
    }
};
template <class Source> state_t<Source> state {};

template <class Source> struct initial_t : public StateBase<Initial<state_t<Source>>> {
    using StateBase<Initial<state_t<Source>>>::operator=;
};
template <class Source> initial_t<Source> initial {};

template <class Parent, class State>
struct direct_t : public StateBase<Direct<state_t<Parent>, state_t<State>>> {
    using StateBase<Direct<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> direct_t<Parent, State> direct {};

template <class Parent, class State>
struct entry_t : public StateBase<Entry<state_t<Parent>, state_t<State>>> {
    using StateBase<Entry<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> entry_t<Parent, State> entry {};

template <class Parent, class State>
struct exit_t : public StateBase<Exit<state_t<Parent>, state_t<State>>> {
    using StateBase<Exit<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> exit_t<Parent, State> exit {};

template <class Parent> struct history_t : public StateBase<History<state_t<Parent>>> {
    using StateBase<History<state_t<Parent>>>::operator=;
};
template <class Parent> history_t<Parent> history {};
}