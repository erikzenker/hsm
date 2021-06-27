#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

using namespace ::testing;
using namespace boost::hana;

// States
struct S1 {
};
struct S2 {
};
struct S3 {
};
struct S4 {
};
struct Exit {
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};
struct e4 {
};
struct e5 {
};
struct e6 {
};

constexpr auto increaseCallCount
    = [](const auto&, const auto&, const auto&, auto& dep) { dep.callCount++; };

struct SubState1 {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>                  / hsm::log = hsm::state<S2>
        );
        // clang-format on
    }
};

struct SubState2 {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>                  / hsm::chain(hsm::log, increaseCallCount) = hsm::state<S2>
            , hsm::state<S2> + hsm::event<e4> / hsm::log = hsm::state<S3>
        );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            * hsm::state<S1>            + hsm::event<e1> / hsm::log = hsm::state<SubState1>
            , hsm::state<S1>            + hsm::event<e2> / hsm::log = hsm::state<SubState2>
            , hsm::state<SubState1>     + hsm::event<e3> / hsm::log = hsm::state<SubState2>
        );
        // clang-format on
    }

    static constexpr auto on_unexpected_event()
    {
        return [](auto event, auto /*currentState*/, auto /* dependency */) {
            throw std::runtime_error(
                std::string("unexpected event ") + experimental::print(typeid_(event)));
        };
    }
};

struct Dependency {
    std::size_t callCount = 0;
};
}

class ReproducerTests : public Test {
  protected:
    ReproducerTests()
        : sm(dep)
    {
    }

    Dependency dep;
    hsm::sm<MainState, Dependency> sm;
};

TEST_F(ReproducerTests, shouldExecuteAnonymousTransitionOnce)
{
    ASSERT_TRUE(sm.is(hsm::state<MainState>, hsm::state<S1>));
    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState2>, hsm::state<S2>));
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(hsm::state<SubState2>, hsm::state<S3>));
    ASSERT_EQ(1, dep.callCount);
}