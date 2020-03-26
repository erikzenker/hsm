#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>
#include <boost/mp11.hpp>

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

    ASSERT_EQ(boost::hana::size_c<0>, boost::hana::find(indexMap, T1).value());
    ASSERT_EQ(boost::hana::size_c<1>, boost::hana::find(indexMap, T2).value());
}

TEST_F(IndexMapTests, should_make_reverse_index_map)
{
    auto T1 = boost::hana::typeid_(S1 {});
    auto T2 = boost::hana::typeid_(S2 {});

    auto tuple = boost::hana::make_tuple(T1, T2);
    auto reverseIndexMap = hsm::make_reverse_index_map(tuple);

    ASSERT_EQ(boost::hana::size_c<2>, boost::hana::size(reverseIndexMap));

    hsm::find(reverseIndexMap, 0, [T1](auto&& element){ASSERT_EQ(T1, element);});
    hsm::find(reverseIndexMap, 1, [T2](auto&& element){ASSERT_EQ(T2, element);});
}

TEST_F(IndexMapTests, should_return_index_of_tuple)
{
    auto T1 = boost::hana::typeid_(S1 {});
    auto T2 = boost::hana::typeid_(S2 {});

    auto tuple = boost::hana::make_tuple(T1, T2);

    ASSERT_EQ(boost::hana::size_c<0>, hsm::index_of(tuple, T1));
    ASSERT_EQ(boost::hana::size_c<1>, hsm::index_of(tuple, T2));
}


