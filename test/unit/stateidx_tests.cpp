#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/state.h"
#include "hsm/details/transition_table.h"
#include "hsm/front/transition_tuple.h"

#include <gtest/gtest.h>
#include <boost/hana.hpp>

using namespace ::testing;

namespace bh {
    using namespace boost::hana;
}

namespace {

class S1 {
};
class S2 {
};

    struct P {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S2> {}, 0, 0, 0, hsm::state_t<S2> {}));
        }
    };

    struct S {
        static constexpr auto make_transition_table()
        {
            return hsm::transition_table(
                hsm::transition(hsm::state_t<S1> {}, 0, 0, 0, hsm::state_t<P> {}));
        }
    };

    constexpr auto getCombinedStateTypeids()
    {
        return hsm::getCombinedStateTypeids(hsm::state_t<S> {});
    }
}

class StateidxTests : public Test {
};

TEST_F(StateidxTests, should_get_stateidx)
{
    EXPECT_EQ(bh::size_c<3>, hsm::getStateIdx(hsm::state_t<S> {}, hsm::state_t<S> {}));
    EXPECT_EQ(bh::size_c<0>, hsm::getStateIdx(hsm::state_t<S> {}, hsm::state_t<S1> {}));
    EXPECT_EQ(bh::size_c<1>, hsm::getStateIdx(hsm::state_t<S> {}, hsm::state_t<P> {}));
    EXPECT_EQ(bh::size_c<2>, hsm::getStateIdx(hsm::state_t<S> {}, hsm::state_t<S2> {}));
}

TEST_F(StateidxTests, should_get_parent_stateidx)
{
    EXPECT_EQ(bh::size_c<0>, hsm::getParentStateIdx(hsm::state_t<S> {}, hsm::state_t<S> {}));
    EXPECT_EQ(bh::size_c<1>, hsm::getParentStateIdx(hsm::state_t<S> {}, hsm::state_t<P> {}));
}

TEST_F(StateidxTests, should_get_combined_stateidx)
{
    EXPECT_EQ(
        bh::size_c<0>,
        hsm::getCombinedStateIdx(
            getCombinedStateTypeids(), hsm::state_t<S> {}, hsm::state_t<S1> {}));
    EXPECT_EQ(
        bh::size_c<6>,
        hsm::getCombinedStateIdx(
            getCombinedStateTypeids(), hsm::state_t<P> {}, hsm::state_t<S2> {}));
}

TEST_F(StateidxTests, should_calculate_combined_stateidx)
{
    EXPECT_EQ(bh::size_c<4>, hsm::nStates(hsm::state_t<S> {}));
    EXPECT_EQ(bh::size_c<2>, hsm::nParentStates(hsm::state_t<S> {}));

    auto s1StateIdx = hsm::getStateIdx(hsm::state_t<S> {}, hsm::state_t<S1> {});
    auto s2StateIdx = hsm::getStateIdx(hsm::state_t<S> {}, hsm::state_t<S2> {});
    auto sParentStateIdx = hsm::getParentStateIdx(hsm::state_t<S> {}, hsm::state_t<S> {});
    auto pParentStateIdx = hsm::getParentStateIdx(hsm::state_t<S> {}, hsm::state_t<P> {});

    EXPECT_EQ(
        hsm::calcCombinedStateIdx(hsm::nStates(hsm::state_t<S> {}), sParentStateIdx, s1StateIdx),
        hsm::getCombinedStateIdx(
            getCombinedStateTypeids(), hsm::state_t<S> {}, hsm::state_t<S1> {}));
    EXPECT_EQ(
        hsm::calcCombinedStateIdx(hsm::nStates(hsm::state_t<S> {}), pParentStateIdx, s2StateIdx),
        hsm::getCombinedStateIdx(
            getCombinedStateTypeids(), hsm::state_t<P> {}, hsm::state_t<S2> {}));
}

TEST_F(StateidxTests, should_calculate_parent_stateidx)
{
    auto s1CombinedStateIdx = hsm::getCombinedStateIdx(
        getCombinedStateTypeids(), hsm::state_t<S> {}, hsm::state_t<S1> {});
    auto s2CombinedStateIdx = hsm::getCombinedStateIdx(
        getCombinedStateTypeids(), hsm::state_t<P> {}, hsm::state_t<S2> {});

    EXPECT_EQ(0, hsm::calcParentStateIdx(hsm::nStates(hsm::state_t<S> {}), s1CombinedStateIdx));
    EXPECT_EQ(1, hsm::calcParentStateIdx(hsm::nStates(hsm::state_t<S> {}), s2CombinedStateIdx));
}

TEST_F(StateidxTests, should_calculate_stateidx)
{
    auto s1CombinedStateIdx = hsm::getCombinedStateIdx(
        getCombinedStateTypeids(), hsm::state_t<S> {}, hsm::state_t<S1> {});
    auto s2CombinedStateIdx = hsm::getCombinedStateIdx(
        getCombinedStateTypeids(), hsm::state_t<P> {}, hsm::state_t<S2> {});

    EXPECT_EQ(0, hsm::calcStateIdx(hsm::nStates(hsm::state_t<S> {}), s1CombinedStateIdx));
    EXPECT_EQ(2, hsm::calcStateIdx(hsm::nStates(hsm::state_t<S> {}), s2CombinedStateIdx));
}