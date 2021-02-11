#pragma once

#include <tuple>

namespace hsm {

/*
 * Chain abitrary number of actions to be used in the single transition
 *
 * @param[in] actions actions that will be chained
 */
constexpr auto chain_actions = [](auto... actions) {
    return [=](auto&&... args) {
        return std::apply([&](auto... f) { (f(args...), ...); }, std::tie(actions...));
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

#include <boost/hana/experimental/printable.hpp>
#include <iostream>

namespace hsm {

constexpr auto log = [](const auto& event, const auto& source, const auto& target, const auto&...) {
    std::cout << boost::hana::experimental::print(boost::hana::typeid_(source)) << " + "
              << boost::hana::experimental::print(boost::hana::typeid_(event)) << " = "
              << boost::hana::experimental::print(boost::hana::typeid_(target)) << std::endl;
};
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

#include <boost/hana/for_each.hpp>

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }

    inline constexpr auto for_each_idx = [](auto list, auto closure) {
        std::size_t index = 0;
        bh::for_each(list, [closure, &index](const auto& elem) {
            closure(elem, index);
            index++;
        });
    };
}

#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/unique.hpp>
#include <boost/mp11/algorithm.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
constexpr auto to_type_pair = [](auto x) { return bh::make_pair(bh::typeid_(x), x); };
}

template <class Tuple> constexpr auto remove_duplicates(Tuple tuple)
{
    return boost::mp11::mp_unique<std::decay_t<decltype(tuple)>> {};
}

template <class Tuple> constexpr auto remove_duplicate_types(Tuple tuple)
{
    return bh::values(bh::to_map(bh::transform(tuple, to_type_pair)));
}
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
#include <boost/hana/not_equal.hpp>
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

constexpr auto unwrap_typeid = [](auto typeid_) { return typename decltype(typeid_)::type {}; };

constexpr auto unwrap_typeid_to_shared_ptr = [](auto typeid_) {
    using UnwrappedType = typename decltype(typeid_)::type;
    if constexpr (is_default_constructable(typeid_)) {
        return std::make_shared<std::unique_ptr<UnwrappedType>>(std::make_unique<UnwrappedType>());
    } else {
        return std::make_shared<std::unique_ptr<UnwrappedType>>(nullptr);
    }
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

template <class Action> constexpr decltype(auto) is_action()
{
    return bh::not_equal(bh::type_c<Action>, bh::typeid_(noAction {}));
}

constexpr auto is_no_guard
    = [](auto guard) { return bh::equal(bh::typeid_(guard), bh::typeid_(noGuard {})); };

template <class Guard> constexpr decltype(auto) is_guard()
{
    return bh::not_equal(bh::type_c<Guard>, bh::typeid_(noGuard {}));
}

constexpr auto is_event = bh::is_valid([](auto&& event) -> decltype(event.typeid_) {});

constexpr auto get_entry_action
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::on_entry(); };
constexpr auto get_exit_action
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::on_exit(); };
constexpr auto get_defer_events
    = [](auto stateTypeid) { return decltype(stateTypeid)::type::defer_events(); };
const auto get_unexpected_event_handler = [](auto rootState) {
    if constexpr (has_unexpected_event_handler(rootState)) {
        return decltype(rootState)::type::on_unexpected_event();
    } else {
        return [](auto /*event*/) {};
    }
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

template <class State> constexpr auto collect_parent_state_typeids(State state)
{
    auto toParentStateTypeid = [](auto transition) { return bh::typeid_(transition.parent()); };

    auto transitions = flatten_transition_table(state);
    auto parentStateTypeids = bh::transform(transitions, toParentStateTypeid);
    return remove_duplicates(parentStateTypeids);
}
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

template <
    class Parent,
    class Source,
    class Event,
    class Guard,
    class Action,
    class Target,
    bool Internal>
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

    [[nodiscard]] constexpr auto internal() const -> bool
    {
        return Internal;
    }

  private:
    const Guard m_guard;
    const Action m_action;
};

constexpr auto transition
    = [](auto&&... xs) { return Transition<std::decay_t<decltype(xs)>...> { xs... }; };

constexpr auto internal_transition
    = [](auto&&... xs) { return InternalTransition<std::decay_t<decltype(xs)>...> { xs... }; };

constexpr auto extended_transition = [](auto parent, auto transition) {
    return ExtendedTransition<
        std::decay_t<decltype(parent)>,
        std::decay_t<decltype(transition.source())>,
        std::decay_t<decltype(transition.event())>,
        std::decay_t<decltype(transition.guard())>,
        std::decay_t<decltype(transition.action())>,
        std::decay_t<decltype(transition.target())>,
        false> { parent,
                 transition.source(),
                 transition.event(),
                 transition.guard(),
                 transition.action(),
                 transition.target() };
};

constexpr auto internal_extended_transition = [](auto parent, auto transition) {
    return ExtendedTransition<
        std::decay_t<decltype(parent)>,
        std::decay_t<decltype(transition.source())>,
        std::decay_t<decltype(transition.event())>,
        std::decay_t<decltype(transition.guard())>,
        std::decay_t<decltype(transition.action())>,
        std::decay_t<decltype(transition.target())>,
        true> { parent,
                transition.source(),
                transition.event(),
                transition.guard(),
                transition.action(),
                transition.target() };
};
}
}

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Event> struct event_t {
    static constexpr bh::type<Event> typeid_ {};

    constexpr auto operator+() const
    {
        return details::internal_transition(event_t<Event> {}, noGuard {}, noAction {});
    }

    template <class Guard> constexpr auto operator[](const Guard& guard) const
    {
        return TransitionEG<event_t<Event>, Guard> { guard };
    }

    template <class Action> constexpr auto operator/(const Action& guard) const
    {
        return TransitionEA<event_t<Event>, Action> { guard };
    }
};

