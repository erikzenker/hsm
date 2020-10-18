#pragma once

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/unpack.hpp>

namespace hsm {

/*
 * Chain abitrary number of actions to be used in the single transition
 *
 * @param[in] actions actions that will be chained
 */
constexpr auto chain_actions = [](auto... actions) {
    constexpr auto actionsTpl = boost::hana::make_basic_tuple(actions...);

    return [actions(actionsTpl)](auto... args) {
        auto argsTpl = boost::hana::make_basic_tuple(args...);

        boost::hana::for_each(actions, [args(argsTpl)](auto action) {
            boost::hana::unpack(args, [action(action)](auto... args) { action(args...); });
        });
    };
};

/*
 * Shortcut for chain_actions
 */
constexpr auto chain = chain_actions;
}

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

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/eval.hpp>
#include <boost/hana/find_if.hpp>
#include <boost/hana/functional/always.hpp>
#include <boost/hana/functional/compose.hpp>
#include <boost/hana/lazy.hpp>
#include <boost/hana/pair.hpp>

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }

    constexpr auto function = bh::second;
    constexpr auto predicate = bh::first;
    constexpr auto case_ = bh::make_pair;

    constexpr auto otherwise = bh::always(bh::true_c);
    constexpr auto switch_ = [](auto&&... cases_) {
        return function(bh::find_if(bh::make_basic_tuple(cases_...), predicate).value());
    };

    constexpr auto lazy_predicate = bh::compose(bh::eval, bh::first);
    constexpr auto lazy_otherwise = bh::make_lazy(bh::always(bh::true_c));
    constexpr auto lazy_switch_ = [](auto&&... cases_) {
        return function(bh::find_if(bh::make_basic_tuple(cases_...), lazy_predicate).value());
    };
}

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

#include <boost/hana/bool.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/functional/compose.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/or.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

#include <functional>
#include <memory>
#include <utility>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace details {
constexpr auto has_internal_transition_table
    = bh::is_valid([](auto&& state) -> decltype(state.make_internal_transition_table()) {});

}

constexpr auto is_default_constructable
    = bh::is_valid([](auto typeid_) -> decltype(typename decltype(typeid_)::type()) {});

constexpr auto is_custom_target_action
    = bh::is_valid([](auto action, auto... args) -> decltype(action(args...)) {});

auto const is_callable = [](auto&& callable, auto&& args) {
    return bh::unpack(args, bh::is_valid([](auto&&... args) -> decltype(callable(args...)) {}));
};

constexpr auto get_parent_state = [](auto state) {
    return decltype(std::declval<typename decltype(state)::type>().get_parent_state())();
};

constexpr auto get_state = [](auto state) {
    return decltype(std::declval<typename decltype(state)::type>().get_state())();
};

constexpr auto unwrap_typeid_to_shared_ptr = [](auto typeid_) {
    return bh::if_(
        is_default_constructable(typeid_),
        [](auto typeid_) {
            using UnwrappedType = typename decltype(typeid_)::type;
            return std::make_shared<std::unique_ptr<UnwrappedType>>(
                std::make_unique<UnwrappedType>());
        },
        [](auto typeid_) {
            using UnwrappedType = typename decltype(typeid_)::type;
            return std::make_shared<std::unique_ptr<UnwrappedType>>(nullptr);
        })(typeid_);
};

constexpr auto make_transition_table = [](auto t) {
    return decltype(std::declval<typename decltype(t)::type>().make_transition_table())();
};

constexpr auto make_internal_transition_table
    = [](auto state) { return decltype(state)::type::make_internal_transition_table(); };

constexpr auto make_transition_table2
    = [](auto state) { return decltype(state)::type::make_transition_table(); };

constexpr auto has_transition_table = bh::is_valid(
    [](auto stateTypeid) -> decltype(std::declval<typename decltype(stateTypeid)::type>()
                                         .make_transition_table()) {});

constexpr auto has_internal_transition_table = bh::is_valid(
    [](auto stateTypeid) -> decltype(std::declval<typename decltype(stateTypeid)::type>()
                                         .make_internal_transition_table()) {});

constexpr auto has_entry_action = bh::is_valid(
    [](auto stateTypeid) -> decltype(
                             std::declval<typename decltype(stateTypeid)::type>().on_entry()) {});

constexpr auto has_exit_action = bh::is_valid(
    [](auto stateTypeid) -> decltype(
                             std::declval<typename decltype(stateTypeid)::type>().on_exit()) {});

constexpr auto has_unexpected_event_handler = bh::is_valid(
    [](auto stateTypeid)
        -> decltype(std::declval<typename decltype(stateTypeid)::type>().on_unexpected_event()) {});

constexpr auto has_deferred_events = bh::is_valid(
    [](auto stateTypeid)
        -> decltype(std::declval<typename decltype(stateTypeid)::type>().defer_events()) {});

constexpr auto is_exit_state = [](auto stateTypeid) {
    return bh::equal(
        bh::bool_c<std::is_base_of<ExitPseudoState, typename decltype(stateTypeid)::type>::value>,
        bh::true_c);
};

constexpr auto is_entry_state = [](auto stateTypeid) {
    return bh::equal(
        bh::bool_c<std::is_base_of<EntryPseudoState, typename decltype(stateTypeid)::type>::value>,
        bh::true_c);
};

constexpr auto is_direct_state = [](auto stateTypeid) {
    return bh::equal(
        bh::bool_c<std::is_base_of<DirectPseudoState, typename decltype(stateTypeid)::type>::value>,
        bh::true_c);
};

constexpr auto is_history_state = [](auto stateTypeid) {
    return bh::equal(
        bh::bool_c<
            std::is_base_of<HistoryPseudoState, typename decltype(stateTypeid)::type>::value>,
        bh::true_c);
};

constexpr auto is_initial_state = [](auto typeid_) {
    return bh::equal(
        bh::bool_c<std::is_base_of<InitialPseudoState, typename decltype(typeid_)::type>::value>,
        bh::true_c);
};

constexpr auto is_no_action
    = [](auto action) { return bh::equal(bh::typeid_(action), bh::typeid_(noAction {})); };

constexpr auto is_no_guard
    = [](auto guard) { return bh::equal(bh::typeid_(guard), bh::typeid_(noGuard {})); };

constexpr auto is_event = bh::is_valid([](auto&& event) -> decltype(event.typeid_) {});

constexpr auto get_entry_action
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::on_entry(); };
constexpr auto get_exit_action
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::on_exit(); };
constexpr auto get_defer_events
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::defer_events(); };
const auto get_unexpected_event_handler = [](auto rootState) {
    return bh::if_(
        has_unexpected_event_handler(rootState),
        [](auto rootState) { return decltype(rootState)::type::on_unexpected_event(); },
        [](auto) { return [](auto /*event*/) {}; })(rootState);
};
}

#include <type_traits>

