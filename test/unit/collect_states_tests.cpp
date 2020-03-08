#include "hsm/details/collect_states.h"
#include "hsm/details/collect_parent_states.h"
#include "hsm/details/transition_table.h"

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
};

// Guards
const auto g1 = [](auto) { return true; };

// Actions
const auto a1 = [](auto event) {};

struct Defer {
    constexpr auto defer_events()
    {
        return hsm::defer(e1{});
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
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Region 0    
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S2 {}, hsm::event<e1> {}, g1, a1, SubState {}),
            // Region 1
            hsm::transition(S3 {}, hsm::event<e1> {}, g1, a1, S4 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {}, S3 {});
    }
};

}

TEST_F(CollectStatesTests, should_collect_child_states)
{
    struct S {
        auto make_transition_table()
        {
            return bh::make_tuple(
                bh::make_tuple(S1 {}, 0, 0, 0, S1 {}),
                bh::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    auto collectedStates = hsm::collect_child_states(S {});
    auto expectedStates = bh::make_tuple(bh::typeid_(S1 {}), bh::typeid_(S2 {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_empty_child_state_recursive)
{
    struct S {
        auto make_transition_table()
        {
            return bh::make_tuple();
        }
    };

    auto collectedStates = hsm::collect_child_state_typeids_recursive(S {});
    auto expectedStates = bh::make_tuple();

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_child_state_typeids_recursive_on_top_level)
{
    struct S {
        auto make_transition_table()
        {
            return bh::make_tuple(
                bh::make_tuple(S1 {}, 0, 0, 0, S1 {}),
                bh::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    auto collectedStates = hsm::collect_child_state_typeids_recursive(S {});
    auto expectedStates = bh::make_tuple(bh::typeid_(S1 {}), bh::typeid_(S2 {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_child_state_typeids_recursive)
{
    struct P {
        auto make_transition_table()
        {
            return bh::make_tuple(bh::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return bh::make_tuple(bh::make_tuple(S1 {}, 0, 0, 0, P {}));
        }
    };

    auto collectedStates = hsm::collect_child_state_typeids_recursive(S {});
    auto expectedStates = bh::make_tuple(bh::typeid_(S1 {}), bh::typeid_(P {}), bh::typeid_(S2 {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_at_least_parent_state)
{
    struct S {
        auto make_transition_table()
        {
            return bh::make_tuple();
        }
    };

    auto collectedStates = hsm::collect_state_typeids_recursive(S {});
    auto expectedStates = bh::make_tuple(bh::typeid_(S {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_state_typeids_recursive)
{
    struct P {
        auto make_transition_table()
        {
            return bh::make_tuple(bh::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return bh::make_tuple(bh::make_tuple(S1 {}, 0, 0, 0, P {}));
        }
    };

    auto collectedStates = hsm::collect_state_typeids_recursive(S {});
    auto expectedStates = bh::make_tuple(
        bh::typeid_(S1 {}), bh::typeid_(P {}), bh::typeid_(S2 {}), bh::typeid_(S {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_states_recursive)
{
    struct P {
        auto make_transition_table()
        {
            return bh::make_tuple(bh::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return bh::make_tuple(bh::make_tuple(S1 {}, 0, 0, 0, P {}));
        }
    };

    auto collectedStates = hsm::collect_states_recursive(S {});

    ASSERT_EQ(bh::size_c<4>, bh::size(collectedStates));
}

TEST_F(CollectStatesTests, should_collect_parent_state_typeids)
{
    auto collectedParentStates = hsm::collect_parent_state_typeids(MainState {});
    ASSERT_EQ(bh::make_tuple(bh::typeid_(MainState{}), bh::typeid_(SubState{})), collectedParentStates);
    ASSERT_EQ(bh::size_c<2>, bh::size(collectedParentStates));
}