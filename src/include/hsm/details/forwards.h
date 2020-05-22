#pragma once

namespace hsm {
template <class Event> struct event;
struct noneEvent;
struct noAction;
struct noGuard;

template <class Event, class Guard> class TransitionEG;
template <class Event, class Action> class TransitionEA;
template <class Event, class Guard, class Action> class TransitionEGA;
template <class Source, class Action> class TransitionSA;
template <class Source, class Guard> class TransitionSG;
template <class Source, class Event> class TransitionSE;
template <class Source, class Event, class Guard> class TransitionSEG;
template <class Source, class Event, class Action> class TransitionSEA;
template <class Source, class Event, class Guard, class Action> class TransitionSEGA;

template <class Source> struct initial;
}