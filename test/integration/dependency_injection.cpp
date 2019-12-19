#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

// States
struct S1 {
};

// Events
struct e1 {
};

// Guards
const auto g1 = [](auto, const auto& dependency) {
    (*dependency.callCount)++;
    return true;
};

// Actions
const auto a1 = [](auto /*event*/, const auto& dependency) { (*dependency.callCount)++; };

using namespace ::testing;

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

}

class DependencyInjectionTests : public Test {
    struct Dependency {
        std::shared_ptr<int> callCount = std::make_shared<int>(0);
    };

  protected:
    DependencyInjectionTests()
        : sm(dependency)
    {
    }

    Dependency dependency;
    hsm::Sm<MainState, Dependency> sm;
};

TEST_F(DependencyInjectionTests, should_inject_dependency_with_shared_ptr)
{
    sm.process_event(e1 {});
    ASSERT_EQ(2, *dependency.callCount);
}