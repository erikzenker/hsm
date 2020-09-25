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