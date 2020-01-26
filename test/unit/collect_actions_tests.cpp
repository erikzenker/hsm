#include "hsm/details/collect_actions.h"
#include "hsm/details/transition_table.h"

#include <gtest/gtest.h>
#include <boost/hana.hpp>   

#include <functional>
#include <future>

using namespace ::testing;

class CollectActionsTests : public Test {
};

class E1 {
};
class E2 {
};
class E3 {
    public:    
        E3(){}
        E3(const std::shared_ptr<std::promise<void>> wasCalled) : wasCalled(wasCalled){}

    std::shared_ptr<std::promise<void>>  wasCalled;
};

const auto e1 = []() {};
const auto e2 = []() {};

const auto a1 = [](auto event) { event.wasCalled->set_value();};

TEST_F(CollectActionsTests, should_collect_action_typeids)
{
    struct S {
        constexpr auto make_transition_table()
        {
            // clang-format off    
            return hsm::transition_table(
                hsm::transition(0, 0, 0, e1, 0),
                hsm::transition(0, 0, 0, e2, 0)
            );
            // clang-format on
        }
    };

    auto collectedActions = hsm::collect_action_typeids_recursive(S {});
    auto expectedActions
        = boost::hana::make_tuple(boost::hana::typeid_(e1), boost::hana::typeid_(e2));

    ASSERT_EQ(expectedActions, collectedActions);
}

TEST_F(CollectActionsTests, should_collect_action_typeids_recursive)
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

    auto collectedActions = hsm::collect_action_typeids_recursive(S {});
    auto expectedActions
        = boost::hana::make_tuple(boost::hana::typeid_(e1), boost::hana::typeid_(e2));

    ASSERT_EQ(expectedActions, collectedActions);
}

TEST_F(CollectActionsTests, should_collect_actions_recursive)
{
    struct S {
        constexpr auto make_transition_table()
        {
            // clang-format off    
            return hsm::transition_table(
                hsm::transition(0, 0, 0, e1, 0),
                hsm::transition(0, 0, 0, e2, 0)
            );
            // clang-format on
        }
    };

    auto collectedActions = hsm::collect_actions_recursive(S {});
    auto expectedActions
        = boost::hana::make_tuple(e1, e2);

    ASSERT_EQ(expectedActions, collectedActions);
}

TEST_F(CollectActionsTests, should_not_collect_actions_twice)
{
    struct S {
        constexpr auto make_transition_table()
        {
            // clang-format off    
            return hsm::transition_table(
                hsm::transition(0, 0, 0, e1, 0),
                hsm::transition(0, 0, 0, e1, 0)
            );
            // clang-format on
        }
    };

    auto collectedActions = hsm::collect_actions_recursive(S {});
    auto expectedActions
        = boost::hana::make_tuple(e1);

    ASSERT_EQ(expectedActions, collectedActions);
}
