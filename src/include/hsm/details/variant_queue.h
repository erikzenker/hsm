#pragma once

#include <boost/hana.hpp>
#include <exception>
#include <queue>
#include <variant>

namespace hsm {

namespace {
template <class EventsTuple> auto constexpr make_variant_queue(EventsTuple events)
{
    using Variant_t =
        typename decltype(boost::hana::unpack(events, boost::hana::template_<std::variant>))::type;
    return std::queue<Variant_t> {};
}
}

template <class EventsTuple>
class variant_queue {
    EventsTuple m_events;
    using Queue_t = decltype(make_variant_queue(m_events));
    Queue_t m_queue;

public:
    variant_queue(const EventsTuple& events) : m_events(events){
    }

    bool empty() const{
        return m_queue.empty();
    }

    std::size_t size() const {
        return m_queue.size();    
    }

    template <class T> void push(const T& e)
    {
        m_queue.push(e);
    }

    template <class T> void push(T&& e)
    {
        m_queue.push(e);
    }

    template <class Callable>
    void visit(const Callable& callable){
        if(empty()){
            throw std::runtime_error("variant_queue is empty");
        }

        auto frontElement = m_queue.front();
        m_queue.pop();

        std::visit([&callable](auto&& arg){
            callable(arg);    
        }, frontElement);
        
    }
};
}