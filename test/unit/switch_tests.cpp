
#include "hsm/details/switch.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <iostream>

using namespace ::testing;
using namespace hsm;

const auto is_true = boost::hana::is_valid([](auto...){ });
const auto is_false = [](auto...){ return boost::hana::false_c;};

const auto is_lazy_false = bh::make_lazy([](auto...) { return boost::hana::false_c; });
const auto is_lazy_true = bh::make_lazy([](auto...) { return boost::hana::true_c; });

class SwitchTests : public Test {
};

TEST_F(SwitchTests, should_call_is_true_case)
{
    // clang-format off
    auto result = switch_(
        case_(is_true(), []() { return true; }), case_(otherwise(), []() { return false; }))();
    // clang-format on
    ASSERT_TRUE(result);
}

TEST_F(SwitchTests, should_call_otherwise_case)
{
    // clang-format off
    auto result = switch_(
        case_(is_false(), []() { return false; }), case_(otherwise(), []() { return true; }))();
    // clang-format on
    ASSERT_TRUE(result);
}

TEST_F(SwitchTests, should_eval_lazy_true)
{
    // clang-format off
    auto result = lazy_switch_(
        case_(is_lazy_true(), []() { return true; }),
        case_(lazy_otherwise(), []() { return false; }))();
    // clang-format on
    ASSERT_TRUE(result);
}

TEST_F(SwitchTests, should_eval_lazy_otherwise)
{
    // clang-format off
    auto result = lazy_switch_(
        case_(is_lazy_false(), []() { return false; }),
        case_(lazy_otherwise(), []() { return true; }))();
    // clang-format on
    ASSERT_TRUE(result);
}