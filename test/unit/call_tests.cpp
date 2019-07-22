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

TEST_F(CallTests, should_call_nth_callable_reverse)
{
    auto c1Called = false;
    auto c2Called = false;

    auto c1 = [&c1Called](){ c1Called = true;};
    auto c2 = [&c2Called](){ c2Called = true;};

    auto callables = boost::hana::make_tuple(c1, c2);
    
    call(1, callables);
    ASSERT_TRUE(c2Called);
    call(0, callables);
    ASSERT_TRUE(c1Called);
}

TEST_F(CallTests, should_call_nth_callable_with_args)
{
    struct Arg0 {
        bool arg0Called = false;    
    };

    struct Arg1 {
        bool arg1Called = false;        
    };    

    Arg0 arg0;
    Arg1 arg1;

    auto c0 = [](Arg0& arg0){ arg0.arg0Called = true;};
    auto c1 = [](Arg1& arg1){ arg1.arg1Called = true;};

    auto callables = boost::hana::make_tuple(c0, c1);
    
    call(0, callables, arg0);
    ASSERT_TRUE(arg0.arg0Called);
    call(1, callables, arg1);
    ASSERT_TRUE(arg1.arg1Called);
}


TEST_F(CallTests, should_forward_arguments)
{
    auto c1Called = false;

    auto c1 = [](bool& called){ called = true;};
    
    auto callables = boost::hana::make_tuple(c1);
    
    call(0, callables, c1Called);
    ASSERT_TRUE(c1Called);
}