#include "hsm/details/transition_table.h"
#include "hsm/hsm.h"

#include <gtest/gtest.h>

namespace {

using namespace ::testing;
using namespace boost::hana;

// States
struct S1 {
    std::string data = "S1";
};

struct S2 {
    std::string data = "S2";
};

// Events
struct readEvent {
    std::string sourceData;
    std::string targetData;
};

struct writeEvent {
    std::string sourceData;
    std::string targetData;
};

struct resetEvent {
};

// Actions
const auto readDataAction = [](auto&& event, auto& source, auto& target) {
    event.sourceData = source.data;
    event.targetData = target.data;
};

const auto writeDataAction = [](auto&& event, auto& source, auto& target) {
    source.data = event.sourceData;
    target.data = event.targetData;
    return true;
};

// Guard
const auto readDataGuard = [](auto&& event, auto& source, auto& target) {
    event.sourceData = source.data;
    event.targetData = target.data;
    return true;
};

const auto writeDataGuard = [](auto&& event, auto& source, auto& target) {
    source.data = event.sourceData;
    target.data = event.targetData;
    return true;
};

struct MainStateWithActions {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::state<S1>{} + hsm::event<readEvent>{}   / readDataAction  = hsm::state<S2>{},
            hsm::state<S1>{} + hsm::event<writeEvent>{}  / writeDataAction = hsm::state<S2>{},
            hsm::state<S2>{} + hsm::event<resetEvent>{}                    = hsm::state<S1>{}
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }

    std::string data = "MainStateWithActions";
};

struct MainStateWithGuards {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::state<S1>{} + hsm::event<readEvent>{}   [readDataGuard]  = hsm::state<S2>{},
            hsm::state<S1>{} + hsm::event<writeEvent>{}  [writeDataGuard] = hsm::state<S2>{},
            hsm::state<S2>{} + hsm::event<resetEvent>{}                   = hsm::state<S1>{}
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }

    std::string data = "MainStateWithGuards";
};
}

class StateDataMembersActionTests : public Test {
  protected:
    hsm::sm<MainStateWithActions> sm;
};

TEST_F(StateDataMembersActionTests, should_read_state_data_member_in_action)
{
    auto event = readEvent { "", "" };

    sm.process_event(event);
    ASSERT_EQ("S1", event.sourceData);
    ASSERT_EQ("S2", event.targetData);
}

TEST_F(StateDataMembersActionTests, should_write_state_data_member_in_action)
{
    sm.process_event(writeEvent { "42", "43" });
    sm.process_event(resetEvent {});

    auto event = readEvent { "", "" };

    sm.process_event(event);
    ASSERT_EQ("42", event.sourceData);
    ASSERT_EQ("43", event.targetData);
}

class StateDataMembersGuardTests : public Test {
  protected:
    hsm::sm<MainStateWithGuards> sm;
};

TEST_F(StateDataMembersGuardTests, should_read_state_data_member_in_guard)
{
    auto event = readEvent { "", "" };

    sm.process_event(event);
    ASSERT_EQ("S1", event.sourceData);
    ASSERT_EQ("S2", event.targetData);
}

TEST_F(StateDataMembersGuardTests, should_write_state_data_member_in_guard)
{
    auto sourceData = std::make_shared<std::string>("42");
    auto targetData = std::make_shared<std::string>("43");

    sm.process_event(writeEvent { "42", "43" });
    sm.process_event(resetEvent {});

    auto event = readEvent { "", "" };

    sm.process_event(event);
    ASSERT_EQ("42", event.sourceData);
    ASSERT_EQ("43", event.targetData);
}