namespace hsm {
namespace details {
template <class Source, class Event, class Guard, class Action, class Target> struct Transition {
    constexpr Transition(Source, Event, Guard guard, Action action, Target)
        : m_guard(guard)
        , m_action(action)
    {
    }

    [[nodiscard]] constexpr auto source() const -> Source
    {
        return Source {};
    }

    [[nodiscard]] constexpr auto event() const -> Event
    {
        return Event {};
    }

    [[nodiscard]] constexpr auto action() const -> Action
    {
        return m_action;
    }

    [[nodiscard]] constexpr auto guard() const -> Guard
    {
        return m_guard;
    }

    [[nodiscard]] constexpr auto target() const -> Target
    {
        return Target {};
    }

  private:
    const Guard m_guard;
    const Action m_action;
};

template <class Event, class Guard, class Action> struct InternalTransition {
    constexpr InternalTransition(Event, Guard guard, Action action)
        : m_guard(guard)
        , m_action(action)
    {
    }

    constexpr auto event() const -> Event
    {
        return Event {};
    }

    constexpr auto action() const -> Action
    {
        return m_action;
    }

    constexpr auto guard() const -> Guard
    {
        return m_guard;
    }

  private:
    const Guard m_guard;
    const Action m_action;
};

template <class Parent, class Source, class Event, class Guard, class Action, class Target>
struct ExtendedTransition {
    constexpr ExtendedTransition(
        Parent, Source, Event, Guard guard, Action action, Target)
        : m_guard(guard)
        , m_action(action)
    {
    }

    [[nodiscard]] constexpr auto parent() const -> Parent
    {
        return Parent {};
    }

    [[nodiscard]] constexpr auto source() const -> Source
    {
        return Source {};
    }

    [[nodiscard]] constexpr auto event() const -> Event
    {
        return Event {};
    }

    [[nodiscard]] constexpr auto action() const -> Action
    {
        return m_action;
    }

    [[nodiscard]] constexpr auto guard() const -> Guard
    {
        return m_guard;
    }

    [[nodiscard]] constexpr auto target() const -> Target
    {
        return Target {};
    }

  private:
    const Guard m_guard;
    const Action m_action;
};

constexpr auto transition
    = [](auto&&... xs) { return Transition<std::decay_t<decltype(xs)>...> { xs... }; };

constexpr auto internal_transition
    = [](auto&&... xs) { return InternalTransition<std::decay_t<decltype(xs)>...> { xs... }; };

constexpr auto extended_transition = [](auto&& parent, auto&& transition) {
    return ExtendedTransition<
        std::decay_t<decltype(parent)>,
        std::decay_t<decltype(transition.source())>,
        std::decay_t<decltype(transition.event())>,
        std::decay_t<decltype(transition.guard())>,
        std::decay_t<decltype(transition.action())>,
        std::decay_t<decltype(transition.target())>> { parent,
                                                       transition.source(),
                                                       transition.event(),
                                                       transition.guard(),
                                                       transition.action(),
                                                       transition.target() };
};
}
}

#include <boost/hana/back.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/prepend.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
template <class State> constexpr auto flatten_sub_transition_table(State state);

template <class State> constexpr auto resolve(State target)
{
    // clang-format off
        return lazy_switch_(
            case_(bh::make_lazy(is_entry_state(target)),   [](auto&& entry) { return get_parent_state(entry); }),
            case_(bh::make_lazy(is_direct_state(target)),  [](auto&& direct) { return get_parent_state(direct); }),
            case_(bh::make_lazy(is_history_state(target)), [](auto&& history) { return get_parent_state(history); }),
            case_(bh::make_lazy(otherwise()),              [](auto&& state) { return state; }))
            (target);
    // clang-format on
}
}

template <class State> constexpr auto flatten_transition_table(State state)
{
    auto flattenSubTransitionTable = [state](auto transition) {
        return bh::prepend(
            flatten_sub_transition_table(resolve(transition.target())),
            details::extended_transition(state, transition));
    };

    constexpr auto transitionTable = make_transition_table2(state);
    constexpr auto extendedTransitionTable
        = bh::transform(transitionTable, flattenSubTransitionTable);
    return bh::flatten(extendedTransitionTable);
}

namespace {

template <class State> constexpr auto flatten_sub_transition_table(State state)
{
    // clang-format off
    return bh::if_(
        has_transition_table(state),
        [](auto stateWithTransitionTable) {
            return flatten_transition_table(stateWithTransitionTable);
        },
        [](auto) { return bh::make_basic_tuple(); })(state);
    // clang-format on
}
}
}

#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
constexpr auto to_pair = [](auto x) { return bh::make_pair(x, x); };

constexpr auto to_type_pair = [](auto x) { return bh::make_pair(bh::typeid_(x), x); };
}

constexpr auto remove_duplicates = [](auto tuple, auto predicate) {
    return bh::values(bh::to_map(bh::transform(tuple, predicate)));
};

constexpr auto remove_duplicate_typeids
    = [](auto tuple) { return remove_duplicates(tuple, to_pair); };

constexpr auto remove_duplicate_types
    = [](auto tuple) { return remove_duplicates(tuple, to_type_pair); };
}

#include <boost/hana/at.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}
constexpr auto to_pairs = [](const auto& tuples) {
    return bh::transform(
        tuples, [](auto tuple) { return bh::make_pair(bh::at_c<0>(tuple), bh::at_c<1>(tuple)); });
};
}

#include <boost/hana/at.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {

constexpr auto resolveInitialState = [](auto transition) {
    return bh::if_(
        is_initial_state(transition.source()),
        [](auto initial) { return get_state(initial); },
        [](auto source) { return source; })(transition.source());
};

constexpr auto resolveExtentedInitialState = [](auto transition) {
    return bh::if_(
        is_initial_state(transition.source()),
        [](auto initial) { return get_state(initial); },
        [](auto source) { return source; })(transition.source());
};

constexpr auto extractExtendedStateTypeids = [](auto transition) {
    return bh::make_basic_tuple(
        bh::typeid_(resolveExtentedInitialState(transition)), bh::typeid_(transition.target()));
};
constexpr auto extractExtendedStates = [](auto transition) {
    return bh::make_basic_tuple(resolveExtentedInitialState(transition), transition.target());
};
const auto extractStates = [](auto transition) {
    return bh::make_basic_tuple(transition.source(), transition.target());
};
constexpr auto extractStateTypeids = [](auto transition) {
    return bh::make_basic_tuple(
        bh::typeid_(resolveInitialState(transition)), bh::typeid_(transition.target()));
};
}

constexpr auto collect_child_state_typeids_recursive = [](auto parentState) {
    auto transitions = flatten_transition_table(parentState);
    auto collectedStates = bh::flatten(bh::transform(transitions, extractExtendedStateTypeids));

    return remove_duplicate_typeids(collectedStates);
};

constexpr auto collect_child_states_recursive = [](auto parentState) {
    auto transitions = flatten_transition_table(parentState);
    auto collectedStates = bh::flatten(bh::transform(transitions, extractExtendedStates));

    return collectedStates;
};

constexpr auto collect_state_typeids_recursive = [](auto&& parentState) {
    auto collectedStates
        = bh::append(collect_child_state_typeids_recursive(parentState), bh::typeid_(parentState));
    return collectedStates;
};

constexpr auto collect_states_recursive = [](auto&& parentState) {
    auto collectedStates = bh::append(collect_child_states_recursive(parentState), parentState);
    return remove_duplicate_types(collectedStates);
};

const auto collect_child_state_typeids = [](auto&& state) {
    auto transitions = make_transition_table2(state);
    auto collectedStates = bh::flatten(bh::transform(transitions, extractStateTypeids));

    return remove_duplicate_typeids(collectedStates);
};

constexpr auto collect_child_states = [](auto&& state) {
    return remove_duplicate_types(
        bh::flatten(bh::transform(make_transition_table2(state), extractStates)));
};
}

