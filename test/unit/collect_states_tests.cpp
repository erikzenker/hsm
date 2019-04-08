#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;

class CollectStatesTests : public Test {
};

class S1 {
};
class S2 {
};

TEST_F(CollectStatesTests, should_collect_child_states)
{
    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(S1 {}, 0, 0, 0, S1 {}),
                boost::hana::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    auto collectedStates = hsm::collect_child_states(S {});
    auto expectedStates
        = boost::hana::make_tuple(boost::hana::typeid_(S1 {}), boost::hana::typeid_(S2 {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_empty_child_state_recursive)
{
    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple();
        }
    };

    auto collectedStates = hsm::collect_child_states_recursive(S {});
    auto expectedStates = boost::hana::make_tuple();

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_child_states_recursive_on_top_level)
{
    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(S1 {}, 0, 0, 0, S1 {}),
                boost::hana::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    auto collectedStates = hsm::collect_child_states_recursive(S {});
    auto expectedStates
        = boost::hana::make_tuple(boost::hana::typeid_(S1 {}), boost::hana::typeid_(S2 {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_child_states_recursive)
{
    struct P {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(S1 {}, 0, 0, 0, P {}));
        }
    };

    auto collectedStates = hsm::collect_child_states_recursive(S {});
    auto expectedStates = boost::hana::make_tuple(
        boost::hana::typeid_(S1 {}), boost::hana::typeid_(P {}), boost::hana::typeid_(S2 {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_at_least_parent_state)
{
    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple();
        }
    };

    auto collectedStates = hsm::collect_states_recursive(S {});
    auto expectedStates = boost::hana::make_tuple(boost::hana::typeid_(S {}));

    ASSERT_EQ(expectedStates, collectedStates);
}

TEST_F(CollectStatesTests, should_collect_states_recursive)
{
    struct P {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(S1 {}, 0, 0, 0, P {}));
        }
    };

    auto collectedStates = hsm::collect_states_recursive(S {});
    auto expectedStates = boost::hana::make_tuple(
        boost::hana::typeid_(S1 {}),
        boost::hana::typeid_(P {}),
        boost::hana::typeid_(S2 {}),
        boost::hana::typeid_(S {}));

    ASSERT_EQ(expectedStates, collectedStates);
}