template <class Event> const event_t<Event> event {};

struct noneEvent {
};

using none = event_t<noneEvent>;
}

#include <boost/hana/at.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}
template <class Tuple> constexpr auto to_pairs(const Tuple& tuple)
{
    return bh::transform(
        tuple, [](auto tuple) { return bh::make_pair(bh::at_c<0>(tuple), bh::at_c<1>(tuple)); });
}
}

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/drop_while.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/minus.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/zip.hpp>

//#include <boost/mp11.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Iterable, class Element>
constexpr auto index_of(Iterable const& iterable, Element const& element)
{
    return bh::apply(
        [](auto size, auto dropped) { return size - dropped; },
        bh::size(iterable),
        bh::size(bh::drop_while(iterable, bh::not_equal.to(element))));
}

template <class Typeids> constexpr auto make_index_map(Typeids typeids)
{
    return bh::apply(
        [](auto typeids, auto range) { return bh::to_map(to_pairs(bh::zip(typeids, range))); },
        typeids,
        bh::to<bh::basic_tuple_tag>(bh::make_range(bh::int_c<0>, bh::size(typeids))));
}

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

using Idx = std::uint64_t;
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

template <class State> constexpr auto getCombinedStateTypeids(const State& rootState)
{
    auto parentStateTypeids = collect_parent_state_typeids(rootState);
    auto stateTypeids = collect_state_typeids_recursive(rootState);
    auto stateCartesianProduct
        = bh::cartesian_product(bh::make_basic_tuple(parentStateTypeids, stateTypeids));
    return bh::transform(stateCartesianProduct, bh::typeid_);
}

template <class ParentState, class State>
constexpr auto getCombinedStateTypeid(const ParentState& parentState, const State& state)
{
    return bh::typeid_(bh::make_basic_tuple(bh::typeid_(parentState), bh::typeid_(state)));
}

template <class StateTypeids, class ParentState, class State>
constexpr auto
getCombinedStateIdx(StateTypeids combinedStateTypids, ParentState parentState, State state)
{
    return index_of(combinedStateTypids, getCombinedStateTypeid(parentState, state));
}

constexpr auto calcCombinedStateIdx
    = [](std::size_t nStates, Idx parentStateIdx, Idx stateIdx) -> Idx {
    return (parentStateIdx * nStates) + stateIdx;
};

constexpr auto calcParentStateIdx
    = [](std::size_t nStates, Idx combinedState) -> Idx { return combinedState / nStates; };

constexpr auto calcStateIdx
    = [](std::size_t nStates, Idx combinedState) -> Idx { return combinedState % nStates; };

constexpr decltype(auto) resolveEvent = [](auto event) {
    if constexpr (is_event(event)) {
        return event.typeid_;
    } else {
        return bh::typeid_(event);
    }
};

constexpr auto getEventIdx = [](auto rootState, auto event) {
    return index_of(collect_event_typeids_recursive(rootState), resolveEvent(event));
};

constexpr auto getActionIdx = [](auto rootState, auto action) {
    return index_of(collect_action_typeids_recursive(rootState), bh::typeid_(action));
};

constexpr auto getGuardIdx = [](auto rootState, auto guard) {
    return index_of(collect_guard_typeids_recursive(rootState), bh::typeid_(guard));
};
}

#include <boost/hana/find.hpp>
#include <boost/hana/maximum.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/zip.hpp>

#include <iostream>
#include <vector>

namespace hsm {

/**
 * Collect the initial states for the parent states
 *
 * Returns: [[State]]
 *
 */
template <class State> constexpr auto collect_initial_states(State parentState)
{
    constexpr auto childStates = collect_child_states(parentState);
    constexpr auto initialStates = bh::filter(childStates, is_initial_state);
    return bh::transform(initialStates, [](auto initialState) { return get_state(initialState); });
}

/**
 * Collect the initial states for the parent states
 * and returns it as tuple of combined state idx.
 *
 * Returns: [[StateIdx]]
 *
 * Example: [[0,1], [0], [1], [1,2]]
 */
template <class State, class Typeids>
constexpr auto collect_initial_state_stateidx(State rootState, Typeids parentStateTypeids)
{
    return bh::transform(parentStateTypeids, [rootState](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;

        constexpr auto initialStates = collect_initial_states(ParentState {});

        return bh::transform(initialStates, [rootState](auto initialState) {
            return getCombinedStateIdx(
                getCombinedStateTypeids(rootState), ParentState {}, initialState);
        });
    });
}

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
template <class State> inline constexpr auto make_initial_state_map(State rootState)
{
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    constexpr auto initialStates = collect_initial_state_stateidx(rootState, parentStateTypeids);
    return bh::to_map(to_pairs(bh::zip(parentStateTypeids, initialStates)));
}

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
template <class State, class StateTable>
constexpr auto fill_initial_state_table(State rootState, StateTable& initialStateTable)
{
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    for_each_idx(
        parentStateTypeids,
        [rootState, &initialStateTable](auto parentStateTypeid, auto parentStateId) {
            bh::apply(
                [](auto parentStateId, auto& initialStateTable, auto initialStates) {
                    auto initialStatesStateIdx = std::vector<std::size_t>(bh::size(initialStates));

                    for_each_idx(
                        initialStates, [&initialStatesStateIdx](auto stateIdx, auto regionId) {
                            initialStatesStateIdx[regionId] = stateIdx;
                        });

                    initialStateTable.at(parentStateId) = initialStatesStateIdx;
                },
                parentStateId,
                initialStateTable,
                bh::find(make_initial_state_map(rootState), parentStateTypeid).value());
        });
}

/**
 * Returns a tuple of initial state sizes
 *
 * Returns: [std::size_t]
 *
 * Example: [3, 1, 2]
 */
template <class Typeids> constexpr auto initialStateSizes(Typeids parentStateTypeids)
{
    return bh::transform(parentStateTypeids, [](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;
        return bh::size(collect_initial_states(ParentState {}));
    });
}

/**
 * Returns the maximal number of initial states
 */
template <class State> constexpr auto maxInitialStates(State rootState)
{
    return bh::maximum(initialStateSizes(collect_parent_state_typeids(rootState)));
}
}