#include <boost/hana/append.hpp>
#include <boost/hana/at.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/prepend.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto isNotEmpty
    = [](const auto& tuple) { return bh::not_(bh::equal(bh::size_c<0>, bh::size(tuple))); };

constexpr auto get_internal_transition_table = [](auto state) {
    return bh::if_(
        has_internal_transition_table(state),
        [](auto parentState) {
            auto internalTransitionTable = make_internal_transition_table(parentState);
            return bh::transform(internalTransitionTable, [parentState](auto internalTransition) {
                return details::extended_transition(
                    parentState,
                    details::transition(
                        parentState,
                        internalTransition.event(),
                        internalTransition.guard(),
                        internalTransition.action(),
                        parentState));
            });
        },
        [](auto) { return bh::make_basic_tuple(); })(state);
};

constexpr auto extend_internal_transition = [](auto internalTransition, auto state) {
    return details::extended_transition(
        internalTransition.parent(),
        details::transition(
            state,
            internalTransition.event(),
            internalTransition.guard(),
            internalTransition.action(),
            state));
};

constexpr auto flatten_internal_transition_table = [](auto parentState) {
    auto states = collect_states_recursive(parentState);

    auto internalTransitionTables = bh::transform(states, get_internal_transition_table);
    auto internalTransitions = bh::flatten(bh::filter(internalTransitionTables, isNotEmpty));

    return bh::to<bh::basic_tuple_tag>(
        bh::flatten(bh::transform(internalTransitions, [states](auto transition) {
            return bh::transform(states, [transition](auto state) {
                return extend_internal_transition(transition, state);
            });
        })));
};

} // namespace hsm

#include <boost/hana/at.hpp>
#include <boost/hana/concat.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/tuple.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
constexpr auto collectEventTypeids = [](auto transition) { return transition.event().typeid_; };
constexpr auto collectEvents = [](auto transition) {
    using Event = typename decltype(transition.event().typeid_)::type;
    return bh::tuple_t<Event>;
};
}

constexpr auto collect_event_typeids_recursive = [](auto state) {
    return remove_duplicate_typeids(bh::transform(
        bh::concat(flatten_transition_table(state), flatten_internal_transition_table(state)),
        collectEventTypeids));
};

constexpr auto collect_event_typeids_recursive_with_transitions = [](auto transitions) {
    return remove_duplicate_typeids(bh::transform(transitions, collectEventTypeids));
};

constexpr auto collect_events_recursive = [](auto state) {
    return remove_duplicate_types(bh::flatten(bh::transform(
        bh::concat(flatten_transition_table(state), flatten_internal_transition_table(state)),
        collectEvents)));
};
}

#include <boost/hana/for_each.hpp>

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }

constexpr auto for_each_idx = [](const auto& list, const auto& closure){
    std::size_t index = 0;
    bh::for_each(list, [closure, &index](const auto& elem){
        closure(elem, index);
        index++;
    });
};
}

#include <boost/hana/at.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
const auto collectGuard = [](auto transition) { return bh::at_c<3>(transition); };
}

const auto collect_guards_recursive = [](auto state) {
    auto collectedGuards = bh::transform(flatten_transition_table(state), collectGuard);
    return remove_duplicate_types(collectedGuards);
};

const auto collect_guard_typeids_recursive = [](auto state) {
    return bh::transform(collect_guards_recursive(state), bh::typeid_);
};
}

#include <boost/hana/front.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto collect_parent_state_typeids = [](auto state) {
    auto toParentStateTypeid = [](auto transition) { return bh::typeid_(transition.parent()); };

    auto transitions = flatten_transition_table(state);
    auto parentStateTypeids = bh::transform(transitions, toParentStateTypeid);
    return remove_duplicate_typeids(parentStateTypeids);
};
}

namespace hsm {
template <class Event> struct event_t;
struct noneEvent;
struct noAction;
struct noGuard;

template <class Event, class Guard> class TransitionEG;
template <class Event, class Action> class TransitionEA;
template <class Event, class Guard, class Action> class TransitionEGA;
template <class Source, class Action> class TransitionSA;
template <class Source, class Guard> class TransitionSG;
template <class Source, class Event> class TransitionSE;
template <class Source, class Event, class Guard> class TransitionSEG;
template <class Source, class Event, class Action> class TransitionSEA;
template <class Source, class Event, class Guard, class Action> class TransitionSEGA;

template <class Source> struct initial_t;
}

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Event> struct event_t {
    static constexpr bh::type<Event> typeid_ {};

    constexpr auto operator+()
    {
        return details::internal_transition(event_t<Event> {}, noGuard {}, noAction {});
    }

    template <class Guard> constexpr auto operator[](const Guard& guard)
    {
        return TransitionEG<event_t<Event>, Guard> { guard };
    }

    template <class Action> constexpr auto operator/(const Action& guard)
    {
        return TransitionEA<event_t<Event>, Action> { guard };
    }
};

template <class Event> event_t<Event> event {};

struct noneEvent {
};

using none = event_t<noneEvent>;
}

#include <boost/hana/drop_while.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/minus.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/zip.hpp>

//#include <boost/mp11.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto index_of = [](auto const& iterable, auto const& element) {
    auto size = decltype(bh::size(iterable)){};
    auto dropped = decltype(bh::size(
        bh::drop_while(iterable, bh::not_equal.to(element))
    )){};
    return size - dropped;
};

const auto make_index_map = [](auto typeids) {
    const auto range = bh::to<bh::basic_tuple_tag>(bh::make_range(bh::int_c<0>, bh::size(typeids)));
    return bh::to_map(to_pairs(bh::zip(typeids, range)));
};

// constexpr auto find = [](auto&& reverseIndexMap, auto index, auto&& closure) {
//     boost::mp11::mp_with_index<bh::size(reverseIndexMap)>(
//         index,
//         [reverseIndexMap, &closure](auto i) { closure(bh::find(reverseIndexMap, i).value()); });
// };

// constexpr auto make_reverse_index_map = [](auto&& tuple) {
//     auto range = bh::to<bh::basic_tuple_tag>(bh::make_range(bh::int_c<0>, bh::size(tuple)));
//     auto ids
//         = bh::transform(range, [](auto&& element) { return boost::mp11::mp_size_t<element> {}; });

//     return bh::to_map(to_pairs(bh::zip(ids, tuple)));
// };
}

#include <boost/hana/at.hpp>
#include <boost/hana/cartesian_product.hpp>
#include <boost/hana/find.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/type.hpp>

#include <cstdint>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

using Idx = std::uint16_t;
using StateIdx = Idx;
using EventIdx = Idx;
using ActionIdx = Idx;
using GuardIdx = Idx;

constexpr auto getIdx = [](auto map, auto type) -> Idx { return bh::find(map, type).value(); };

constexpr auto getParentStateIdx = [](auto rootState, auto parentState) {
    return index_of(collect_parent_state_typeids(rootState), bh::typeid_(parentState));
};

constexpr auto getStateIdx = [](auto rootState, auto state) {
    return index_of(collect_state_typeids_recursive(rootState), bh::typeid_(state));
};

constexpr auto getCombinedStateTypeids = [](const auto& rootState) {
    auto parentStateTypeids = collect_parent_state_typeids(rootState);
    auto stateTypeids = collect_state_typeids_recursive(rootState);
    auto stateCartesianProduct
        = bh::cartesian_product(bh::make_basic_tuple(parentStateTypeids, stateTypeids));
    return bh::transform(stateCartesianProduct, bh::typeid_);
};

