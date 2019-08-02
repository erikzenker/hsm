#pragma once

namespace hsm {

template <class ParentState, class State> class Exit {
  public:
    constexpr Exit(ParentState parentState, State state)
        : parentState(parentState)
        , state(state)
    {
    }

    constexpr auto get_parent_state()
    {
        return parentState;
    }

    constexpr auto get_state()
    {
        return state;
    }

  private:
    ParentState parentState;
    State state;
};

}