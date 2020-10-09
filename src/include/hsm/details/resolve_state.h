#pragma once

#include "hsm/details/resolve_state.h"
#include "hsm/details/switch.h"

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto resolveDst = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dstTypeid){
        return lazy_switch_(
            // TODO: make multi region capable    
            case_(bh::make_lazy(has_transition_table(dstTypeid)), [](auto&& submachine, auto&&) { return bh::at_c<0>(collect_initial_states(submachine));}),
            case_(bh::make_lazy(is_entry_state(dstTypeid)),       [](auto&& entry, auto&&) { return unwrap_typeid(entry).get_state(); }),
            case_(bh::make_lazy(is_direct_state(dstTypeid)),      [](auto&& direct, auto&&) { return unwrap_typeid(direct).get_state(); }),
            // TODO: make multi region capable 
            case_(bh::make_lazy(is_history_state(dstTypeid)),     [](auto&&, auto&& history) { return bh::at_c<0>(collect_initial_states(history.get_parent_state()));}),
            case_(bh::make_lazy((otherwise())),                   [](auto&& dstTypeid, auto&&) { return dstTypeid; }))
            (dstTypeid, unwrap_typeid(dstTypeid));
    },
    transition.target());
    // clang-format on
};

constexpr auto resolveDstParent = [](auto transition) {
    // clang-format off
    return bh::apply([](auto&& dstTypeid, auto&& transition){
        return lazy_switch_(
            case_(bh::make_lazy(has_transition_table(dstTypeid)), [](auto&& submachine, auto&&) { return submachine; }),
            case_(bh::make_lazy(is_entry_state(dstTypeid)),       [](auto&& entry, auto&&) { return unwrap_typeid(entry).get_parent_state(); }),
            case_(bh::make_lazy(is_direct_state(dstTypeid)),      [](auto&& direct, auto&&) { return unwrap_typeid(direct).get_parent_state(); }),
            case_(bh::make_lazy(is_history_state(dstTypeid)),     [](auto&& history, auto&&) { return unwrap_typeid(history).get_parent_state(); }),
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
            case_(bh::make_lazy(is_initial_state(src)), [](auto&& initial) { return unwrap_typeid(initial).get_state(); }),    
            case_(bh::make_lazy(is_exit_state(src)),    [](auto&& exit) { return unwrap_typeid(exit).get_state(); }),
            case_(bh::make_lazy(is_direct_state(src)),  [](auto&& direct) { return unwrap_typeid(direct).get_state(); }),
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
            case_(bh::make_lazy(is_exit_state(src)),   [](auto&& exit, auto&&) { return unwrap_typeid(exit).get_parent_state(); }),
            case_(bh::make_lazy(is_direct_state(src)), [](auto&& direct, auto&&) { return unwrap_typeid(direct).get_parent_state(); }),
            case_(bh::make_lazy(otherwise()),          [](auto&&, auto&& transition) { return transition.parent(); }))
            (src, transition);
    },
    transition.source(), transition);
    // clang-format on
};

}