constexpr auto getCombinedStateTypeid = [](const auto& parentState, const auto& state) {
    return bh::typeid_(bh::make_basic_tuple(bh::typeid_(parentState), bh::typeid_(state)));
};

constexpr auto getCombinedStateIdx = [](auto combinedStateTypids, auto parentState, auto state) {
    constexpr auto combinedStateTypeid = getCombinedStateTypeid(parentState, state);
    return index_of(combinedStateTypids, combinedStateTypeid);
};

constexpr auto calcCombinedStateIdx
    = [](std::size_t nStates, Idx parentStateIdx, Idx stateIdx) -> Idx {
    return (parentStateIdx * nStates) + stateIdx;
};

constexpr auto calcParentStateIdx
    = [](std::size_t nStates, Idx combinedState) -> Idx { return combinedState / nStates; };

constexpr auto calcStateIdx
    = [](std::size_t nStates, Idx combinedState) -> Idx { return combinedState % nStates; };

constexpr auto getEventIdx = [](auto rootState, auto event) {
    auto takeWrappedEvent = [](auto event) { return event.typeid_; };
    auto takeEvent = [](auto event) { return bh::typeid_(event); };
    auto eventId = bh::if_(is_event(event), takeWrappedEvent, takeEvent)(event);

    return index_of(collect_event_typeids_recursive(rootState), eventId);
};

constexpr auto getActionIdx = [](auto rootState, auto action) {
    return index_of(collect_action_typeids_recursive(rootState), bh::typeid_(action));
};

constexpr auto getGuardIdx = [](auto rootState, auto guard) {
    return index_of(collect_guard_typeids_recursive(rootState), bh::typeid_(guard));
};

const auto is_anonymous_transition
    = [](auto transition) { return bh::typeid_(transition.event()) == bh::typeid_(none {}); };

const auto is_history_transition
    = [](auto transition) { return is_history_state(transition.target()); };

const auto has_anonymous_transition = [](auto rootState) {
    auto transitions = flatten_transition_table(rootState);
    auto anonymousTransition = bh::filter(transitions, is_anonymous_transition);
    return bh::size(anonymousTransition);
};

const auto has_history = [](auto rootState) {
    auto transitions = flatten_transition_table(rootState);
    auto historyTransitions = bh::filter(transitions, is_history_transition);
    return bh::size(historyTransitions);
};
}

#include <boost/hana/find.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/maximum.hpp>

#include <iostream>
#include <vector>

namespace hsm {

/**
 * Collect the initial states for the parent states
 *
 * Returns: [[State]]
 *
 */
constexpr auto collect_initial_states = [](auto parentState) {
    constexpr auto childStates = collect_child_states(parentState);
    constexpr auto initialStates = bh::filter(childStates, is_initial_state);
    return bh::transform(initialStates, [](auto initialState) { return get_state(initialState); });
};

/**
 * Collect the initial states for the parent states
 * and returns it as tuple of combined state idx.
 *
 * Returns: [[StateIdx]]
 *
 * Example: [[0,1], [0], [1], [1,2]]
 */
constexpr auto collect_initial_state_stateidx = [](auto rootState, auto parentStateTypeids) {
    return bh::transform(parentStateTypeids, [rootState](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;

        constexpr auto initialStates = collect_initial_states(ParentState {});
        constexpr auto initialStatesStateIdx
            = bh::transform(initialStates, [rootState](auto initialState) {
                  return getCombinedStateIdx(
                      getCombinedStateTypeids(rootState), ParentState {}, initialState);
              });

        return initialStatesStateIdx;
    });
};

/**
 * Return a map from parent state id to inital state ids
 *
 * Returns: (ParentStateIdx -> [StateIdx])
 *
 * Example:
 * [[0 -> [0, 1]],
 *  [1 -> [3, 1]],
 *  [2 -> [0, 2]]]
 */
constexpr auto make_initial_state_map = [](auto rootState) {
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    constexpr auto initialStates = collect_initial_state_stateidx(rootState, parentStateTypeids);
    return bh::to_map(to_pairs(bh::zip(parentStateTypeids, initialStates)));
};

/**
 * Fills the initial state table with the state idx of the initial
 * states.
 *
 * Parameters:
 *  initialStateTable : [[StateIdx]]
 *  initialStateTable[parentStateIdx].size() is the number regions
 *  in the parent state.
 *  initialStateTable[parentStateIdx][regionIdx] is a state idx.
 */
constexpr auto fill_initial_state_table = [](auto rootState, auto& initialStateTable) {
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    for_each_idx(
        parentStateTypeids,
        [rootState, &initialStateTable](auto parentStateTypeid, auto parentStateId) {
            constexpr auto initialStates
                = bh::find(make_initial_state_map(rootState), parentStateTypeid).value();
            auto initialStatesStateIdx = std::vector<std::size_t>(bh::size(initialStates));

            for_each_idx(initialStates, [&initialStatesStateIdx](auto stateIdx, auto regionId) {
                initialStatesStateIdx[regionId] = stateIdx;
            });

            initialStateTable.at(parentStateId) = initialStatesStateIdx;
        });
};

/**
 * Returns a tuple of initial state sizes
 *
 * Returns: [std::size_t]
 *
 * Example: [3, 1, 2]
 */
constexpr auto initialStateSizes = [](auto parentStateTypeids) {
    return bh::transform(parentStateTypeids, [](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;
        return bh::size(collect_initial_states(ParentState {}));
    });
};

/**
 * Returns the maximal number of initial states
 */
constexpr auto maxInitialStates = [](auto rootState) {
    return bh::maximum(initialStateSizes(collect_parent_state_typeids(rootState)));
};
}

#include <boost/any.hpp>

#include <array>
#include <functional>
#include <memory>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class T> auto get(std::reference_wrapper<T> ref) -> auto&
{
    return ref.get();
}

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)    
template <class Event> struct IDispatchTableEntry {
    virtual ~IDispatchTableEntry() = default;
    virtual void executeAction(Event& event) = 0;
    virtual auto executeGuard(Event& event) -> bool = 0;
};

template <
    class Action,
    class Guard,
    class SourcePtr,
    class TargetPtr,
    class Event,
    class OptionalDependency>
class DispatchTableEntry final : public IDispatchTableEntry<Event> {
  public:
    DispatchTableEntry(
        Action action,
        Guard guard,
        SourcePtr  source,
        TargetPtr  target,
        OptionalDependency optionalDependency)
        : m_action(action)
        , m_guard(guard)
        , m_source(std::move(source))
        , m_target(std::move(target))
        , m_optionalDependency(optionalDependency)
    {
    }

    void executeAction(Event& event) override
    {
        // clang-format off
        bh::if_(
            is_no_action(m_action),
            [](auto&&...) {},
            [](auto& action, 
               auto& event, 
               const auto& source, 
               auto& target, 
               const auto& optionalDependency) {
                bh::unpack(optionalDependency, [&action, &event, &source, &target](const auto&... optionalDependency){
                    using Target = typename TargetPtr::element_type::element_type;
                    bh::if_(is_default_constructable(bh::type_c<Target>),
                        [](auto action, auto &event, auto source, auto target, auto... optionalDependency){
                            action(event, **source, **target, get(optionalDependency)...);    
                        },
                        [](auto action, auto &event, auto source, auto &target, auto... optionalDependency){
                            action(event, **source, target, get(optionalDependency)...);    
                        })(action, event, source, target, optionalDependency...);
                });
            })
        (m_action, event, m_source, m_target, m_optionalDependency);
        // clang-format on
    }

