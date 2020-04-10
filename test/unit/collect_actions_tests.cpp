#include "hsm/details/collect_actions.h"
#include "hsm/details/transition_table.h"

#include <gtest/gtest.h>
#include <boost/hana.hpp>   

#include <functional>
#include <future>

using namespace ::testing;

class CollectActionsTests : public Test {
};

namespace {
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
class S1 {
};

const auto e1 = []() {};
const auto e2 = []() {};

const auto a1 = [](auto event) { event.wasCalled->set_value();};
}

TEST_F(CollectActionsTests, should_collect_action_typeids)
{
    struct S {
        constexpr auto make_transition_table()
        {
            // clang-format off
            return hsm::transition_table(
                hsm::transition(hsm::state<S1> {}, 0, 0, e1, hsm::state<S1> {}),
                hsm::transition(hsm::state<S1> {}, 0, 0, e2, hsm::state<S1> {}));
            // clang-format on
        }
    };

    auto collectedActions = hsm::collect_action_typeids_recursive(hsm::state<S> {});
    auto expectedActions
        = boost::hana::make_tuple(boost::hana::typeid_(e1), boost::hana::typeid_(e2));

    ASSERT_EQ(expectedActions, collectedActions);
}

TEST_F(CollectActionsTests, should_collect_action_typeids_recursive)
{
    struct P {
        constexpr auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(hsm::state<S1> {}, 0, 0, e2, hsm::state<S1> {}));
        }
    };

    struct S {
        constexpr auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(hsm::state<S1> {}, 0, 0, e1, hsm::state<P> {}));
        }
    };

    auto collectedActions = hsm::collect_action_typeids_recursive(hsm::state<S> {});
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
                hsm::transition(hsm::state<S1> {}, 0, 0, e1, hsm::state<S1> {}),
                hsm::transition(hsm::state<S1> {}, 0, 0, e2, hsm::state<S1> {}));
            // clang-format on
        }
    };

    auto collectedActions = hsm::collect_actions_recursive(hsm::state<S> {});
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
                hsm::transition(hsm::state<S1> {}, 0, 0, e1, hsm::state<S1> {}),
                hsm::transition(hsm::state<S1> {}, 0, 0, e1, hsm::state<S1> {}));
            // clang-format on
        }
    };

    auto collectedActions = hsm::collect_actions_recursive(hsm::state<S> {});
    auto expectedActions
        = boost::hana::make_tuple(e1);

    ASSERT_EQ(expectedActions, collectedActions);
}
