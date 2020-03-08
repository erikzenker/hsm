#pragma once

namespace hsm {

template <class ParentState, class State> class PseudoState {
  public:
    constexpr PseudoState(ParentState parentState, State state)
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

class ExitPseudoState {
};
class EntryPseudoState {
};
class DirectPseudoState {
};
class HistoryPseudoState {
};

template <class ParentState, class State>
class Exit final : public PseudoState<ParentState, State>, public ExitPseudoState {
  public:
    constexpr Exit(ParentState parentState, State state)
        : PseudoState<ParentState, State>(parentState, state)
    {
    }
};

template <class ParentState, class State>
class Entry : public PseudoState<ParentState, State>, public EntryPseudoState {
  public:
    constexpr Entry(ParentState parentState, State state)
        : PseudoState<ParentState, State>(parentState, state)
    {
    }
};

template <class ParentState, class State>
class Direct : public PseudoState<ParentState, State>, public DirectPseudoState {
  public:
    constexpr Direct(ParentState parentState, State state)
        : PseudoState<ParentState, State>(parentState, state)
    {
    }
};

template <class ParentState> class History : public HistoryPseudoState {
  public:
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