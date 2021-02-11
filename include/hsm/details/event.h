#pragma once

#include "hsm/details/forwards.h"
#include "hsm/details/pseudo_states.h"
#include "hsm/details/transition.h"

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Event> struct event_t {
    static constexpr bh::type<Event> typeid_ {};

    constexpr auto operator+() const
    {
        return details::internal_transition(event_t<Event> {}, noGuard {}, noAction {});
    }

    template <class Guard> constexpr auto operator[](const Guard& guard) const
    {
        return TransitionEG<event_t<Event>, Guard> { guard };
    }

    template <class Action> constexpr auto operator/(const Action& guard) const
    {
        return TransitionEA<event_t<Event>, Action> { guard };
    }
};

template <class Event> const event_t<Event> event {};

struct noneEvent {
};

using none = event_t<noneEvent>;
}