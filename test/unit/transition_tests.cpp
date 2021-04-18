#include "hsm/details/transition.h"

#include <gtest/gtest.h>

using namespace ::testing;

namespace hsm::tests {

class TransitionTests : public Test {
};

class S1 {
};
class S2 {
};

TEST_F(TransitionTests, should_recognize_internal_transition)
{
    auto transition = hsm::details::transition("source", "event", "guard", "avtion", "target");
    auto internalExtendedTransition
        = hsm::details::internal_extended_transition("parent", transition);
    auto externalExtendedTransition = hsm::details::extended_transition("parent", transition);

    static_assert(!externalExtendedTransition.internal(), "");
    static_assert(internalExtendedTransition.internal(), "");
}

}