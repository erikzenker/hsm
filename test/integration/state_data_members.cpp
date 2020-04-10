#include "hsm/details/transition_table.h"
#include "hsm/hsm.h"

#include <gtest/gtest.h>

namespace {

// States
struct S1 {
    std::string data = "S1";
};

struct S2 {
    std::string data = "S2";
};

// Events
struct e1 {
    std::shared_ptr<std::string> sourceData;
    std::shared_ptr<std::string> targetData;
};

struct e2 {
    std::shared_ptr<std::string> sourceData;
    std::shared_ptr<std::string> targetData;
};

// Actions
const auto readData = [](auto event, auto source, auto target) {
    *(event.sourceData) = source.data;
    *(event.targetData) = target.data;
};

const auto writeData = [](auto event, auto source, auto target) {
    source.data = *(event.sourceData);
    target.data = *(event.targetData);
};

using namespace ::testing;
using namespace boost::hana;

struct MainState {
    using type = MainState;
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::state<S1>{} + hsm::event<e1>{}  / readData  = hsm::state<S1>{},
            hsm::state<S1>{} + hsm::event<e2>{}  / writeData = hsm::state<S1>{}
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<S1> {});
    }
};
}

class StateDataMembersTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(StateDataMembersTests, should_read_state_data_member)
{
    auto sourceData = std::make_shared<std::string>("");
    auto targetData = std::make_shared<std::string>("");

    sm.process_event(e1 { sourceData, targetData });
    ASSERT_EQ("S1", *sourceData);
    ASSERT_EQ("S1", *targetData);
}

TEST_F(StateDataMembersTests, should_write_state_data_member)
{
    auto sourceData = std::make_shared<std::string>("42");
    auto targetData = std::make_shared<std::string>("42");

    sm.process_event(e2 { sourceData, targetData });

    *sourceData = "";
    *targetData = "";

    sm.process_event(e1 { sourceData, targetData });
    ASSERT_EQ("42", *sourceData);
    ASSERT_EQ("42", *targetData);
}