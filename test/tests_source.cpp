#include "hsm/hsm.h"

#include <gtest/gtest.h>


// States
struct S1{};
struct S2{};
struct S3{};

// Events
struct e1{};
struct e2{};
struct e3{};

// Guards
struct g1{};

// Actions
struct a1{};

using namespace ::testing;
using namespace boost::hana;

struct mainState {
    constexpr auto make_transition_table(){
        return boost::hana::make_tuple(
              boost::hana::make_tuple(type<S1>{}, type<e1>{}, type<g1>{}, type<a1>{}, type<S2>{}) 
            , boost::hana::make_tuple(type<S1>{}, type<e2>{}, type<g1>{}, type<a1>{}, type<S3>{}) 
            , boost::hana::make_tuple(type<S2>{}, type<e1>{}, type<g1>{}, type<a1>{}, type<S1>{})
            , boost::hana::make_tuple(type<S2>{}, type<e2>{}, type<g1>{}, type<a1>{}, type<S1>{})
            , boost::hana::make_tuple(type<S2>{}, type<e3>{}, type<g1>{}, type<a1>{}, type<S3>{})
        );
    }

    constexpr auto initial_state(){
        return type<S1>{};
    }  
};

class HsmTests : public Test {
public:

  
};

TEST_F(HsmTests, should_start_in_initial_state){
    hsm::Sm<mainState> sm;
    ASSERT_TRUE(sm.is(type<S1>{}));
}

TEST_F(HsmTests, should_process_event){
    hsm::Sm<mainState> sm;   
    ASSERT_TRUE(sm.is(type<S1>{}));

    sm.process_event(type<e1>{});
    ASSERT_TRUE(sm.is(type<S2>{}));
}

TEST_F(HsmTests, should_throw_on_unexpected_event){
    hsm::Sm<mainState> sm;     
    EXPECT_THROW(sm.process_event(type<e3>{}), std::exception);
}

TEST_F(HsmTests, should_process_alot_event){
    hsm::Sm<mainState> sm;            
    ASSERT_TRUE(sm.is(type<S1>{}));

    for(int i = 0; i < 1000000; i++){
        sm.process_event(type<e1>{});
    }
}