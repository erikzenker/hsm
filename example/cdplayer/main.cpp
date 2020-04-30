#include "hsm/hsm.h"

#include <cassert>

// States
struct Empty {
};
struct Open {
};
struct Stopped {
};
struct Paused {
};
struct Song1{
};
struct Song2{
};
struct Song3{
};

// Events
struct cd_detected {
};
struct open_close {
};
struct stop {
};
struct play {
};
struct pause2 {
};
struct end_pause {
};
struct next_song {
};
struct prev_song {
};

// Actions
const auto store_cd_info = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto open_drawer = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto close_drawer = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto stopped_again = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto start_playback = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto stop_and_open = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto pause_playback = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto stop_playback = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto resume_playback = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto start_next_song = [](auto /*event*/, auto /*source*/, auto /*target*/) {};
const auto start_prev_song = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

struct Playing {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Start      , Event                    , Guard   , Action            , Target
            //             +-----------+--------------------------+---------+-------------------+---------------+
            hsm::state<Song1> {}   + hsm::event<next_song> {} / start_next_song   = hsm::state<Song2> {},
            //            +------------+--------------------------+---------+-------------------+---------------+
            hsm::state<Song2> {}   + hsm::event<prev_song> {} / start_prev_song   = hsm::state<Song1> {},            
            hsm::state<Song2> {}   + hsm::event<next_song> {} / start_next_song   = hsm::state<Song3> {},                        
            //            +------------+--------------------------+---------+-------------------+---------------+
            hsm::state<Song3> {}   + hsm::event<prev_song> {} / start_prev_song   = hsm::state<Song2> {}
            //            +------------+--------------------------+---------+-------------------+---------------+                        
        );

        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<Song1> {});
    }
};


struct CdPlayer {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Start       , Event                      , Guard   , Action           , Target
            //             +------------+----------------------------+---------+------------------+---------------+
            hsm::state<Empty> {}    + hsm::event<cd_detected> {} / store_cd_info    = hsm::state<Stopped> {},
            hsm::state<Empty> {}    + hsm::event<open_close> {}  / open_drawer      = hsm::state<Open> {},            
            //            +-------------+----------------------------+---------+------------------+---------------+
            hsm::state<Open> {}     + hsm::event<open_close> {}  / close_drawer     = hsm::state<Empty> {},            
            //            +-------------+----------------------------+---------+------------------+---------------+
            hsm::state<Stopped> {}  + hsm::event<open_close> {}  / open_drawer      = hsm::state<Open> {},
            hsm::state<Stopped> {}  + hsm::event<stop> {}        / stopped_again    = hsm::state<Stopped> {},
            hsm::state<Stopped> {}  + hsm::event<play> {}        / start_playback   = hsm::state<Playing> {},
            //            +-------------+----------------------------+---------+------------------+---------------+            
            hsm::state<Playing> {}  + hsm::event<open_close> {}  / stop_and_open    = hsm::state<Open> {},            
            hsm::state<Playing> {}  + hsm::event<pause2> {}       / pause_playback   = hsm::state<Paused> {},                        
            hsm::state<Playing> {}  + hsm::event<stop> {}        / stop_playback    = hsm::state<Stopped> {},                                    
            //            +-------------+----------------------------+---------+------------------+---------------+                        
            hsm::state<Paused> {}   + hsm::event<end_pause> {}   / resume_playback  = hsm::state<Playing> {},                                                
            hsm::state<Paused> {}   + hsm::event<stop> {}        / stop_playback    = hsm::state<Stopped> {},                                                            
            hsm::state<Paused> {}   + hsm::event<open_close> {}  / stop_and_open    = hsm::state<Open> {}
            //            +-------------+----------------------------+---------+------------------+---------------+                                    
        );
        // clang-format on
    }

    static constexpr auto initial_state()
    {
        return hsm::initial(hsm::state<Empty> {});
    }
};

int main()
{
    hsm::sm<CdPlayer> CdPlayerSm;

    // Cd player is empty
    assert(CdPlayerSm.is(hsm::state<Empty> {}));

    return 0;
}