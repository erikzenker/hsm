#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <iostream>

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

struct S3 {
    constexpr auto on_entry()
    {
        return [](auto event, auto& source, auto& target) {
            source.data = event.sourceData;
            target.data = event.targetData;
        };
    }

    constexpr auto on_exit()
    {
        return [](auto& event, auto source, auto target) {
            event.sourceData = source.data;
            event.targetData = target.data;
        };
    }

    std::string data = "S3";
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

struct enterSubState {
    std::string sourceData;
    std::string targetData;
};

struct leaveSubState {
};

struct identity {
};

// Actions
const auto readDataAction = [](auto&& event, auto& source, auto& target) {
    event.sourceData = source.data;
    event.targetData = target.data;
};

const auto writeDataAction = [](auto&& event, auto& source, auto& target) {
    source.data = event.sourceData;
    target.data = event.targetData;
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

struct SubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S3> {} + hsm::event<readEvent> {} / readDataAction =  hsm::state<S3> {}
        );
        // clang-format on
    }

    constexpr auto on_entry()
    {
        return [](auto event, auto& source, auto& target) {
            source.data = event.sourceData;
            target.data = event.targetData;
        };
    }

    std::string data = "SubState";
};

struct MainStateWithActions {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>{} + hsm::event<readEvent>{}   / readDataAction  = hsm::state<S2>{},
              hsm::state<S1>{} + hsm::event<writeEvent>{}  / writeDataAction = hsm::state<S2>{},
              hsm::state<S2>{} + hsm::event<resetEvent>{}                    = hsm::state<S1>{}
        );
        // clang-format on
    }

    std::string data = "MainStateWithActions";
};

struct MainStateWithGuards {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>{} + hsm::event<readEvent>{}   [readDataGuard]  = hsm::state<S2>{},
              hsm::state<S1>{} + hsm::event<writeEvent>{}  [writeDataGuard] = hsm::state<S2>{},
              hsm::state<S2>{} + hsm::event<resetEvent>{}                   = hsm::state<S1>{}
        );
        // clang-format on
    }

    std::string data = "MainStateWithGuards";
};

struct MainStateWithSubState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>{}       + hsm::event<enterSubState>{}                 = hsm::state<SubState>{},
              hsm::state<S1>{}       + hsm::event<readEvent>{}    / readDataAction = hsm::state<SubState>{}, 
              hsm::state<SubState>{} + hsm::event<leaveSubState>{}                 = hsm::state<S1>{}
        );
        // clang-format on
    }

    std::string data = "MainStateWithSubState";
};

struct MainStateWithEntry {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            * hsm::state<S1>{} + hsm::event<writeEvent>{} = hsm::state<S3>{},
              hsm::state<S3>{} + hsm::event<readEvent>{}  = hsm::state<S1>{}
        );
        // clang-format on
    }

    std::string data = "MainStateWithEntry";
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
    sm.process_event(writeEvent { "42", "43" });
    sm.process_event(resetEvent {});

    auto event = readEvent { "", "" };

    sm.process_event(event);
    ASSERT_EQ("42", event.sourceData);
    ASSERT_EQ("43", event.targetData);
}

class StateDataMembersEntryTests : public Test {
  protected:
    hsm::sm<MainStateWithEntry> sm;
};

TEST_F(StateDataMembersEntryTests, should_write_state_data_member_on_entry)
{
    sm.process_event(writeEvent { "42", "42" });
    ASSERT_TRUE(sm.is(hsm::state<S3> {}));

    auto event = readEvent { "", "" };
    sm.process_event(event);
    ASSERT_TRUE(sm.is(hsm::state<S1> {}));

    ASSERT_EQ("42", event.sourceData);
    ASSERT_EQ("42", event.targetData);
}

class StateDataMembersSubStateTests : public Test {
  protected:
    hsm::sm<MainStateWithSubState> sm;
};

TEST_F(StateDataMembersSubStateTests, should_write_substate_data_member_on_entry)
{
    sm.process_event(enterSubState { "42", "42" });
    sm.process_event(leaveSubState {});

    auto event = readEvent { "", "" };
    sm.process_event(event);

    ASSERT_EQ("42", event.sourceData);
    ASSERT_EQ("42", event.targetData);
}