#include <boost/hana/bool.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/or.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto has_substate_initial_state_entry_action = [](auto target) {
    if constexpr (has_transition_table(target)) {
        return has_entry_action(bh::at_c<0>(collect_initial_states(target)));
    } else {
        return bh::false_c;
    }
};

constexpr auto has_pseudo_exit_action = [](auto transition) {
    return bh::and_(
        is_exit_state(transition.source()), has_exit_action(resolveSrcParent(transition)));
};

constexpr auto has_action = [](auto transition) {
    return bh::or_(
        bh::not_(is_no_action(transition.action())),
        has_entry_action(transition.target()),
        has_substate_initial_state_entry_action(transition.target()),
        has_exit_action(transition.source()),
        has_pseudo_exit_action(transition));
};
}

#include <boost/hana/functional/apply.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Transition> constexpr auto resolveSubStateParent(Transition transition)
{
    auto constexpr target = transition.target();
    if constexpr (is_entry_state(target)) {
        return get_parent_state(target);
    } else if constexpr (is_direct_state(target)) {
        return get_parent_state(target);
    } else if constexpr (is_history_state(target)) {
        return get_parent_state(target);
    } else {
        return target;
    }
}

template <class Transition> constexpr auto resolveDst(Transition transition)
{
    auto constexpr dst = transition.target();

    if constexpr (has_transition_table(dst)) {
        return bh::at_c<0>(collect_initial_states(dst));
    } else if constexpr (is_entry_state(dst)) {
        return get_state(dst);
    } else if constexpr (is_direct_state(dst)) {
        return get_state(dst);
    } else if constexpr (is_history_state(dst)) {
        return bh::at_c<0>(collect_initial_states(get_parent_state(dst)));
    } else if constexpr (is_initial_state(dst)) {
        return get_state(dst);
    } else {
        return dst;
    }
}

template <class Transition> constexpr auto resolveDstParent(Transition transition)
{
    auto constexpr target = transition.target();

    if constexpr (has_transition_table(target)) {
        return target;
    } else if constexpr (is_entry_state(target)) {
        return get_parent_state(target);
    } else if constexpr (is_direct_state(target)) {
        return get_parent_state(target);
    } else if constexpr (is_history_state(target)) {
        return get_parent_state(target);
    } else {
        return transition.parent();
    }
}

template <class Transition> constexpr auto resolveSrc(Transition transition)
{
    auto constexpr src = transition.source();

    if constexpr (is_initial_state(src)) {
        return get_state(src);
    } else if constexpr (is_exit_state(src)) {
        return get_state(src);
    } else if constexpr (is_direct_state(src)) {
        return get_state(src);
    } else {
        return src;
    }
}

template <class Transition> constexpr auto resolveSrcParent(Transition transition)
{
    auto constexpr src = transition.source();

    if constexpr (is_exit_state(src)) {
        return get_parent_state(src);
    } else if constexpr (is_direct_state(src)) {
        return get_parent_state(src);
    } else {
        return transition.parent();
    }
}

template <class Transition> constexpr auto resolveEntryAction(Transition transition)
{
    if constexpr (transition.internal()) {
        return [](auto&&...) {};
    } else if constexpr (has_entry_action(transition.target())) {
        return get_entry_action(transition.target());
    } else {
        return [](auto&&...) {};
    }
}

template <class Transition> constexpr auto resolveInitialStateEntryAction(Transition transition)
{
    if constexpr (has_substate_initial_state_entry_action(transition.target())) {
        return get_entry_action(bh::at_c<0>(collect_initial_states(transition.target())));
    } else {
        return [](auto&&...) {};
    }
}

template <class Transition> constexpr auto resolveExitAction(Transition transition)
{
    const auto hasPseudoExitAction = has_pseudo_exit_action(transition);
    (void) hasPseudoExitAction;

    if constexpr (transition.internal()) {
        return [](auto&&...) {};
    } else if constexpr (has_exit_action(transition.source())) {
        return get_exit_action(transition.source());
    } else if constexpr (hasPseudoExitAction) {
        return get_exit_action(resolveSrcParent(transition));
    } else {
        return [](auto&&...) {};
    }
}

template <class Transition> constexpr auto resolveNoAction(Transition transition)
{
    const auto isNoAction = is_no_action(transition.action());

    if constexpr (isNoAction) {
        return [](auto&&...) {};
    } else {
        return transition.action();
    }
}

template <class Transition> constexpr auto resolveEntryExitAction(Transition transition)
{
    return [exitAction(resolveExitAction(transition)),
            action(resolveNoAction(transition)),
            entryAction(resolveEntryAction(transition)),
            initialStateEntryAction(resolveInitialStateEntryAction(transition))](auto&&... params) {
        exitAction(std::forward<decltype(params)>(params)...);
        action(std::forward<decltype(params)>(params)...);
        entryAction(std::forward<decltype(params)>(params)...);
        initialStateEntryAction(std::forward<decltype(params)>(params)...);
    };
}