    auto executeGuard(Event& event) -> bool override
    {
        // clang-format off
        return bh::if_(
            is_no_guard(m_guard),
            [](auto&&...) { return true; },
            [](auto& guard,
               auto& event,
               const auto& source,
               const auto& target,
               const auto& optionalDependency) {
                return bh::unpack(
                    optionalDependency,
                    [&guard, &event, &source, &target](const auto&... optionalDependency) {
                        return guard(event, **source, **target, get(optionalDependency)...);
                    });
            })(m_guard, event, m_source, m_target, m_optionalDependency);
        // clang-format on
    }

  private:
    Action m_action;
    Guard m_guard;
    SourcePtr m_source;
    TargetPtr m_target;
    OptionalDependency m_optionalDependency;
};

constexpr auto make_transition = [](auto action,
                                    auto guard,
                                    auto eventTypeid,
                                    auto source,
                                    auto target,
                                    auto optionalDependency) {
    using Event = typename decltype(eventTypeid)::type;

    return std::make_unique<DispatchTableEntry<
        decltype(action),
        decltype(guard),
        decltype(source),
        decltype(target),
        Event,
        decltype(optionalDependency)>>(action, guard, source, target, optionalDependency);
};

template <class Event> struct NextState {
    StateIdx combinedState{};
    bool history{};
    bool defer{};
    bool valid = false;
    std::unique_ptr<IDispatchTableEntry<Event>> transition;
};

template <StateIdx NStates, class Event>
using DispatchArray = std::array<NextState<Event>, NStates>;

template <StateIdx NStates, class Event> struct DispatchTable {
    static DispatchArray<NStates, Event> table;
};

template <StateIdx NStates, class Event>
DispatchArray<NStates, Event> DispatchTable<NStates, Event>::table {};
}

#include <boost/hana/bool.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/or.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto has_substate_initial_state_entry_action = [](auto&& target) {
    return bh::if_(
        has_transition_table(target),
        [](auto&& target) { return has_entry_action(bh::at_c<0>(collect_initial_states(target))); },
        [](auto&&) { return bh::false_c; })(target);
};

constexpr auto has_action = [](auto&& transition) {
    return bh::or_(
        bh::not_(is_no_action(transition.action())),
        has_entry_action(transition.target()),
        has_substate_initial_state_entry_action(transition.target()),
        has_exit_action(transition.source()));
};
}

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto resolveDst = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dstTypeid){
        return lazy_switch_(
            // TODO: make multi region capable    
            case_(bh::make_lazy(has_transition_table(dstTypeid)), [](auto&& submachine) { return bh::at_c<0>(collect_initial_states(submachine));}),
            case_(bh::make_lazy(is_entry_state(dstTypeid)),       [](auto&& entry) { return get_state(entry); }),
            case_(bh::make_lazy(is_direct_state(dstTypeid)),      [](auto&& direct) { return get_state(direct); }),
            // TODO: make multi region capable 
            case_(bh::make_lazy(is_history_state(dstTypeid)),     [](auto&& history) { return bh::at_c<0>(collect_initial_states(get_parent_state(history)));}),
            case_(bh::make_lazy((otherwise())),                   [](auto&& dstTypeid) { return dstTypeid; }))
            (dstTypeid);
    },
    transition.target());
    // clang-format on
};

constexpr auto resolveDstParent = [](auto transition) {
    // clang-format off
    return bh::apply([](auto&& dstTypeid, auto&& transition){
        return lazy_switch_(
            case_(bh::make_lazy(has_transition_table(dstTypeid)), [](auto&& submachine, auto&&) { return submachine; }),
            case_(bh::make_lazy(is_entry_state(dstTypeid)),       [](auto&& entry, auto&&) { return get_parent_state(entry); }),
            case_(bh::make_lazy(is_direct_state(dstTypeid)),      [](auto&& direct, auto&&) { return get_parent_state(direct); }),
            case_(bh::make_lazy(is_history_state(dstTypeid)),     [](auto&& history, auto&&) { return get_parent_state(history); }),
            case_(bh::make_lazy(otherwise()),                     [](auto&&, auto&& transition) { return transition.parent(); }))
            (dstTypeid, transition);
    },
    transition.target(), transition);
    // clang-format on
};

constexpr auto resolveSrc = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src){
        return lazy_switch_(
            case_(bh::make_lazy(is_initial_state(src)), [](auto&& initial) { return get_state(initial); }),    
            case_(bh::make_lazy(is_exit_state(src)),    [](auto&& exit) { return get_state(exit); }),
            case_(bh::make_lazy(is_direct_state(src)),  [](auto&& direct) { return get_state(direct); }),
            case_(bh::make_lazy(otherwise()),           [](auto&& state) { return state; }))
            (src);    
    },
    transition.source());
    // clang-format on
};

constexpr auto resolveSrcParent = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src, auto&& transition){
        return lazy_switch_(
            case_(bh::make_lazy(is_exit_state(src)),   [](auto&& exit, auto&&) { return get_parent_state(exit); }),
            case_(bh::make_lazy(is_direct_state(src)), [](auto&& direct, auto&&) { return get_parent_state(direct); }),
            case_(bh::make_lazy(otherwise()),          [](auto&&, auto&& transition) { return transition.parent(); }))
            (src, transition);
    },
    transition.source(), transition);
    // clang-format on
};

}

#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/find.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/lazy.hpp>
#include <boost/hana/length.hpp>
#include <boost/hana/mult.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto nParentStates
    = [](auto rootState) { return bh::length(collect_parent_state_typeids(rootState)); };
constexpr auto nStates
    = [](auto rootState) { return bh::length(collect_state_typeids_recursive(rootState)); };
constexpr auto nEvents
    = [](auto rootState) { return bh::length(collect_event_typeids_recursive(rootState)); };

constexpr auto hasRegions
    = [](auto rootState) { return bh::equal(bh::size_c<1>, maxInitialStates(rootState)); };

constexpr auto makeInvalidGuard
    = [](auto dispatchTable) { return decltype(dispatchTable[0].guard) {}; };

constexpr auto makeInvalidAction
    = [](auto dispatchTable) { return decltype(dispatchTable[0].action) {}; };

constexpr auto resolveEntryAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dst){
        return bh::if_(has_entry_action(dst)
            , [](auto&& dst) { return get_entry_action(dst); }
            , [](auto&&) { return [](auto...) {}; })
            (dst);    
    },
    transition.target());
    // clang-format on
};

constexpr auto resolveInitialStateEntryAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& target){
        return bh::if_(has_substate_initial_state_entry_action(target)
            , [](auto&& target) { return get_entry_action(bh::at_c<0>(collect_initial_states(target)));}
            , [](auto&&) { return [](auto...) {}; })
            (target);    
    },
    transition.target());
    // clang-format on
};

constexpr auto resolveExitAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src){
        return bh::if_(has_exit_action(src)
            , [](auto&& src) { return get_exit_action(src); }
            , [](auto&&) { return [](auto...) {}; })
            (src);
    },
    transition.source());
    // clang-format on
};

constexpr auto resolveNoAction = [](auto&& transition) {
    return bh::if_(
        is_no_action(transition.action()),
        [](auto&&) { return [](auto&&...) {}; },
        [](auto&& transition) { return transition.action(); })(transition);
};

