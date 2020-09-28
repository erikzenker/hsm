#pragma once

namespace hsm {

struct noAction {
};

struct noGuard {
};

template <class Source> struct state_t;

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
class InitialPseudoState {
};

template <class ParentState, class State>
class Exit final : public PseudoState<ParentState, State>, public ExitPseudoState {
  public:
    constexpr Exit()
        : PseudoState<ParentState, State>(ParentState {}, State {})
    {
    }
};

template <class ParentState, class State>
class Entry final : public PseudoState<ParentState, State>, public EntryPseudoState {
  public:
    constexpr Entry()
        : PseudoState<ParentState, State>(ParentState {}, State {})
    {
    }
};

template <class ParentState, class State>
class Direct final : public PseudoState<ParentState, State>, public DirectPseudoState {
  public:
    constexpr Direct()
        : PseudoState<ParentState, State>(ParentState {}, State {})
    {
    }
};

template <class ParentState> class History final : public HistoryPseudoState {
  public:
    constexpr History()
        : parentState(ParentState {})
    {
    }

    constexpr auto get_parent_state()
    {
        return parentState;
    }

  private:
    ParentState parentState;
};

template <class State> class Initial final : public InitialPseudoState {
  public:
    constexpr Initial()
        : state(State {})
    {
    }

    constexpr auto get_state()
    {
        return state;
    }

  private:
    State state;
};
}