template <class Transition> constexpr auto resolveAction(Transition transition)
{
    const auto hasAction = has_action(transition);

    if constexpr (hasAction) {
        return resolveEntryExitAction(transition);
    } else {
        return transition.action();
    }
}

template <class Transition> constexpr auto resolveHistory(Transition transition)
{
    if constexpr (is_history_state(transition.target())) {
        return true;
    } else {
        return false;
    }
}
}

#include <boost/hana/eval.hpp>
#include <boost/hana/find_if.hpp>
#include <boost/hana/functional/always.hpp>
#include <boost/hana/lazy.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/functional/compose.hpp>

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }

    constexpr auto function = bh::second;
    constexpr auto predicate = bh::first;
    constexpr auto case_ = bh::make_pair;

    constexpr auto otherwise = bh::always(bh::true_c);
    template <class... Case> constexpr auto switch_(Case&&... cases_)
    {
        return function(bh::find_if(bh::make_basic_tuple(cases_...), predicate).value());
    }

    constexpr auto lazy_predicate = bh::compose(bh::eval, bh::first);
    constexpr auto lazy_otherwise = bh::make_lazy(bh::always(bh::true_c));

    template <class... Case> constexpr auto lazy_switch_(Case&&... cases_)
    {
        return function(bh::find_if(bh::make_basic_tuple(cases_...), lazy_predicate).value());
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

}

template <class State> constexpr auto flatten_transition_table(State state)
{
    auto flattenSubTransitionTable = [state](auto transition) {
        return bh::prepend(
            flatten_sub_transition_table(resolveSubStateParent(transition)),
            details::extended_transition(state, transition));
    };

    return bh::flatten(bh::transform(make_transition_table2(state), flattenSubTransitionTable));
}

namespace {

template <class State> constexpr auto flatten_sub_transition_table(State state)
{
    // clang-format off
    if constexpr(has_transition_table(state)){
        return flatten_transition_table(state);
    }
    else {
        return bh::make_basic_tuple();
    }
    // clang-format on
}
}
}

#include <boost/hana/at.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {

constexpr auto resolveInitialState = [](auto transition) {
    if constexpr (is_initial_state(transition.source())) {
        return get_state(transition.source());
    } else {
        return transition.source();
    }
};

constexpr auto resolveExtentedInitialState = [](auto transition) {
    if constexpr (is_initial_state(transition.source())) {
        return get_state(transition.source());
    } else {
        return transition.source();
    }
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

template <class State> constexpr auto collect_child_state_typeids_recursive(State parentState)
{
    auto transitions = flatten_transition_table(parentState);
    auto collectedStates = bh::flatten(bh::transform(transitions, extractExtendedStateTypeids));

    return remove_duplicates(collectedStates);
}

template <class State> constexpr auto collect_child_states_recursive(State parentState)
{
    return bh::flatten(bh::transform(flatten_transition_table(parentState), extractExtendedStates));
    ;
}

template <class State> constexpr auto collect_state_typeids_recursive(State parentState)
{
    auto collectedStates
        = bh::append(collect_child_state_typeids_recursive(parentState), bh::typeid_(parentState));
    return collectedStates;
}

template <class State> constexpr auto collect_states_recursive(State parentState)
{
    return remove_duplicates(bh::append(collect_child_states_recursive(parentState), parentState));
}

template <class State> constexpr auto collect_child_state_typeids(State state)
{
    auto transitions = make_transition_table2(state);
    auto collectedStates = bh::flatten(bh::transform(transitions, extractStateTypeids));

    return remove_duplicates(collectedStates);
}

template <class State> constexpr auto collect_child_states(State state)
{
    return remove_duplicates(
        bh::flatten(bh::transform(make_transition_table2(state), extractStates)));
}
}

#include <boost/hana/append.hpp>
#include <boost/hana/at.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/functional/capture.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/prepend.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

/**
 * Predicate that checks for an empty tuple
 */
constexpr auto isNotEmpty
    = [](const auto& tuple) { return bh::not_(bh::equal(bh::size_c<0>, bh::size(tuple))); };

/**
 * Returns a typle of internal transitions of a state if it exists.
 * Otherwise a empty tuple is returned. Source and target of the
 * transition are set to parentstate as a placeholder and need to
 * be filled with all child states of the particular state.
 *
 * @param state State for which the internal transitions should be returned
 *
 */
constexpr auto get_internal_transition_table = [](auto parentState) {
    if constexpr (has_internal_transition_table(parentState)) {
        return bh::transform(
            make_internal_transition_table(parentState), [parentState](auto internalTransition) {
                return details::extended_transition(
                    parentState,
                    details::transition(
                        parentState,
                        internalTransition.event(),
                        internalTransition.guard(),
                        internalTransition.action(),
                        parentState));
            });
    } else {
        return bh::make_basic_tuple();
    }
};

/**
 * Extends an internal transitions to all provided states
 *
 * @param internalTranstion Internal transition that should be extended
 * @param states tuple of states
 */
template <class Transition, class States>
constexpr auto extend_internal_transition(Transition internalTransition, States states)
{
    return bh::transform(states, [internalTransition](auto state) {
        return details::internal_extended_transition(
            internalTransition.parent(),
            details::transition(
                state,
                internalTransition.event(),
                internalTransition.guard(),
                internalTransition.action(),
                state));
    });
}

/**
 * Returns the internal transitions for each for each state
 * [[transition1, transition2], [transition3, transition4], []]
 *
 * @param states a tuple of states
 */