constexpr auto resolveEntryExitAction = [](auto&& transition) {
    return [exitAction(resolveExitAction(transition)),
            action(resolveNoAction(transition)),
            entryAction(resolveEntryAction(transition)),
            initialStateEntryAction(resolveInitialStateEntryAction(transition))](auto&&... params) {
        exitAction(params...);
        action(params...);
        entryAction(params...);
        initialStateEntryAction(params...);
    };
};

constexpr auto resolveAction = [](auto&& transition) {
    // clang-format off
    return bh::if_(
        has_action(transition),
        [](auto&& transition) { return resolveEntryExitAction(transition);},
        [](auto&& transition) { return transition.action(); })
        (transition);
    // clang-format on
};

constexpr auto resolveHistory = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dst){
        return bh::if_(is_history_state(dst)
            , [](auto&&) { return true; }
            , [](auto&&) { return false; })
            (dst);
    }, 
    transition.target());
    // clang-format on                   
};

constexpr auto addDispatchTableEntry = [](auto&& combinedStateTypids, auto&& transition, auto& dispatchTable, auto eventTypeid, auto&& statesMap, auto optionalDependency) {
          const auto source = resolveSrc(transition);
          const auto target = resolveDst(transition);
          const auto from = getCombinedStateIdx(combinedStateTypids, resolveSrcParent(transition), source);
          const auto guard = transition.guard();
          const auto action = resolveAction(transition);
          const auto to = getCombinedStateIdx(combinedStateTypids, resolveDstParent(transition), target);
          const auto history = resolveHistory(transition);
          const auto defer = false;
          const auto valid = true;

          auto mappedSource = bh::find(statesMap, bh::typeid_(source)).value();
          auto mappedTarget = bh::find(statesMap, bh::typeid_(target)).value();

          dispatchTable[from] = { to, history, defer, valid, make_transition(action, guard, eventTypeid, mappedSource, mappedTarget, optionalDependency)};
      };

const auto addDispatchTableEntryOfSubMachineExits
    = [](auto&& combinedStateTypids, auto&& transition, auto& dispatchTable, auto&& eventTypeid, auto&& statesMap, auto optionalDependency) {
          bh::if_(
              has_transition_table(transition.source()),
              [&](auto parentState) {
                  auto states = collect_child_state_typeids(parentState);

                  bh::for_each(
                      states, [&](auto state) {
                          const auto target = resolveDst(transition);                          
                          const auto from = getCombinedStateIdx(combinedStateTypids, parentState, state);
                          const auto guard = transition.guard();
                          const auto action = resolveAction(transition);
                          const auto to = getCombinedStateIdx(
                              combinedStateTypids, resolveDstParent(transition), target);
                          const auto history = resolveHistory(transition);
                          const auto defer = false;
                          const auto valid = true;

                          auto mappedParentState = bh::find(statesMap, bh::typeid_(parentState)).value();
                          auto mappedTarget = bh::find(statesMap, bh::typeid_(target)).value();

                          dispatchTable[from] = { to, history, defer, valid, make_transition(action, guard, eventTypeid, mappedParentState, mappedTarget, optionalDependency)};
                      });
              },
              [](auto) {})(transition.source());
      };

constexpr auto filter_transitions = [](auto transitions, auto eventTypeid) {
    auto isEvent = [eventTypeid](auto transition) {
        return bh::equal(transition.event().typeid_, eventTypeid);
    };

    return bh::filter(transitions, isEvent);
};

constexpr auto fill_dispatch_table_with_transitions = [](
    auto rootState, auto&& statesMap, auto&& optionalDependency, auto transitions)
{
    auto eventTypeids = collect_event_typeids_recursive_with_transitions(transitions);
    constexpr auto combinedStateTypeids = getCombinedStateTypeids(rootState);
    constexpr StateIdx states = nStates(rootState) * nParentStates(rootState);

    bh::for_each(eventTypeids, [&](auto eventTypeid) {
        using Event = typename decltype(eventTypeid)::type;

        auto filteredTransitions = filter_transitions(transitions, eventTypeid);
        auto& dispatchTable = DispatchTable<states, Event>::table;

        bh::for_each(filteredTransitions, [&](auto transition) {
            addDispatchTableEntry(combinedStateTypeids, transition, dispatchTable, eventTypeid, statesMap, optionalDependency);
            addDispatchTableEntryOfSubMachineExits(combinedStateTypeids, transition, dispatchTable, eventTypeid, statesMap, optionalDependency);
        });
    });

};

constexpr auto getDeferingTransitions = [](auto rootState) {
    constexpr auto transitionHasDeferedEvents
        = [](auto transition) { return has_deferred_events(resolveExtentedInitialState(transition)); };

    constexpr auto transitions = flatten_transition_table(rootState);
    return bh::filter(transitions, transitionHasDeferedEvents);
};

constexpr auto hasDeferedEvents
    = [](auto rootState) { return bh::size(getDeferingTransitions(rootState)); };

template <class RootState, class OptionalDependency>
constexpr auto
fill_dispatch_table_with_deferred_events(RootState rootState, OptionalDependency /*optionalDependency*/)
{
    const auto combinedStateTypeids = getCombinedStateTypeids(rootState);
    const auto transitions = getDeferingTransitions(rootState);
    constexpr StateIdx states = nStates(rootState) * nParentStates(rootState);

    bh::for_each(transitions, [&](auto transition) {
        const auto deferredEvents = get_defer_events(resolveExtentedInitialState(transition));

        bh::for_each(deferredEvents, [&](auto event) {
            using Event = typename decltype(event)::type;

            auto& dispatchTable = DispatchTable<states, Event>::table;
            const auto from = getCombinedStateIdx(
                combinedStateTypeids, resolveSrcParent(transition), resolveSrc(transition));
            dispatchTable[from].defer = true;
        });
    });
}

template <class RootState, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_external_transitions(
    const RootState& rootState, StatesMap&& statesMap, OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions(
        rootState, statesMap, optionalDependecy, flatten_transition_table(rootState));
}

template <class RootState, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_internal_transitions(
    const RootState& rootState, StatesMap&& statesMap, OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions(
        rootState, statesMap, optionalDependecy, flatten_internal_transition_table(rootState));
}
}

#include <boost/hana/append.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/zip.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

/***
 * Returns a map from typeid(state<State>) -> State
 * of all states found recursive under parentState
 *
 */
constexpr auto make_states_map = [](auto&& parentState) {
    auto collectedStateTypeids = collect_state_typeids_recursive(parentState);
    auto collectedStates = bh::transform(
        remove_duplicate_types(
            bh::append(collect_child_states_recursive(parentState), parentState)),
        unwrap_typeid_to_shared_ptr);
    return bh::to_map(to_pairs(bh::zip(collectedStateTypeids, collectedStates)));
};
}

#include <boost/hana/type.hpp>
#include <boost/hana/unpack.hpp>

#include <stdexcept>
#include <queue>
#include <variant>

namespace hsm {

namespace {
template <class EventsTuple> auto constexpr make_variant_queue(EventsTuple events)
{
    using Variant_t =
        typename decltype(boost::hana::unpack(events, boost::hana::template_<std::variant>))::type;
    return std::queue<Variant_t> {};
}
}

template <class EventsTuple>
class variant_queue {
    EventsTuple m_events;
    using Queue_t = decltype(make_variant_queue(m_events));
    Queue_t m_queue;

public:
    variant_queue(const EventsTuple& events) : m_events(events){
    }

