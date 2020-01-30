//
// Copyright (c) 2016-2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "benchmark.hpp"
#include <hsm/hsm.h>
    
using namespace hsm;

struct Empty{};
struct Open{};
struct Stopped{};
struct Playing{};
struct Pause{};

struct play {};
struct end_pause {};
struct stop {};
struct pause {};
struct open_close {};
struct cd_detected {};

auto start_playback = [](auto) {};
auto resume_playback = [](auto) {};
auto close_drawer = [](auto) {};
auto open_drawer = [](auto) {};
auto stop_and_open = [](auto) {};
auto stopped_again = [](auto) {};
auto store_cd_info = [](auto) {};
auto pause_playback = [](auto) {};
auto stop_playback = [](auto) {};

struct player {
  constexpr auto make_transition_table(){

    // clang-format off
    return transition_table(
        row( Stopped{}, event<play>{}       , noGuard{},  start_playback,   Playing{}),
        row( Pause{},   event<end_pause>{}  , noGuard{},  resume_playback,  Playing{}),
        row( Open{},    event<open_close>{} , noGuard{},  close_drawer,     Empty{}),
        row( Empty{},   event<open_close>{} , noGuard{},  open_drawer,      Open{}),
        row( Pause{},   event<open_close>{} , noGuard{},  stop_and_open,    Open{}),
        row( Stopped{}, event<open_close>{} , noGuard{},  open_drawer,      Open{}),
        row( Playing{}, event<open_close>{} , noGuard{},  stop_and_open,    Open{}),
        row( Playing{}, event<pause>{}      , noGuard{},  pause_playback,   Pause{}),
        row( Playing{}, event<stop>{}       , noGuard{},  stop_playback,    Stopped{}),
        row( Pause{},   event<stop>{}       , noGuard{},  stop_playback,    Stopped{}),
        row( Empty{},   event<cd_detected>{}, noGuard{},  store_cd_info,    Stopped{}),
        row( Stopped{}, event<stop>{}       , noGuard{},  stopped_again,    Stopped{})
    );
    // clang-format on
  }

  auto constexpr initial_state(){
    return initial(Empty{});  
  }

};

int main() {
  hsm::sm<player> sm;

  auto a = open_close{};  
  auto b = cd_detected{};
  auto c = play{};
  auto d = pause{};
  auto e = stop{};
  auto f = end_pause{};

  benchmark_execution_speed([&] {
    for (auto i = 0; i < 1'000'000; ++i) {
      sm.process_event(a);
      sm.process_event(a);
      sm.process_event(b);
      sm.process_event(c);
      sm.process_event(d);
      // go back to Playing
      sm.process_event(f);
      sm.process_event(d);
      sm.process_event(e);
      // event leading to the same state
      sm.process_event(e);
      sm.process_event(a);
      sm.process_event(a);
    }
  });
  benchmark_memory_usage(sm);
}
