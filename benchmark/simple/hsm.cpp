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
struct pause2 {
};
struct open_close {};
struct cd_detected {};

constexpr auto start_playback = [](auto, auto, auto) {};
constexpr auto resume_playback = [](auto, auto, auto) {};
constexpr auto close_drawer = [](auto, auto, auto) {};
constexpr auto open_drawer = [](auto, auto, auto) {};
constexpr auto stop_and_open = [](auto, auto, auto) {};
constexpr auto stopped_again = [](auto, auto, auto) {};
constexpr auto store_cd_info = [](auto, auto, auto) {};
constexpr auto pause_playback = [](auto, auto, auto) {};
constexpr auto stop_playback = [](auto, auto, auto) {};

struct player {
    static constexpr auto make_transition_table()
    {
        // clang-format off
    return transition_table(
          state<Stopped> + event<play>        /  start_playback  = state<Playing>,
          state<Pause>   + event<end_pause>   /  resume_playback = state<Playing>,
          state<Open>    + event<open_close>  /  close_drawer    = state<Empty>,
        * state<Empty>   + event<open_close>  /  open_drawer     = state<Open>,
          state<Pause>   + event<open_close>  /  stop_and_open   = state<Open>,
          state<Stopped> + event<open_close>  /  open_drawer     = state<Open>,
          state<Playing> + event<open_close>  /  stop_and_open   = state<Open>,
          state<Playing> + event<pause2>      /  pause_playback  = state<Pause>,
          state<Playing> + event<stop>        /  stop_playback   = state<Stopped>,
          state<Pause>   + event<stop>        /  stop_playback   = state<Stopped>,
          state<Empty>   + event<cd_detected> /  store_cd_info   = state<Stopped>,
          state<Stopped> + event<stop>        /  stopped_again   = state<Stopped>
    );
        // clang-format on
    }
};

auto main() -> int{
  hsm::sm<player> sm;

  auto a = open_close{};  
  auto b = cd_detected{};
  auto c = play{};
  auto d = pause2 {};
  auto e = stop{};
  auto f = end_pause{};
  
  const auto nRuns = 1'000'000;

  benchmark_execution_speed([&] {
    for (auto i = 0; i < nRuns; ++i) {
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
