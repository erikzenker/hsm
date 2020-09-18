#include "ComplexStateMachine.h"
#include "benchmark.hpp"

using namespace hsm;


auto main() -> int
{
    hsm::sm<ComplexStateMachine> sm;

    const auto nRuns = 1'000'000;

    benchmark_execution_speed([&] {
        for (auto i = 0; i < nRuns; ++i) {
            if (rand() % 2)
                sm.process_event(e1());
            if (rand() % 2)
                sm.process_event(e2());
            if (rand() % 2)
                sm.process_event(e3());
            if (rand() % 2)
                sm.process_event(e4());
            if (rand() % 2)
                sm.process_event(e5());
            if (rand() % 2)
                sm.process_event(e6());
            if (rand() % 2)
                sm.process_event(e7());
            if (rand() % 2)
                sm.process_event(e8());
            if (rand() % 2)
                sm.process_event(e9());
            if (rand() % 2)
                sm.process_event(e10());
            if (rand() % 2)
                sm.process_event(e11());
            if (rand() % 2)
                sm.process_event(e12());
            if (rand() % 2)
                sm.process_event(e13());
            if (rand() % 2)
                sm.process_event(e14());
            if (rand() % 2)
                sm.process_event(e15());
            if (rand() % 2)
                sm.process_event(e16());
            if (rand() % 2)
                sm.process_event(e17());
            if (rand() % 2)
                sm.process_event(e18());
            if (rand() % 2)
                sm.process_event(e19());
            if (rand() % 2)
                sm.process_event(e20());
            if (rand() % 2)
                sm.process_event(e21());
            if (rand() % 2)
                sm.process_event(e22());
            if (rand() % 2)
                sm.process_event(e23());
            if (rand() % 2)
                sm.process_event(e24());
            if (rand() % 2)
                sm.process_event(e25());
            if (rand() % 2)
                sm.process_event(e26());
            if (rand() % 2)
                sm.process_event(e27());
            if (rand() % 2)
                sm.process_event(e28());
            if (rand() % 2)
                sm.process_event(e29());
            if (rand() % 2)
                sm.process_event(e30());
            if (rand() % 2)
                sm.process_event(e31());
            if (rand() % 2)
                sm.process_event(e32());
            if (rand() % 2)
                sm.process_event(e33());
            if (rand() % 2)
                sm.process_event(e34());
            if (rand() % 2)
                sm.process_event(e35());
            if (rand() % 2)
                sm.process_event(e36());
            if (rand() % 2)
                sm.process_event(e37());
            if (rand() % 2)
                sm.process_event(e38());
            if (rand() % 2)
                sm.process_event(e39());
            if (rand() % 2)
                sm.process_event(e40());
            if (rand() % 2)
                sm.process_event(e41());
            if (rand() % 2)
                sm.process_event(e42());
            if (rand() % 2)
                sm.process_event(e43());
            if (rand() % 2)
                sm.process_event(e44());
            if (rand() % 2)
                sm.process_event(e45());
            if (rand() % 2)
                sm.process_event(e46());
            if (rand() % 2)
                sm.process_event(e47());
            if (rand() % 2)
                sm.process_event(e48());
            if (rand() % 2)
                sm.process_event(e49());
        }
    });
    benchmark_memory_usage(sm);
}