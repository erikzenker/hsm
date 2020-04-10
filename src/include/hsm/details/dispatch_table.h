#pragma once

#include "transition_table.h"

#include <array>
#include <functional>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class... Parameters> struct NextState {
    StateIdx combinedState;
    std::function<bool(Parameters...)> guard;
    std::function<void(Parameters...)> action;
    bool history;
    bool defer;
    bool valid = false;
};

template <StateIdx NStates, class... Parameters>
using DispatchArray = std::array<NextState<Parameters...>, NStates>;

template <StateIdx NStates, class... Parameters> struct DispatchTable {
    static DispatchArray<NStates, Parameters...> table;
};

template <StateIdx NStates, class... Parameters>
DispatchArray<NStates, Parameters...> DispatchTable<NStates, Parameters...>::table {};
}