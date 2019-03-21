#include "hsm/hsm.h"

#include <gtest/gtest.h>


// States
struct S1{};
struct S2{};
struct S3{};
struct S4{};

// Events
struct e1{};
struct e2{};
struct e3{};
struct e4{};

// Guards
struct g1{};

// Actions
struct a1{};

using namespace ::testing;
using namespace boost::hana;

struct SubState {
    constexpr auto make_transition_table(){
        return boost::hana::make_tuple(
              boost::hana::make_tuple(S4{}, e1{}, type<g1>{}, type<a1>{}, S2{}) 
        );
    }

    constexpr auto initial_state(){
        return S4{};
    }  
};

struct MainState {
    constexpr auto make_transition_table(){
        return boost::hana::make_tuple(
              boost::hana::make_tuple(S1{}, e1{}, type<g1>{}, type<a1>{}, S2{}) 
            , boost::hana::make_tuple(S1{}, e2{}, type<g1>{}, type<a1>{}, S3{})
            , boost::hana::make_tuple(S1{}, e4{}, type<g1>{}, type<a1>{}, SubState{}) 
            , boost::hana::make_tuple(S2{}, e1{}, type<g1>{}, type<a1>{}, S1{})
            , boost::hana::make_tuple(S2{}, e2{}, type<g1>{}, type<a1>{}, S1{})
            , boost::hana::make_tuple(S2{}, e3{}, type<g1>{}, type<a1>{}, S3{})
        );
    }

    constexpr auto initial_state(){
        return S1{};
    }  
};

class HsmTests : public Test {
    hsm::Sm<MainState> sm;    
};

TEST_F(HsmTests, should_start_in_initial_state){
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1{}));
}

TEST_F(HsmTests, should_start_in_root_state){
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(type<MainState>{}, S1{}));
}

TEST_F(HsmTests, should_process_event){
    hsm::Sm<MainState> sm;   
    ASSERT_TRUE(sm.is(S1{}));

    sm.process_event(e1{});
    ASSERT_TRUE(sm.is(S2{}));
}

TEST_F(HsmTests, should_throw_on_unexpected_event){
    hsm::Sm<MainState> sm;     
    EXPECT_THROW(sm.process_event(e3{}), std::exception);
}

TEST_F(HsmTests, should_transit_into_SubState){
    hsm::Sm<MainState> sm;     
    sm.process_event(e4{});

    ASSERT_TRUE(sm.is(type<SubState>{}, S4{}));    
}

TEST_F(HsmTests, should_process_alot_event){
    hsm::Sm<MainState> sm;            
    ASSERT_TRUE(sm.is(S1{}));

    for(int i = 0; i < 1000000; i++){
        sm.process_event(e1{});
    }
}