#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <future>
#include <memory>

using namespace ::testing;

namespace {
namespace bh = boost::hana;
}

class DispatchTableTests : public Test {
};

struct S {

    constexpr auto on_entry()
    {
        return [](auto counter) { (*counter)++; };
    }
    constexpr auto on_exit()
    {
        return [](auto counter) { (*counter)++; };
    }
};

TEST_F(DispatchTableTests, should_resolve_action)
{
    auto counter = std::make_shared<int>(0);

    auto action = [](auto counter) { (*counter)++; };

    auto transition = bh::make_tuple(0, S {}, 2, 3, action, S {});

    auto resolvedAction = hsm::resolveAction(transition);

    resolvedAction(counter);

    ASSERT_EQ(3, *counter);
}