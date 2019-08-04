#pragma once

namespace hsm {

template <class Event> struct event {
    auto getEvent() const
    {
        return Event {};
    };
};

struct noneEvent {
};

using none = event<noneEvent>;

}