    [[nodiscard]] auto empty() const -> bool{
        return m_queue.empty();
    }

    [[nodiscard]] auto size() const -> std::size_t {
        return m_queue.size();    
    }

    template <class T> void push(const T& e)
    {
        m_queue.push(e);
    }

    template <class T> void push(T&& e)
    {
        m_queue.push(e);
    }

    template <class Callable>
    void visit(const Callable& callable){
        if(empty()){
            throw std::runtime_error("variant_queue is empty");
        }

        auto frontElement = m_queue.front();
        m_queue.pop();

        std::visit([&callable](auto&& arg){
            callable(arg);    
        }, frontElement);
        
    }
};
}

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/if.hpp>

#include <array>
#include <cstdint>
#include <sstream>
#include <vector>
#include <iostream>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class RootState, class... OptionalParameters> class sm {
    using Region = std::uint8_t;
    using Events = decltype(collect_events_recursive(state_t<RootState> {}));
    using StatesMap = decltype(make_states_map(state_t<RootState> {}));
    std::array<StateIdx, maxInitialStates(state_t<RootState> {})> m_currentCombinedState;
    std::array<std::vector<std::size_t>, nParentStates(state_t<RootState> {})> m_initial_states;
    std::array<std::vector<std::size_t>, nParentStates(state_t<RootState> {})> m_history;
    variant_queue<Events> m_defer_queue;
    std::size_t m_currentRegions{};
    StatesMap m_statesMap;

  public:
    sm(OptionalParameters&... optionalParameters)
        : m_defer_queue(collect_events_recursive(state_t<RootState> {}))
        , m_statesMap(make_states_map(state_t<RootState> {}))
    {
        fill_dispatch_table(optionalParameters...);
        fill_initial_state_table(rootState(), m_initial_states);
        fill_initial_state_table(rootState(), m_history);
        init_current_state();
        update_current_regions();
    }

    template <class Event> auto process_event(Event&& event)
    {
        if (!process_event_internal(event)) {
            call_unexpected_event_handler(event);
            return;
        }

        process_deferred_events();
    }

    template <class State> auto is(State state) -> bool
    {
        return currentState(0) == getStateIdx(rootState(), state);
    }

    template <class ParentState, class State> auto is(ParentState parentState, State state) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState(), parentState)
            && currentState(0) == getStateIdx(rootState(), state);
    }

    template <class ParentState, class State>
    auto is(Region region, ParentState parentState, State state) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState(), parentState)
            && currentState(region) == getStateIdx(rootState(), state);
    }

    template <class ParentState> auto parent_is(ParentState parentState) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState(), parentState);
    }

    auto status() -> std::string
    {
        std::stringstream statusStream;
        for (Region region = 0; region < current_regions(); region++) {
            statusStream << "[" << region << "] "
                         << "combined: " << m_currentCombinedState[region] << " "
                         << "parent: " << currentParentState() << " "
                         << "state: " << currentState(region);
        }
        return statusStream.str();
    }

    auto set_dependency(OptionalParameters&... optionalParameters)
    {
        fill_dispatch_table(optionalParameters...);
    }

  private:
    template <class Event> auto process_event_internal(Event&& event) -> bool
    {
        bool allGuardsFailed = true;

        for (Region region = 0; region < current_regions(); region++) {

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            auto& result = dispatch_table_at(m_currentCombinedState[region], event);

            if(result.defer){
                m_defer_queue.push(event);
                return true;
            }

            if(!result.valid){
                return false;
            }

            if (!result.transition->executeGuard(event)) {
                continue;
            }

            allGuardsFailed = false;
            update_current_state(region, result);

            result.transition->executeAction(event);
        }

        if (allGuardsFailed) {
            return true;
        }

        apply_anonymous_transitions();
        return true;
    }

    auto process_deferred_events()
    {
        bh::if_(
            hasDeferedEvents(rootState()),
            [this]() {
                if (!m_defer_queue.empty()) {
                    m_defer_queue.visit([this](auto event) { this->process_event_internal(event); });
                }
            },
            []() {})();
    }

    auto apply_anonymous_transitions()
    {
        bh::if_(
            has_anonymous_transition(rootState()),
            [this]() {
                while (true) {

                    for (Region region = 0; region < current_regions(); region++) {

                        auto event = noneEvent {};
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                        auto& result = dispatch_table_at(m_currentCombinedState[region], event);

                        if (!result.valid) {
                            return;
                        }

                        if (!result.transition->executeGuard(event)) {
                            continue;
                        }

                        update_current_state(region, result);
                        result.transition->executeAction(event);
                    }
                }
            },
            []() {})();
    }

    template <class Event> constexpr auto dispatch_table_at(StateIdx index, const Event& /*event*/) -> auto&
    {
        constexpr auto states
            = nStates(state_t<RootState> {}) * nParentStates(state_t<RootState> {});
        return DispatchTable<states, Event>::table[index];
    }

    template <class DispatchTableEntry>
    void update_current_state(Region region, const DispatchTableEntry& dispatchTableEntry)
    {
        bh::if_(
            has_history(rootState()),
            [&, this]() {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)    
                m_history[currentParentState()][region] = m_currentCombinedState[region];

                if (dispatchTableEntry.history) {
                    auto parent = calcParentStateIdx(
                        nStates(rootState()), dispatchTableEntry.combinedState);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                    auto combined = m_history[parent][region];
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                    m_currentCombinedState[region] = combined;
                } else {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)    
                    m_currentCombinedState[region] = dispatchTableEntry.combinedState;
                }
            },
            [&, this]() { 
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)        
                m_currentCombinedState[region] = dispatchTableEntry.combinedState; 
            })();

        update_current_regions();
    }

    void update_current_regions()
    {
        bh::if_(
            hasRegions(rootState()),
            []() {},
            [this]() { 
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)        
                m_currentRegions = m_initial_states[currentParentState()].size(); 
            })();
    }

    auto current_regions() -> std::size_t
    {
        return bh::if_(
            hasRegions(rootState()), [](auto) { return 1; }, [](auto regions) { return regions; })(
            m_currentRegions);
    }

    template <class Event> auto call_unexpected_event_handler(Event event)
    {
        const auto handler = get_unexpected_event_handler(rootState());
        handler(event);
    }

    constexpr auto rootState()
    {
        return state_t<RootState> {};
    }

    auto currentState(Region region)
    {
        return calcStateIdx(nStates(rootState()), m_currentCombinedState.at(region));
    }

    auto currentParentState()
    {
        return calcParentStateIdx(nStates(rootState()), m_currentCombinedState[0]);
    }

    void init_current_state()
    {
        const auto initialParentState = getParentStateIdx(rootState(), rootState());

        for (Region region = 0; region < m_initial_states[initialParentState].size();
             region++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_currentCombinedState[region] = m_initial_states[initialParentState][region];
        }
    }

    void fill_dispatch_table(OptionalParameters&... optionalParameters)
    {
        auto optionalDependency = bh::make_basic_tuple(std::ref(optionalParameters)...);
        fill_dispatch_table_with_external_transitions(rootState(), m_statesMap, optionalDependency);
        fill_dispatch_table_with_internal_transitions(rootState(), m_statesMap, optionalDependency);
        fill_dispatch_table_with_deferred_events(rootState(), optionalDependency);
    }
};
}

