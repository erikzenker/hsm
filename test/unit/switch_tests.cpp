#include <hsm/hsm.h>

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
    bool isTrueWasCalled = false;
    bool otherwiseWasCalled = false;

    switch_(
        case_(is_true, [&](){
            isTrueWasCalled = true;    
        }),
        case_(otherwise, [&](){
            otherwiseWasCalled = true;
        })
    )();

    ASSERT_TRUE(isTrueWasCalled);
    ASSERT_FALSE(otherwiseWasCalled);
}

TEST_F(SwitchTests, should_call_otherwise_case)
{
    bool isFalseWasCalled = false;
    bool otherwiseWasCalled = false;

    switch_(
        case_(is_false, [&](){
            isFalseWasCalled = true;    
        }),
        case_(otherwise, [&](){
            otherwiseWasCalled = true;
        })
    )();

    ASSERT_FALSE(isFalseWasCalled);
    ASSERT_TRUE(otherwiseWasCalled);
}