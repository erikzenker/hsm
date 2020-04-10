#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/transition_table.h"

#include <gtest/gtest.h>
#include <boost/hana.hpp>

using namespace ::testing;

namespace bh {
    using namespace boost::hana;
}

class StateidxTests : public Test {
};

namespace {

class S1 {
};
class S2 {
};

    struct P {
        auto make_transition_table()
        {
            return bh::make_tuple(bh::make_tuple(S2 {}, 0, 0, 0, S2 {}));
        }
    };

    struct S {
        auto make_transition_table()
        {
            return bh::make_tuple(bh::make_tuple(S1 {}, 0, 0, 0, P {}));
        }
    };


}

TEST_F(StateidxTests, should_get_stateidx)
{
    EXPECT_EQ(bh::size_c<3>, hsm::getStateIdx(S{}, S{}));
    EXPECT_EQ(bh::size_c<0>, hsm::getStateIdx(S {}, S1 {}));
    EXPECT_EQ(bh::size_c<1>, hsm::getStateIdx(S{}, P{}));
    EXPECT_EQ(bh::size_c<2>, hsm::getStateIdx(S {}, S2 {}));
}

TEST_F(StateidxTests, should_get_parent_stateidx)
{
    EXPECT_EQ(bh::size_c<0>, hsm::getParentStateIdx(S {}, S {}));
    EXPECT_EQ(bh::size_c<1>, hsm::getParentStateIdx(S {}, P {}));
}

TEST_F(StateidxTests, should_get_combined_stateidx)
{
    EXPECT_EQ(
        bh::size_c<0>, hsm::getCombinedStateIdx(hsm::getCombinedStateTypeids(S {}), S {}, S1 {}));
    EXPECT_EQ(
        bh::size_c<6>, hsm::getCombinedStateIdx(hsm::getCombinedStateTypeids(S {}), P {}, S2 {}));
}

TEST_F(StateidxTests, should_calculate_combined_stateidx)
{
    EXPECT_EQ(bh::size_c<4>, hsm::nStates(S{}));
    EXPECT_EQ(bh::size_c<2>, hsm::nParentStates(S{}));

    auto s1StateIdx = hsm::getStateIdx(S{}, S1{});
    auto s2StateIdx = hsm::getStateIdx(S{}, S2{});
    auto sParentStateIdx = hsm::getParentStateIdx(S{}, S{});
    auto pParentStateIdx = hsm::getParentStateIdx(S{}, P{});

    EXPECT_EQ(hsm::calcCombinedStateIdx(hsm::nStates(S{}), sParentStateIdx, s1StateIdx), hsm::getCombinedStateIdx(hsm::getCombinedStateTypeids(S{}), S{}, S1{}));
    EXPECT_EQ(hsm::calcCombinedStateIdx(hsm::nStates(S{}), pParentStateIdx, s2StateIdx), hsm::getCombinedStateIdx(hsm::getCombinedStateTypeids(S{}), P{}, S2{}));
}

TEST_F(StateidxTests, should_calculate_parent_stateidx)
{
    auto s1CombinedStateIdx = hsm::getCombinedStateIdx(hsm::getCombinedStateTypeids(S{}), S {}, S1 {});
    auto s2CombinedStateIdx = hsm::getCombinedStateIdx(hsm::getCombinedStateTypeids(S{}), P {}, S2 {});

    EXPECT_EQ(0, hsm::calcParentStateIdx(hsm::nStates(S {}), s1CombinedStateIdx));
    EXPECT_EQ(1, hsm::calcParentStateIdx(hsm::nStates(S {}), s2CombinedStateIdx));
}

TEST_F(StateidxTests, should_calculate_stateidx)
{
    auto s1CombinedStateIdx = hsm::getCombinedStateIdx(hsm::getCombinedStateTypeids(S{}), S {}, S1 {});
    auto s2CombinedStateIdx = hsm::getCombinedStateIdx(hsm::getCombinedStateTypeids(S{}), P {}, S2 {});

    EXPECT_EQ(0, hsm::calcStateIdx(hsm::nStates(S {}), s1CombinedStateIdx));
    EXPECT_EQ(2, hsm::calcStateIdx(hsm::nStates(S {}), s2CombinedStateIdx));
}