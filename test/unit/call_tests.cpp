#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

using namespace ::testing;
using namespace hsm;

class CallTests : public Test {
};

TEST_F(CallTests, should_call_nth_callable)
{
    auto c1Called = false;
    auto c2Called = false;

    auto c1 = [&c1Called](){ c1Called = true;};
    auto c2 = [&c2Called](){ c2Called = true;};

    auto callables = boost::hana::make_tuple(c1, c2);
    
    call(0, callables);
    ASSERT_TRUE(c1Called);
    call(1, callables);
    ASSERT_TRUE(c2Called);
}

TEST_F(CallTests, should_forward_arguments)
{
    auto c1Called = false;

    auto c1 = [](bool& called){ called = true;};
    
    auto callables = boost::hana::make_tuple(c1);
    
    call(0, callables, c1Called);
    ASSERT_TRUE(c1Called);
}