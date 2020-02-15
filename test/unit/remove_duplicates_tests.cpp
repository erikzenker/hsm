#include "hsm/details/remove_duplicates.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>

using namespace ::testing;
using namespace hsm;

class RemoveDuplicatesTests : public Test {
};

TEST_F(RemoveDuplicatesTests, should_remove_duplicate_typeids)
{
    auto tuple = boost::hana::make_tuple(int{}, std::string{}, int{}, std::string{});
    auto typeids = boost::hana::transform(tuple, boost::hana::typeid_);
    auto noDuplicatesTuple = remove_duplicate_typeids(typeids);
    auto expectedTuple = boost::hana::make_tuple(boost::hana::typeid_(std::string{}), boost::hana::typeid_(int{}));

    ASSERT_EQ(expectedTuple, noDuplicatesTuple);
}

TEST_F(RemoveDuplicatesTests, should_remove_duplicates)
{
    auto tuple = boost::hana::make_tuple('c', 42, 42, 'c', 42, 'c');

    auto noDuplicatesTuple = remove_duplicate_types(tuple);
    auto expectedTuple = boost::hana::make_tuple('c', 42);

    ASSERT_EQ(expectedTuple, noDuplicatesTuple);
}