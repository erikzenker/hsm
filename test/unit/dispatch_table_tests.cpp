#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <future>
#include <memory>

using namespace ::testing;

class DispatchTableTests : public Test {
};

struct S {

    constexpr auto on_entry()
    {
        return [](){return true;};
    }
};

TEST_F(DispatchTableTests, should_resolve_action)
{
    namespace bh = boost::hana;

    auto action = []() {return false;};

    auto foo = S{}.on_entry();

    auto transition = bh::make_tuple(0, 1, 2, 3, action, S{});

    auto resolvedAction = hsm::resolveAction(transition);

    ASSERT_TRUE(resolvedAction());
}