constexpr auto get_internal_transitions = [](auto states) {
    return bh::flatten(bh::filter(
        bh::transform(
            states,
            [](auto parentState) {
                constexpr auto extend
                    = bh::capture(parentState)([](auto parentState, auto transition) {
                          // Folowing lines satisfies older gcc -Werror=unused-but-set-parameter
                          (void)transition;
                          if constexpr (has_transition_table(parentState)) {
                              return extend_internal_transition(
                                  transition, collect_child_states(parentState));
                          } else {
                              return bh::make_basic_tuple();
                          }
                      });

                return bh::transform(get_internal_transition_table(parentState), extend);
            }),
        isNotEmpty));
};

/**
 * Returns a tuple of extended internal transitions reachable from a given rootState
 *
 * @param rootState
 */
template <class State> constexpr auto flatten_internal_transition_table(State rootState)
{
    return [](auto states) {
        return bh::to<bh::basic_tuple_tag>(bh::flatten(get_internal_transitions(states)));
    }(collect_states_recursive(rootState));
}

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

template <class State> constexpr auto collect_event_typeids_recursive(State state)
{
    return remove_duplicates(bh::transform(
        bh::concat(flatten_transition_table(state), flatten_internal_transition_table(state)),
        collectEventTypeids));
}

template <class TransitionTuple>
constexpr auto collect_event_typeids_recursive_with_transitions(TransitionTuple transitions)
{
    return remove_duplicates(bh::transform(transitions, collectEventTypeids));
}

template <class State> constexpr auto collect_events_recursive(State state)
{
    return remove_duplicate_types(bh::flatten(bh::transform(
        bh::concat(flatten_transition_table(state), flatten_internal_transition_table(state)),
        collectEvents)));
}
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
        if constexpr(is_action<Action>()){
            [](auto& action, 
               auto& event, 
               const auto& source, 
               auto& target, 
               const auto& optionalDependency) {
                bh::unpack(optionalDependency, 
                    [&action, &event, &source, &target](const auto&... optionalDependency){
                    using Target = typename TargetPtr::element_type::element_type;
                    if constexpr(is_default_constructable(bh::type_c<Target>)){
                        action(event, **source, **target, get(optionalDependency)...);
                    }
                    else {
                        action(event, **source, target, get(optionalDependency)...);
                    }
                });
            }(m_action, event, m_source, m_target, m_optionalDependency);
        }
        // clang-format on
    }

    auto executeGuard(Event& event) -> bool override
    {
        // clang-format off
        if constexpr(is_guard<Guard>()){
            return [](auto& guard,
               auto& event,
               const auto& source,
               const auto& target,
               const auto& optionalDependency) {
                return bh::unpack(
                    optionalDependency,
                    [&guard, &event, &source, &target](const auto&... optionalDependency) {
                        return guard(event, **source, **target, get(optionalDependency)...);
                    });
            }(m_guard, event, m_source, m_target, m_optionalDependency);
        }
        else {
            return true;
        }
        // clang-format on
    }

  private:
    Action m_action;
    Guard m_guard;
    SourcePtr m_source;
    TargetPtr m_target;
    OptionalDependency m_optionalDependency;
};

template <
    class Action,
    class Guard,
    class EventTypeid,
    class Source,
    class Target,
    class Dependency>
constexpr auto make_transition(
    Action action,
    Guard guard,
    EventTypeid eventTypeid,
    Source source,
    Target target,
    Dependency optionalDependency)
{
    using Event = typename decltype(eventTypeid)::type;

    return std::make_unique<
        DispatchTableEntry<Action, Guard, Source, Target, Event, decltype(optionalDependency)>>(
        action, guard, source, target, optionalDependency);
}

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
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    static DispatchArray<NStates, Event> table;
};

template <StateIdx NStates, class Event>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
DispatchArray<NStates, Event> DispatchTable<NStates, Event>::table {};

constexpr auto get_dispatch_table = [](auto rootState, auto eventTypeid) -> auto&
{
    using Event = typename decltype(eventTypeid)::type;
    return bh::apply(
        [](auto states) -> auto& { return DispatchTable<states, Event>::table; },
        nStates(rootState) * nParentStates(rootState));
};
}

#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/find.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/functional/capture.hpp>
#include <boost/hana/greater.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/length.hpp>
#include <boost/hana/mult.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class State> constexpr auto nParentStates(State rootState)
{
    return bh::length(collect_parent_state_typeids(rootState));
}

template <class State> constexpr auto nStates(State rootState)
{
    return bh::length(collect_state_typeids_recursive(rootState));
}

template <class State> constexpr auto nEvents(State rootState)
{
    return bh::length(collect_event_typeids_recursive(rootState));
}

template <class State> constexpr decltype(auto) hasParallelRegions(State rootState)
{
    return bh::greater(maxInitialStates(rootState), bh::size_c<1>);
}

template <
    class State,
    class TransitionTuple,
    class EventTypeid,
    class StatesMap,
    class Dependencies>
