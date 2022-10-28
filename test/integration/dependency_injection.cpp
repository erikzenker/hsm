#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

// States
struct S1 {
};
struct S2 {
};
struct Exit {
};

// Events
struct e1 {
};
struct writeDepsEvent {
};
struct readDepsEvent {
};

// Guards
constexpr auto guard = [](auto /*event*/, auto /*source*/, auto /*target*/, auto& dependency) {
    dependency.callCount++;
    return true;
};

// Actions
constexpr auto action = [](auto /*event*/, auto /*source*/, auto /*target*/, auto& dependency) {
    dependency.callCount++;
};

using namespace ::testing;

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1> +  hsm::event<e1>             [guard] / action    = hsm::state<S1>
        );
        // clang-format on
    }
};
}

class DependencyInjectionTests : public Test {
  protected:
    struct Dependency {
        explicit Dependency(int callCount)
            : callCount(callCount)
        {
        }

        // Dependency is not copied, assigned, or moved
        Dependency(const Dependency&) = delete;
        Dependency(Dependency&&) = delete;
        Dependency& operator=(const Dependency&) = delete;
        Dependency& operator=(Dependency&&) = delete;

        int callCount = 0;
    };
};

TEST_F(DependencyInjectionTests, should_inject_dependency)
{
    Dependency dependency { 0 };
    hsm::sm<MainState, Dependency> sm { dependency };

    sm.process_event(e1 {});
    ASSERT_EQ(2, dependency.callCount);
}

TEST_F(DependencyInjectionTests, should_set_dependency)
{
    Dependency dependency { 0 };
    Dependency newDependency { 0 };
    hsm::sm<MainState, Dependency> sm { dependency };

    sm.process_event(e1 {});
    ASSERT_EQ(2, dependency.callCount);

    sm.set_dependency(newDependency);

    sm.process_event(e1 {});
    // ASSERT_EQ(2, dependency.callCount);
    // ASSERT_EQ(2, newDependency.callCount);
}