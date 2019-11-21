#pragma once

namespace hsm {

template <class ParentState, class State> class Exit {
  public:
    bool isExitState = true;

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

template <class ParentState, class State> class Entry {
  public:
    bool isEntryState = true;

    constexpr Entry(ParentState parentState, State state)
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

template <class ParentState, class State> class Direct {
  public:
    bool isDirectState = true;

    constexpr Direct(ParentState parentState, State state)
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

template <class ParentState> class History {
  public:
    bool isHistoryState = true;

    constexpr History(ParentState parentState)
        : parentState(parentState)
    {
    }

    constexpr auto get_parent_state()
    {
        return parentState;
    }

  private:
    ParentState parentState;
};
}