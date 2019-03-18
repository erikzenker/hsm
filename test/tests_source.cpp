#include "hsm/hsm.h"

#include <gtest/gtest.h>


// States
struct S1{};
struct S2{};
struct S3{};

// Events
struct e1{};
struct e2{};

// Guards
struct g1{};

// Actions
struct a1{};

using namespace ::testing;
using namespace boost::hana;

class HsmTests : public Test {
public:
    auto make_transition_table(){
        return boost::hana::make_tuple(
            boost::hana::make_tuple(type<S1>{}, type<e1>{}, type<g1>{}, type<a1>{}, type<S2>{}), 
            boost::hana::make_tuple(type<S1>{}, type<e2>{}, type<g1>{}, type<a1>{}, type<S3>{}), 
            boost::hana::make_tuple(type<S2>{}, type<e2>{}, type<g1>{}, type<a1>{}, type<S1>{})
        );
    }

    auto initialState(){
        return  type<S1>{};
    }    
};

TEST_F(HsmTests, should_start_in_initial_state){
    hsm::Sm sm(make_transition_table(), initialState());
    ASSERT_TRUE(sm.is(initialState()));
}

TEST_F(HsmTests, should_process_event){
    hsm::Sm sm(make_transition_table(), initialState());
    ASSERT_TRUE(sm.is(type<S1>{}));

    sm.process_event(type<e1>{});
    ASSERT_TRUE(sm.is(type<S2>{}));
}
