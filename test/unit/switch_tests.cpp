
#include "hsm/details/switch.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <iostream>

using namespace ::testing;
using namespace hsm;

const auto is_true = boost::hana::is_valid([](auto...){ });
const auto is_false = [](auto...){ return boost::hana::false_c;};


class SwitchTests : public Test {
};

TEST_F(SwitchTests, should_call_is_true_case)
{
    auto result = switch_(
        case_(is_true(), []() { return true; }), case_(otherwise(), []() { return false; }))();

    ASSERT_TRUE(result);
}

TEST_F(SwitchTests, should_call_otherwise_case)
{
    auto result = switch_(
        case_(is_false(), []() { return false; }), case_(otherwise(), []() { return true; }))();

    ASSERT_TRUE(result);
}