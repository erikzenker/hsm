#pragma once

#include <boost/hana.hpp>

namespace hsm {

template <class Event> struct event {
    static constexpr boost::hana::type<Event> typeid_{};
};

struct noneEvent {
};

using none = event<noneEvent>;

}