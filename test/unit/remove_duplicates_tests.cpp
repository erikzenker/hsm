#include "hsm/details/remove_duplicates.h"
#include "hsm/details/utils/toTypeid.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>

using namespace ::testing;
using namespace hsm::details::utils;
using namespace hsm;

class RemoveDuplicatesTests : public Test {
};

TEST_F(RemoveDuplicatesTests, should_remove_duplicates)
{
    auto tuple = boost::hana::make_tuple('c', 42, 42, 'c', 42, 'c', 'c');
    auto typeids = boost::hana::transform(tuple, boost::hana::typeid_);
    auto noDuplicatesTuple = remove_duplicates(typeids);
    auto expectedTuple
        = boost::hana::transform(boost::hana::make_tuple('c', 42), boost::hana::typeid_);

    ASSERT_EQ(expectedTuple, noDuplicatesTuple);
}

TEST_F(RemoveDuplicatesTests, should_remove_duplicate_types)
{
    auto tuple = boost::hana::make_tuple('c', 42, 42, 'c', 42, 'c', 'c');
    auto noDuplicatesTuple = toTypeid(remove_duplicate_types(tuple));
    auto expectedTuple = toTypeid(boost::hana::make_tuple('c', 42));

    ASSERT_EQ(expectedTuple, noDuplicatesTuple);
}