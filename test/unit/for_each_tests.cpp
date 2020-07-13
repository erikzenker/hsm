#include "hsm/details/for_each.h"

#include <boost/hana/basic_tuple.hpp>

#include <gtest/gtest.h>

using namespace boost::hana;
using namespace ::testing;

class ForEachTests : public Test {
};

TEST_F(ForEachTests, should_iterate_over_sequence)
{
    const auto expectedElement = 1;
    const auto sequence = make_basic_tuple(expectedElement, expectedElement);

    hsm::for_each(sequence, [expectedElement](auto element){
        ASSERT_EQ(expectedElement, element);
    });
}

TEST_F(ForEachTests, should_iterate_over_sequence_in_correct_order)
{
    const auto sequence = make_basic_tuple(0, 1, 2, 3, 4);
    auto expectedElement = 0;

    hsm::for_each(sequence, [&expectedElement](auto element){
        ASSERT_EQ(expectedElement, element);
        expectedElement++;
    });
}