constexpr auto addDispatchTableEntry(
    State rootState,
    TransitionTuple&& transition,
    EventTypeid eventTypeid,
    StatesMap&& statesMap,
    Dependencies optionalDependency)
{
    bh::apply(
        [=](auto combinedStateIds, auto source, auto target) {
            bh::apply(
                [=](auto fromIdx, auto toIdx, auto history, auto mappedSource, auto mappedTarget) {
                    bh::apply(
                        [=](auto& dispatchTable, auto&& transition2) {
                            const auto defer = false;
                            const auto valid = true;
                            dispatchTable[fromIdx]
                                = { toIdx, history, defer, valid, std::move(transition2) };
                        },
                        get_dispatch_table(rootState, eventTypeid),
                        make_transition(
                            resolveAction(transition),
                            transition.guard(),
                            eventTypeid,
                            mappedSource,
                            mappedTarget,
                            optionalDependency));
                },
                getCombinedStateIdx(combinedStateIds, resolveSrcParent(transition), source),
                getCombinedStateIdx(combinedStateIds, resolveDstParent(transition), target),
                resolveHistory(transition),
                bh::find(statesMap, bh::typeid_(source)).value(),
                bh::find(statesMap, bh::typeid_(target)).value());
        },
        getCombinedStateTypeids(rootState),
        resolveSrc(transition),
        resolveDst(transition));
}

template <
    class State,
    class TransitionTuple,
    class EventTypeid,
    class StatesMap,
    class Dependencies>
constexpr auto addDispatchTableEntryOfSubMachineExits(
    State rootState,
    TransitionTuple transition,
    EventTypeid eventTypeid,
    StatesMap&& statesMap,
    Dependencies optionalDependency)
{
    (void)rootState;
    (void)eventTypeid;
    (void)optionalDependency;

    constexpr auto parentState = transition.source();
    if constexpr (has_transition_table(parentState)) {
        bh::for_each(collect_child_state_typeids(parentState), [=](auto state) {
            bh::apply(
                [=](auto combinedStateTypeids, auto target) {
                    bh::apply(
                        [=](auto fromIdx,
                            auto toIdx,
                            auto history,
                            auto mappedParent,
                            auto mappedTarget) {
                            bh::apply(
                                [=](auto& dispatchTable, auto&& transition2) {
                                    const auto defer = false;
                                    const auto valid = true;
                                    dispatchTable[fromIdx]
                                        = { toIdx, history, defer, valid, std::move(transition2) };
                                },
                                get_dispatch_table(rootState, eventTypeid),
                                make_transition(
                                    resolveAction(transition),
                                    transition.guard(),
                                    eventTypeid,
                                    mappedParent,
                                    mappedTarget,
                                    optionalDependency));
                        },
                        getCombinedStateIdx(combinedStateTypeids, parentState, state),
                        getCombinedStateIdx(
                            combinedStateTypeids, resolveDstParent(transition), target),
                        resolveHistory(transition),
                        bh::find(statesMap, bh::typeid_(parentState)).value(),
                        bh::find(statesMap, bh::typeid_(target)).value());
                },
                getCombinedStateTypeids(rootState),
                resolveDst(transition));
        });
    }
}

constexpr auto filter_transitions = [](auto transitions, auto eventTypeid) {
    auto isEvent = [eventTypeid](auto transition) {
        return bh::equal(transition.event().typeid_, eventTypeid);
    };

    return bh::filter(transitions, isEvent);
};

constexpr auto fill_dispatch_table_for_filtered_transitions = [](auto rootState,
                                                                 auto&& statesMap,
                                                                 auto&& optionalDependency,
                                                                 auto eventTypeid,
                                                                 auto transition) {
    addDispatchTableEntry(rootState, transition, eventTypeid, statesMap, optionalDependency);
    addDispatchTableEntryOfSubMachineExits(
        rootState, transition, eventTypeid, statesMap, optionalDependency);
};

constexpr auto fill_dispatch_table_for_event = [](auto rootState, auto&& statesMap, auto&& optionalDependency, auto transitions, auto eventTypeid){
    
    auto filteredTransitions = filter_transitions(transitions, eventTypeid);
    bh::for_each(filteredTransitions, bh::capture(rootState, statesMap, optionalDependency, eventTypeid)(fill_dispatch_table_for_filtered_transitions));
};

template <class State, class StatesMap, class Dependencies, class TransitionTuple>
constexpr auto fill_dispatch_table_with_transitions(
    State rootState,
    StatesMap&& statesMap,
    Dependencies&& optionalDependency,
    TransitionTuple transitions)
{
    auto eventTypeids = collect_event_typeids_recursive_with_transitions(transitions);
    bh::for_each(eventTypeids, bh::capture(rootState, statesMap, optionalDependency, transitions)(fill_dispatch_table_for_event));
}

constexpr auto getDeferingTransitions = [](auto rootState) {
    constexpr auto transitionHasDeferedEvents
        = [](auto transition) { return has_deferred_events(resolveExtentedInitialState(transition)); };

    constexpr auto transitions = flatten_transition_table(rootState);
    return bh::filter(transitions, transitionHasDeferedEvents);
};

constexpr auto hasDeferedEvents = [](auto rootState) {
    return bh::not_equal(bh::size_c<0>, bh::size(getDeferingTransitions(rootState)));
};

template <class RootState, class OptionalDependency>
constexpr auto
fill_dispatch_table_with_deferred_events(RootState rootState, OptionalDependency /*optionalDependency*/)
{
    const auto transitions = getDeferingTransitions(rootState);
    bh::for_each(transitions, bh::capture(rootState)([](auto rootState, auto transition){
        const auto deferredEvents = get_defer_events(resolveExtentedInitialState(transition));
        bh::for_each(deferredEvents, bh::capture(rootState, transition)([](auto rootState, auto transition, auto event){
            using Event = typename decltype(event)::type;
            const auto combinedStateTypeids = getCombinedStateTypeids(rootState);
            constexpr StateIdx states = nStates(rootState) * nParentStates(rootState);
            auto& dispatchTable = DispatchTable<states, Event>::table;
            const auto from = getCombinedStateIdx(
                combinedStateTypeids, resolveSrcParent(transition), resolveSrc(transition));
            dispatchTable[from].defer = true;
        }));
    }));
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

constexpr auto to_map = [](auto tupleOfPairs) {
    return bh::unpack(tupleOfPairs, [](auto... pairs) { return bh::make_map(pairs...); });
};

/***
 * Returns a map from typeid(state<State>) -> State
 * of all states found recursive under parentState
 *
 */
template <class State> constexpr auto make_states_map(State&& parentState)
{
    auto collectedStateTypeids = collect_state_typeids_recursive(parentState);
    auto collectedStatesPtr
        = bh::transform(collect_states_recursive(parentState), unwrap_typeid_to_shared_ptr);
    return to_map(to_pairs(bh::zip(collectedStateTypeids, collectedStatesPtr)));
}
}

