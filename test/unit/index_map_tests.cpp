#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;

class IndexMapTests : public Test {
};

class S1 {
};
class S2 {
};

TEST_F(IndexMapTests, should_make_index_map)
{
    auto T1 = boost::hana::typeid_(S1 {});
    auto T2 = boost::hana::typeid_(S2 {});

    auto tuple = boost::hana::make_tuple(T1, T2);
    auto indexMap = hsm::make_index_map(tuple);

    ASSERT_EQ(boost::hana::int_c<0>, boost::hana::find(indexMap, T1).value());
    ASSERT_EQ(boost::hana::int_c<1>, boost::hana::find(indexMap, T2).value());
}
