#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;

class CollectEventsTests : public Test {
};

class E1 {
};
class E2 {
};

TEST_F(CollectEventsTests, should_collect_events)
{
    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(
                boost::hana::make_tuple(0, E1 {}, 0, 0, 0),
                boost::hana::make_tuple(0, E2 {}, 0, 0, 0));
        }
    };

    auto collectedEvents = hsm::collect_events(S {});
    auto expectedEvents
        = boost::hana::make_tuple(boost::hana::typeid_(E1 {}), boost::hana::typeid_(E2 {}));

    ASSERT_EQ(expectedEvents, collectedEvents);
}

TEST_F(CollectEventsTests, should_collect_events_recursive)
{
    struct P {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(0, E2 {}, 0, 0, 0));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(0, E1 {}, 0, 0, P {}));
        }
    };

    auto collectedEvents = hsm::collect_events_recursive(S {});
    auto expectedEvents
        = boost::hana::make_tuple(boost::hana::typeid_(E1 {}), boost::hana::typeid_(E2 {}));

    ASSERT_EQ(expectedEvents, collectedEvents);
}