#include <boost/hana/filter.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/size.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

const auto is_anonymous_transition
    = [](auto transition) { return bh::typeid_(transition.event()) == bh::typeid_(none {}); };

const auto is_history_transition
    = [](auto transition) { return is_history_state(transition.target()); };

template <class State> constexpr auto has_anonymous_transition(State rootState)
{
    auto transitions = flatten_transition_table(rootState);
    auto anonymousTransition = bh::filter(transitions, is_anonymous_transition);
    return bh::not_equal(bh::size_c<0>, bh::size(anonymousTransition));
}

template <class State> constexpr auto has_history(State rootState)
{
    auto transitions = flatten_transition_table(rootState);
    auto historyTransitions = bh::filter(transitions, is_history_transition);
    return bh::not_equal(bh::size_c<0>, bh::size(historyTransitions));
}
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
#include <iostream>
#include <sstream>
#include <vector>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class RootState, class... OptionalParameters> class sm {
    static constexpr state_t<RootState> rootState {};

    using Region = std::uint8_t;
    using Events = decltype(collect_event_typeids_recursive(rootState));
    using StatesMap = decltype(make_states_map(rootState));
    std::array<StateIdx, maxInitialStates(rootState)> m_currentCombinedState;
    std::array<std::vector<std::size_t>, nParentStates(rootState)> m_initial_states;
    std::array<std::vector<std::size_t>, nParentStates(rootState)> m_history;
    variant_queue<Events> m_defer_queue;
    std::size_t m_currentRegions {};
    StatesMap m_statesMap;

  public:
    sm(OptionalParameters&... optionalParameters)
        : m_initial_states()
        , m_history()
        , m_defer_queue(collect_event_typeids_recursive(rootState))
        , m_statesMap(make_states_map(rootState))
    {
        static_assert(
            has_transition_table(rootState), "Root state has no make_transition_table method");
        static_assert(
            bh::size(flatten_transition_table(rootState)),
            "Transition table needs at least one transition");
        static_assert(
            maxInitialStates(rootState),
            "Transition table needs to have at least one initial state");

        fill_dispatch_table(optionalParameters...);
        fill_initial_state_table(rootState, m_initial_states);
        fill_initial_state_table(rootState, m_history);
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
        return currentState(0) == getStateIdx(rootState, state);
    }

    template <class ParentState, class State> auto is(ParentState parentState, State state) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState, parentState)
            && currentState(0) == getStateIdx(rootState, state);
    }

    template <class ParentState, class State>
    auto is(Region region, ParentState parentState, State state) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState, parentState)
            && currentState(region) == getStateIdx(rootState, state);
    }

    template <class ParentState> auto parent_is(ParentState parentState) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState, parentState);
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
        bool allTransitionsInvalid = true;

        for (Region region = 0; region < current_regions(); region++) {

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            auto& result = dispatch_table_at(m_currentCombinedState[region], event);

            if (result.defer) {
                m_defer_queue.push(event);
                return true;
            }

            if (!result.valid) {
                continue;
            }

            if (!result.transition->executeGuard(event)) {
                continue;
            }

            allTransitionsInvalid = false;
            allGuardsFailed = false;
            update_current_state(region, result);

            result.transition->executeAction(event);
        }

        if (allTransitionsInvalid) {
            return false;
        }

        if (allGuardsFailed) {
            return true;
        }

        apply_anonymous_transitions();
        return true;
    }

    auto process_deferred_events()
    {
        if constexpr (hasDeferedEvents(rootState)) {
            if (!m_defer_queue.empty()) {
                m_defer_queue.visit([this](auto event) { this->process_event_internal(event); });
            }
        }
    }

    auto apply_anonymous_transitions()
    {
        if constexpr (has_anonymous_transition(rootState)) {
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
        }
    }

    template <class Event>
    constexpr auto dispatch_table_at(StateIdx index, const Event /*event*/) -> auto&
    {
        return bh::apply(
            [](auto states, StateIdx index) -> auto& {
                return DispatchTable<states, Event>::table[index];
            },
            nStates(rootState) * nParentStates(rootState),
            index);
    }

    template <class DispatchTableEntry>
    void update_current_state(Region region, const DispatchTableEntry& dispatchTableEntry)
    {
        if constexpr (has_history(rootState)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_history[currentParentState()][region] = m_currentCombinedState[region];

            if (dispatchTableEntry.history) {
                auto parent
                    = calcParentStateIdx(nStates(rootState), dispatchTableEntry.combinedState);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                auto combined = m_history[parent][region];
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                m_currentCombinedState[region] = combined;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                m_currentCombinedState[region] = dispatchTableEntry.combinedState;
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_currentCombinedState[region] = dispatchTableEntry.combinedState;
        }

        update_current_regions();
    }

    void update_current_regions()
    {
        if constexpr (hasParallelRegions(rootState)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_currentRegions = m_initial_states[currentParentState()].size();
        }
    }

    auto current_regions() -> std::size_t
    {
        if constexpr (hasParallelRegions(rootState)) {
            return m_currentRegions;
        } else {
            return 1;
        }
    }

    template <class Event> auto call_unexpected_event_handler(Event event)
    {
        const auto handler = get_unexpected_event_handler(rootState);
        handler(event);
    }

    auto currentState(Region region)
    {
        return calcStateIdx(nStates(rootState), m_currentCombinedState.at(region));
    }

    auto currentParentState()
    {
        return calcParentStateIdx(nStates(rootState), m_currentCombinedState[0]);
    }

    void init_current_state()
    {
        const auto initialParentState = getParentStateIdx(rootState, rootState);

        for (Region region = 0; region < m_initial_states[initialParentState].size(); region++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_currentCombinedState[region] = m_initial_states[initialParentState][region];
        }
    }

    void fill_dispatch_table(OptionalParameters&... optionalParameters)
    {
        auto optionalDependency = bh::make_basic_tuple(std::ref(optionalParameters)...);
        fill_dispatch_table_with_internal_transitions(rootState, m_statesMap, optionalDependency);
        fill_dispatch_table_with_external_transitions(rootState, m_statesMap, optionalDependency);
        fill_dispatch_table_with_deferred_events(rootState, optionalDependency);
    }
};
}

