
#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <boost/hana.hpp>   
#include <functional>
#include <future>

using namespace ::testing;

class VariantQueue : public Test {
};

class E1 {
};
class E2 {
};

TEST_F(VariantQueue, should_construct_variant_queue)
{
    auto events = boost::hana::make_tuple(boost::hana::typeid_(E1{}), boost::hana::typeid_(E2{}));
    hsm::variant_queue queue{events};
    ASSERT_EQ(0, queue.size());
}

TEST_F(VariantQueue, should_throw_on_visit_empty_queue)
{
    auto events = boost::hana::make_tuple(boost::hana::typeid_(E1{}), boost::hana::typeid_(E2{}));
    hsm::variant_queue queue{events};
    ASSERT_THROW(queue.visit([](auto){}), std::runtime_error);
}

TEST_F(VariantQueue, should_push_elements)
{
    auto events = boost::hana::make_tuple(boost::hana::typeid_(E1{}), boost::hana::typeid_(E2{}));
    hsm::variant_queue queue{events};

    queue.push(E1{});
    ASSERT_EQ(1, queue.size());
}

TEST_F(VariantQueue, should_remove_elements_from_queue_when_calling_visit)
{
    auto events = boost::hana::make_tuple(boost::hana::typeid_(E1{}), boost::hana::typeid_(E2{}));
    hsm::variant_queue queue{events};

    queue.push(E1{});
    ASSERT_EQ(1, queue.size());
    queue.visit([](auto){});
    ASSERT_EQ(0, queue.size());
}

TEST_F(VariantQueue, should_visit_types_in_right_order)
{
    auto events = boost::hana::make_tuple(boost::hana::typeid_(E1{}), boost::hana::typeid_(E2{}));
    hsm::variant_queue queue{events};

    auto E1Visited = false;
    auto E2Visited = false;    

    auto queueHandler = [&](auto arg){
        using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, E1>){
                E1Visited = true;
            }
            else if constexpr (std::is_same_v<T, E2>){
                E2Visited = true;    
            }
    };

    queue.push(E1{});
    queue.push(E2{});
    ASSERT_EQ(2, queue.size());
    queue.visit(queueHandler);
    ASSERT_TRUE(E1Visited);
    ASSERT_FALSE(E2Visited);
    queue.visit(queueHandler);
    ASSERT_TRUE(E1Visited);
    ASSERT_TRUE(E2Visited);
}
