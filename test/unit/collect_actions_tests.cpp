#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;

class CollectActionsTests : public Test {
};

class E1 {
};
class E2 {
};

const auto e1 = []() {};
const auto e2 = []() {};

auto a1Called = false;
auto a1 = [&a1Called]() { a1Called = true; };

TEST_F(CollectActionsTests, should_collect_actions)
{
    struct S {
        constexpr auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(0, 0, 0, e1, 0), boost::hana::make_tuple(0, 0, 0, e2, 0));
        }
    };

    auto collectedActions = hsm::collect_actions_recursive(S {});
    auto expectedActions
        = boost::hana::make_tuple(boost::hana::typeid_(e1), boost::hana::typeid_(e2));

    ASSERT_EQ(expectedActions, collectedActions);
}

TEST_F(CollectActionsTests, should_call_actions)
{

    struct S {
        constexpr auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(0, 0, 0, a1, 0));
        }
    };

    hsm::call_actions_recursive(S {});
    ASSERT_TRUE(a1Called);
}

TEST_F(CollectActionsTests, should_collect_actions_recursive)
{
    struct P {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(0, 0, 0, e2, 0));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(0, 0, 0, e1, P {}));
        }
    };

    auto collectedActions = hsm::collect_actions_recursive(S {});
    auto expectedActions
        = boost::hana::make_tuple(boost::hana::typeid_(e1), boost::hana::typeid_(e2));

    ASSERT_EQ(expectedActions, collectedActions);
}