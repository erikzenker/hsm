#pragma once

#include "hsm/details/event.h"
#include "hsm/details/forwards.h"
#include "hsm/details/transition.h"

namespace hsm {

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

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state<Type> {}, event<noneEvent> {}, noGuard {}, noAction {}, target);
    }

    template <class Source, class Event>
    constexpr auto operator<=(const TransitionSE<Source, Event>&)
    {
        return details::transition(
            state<Source> {}, Event {}, noGuard {}, noAction {}, state<Type> {});
    }

    template <class Source, class Event, class Guard>
    constexpr auto operator<=(const TransitionSEG<Source, Event, Guard>& transitionSeg)
    {
        return details::transition(
            state<Source> {}, Event {}, transitionSeg.guard, noAction {}, state<Type> {});
    }

    template <class Source, class Event, class Action>
    constexpr auto operator<=(const TransitionSEA<Source, Event, Action>& transitionSea)
    {
        return details::transition(
            state<Source> {}, Event {}, noGuard {}, transitionSea.action, state<Type> {});
    }

    template <class Source, class Event, class Guard, class Action>
    constexpr auto operator<=(const TransitionSEGA<Source, Event, Guard, Action>& transitionSega)
    {
        return details::transition(
            state<Source> {},
            Event {},
            transitionSega.guard,
            transitionSega.action,
            state<Type> {});
    }

    template <class Source> constexpr auto operator<=(const state<Source>& source)
    {
        return details::transition(
            source, event<noneEvent> {}, noGuard {}, noAction {}, state<Type> {});
    }

    template <class OtherState> auto operator==(OtherState) -> bool
    {
        return boost::hana::equal(
            boost::hana::type_c<typename OtherState::type>, boost::hana::type_c<Type>);
    }
};

template <class Source> struct state : public StateBase<Source> {
    using StateBase<Source>::operator=;

    constexpr auto operator*()
    {
        return initial<Source> {};
    }
};

template <class Source> struct initial : public StateBase<Initial<state<Source>>> {
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
}