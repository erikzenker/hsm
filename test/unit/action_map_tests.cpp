#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>

#include <functional>

using namespace ::testing;
using namespace hsm;

struct e1 {
    std::string name = "e1";
};
struct e2 {
    std::string name = "e2";
};
struct e3 {
    std::string name = "e3";
};
struct e4 {
    std::string name = "e4";
};

auto g1 = [](auto event) {
    std::cout << event.name << std::endl;
    return true;
};
auto g2 = [](auto event) {
    std::cout << event.name << std::endl;
    return true;
};
auto g3 = [](auto event) {
    std::cout << event.name << std::endl;
    return false;
};
auto g4 = [](auto event) {
    std::cout << event.name << std::endl;
    return false;
};

const auto a1 = [](auto event) { std::cout << event.name << std::endl; };
const auto a2 = [](auto event) { std::cout << event.name << std::endl; };
const auto a3 = [](auto event) { std::cout << event.name << std::endl; };
const auto a4 = [](auto event) { std::cout << event.name << std::endl; };

struct T {
};

struct P {
    auto make_transition_table()
    {
        return boost::hana::make_tuple(
            boost::hana::make_tuple(T {}, hsm::event<e4> {}, g4, a4, T {}));
    }
};

struct S {
    auto make_transition_table()
    {
        return boost::hana::make_tuple(
            boost::hana::make_tuple(T {}, hsm::event<e1> {}, g1, a1, T {}),
            boost::hana::make_tuple(T {}, hsm::event<e2> {}, g2, a2, T {}),
            boost::hana::make_tuple(T {}, hsm::event<e3> {}, g2, a2, T {}),
            boost::hana::make_tuple(T {}, hsm::event<e3> {}, g3, a3, P {}));
    }
};

class ActionMapTests : public Test {
};

TEST_F(ActionMapTests, should_make_action_map)
{
    auto actionMap = hsm::make_action_map(S {});

    ASSERT_EQ(boost::hana::size_c<4>, boost::hana::size(actionMap));
    ASSERT_EQ(1, boost::hana::find(actionMap, boost::hana::typeid_(e1 {})).value().size());
    ASSERT_EQ(1, boost::hana::find(actionMap, boost::hana::typeid_(e2 {})).value().size());
    ASSERT_EQ(2, boost::hana::find(actionMap, boost::hana::typeid_(e3 {})).value().size());
    ASSERT_EQ(1, boost::hana::find(actionMap, boost::hana::typeid_(e4 {})).value().size());

    boost::hana::find(actionMap, boost::hana::typeid_(e1 {})).value()[0](e1 {});
    boost::hana::find(actionMap, boost::hana::typeid_(e2 {})).value()[1](e2 {});
    boost::hana::find(actionMap, boost::hana::typeid_(e3 {})).value()[1](e3 {});
    boost::hana::find(actionMap, boost::hana::typeid_(e3 {})).value()[2](e3 {});
    boost::hana::find(actionMap, boost::hana::typeid_(e4 {})).value()[3](e4 {});
}

TEST_F(ActionMapTests, should_make_guard_map)
{
    auto guardMap = hsm::make_guard_map(S {});

    ASSERT_EQ(boost::hana::size_c<4>, boost::hana::size(guardMap));
    ASSERT_EQ(1, boost::hana::find(guardMap, boost::hana::typeid_(e1 {})).value().size());
    ASSERT_EQ(1, boost::hana::find(guardMap, boost::hana::typeid_(e2 {})).value().size());
    ASSERT_EQ(2, boost::hana::find(guardMap, boost::hana::typeid_(e3 {})).value().size());
    ASSERT_EQ(1, boost::hana::find(guardMap, boost::hana::typeid_(e4 {})).value().size());

    ASSERT_TRUE(boost::hana::find(guardMap, boost::hana::typeid_(e1 {})).value()[0](e1 {}));
    ASSERT_TRUE(boost::hana::find(guardMap, boost::hana::typeid_(e2 {})).value()[1](e2 {}));
    ASSERT_TRUE(boost::hana::find(guardMap, boost::hana::typeid_(e3 {})).value()[1](e3 {}));
    ASSERT_FALSE(boost::hana::find(guardMap, boost::hana::typeid_(e3 {})).value()[2](e3 {}));
    ASSERT_FALSE(boost::hana::find(guardMap, boost::hana::typeid_(e4 {})).value()[3](e4 {}));
}