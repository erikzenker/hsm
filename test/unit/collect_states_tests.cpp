#include "hsm/details/collect_parent_states.h"
#include "hsm/details/collect_states.h"
#include "hsm/details/make_states_map.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;

namespace bh {
using namespace boost::hana;
}


namespace {

class CollectStatesTests : public Test {
};

// States
struct S1 {
};
struct S2 {
};
struct S3 {
};
struct S4 {
};

// Events
struct e1 {
    e1(int i)
        : i(i)
    {
    }
    int i;
};

// Guards
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto /*event*/) {};

struct Defer {
    constexpr auto defer_events()
    {
        return hsm::events<e1>;
    }    
};

struct S {

    constexpr auto on_entry()
    {
        return [](auto counter) { (*counter)++; };
    }
    constexpr auto on_exit()
    {
        return [](auto counter) { (*counter)++; };
    }
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(hsm::initial_t<S1> {}, hsm::event_t<e1> {}, g1, a1, hsm::state_t<S2> {})
        );
        // clang-format on
    }
};

struct HistorySubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(hsm::initial_t<S1> {}, hsm::event_t<e1> {}, g1, a1, hsm::state_t<S2> {})
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(hsm::initial_t<S1> {}, hsm::event_t<e1> {}, g1, a1, hsm::state_t<S2> {}),
            hsm::transition(hsm::state_t<S2> {}, hsm::event_t<e1> {}, g1, a1, hsm::state_t<SubState> {}),
            hsm::transition(hsm::state_t<S2> {}, hsm::event_t<e1> {}, g1, a1, hsm::history_t<HistorySubState> {}),
            // Region 1
            hsm::transition(hsm::initial_t<S3> {}, hsm::event_t<e1> {}, g1, a1, hsm::state_t<S4> {})
        );
        // clang-format on
    }
};

}

TEST_F(CollectStatesTests, should_collect_child_states_typeids)
{
    struct S {
        auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S1> {}, 0, 0, 0, hsm::state_t<S1> {}),
                hsm::transition(hsm::state_t<S2> {}, 0, 0, 0, hsm::state_t<S2> {}));
        }
    };

    auto collectedStates = hsm::collect_child_state_typeids(hsm::state_t<S> {});
    auto expectedStates
        = bh::make_tuple(bh::typeid_(hsm::state_t<S1> {}), bh::typeid_(hsm::state_t<S2> {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_resolve_initial_state_on_collect_child_state_typeids)
{
    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(*hsm::state_t<S1> {}, 0, 0, 0, hsm::state_t<S1> {}));
        }
    };

    auto collectedStates = hsm::collect_child_state_typeids(hsm::state_t<S> {});
    auto expectedStates = bh::make_tuple(bh::typeid_(hsm::state_t<S1> {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_child_states)
{
    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S1> {}, 0, 0, 0, hsm::state_t<S1> {}),
                hsm::transition(hsm::state_t<S2> {}, 0, 0, 0, hsm::state_t<S2> {}));
        }
    };

    auto collectedStates
        = bh::transform(hsm::collect_child_states(hsm::state_t<S> {}), bh::typeid_);
    auto expectedStates
        = bh::make_tuple(bh::typeid_(hsm::state_t<S1> {}), bh::typeid_(hsm::state_t<S2> {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_empty_child_state_recursive)
{
    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table();
        }
    };

    auto collectedStates = hsm::collect_child_state_typeids_recursive(hsm::state_t<S> {});
    auto expectedStates = bh::make_tuple();

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_child_state_typeids_recursive_on_top_level)
{
    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S1> {}, 0, 0, 0, hsm::state_t<S1> {}),
                hsm::transition(hsm::state_t<S2> {}, 0, 0, 0, hsm::state_t<S2> {}));
        }
    };

    auto collectedStates = hsm::collect_child_state_typeids_recursive(hsm::state_t<S> {});
    auto expectedStates
        = bh::make_tuple(bh::typeid_(hsm::state_t<S1> {}), bh::typeid_(hsm::state_t<S2> {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_child_state_typeids_recursive)
{
    struct P {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S2> {}, 0, 0, 0, hsm::state_t<S2> {}));
        }
    };

    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S1> {}, 0, 0, 0, hsm::state_t<P> {}));
        }
    };

    auto collectedStates = hsm::collect_child_state_typeids_recursive(hsm::state_t<S> {});
    auto expectedStates = bh::make_tuple(
        bh::typeid_(hsm::state_t<S1> {}),
        bh::typeid_(hsm::state_t<P> {}),
        bh::typeid_(hsm::state_t<S2> {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_at_least_parent_state)
{
    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table();
        }
    };

    auto collectedStates = hsm::collect_state_typeids_recursive(hsm::state_t<S> {});
    auto expectedStates = bh::make_tuple(bh::typeid_(hsm::state_t<S> {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_state_typeids_recursive)
{
    struct P {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S2> {}, 0, 0, 0, hsm::state_t<S2> {}));
        }
    };

    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S1> {}, 0, 0, 0, hsm::state_t<P> {}));
        }
    };

    auto collectedStates = hsm::collect_state_typeids_recursive(hsm::state_t<S> {});
    auto expectedStates = bh::make_tuple(
        bh::typeid_(hsm::state_t<S1> {}),
        bh::typeid_(hsm::state_t<P> {}),
        bh::typeid_(hsm::state_t<S2> {}),
        bh::typeid_(hsm::state_t<S> {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_states_recursive)
{
    struct P {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S2> {}, 0, 0, 0, hsm::state_t<S2> {}));
        }
    };

    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S1> {}, 0, 0, 0, hsm::state_t<P> {}));
        }
    };

    auto collectedStates = hsm::collect_states_recursive(hsm::state_t<S> {});

    ASSERT_EQ(bh::size_c<4>, bh::size(collectedStates));
}

TEST_F(CollectStatesTests, should_collect_parent_state_typeids)
{
    auto collectedParentStates = hsm::collect_parent_state_typeids(hsm::state_t<MainState> {});

    ASSERT_EQ(bh::size_c<3>, bh::size(collectedParentStates));

    auto expectedParentStates = bh::transform(
        bh::make_tuple(
            hsm::state_t<MainState> {},
            hsm::state_t<SubState> {},
            hsm::state_t<HistorySubState> {}),
        bh::typeid_);

    ASSERT_EQ(expectedParentStates, collectedParentStates);
}