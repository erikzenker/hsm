#pragma once

#include "hsm/details/forwards.h"

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Event> struct event {
    static constexpr bh::type<Event> typeid_ {};

    constexpr auto operator+()
    {
        return bh::make_basic_tuple(event<Event> {}, noGuard {}, noAction {});
    }

    template <class Guard> constexpr auto operator[](const Guard& guard)
    {
        return TransitionEG<event<Event>, Guard> { guard };
    }

    template <class Action> constexpr auto operator/(const Action& guard)
    {
        return TransitionEA<event<Event>, Action> { guard };
    }
};

struct noneEvent {
};

using none = event<noneEvent>;

}