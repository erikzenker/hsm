#include "hsm/details/remove_duplicates.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;
using namespace hsm;

class RemoveDuplicatesTests : public Test {
};

TEST_F(RemoveDuplicatesTests, should_remove_duplicate_typeids)
{
    auto tuple = boost::hana::make_tuple(boost::hana::typeid_(int{}), boost::hana::typeid_(int{}));

    auto noDuplicatesTuple = remove_duplicate_typeids(tuple);
    auto expectedTuple = boost::hana::make_tuple(boost::hana::typeid_(int{}));

    ASSERT_EQ(expectedTuple, noDuplicatesTuple);
}

TEST_F(RemoveDuplicatesTests, should_remove_duplicates)
{
    auto tuple = boost::hana::make_tuple(42, 42, 'c');

    auto noDuplicatesTuple = remove_duplicate_types(tuple);
    auto expectedTuple = boost::hana::make_tuple(42, 'c');

    ASSERT_EQ(expectedTuple, noDuplicatesTuple);
}