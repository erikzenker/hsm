#pragma once

namespace hsm {

template <class StateFactory> class CreateState {
  public:
    constexpr CreateState(StateFactory stateFactory)
        : m_stateFactory(stateFactory)
    {
    }

    template <class Event, class Source, class Target>
    constexpr auto operator()(Event event, Source source, Target target) const
    {
        *target = m_stateFactory(event, source);
    }

  private:
    StateFactory m_stateFactory;
};

template <class StateFactory> constexpr auto create_state(StateFactory stateFactory)
{
    return CreateState<StateFactory> { stateFactory };
}
}