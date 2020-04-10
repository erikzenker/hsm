#include "hsm/details/collect_events.h"
#include "hsm/details/transition_table.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <memory>

using namespace ::testing;

class CollectEventsTests : public Test {
};

namespace {
class E1 {
};
class E2 {
};
class S1 {
};
}

TEST_F(CollectEventsTests, should_collect_event_typeids_recursive)
{
    struct P {
        constexpr auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(
                hsm::state<S1> {}, hsm::event<E2> {}, 0, 0, hsm::state<S1> {}));
        }
    };

    struct S {
        constexpr auto make_transition_table()
        {
            return boost::hana::make_tuple(boost::hana::make_tuple(
                hsm::state<S1> {}, hsm::event<E1> {}, 0, 0, hsm::state<P> {}));
        }
    };

    auto collectedEvents = hsm::collect_event_typeids_recursive(hsm::state<S> {});
    auto expectedEvents
        = boost::hana::make_tuple(boost::hana::typeid_(E1 {}), boost::hana::typeid_(E2 {}));

    ASSERT_EQ(expectedEvents, collectedEvents);
}