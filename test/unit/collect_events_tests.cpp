#include "hsm/details/collect_events.h"
#include "hsm/details/transition_table.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <memory>

using namespace ::testing;

namespace bh {
using namespace boost::hana;
}

class CollectEventsTests : public Test {
};

namespace {
class E1 {
};
class E2 {
};
class E3 {
};
class E4 {
};
class S1 {
};

struct P {
    static constexpr auto make_transition_table()
    {
        return bh::make_tuple(
            bh::make_tuple(hsm::state<S1> {}, hsm::event<E2> {}, 0, 0, hsm::state<S1> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return bh::make_tuple(bh::make_tuple(hsm::event<E4> {}, 0, 0));
    }
};

struct S {
    static constexpr auto make_transition_table()
    {
        return bh::make_tuple(
            bh::make_tuple(hsm::state<S1> {}, hsm::event<E1> {}, 0, 0, hsm::state<P> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return bh::make_tuple(bh::make_tuple(hsm::event<E3> {}, 0, 0));
    }
};
}

TEST_F(CollectEventsTests, should_collect_event_typeids_recursive)
{
    auto collectedEvents = hsm::collect_event_typeids_recursive(hsm::state<S> {});
    auto expectedEvents = bh::make_tuple(
        bh::typeid_(E1 {}), bh::typeid_(E2 {}), bh::typeid_(E4 {}), bh::typeid_(E3 {}));

    ASSERT_EQ(bh::size(expectedEvents), bh::size(collectedEvents));
    ASSERT_EQ(expectedEvents, collectedEvents);
}

TEST_F(CollectEventsTests, should_collect_events_recursive)
{
    auto collectedEvents
        = bh::transform(hsm::collect_events_recursive(hsm::state<S> {}), bh::typeid_);
    auto expectedEvents = bh::make_tuple(
        bh::typeid_(E1 {}), bh::typeid_(E2 {}), bh::typeid_(E4 {}), bh::typeid_(E3 {}));

    ASSERT_EQ(bh::size(expectedEvents), bh::size(collectedEvents));
    ASSERT_EQ(expectedEvents, collectedEvents);
}