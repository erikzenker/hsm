#pragma once

namespace hsm {

template <class Action> class ReuseState {
  public:
    constexpr ReuseState(Action action)
        : m_action(action)
    {
    }

    template <class Event, class Source, class Target>
    constexpr auto operator()(Event event, Source source, Target target) const
    {
        m_action(event, source, **target);
    }

  private:
    Action m_action;
};

template <class Action> constexpr auto reuse_state(Action action)
{
    return ReuseState<Action> { action };
}
}