namespace hsm {

template <class Type> struct StateBase {
    using type = Type;

    template <class Event> constexpr auto operator+(const event_t<Event>&) const
    {
        return TransitionSE<Type, event_t<Event>> {};
    }

    template <class Event, class Guard>
    constexpr auto operator+(const TransitionEG<Event, Guard>& transition) const
    {
        return TransitionSEG<Type, Event, Guard> { transition.guard };
    }

    template <class Event, class Action>
    constexpr auto operator+(const TransitionEA<Event, Action>& transition) const
    {
        return TransitionSEA<Type, Event, Action> { transition.action };
    }

    template <class Event, class Guard, class Action>
    constexpr auto operator+(const TransitionEGA<Event, Guard, Action>& transition) const
    {
        return TransitionSEGA<Type, Event, Guard, Action> { transition.guard, transition.action };
    }

    template <class Action> constexpr auto operator/(const Action& action) const
    {
        return TransitionSA<Type, Action> { action };
    }

    template <class Guard> constexpr auto operator[](const Guard& guard) const
    {
        return TransitionSG<Type, Guard> { guard };
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
    template <class Target> constexpr auto operator=(const Target& target) const
    {
        return details::transition(
            state_t<Type> {}, event_t<noneEvent> {}, noGuard {}, noAction {}, target);
    }

    template <class Source, class Event>
    constexpr auto operator<=(const TransitionSE<Source, Event>&) const
    {
        return details::transition(
            state_t<Source> {}, Event {}, noGuard {}, noAction {}, state_t<Type> {});
    }

    template <class Source, class Event, class Guard>
    constexpr auto operator<=(const TransitionSEG<Source, Event, Guard>& transitionSeg) const
    {
        return details::transition(
            state_t<Source> {}, Event {}, transitionSeg.guard, noAction {}, state_t<Type> {});
    }

    template <class Source, class Event, class Action>
    constexpr auto operator<=(const TransitionSEA<Source, Event, Action>& transitionSea) const
    {
        return details::transition(
            state_t<Source> {}, Event {}, noGuard {}, transitionSea.action, state_t<Type> {});
    }

    template <class Source, class Event, class Guard, class Action>
    constexpr auto
    operator<=(const TransitionSEGA<Source, Event, Guard, Action>& transitionSega) const
    {
        return details::transition(
            state_t<Source> {},
            Event {},
            transitionSega.guard,
            transitionSega.action,
            state_t<Type> {});
    }

    template <class Source> constexpr auto operator<=(const state_t<Source>& source) const
    {
        return details::transition(
            source, event_t<noneEvent> {}, noGuard {}, noAction {}, state_t<Type> {});
    }

    template <class OtherState> auto operator==(OtherState) const -> bool
    {
        return boost::hana::equal(
            boost::hana::type_c<typename OtherState::type>, boost::hana::type_c<Type>);
    }
};

template <class Source> struct state_t : public StateBase<Source> {
    using StateBase<Source>::operator=;

    constexpr auto operator*() const
    {
        return initial_t<Source> {};
    }
};

template <class Source> const state_t<Source> state {};

template <class Source> struct initial_t : public StateBase<Initial<state_t<Source>>> {
    using StateBase<Initial<state_t<Source>>>::operator=;
};
template <class Source> const initial_t<Source> initial {};

template <class Parent, class State>
struct direct_t : public StateBase<Direct<state_t<Parent>, state_t<State>>> {
    using StateBase<Direct<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> const direct_t<Parent, State> direct {};

template <class Parent, class State>
struct entry_t : public StateBase<Entry<state_t<Parent>, state_t<State>>> {
    using StateBase<Entry<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> const entry_t<Parent, State> entry {};

template <class Parent, class State>
struct exit_t : public StateBase<Exit<state_t<Parent>, state_t<State>>> {
    using StateBase<Exit<state_t<Parent>, state_t<State>>>::operator=;
};
template <class Parent, class State> const exit_t<Parent, State> exit {};

template <class Parent> struct history_t : public StateBase<History<state_t<Parent>>> {
    using StateBase<History<state_t<Parent>>>::operator=;
};
template <class Parent> const history_t<Parent> history {};
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