namespace hsm {

template <class Type> struct StateBase {
    using type = Type;

    template <class Event> constexpr auto operator+(const event_t<Event>&)
    {
        return TransitionSE<Type, event_t<Event>> {};
    }

    template <class Event, class Guard>
    constexpr auto operator+(const TransitionEG<Event, Guard>& transition)
    {
        return TransitionSEG<Type, Event, Guard> { transition.guard };
    }

    template <class Event, class Action>
    constexpr auto operator+(const TransitionEA<Event, Action>& transition)
    {
        return TransitionSEA<Type, Event, Action> { transition.action };
    }

    template <class Event, class Guard, class Action>
    constexpr auto operator+(const TransitionEGA<Event, Guard, Action>& transition)
    {
        return TransitionSEGA<Type, Event, Guard, Action> { transition.guard, transition.action };
    }

    template <class Action> constexpr auto operator/(const Action& action)
    {
        return TransitionSA<Type, Action> { action };
    }

    template <class Guard> constexpr auto operator[](const Guard& guard)
    {
        return TransitionSG<Type, Guard> { guard };
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state_t<Type> {}, event_t<noneEvent> {}, noGuard {}, noAction {}, target);
    }

    template <class Source, class Event>
    constexpr auto operator<=(const TransitionSE<Source, Event>&)
    {
        return details::transition(
            state_t<Source> {}, Event {}, noGuard {}, noAction {}, state_t<Type> {});
    }

    template <class Source, class Event, class Guard>
    constexpr auto operator<=(const TransitionSEG<Source, Event, Guard>& transitionSeg)
    {
        return details::transition(
            state_t<Source> {}, Event {}, transitionSeg.guard, noAction {}, state_t<Type> {});
    }

    template <class Source, class Event, class Action>
    constexpr auto operator<=(const TransitionSEA<Source, Event, Action>& transitionSea)
    {
        return details::transition(
            state_t<Source> {}, Event {}, noGuard {}, transitionSea.action, state_t<Type> {});
    }

    template <class Source, class Event, class Guard, class Action>
    constexpr auto operator<=(const TransitionSEGA<Source, Event, Guard, Action>& transitionSega)
    {
        return details::transition(
            state_t<Source> {},
            Event {},
            transitionSega.guard,
            transitionSega.action,
            state_t<Type> {});
    }

    template <class Source> constexpr auto operator<=(const state_t<Source>& source)
    {
        return details::transition(
            source, event_t<noneEvent> {}, noGuard {}, noAction {}, state_t<Type> {});
    }

    template <class OtherState> auto operator==(OtherState) -> bool
    {
        return boost::hana::equal(
            boost::hana::type_c<typename OtherState::type>, boost::hana::type_c<Type>);
    }
};

template <class Source> struct state_t : public StateBase<Source> {
    using StateBase<Source>::operator=;

    constexpr auto operator*()
    {
        return initial_t<Source> {};
    }
};
template <class Source> state_t<Source> state {};

template <class Source> struct initial_t : public StateBase<Initial<state_t<Source>>> {
    using StateBase<Initial<state_t<Source>>>::operator=;
};
template <class Source> initial_t<Source> initial {};

template <class Parent, class State>
struct direct_t : public StateBase<Direct<state_t<Parent>, state_t<State>>> {
    using StateBase<Direct<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> direct_t<Parent, State> direct {};

template <class Parent, class State>
struct entry_t : public StateBase<Entry<state_t<Parent>, state_t<State>>> {
    using StateBase<Entry<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> entry_t<Parent, State> entry {};

template <class Parent, class State>
struct exit_t : public StateBase<Exit<state_t<Parent>, state_t<State>>> {
    using StateBase<Exit<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> exit_t<Parent, State> exit {};

template <class Parent> struct history_t : public StateBase<History<state_t<Parent>>> {
    using StateBase<History<state_t<Parent>>>::operator=;
};
template <class Parent> history_t<Parent> history {};
}

#include <boost/hana/tuple.hpp>

namespace hsm {
namespace bh {
using namespace boost::hana;
}
constexpr auto transition_table = bh::make_basic_tuple;

template <class... Events> constexpr auto events = bh::tuple_t<Events...>;
}

#include <boost/hana/bool.hpp>
#include <boost/hana/tuple.hpp>

namespace hsm {

template <class Event, class Guard, class Action> class TransitionEGA {
  public:
    constexpr TransitionEGA(const Guard& guard, const Action& action)
        : guard(guard)
        , action(action)
    {
    }

    constexpr auto operator+()
    {
        return details::internal_transition(Event {}, guard, action);
    }

  public:
    const Guard guard;
    const Action action;
};

template <class Event, class Guard> class TransitionEG {
  public:
    constexpr TransitionEG(const Guard& guard)
        : guard(guard)
    {
    }

    template <class Action> constexpr auto operator/(const Action& action)
    {
        return TransitionEGA<Event, Guard, Action> { guard, action };
    }

    constexpr auto operator+()
    {
        return details::internal_transition(Event {}, guard, noAction {});
    }

  public:
    const Guard guard;
};

template <class Event, class Action> class TransitionEA {
  public:
    constexpr TransitionEA(const Action& action)
        : action(action)
    {
    }

    constexpr auto operator+()
    {
        return details::internal_transition(Event {}, noGuard {}, action);
    }

  public:
    const Action action;
};

template <class Source, class Event, class Guard, class Action> class TransitionSEGA {
  public:
    constexpr TransitionSEGA(const Guard& guard, const Action& action)
        : guard(guard)
        , action(action)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(state_t<Source> {}, Event {}, guard, action, target);
    }

    const Guard guard;
    const Action action;
};

template <class Source, class Event, class Guard> class TransitionSEG {
  public:
    constexpr TransitionSEG(const Guard& guard)
        : guard(guard)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(state_t<Source> {}, Event {}, guard, noAction {}, target);
    }

    const Guard guard;
};

template <class Source, class Event, class Action> class TransitionSEA {
  public:
    constexpr TransitionSEA(const Action& action)
        : action(action)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(state_t<Source> {}, Event {}, noGuard {}, action, target);
    }

    const Action action;
};

template <class Source, class Event> class TransitionSE {
  public:
    
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(state_t<Source> {}, Event {}, noGuard {}, noAction {}, target);
    }
};

template <class Source, class Action> class TransitionSA {
  public:
    constexpr TransitionSA(const Action& action)
        : action(action)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state_t<Source> {}, event_t<noneEvent> {}, noGuard {}, action, target);
    }

  private:
    const Action action;
};

template <class Source, class Guard, class Action> class TransitionSGA {
  public:
    constexpr TransitionSGA(const Guard& guard, const Action& action)
        : guard(guard)
        , action(action)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state_t<Source> {}, event_t<noneEvent> {}, guard, action, target);
    }

  private:
    const Guard guard;
    const Action action;
};

template <class Source, class Guard> class TransitionSG {
  public:
    constexpr TransitionSG(const Guard& guard)
        : guard(guard)
    {
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)    
    template <class Target> constexpr auto operator=(const Target& target)
    {
        return details::transition(
            state_t<Source> {}, event_t<noneEvent> {}, guard, noAction {}, target);
    }

    template <class Action> constexpr auto operator/(const Action& action)
    {
        return TransitionSGA<Source, Guard, Action> { guard, action };
    }

  private:
    const Guard guard;
};

}

namespace hsm {
constexpr auto transition = details::transition;
constexpr auto internal_transition = details::internal_transition;
}