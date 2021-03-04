#include "hsm/details/make_dispatch_tables.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

using namespace ::testing;

namespace hsm::tests {
namespace bh = boost::hana;

// States
struct S1 {
};
struct S2 {
};

// Events
struct e1 {
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(hsm::initial_t<S1>{}, hsm::event_t<e1>{}, hsm::noGuard {}, hsm::noAction {}, hsm::state_t<S2>{}),
            hsm::transition(hsm::initial_t<S2>{}, hsm::event_t<hsm::noneEvent>{}, hsm::noGuard {}, hsm::noAction {}, hsm::state_t<S2>{})
        );
        // clang-format on
    }
};

class MakeDispatchTablesTests : public Test {
};

TEST(MakeDispatchTablesTests, should_make_dispatch_tables)
{
    auto dispatchTables = hsm::make_dispatch_tables(hsm::state_t<MainState> {});
    ASSERT_EQ(2u, bh::size(dispatchTables));
    ASSERT_EQ(3u, dispatchTables[bh::typeid_(e1 {})].size());
    ASSERT_EQ(3u, dispatchTables[bh::typeid_(hsm::noneEvent {})].size());
}
}