#include "hsm/details/collect_events.h"
#include "hsm/details/idx.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

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
        return hsm::transition_table(
            hsm::transition(hsm::state_t<S1> {}, hsm::event_t<E2> {}, 0, 0, hsm::state_t<S1> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition(hsm::event_t<E4> {}, 0, 0));
    }
};

struct S {
    static constexpr auto make_transition_table()
    {
        return hsm::transition_table(
            hsm::transition(hsm::state_t<S1> {}, hsm::event_t<E1> {}, 0, 0, hsm::state_t<P> {}));
    }

    static constexpr auto make_internal_transition_table()
    {
        return hsm::transition_table(hsm::internal_transition(hsm::event_t<E3> {}, 0, 0));
    }
};
}

TEST_F(CollectEventsTests, should_collect_event_typeids_recursive)
{
    auto collectedEvents = hsm::collect_event_typeids_recursive(hsm::state_t<S> {});
    auto expectedEvents = bh::make_basic_tuple(
        bh::typeid_(E1 {}), bh::typeid_(E2 {}), bh::typeid_(E4 {}), bh::typeid_(E3 {}));

    ASSERT_EQ(bh::size(expectedEvents), bh::size(collectedEvents));
    ASSERT_TRUE(bh::equal(expectedEvents, collectedEvents));
}

TEST_F(CollectEventsTests, should_collect_events_recursive)
{
    auto collectedEvents
        = bh::transform(hsm::collect_events_recursive(hsm::state_t<S> {}), bh::typeid_);
    auto expectedEvents = bh::make_basic_tuple(
        bh::typeid_(E1 {}), bh::typeid_(E2 {}), bh::typeid_(E4 {}), bh::typeid_(E3 {}));

    ASSERT_EQ(bh::size(expectedEvents), bh::size(collectedEvents));
    ASSERT_TRUE(bh::equal(expectedEvents, collectedEvents));
}