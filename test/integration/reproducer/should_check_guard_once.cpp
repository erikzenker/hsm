#include "hsm/hsm.h"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace boost::hana;

namespace {

// States
// -- sub
struct SubStateEntry { };
struct SubStateIntermediate { };
struct SubExit1 { };
struct SubExit2 { };
// -- main
struct Initial { };
struct Intermediate { };
struct Final { };

// Events
struct entrySubEvent { };
struct exitSubEvent1 { };
struct exitSubEvent2 { };
struct mainEvent1 { };

// Guards
const auto guard = [](auto /*event*/, auto /*source*/, auto /*target*/, auto& dependency) {
    dependency.callCount++;
    return false;
};

struct SubState {
    static constexpr auto make_transition_table()
    {
        using namespace hsm;

        // clang-format off
        return transition_table(
            * state<SubStateEntry>                                                                = state<SubStateIntermediate>,
              state<SubStateIntermediate>                 + event<exitSubEvent1>                  = state<SubExit1>,
              state<SubStateIntermediate>                 + event<exitSubEvent2>                  = state<SubExit2>
            );
        // clang-format on
    }
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        using namespace hsm;

        // clang-format off
        return transition_table(
            * state<Initial>                             + event<mainEvent1>                     = state<Intermediate>,
              state<Intermediate>                        [ guard ]                               = state<SubState>,
              state<Intermediate>                        + event<entrySubEvent>                  = state<SubState>,
              state<SubState>                            + event<entrySubEvent>                  = history<SubState>,
              state<Final>                                                                       = state<Intermediate>,
              hsm::exit<SubState, SubExit1>                                                      = state<Final>,
              hsm::exit<SubState, SubExit2>                                                      = state<Intermediate>
            );
        // clang-format on
    }
};

}
class GuardCallCountTest : public Test {
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

    GuardCallCountTest()
        : _dependency(0)
        , _fsm { _dependency }
    {
    }

    Dependency _dependency;
    hsm::sm<MainState, Dependency> _fsm;
};

TEST_F(GuardCallCountTest, should_check_guard_only_once)
{
    _fsm.process_event(mainEvent1 {});
    _fsm.process_event(entrySubEvent {});
    _fsm.process_event(exitSubEvent2 {});
    ASSERT_EQ(_dependency.callCount, 1);
}
