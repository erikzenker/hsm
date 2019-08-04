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
struct pause {
};
struct end_pause {
};
struct next_song {
};
struct prev_song {
};

// Actions
const auto store_cd_info = []() {};
const auto open_drawer = []() {};
const auto close_drawer = []() {};
const auto stopped_again = []() {};
const auto start_playback = []() {};
const auto stop_and_open = []() {};
const auto pause_playback = []() {};
const auto stop_playback = []() {};
const auto resume_playback = []() {};
const auto start_next_song = [](){};
const auto start_prev_song = [](){};

// Guards
const auto none = [](){};


struct Playing {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Start      , Event                    , Guard   , Action            , Target
            //             +-----------+--------------------------+---------+-------------------+---------------+
            hsm::transition(Song1 {}   , hsm::event<next_song> {} , none    , start_next_song   , Song2 {}      ),
            //            +------------+--------------------------+---------+-------------------+---------------+
            hsm::transition(Song2 {}   , hsm::event<prev_song> {} , none    , start_prev_song   , Song1 {}      ),            
            hsm::transition(Song2 {}   , hsm::event<next_song> {} , none    , start_next_song   , Song3 {}      ),                        
            //            +------------+--------------------------+---------+-------------------+---------------+
            hsm::transition(Song3 {}   , hsm::event<prev_song> {} , none    , start_prev_song   , Song2 {}     ));
            //            +------------+--------------------------+---------+-------------------+---------------+            
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return Song1 {};
    }
};


struct CdPlayer {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Start       , Event                      , Guard   , Action           , Target
            //             +------------+----------------------------+---------+------------------+---------------+
            hsm::transition(Empty {}    , hsm::event<cd_detected> {} , none    , store_cd_info    , Stopped {}    ),
            hsm::transition(Empty {}    , hsm::event<open_close> {}  , none    , open_drawer      , Open {}       ),            
            //            +-------------+----------------------------+---------+------------------+---------------+
            hsm::transition(Open {}     , hsm::event<open_close> {}  , none    , close_drawer     , Empty {}      ),            
            //            +-------------+----------------------------+---------+------------------+---------------+
            hsm::transition(Stopped {}  , hsm::event<open_close> {}  , none    , open_drawer      , Open {}       ),
            hsm::transition(Stopped {}  , hsm::event<stop> {}        , none    , stopped_again    , Stopped {}    ),
            hsm::transition(Stopped {}  , hsm::event<play> {}        , none    , start_playback   , Playing {}    ),
            //            +-------------+----------------------------+---------+------------------+---------------+            
            hsm::transition(Playing {}  , hsm::event<open_close> {}  , none    , stop_and_open    , Open {}       ),            
            hsm::transition(Playing {}  , hsm::event<pause> {}       , none    , pause_playback   , Paused {}     ),                        
            hsm::transition(Playing {}  , hsm::event<stop> {}        , none    , stop_playback    , Stopped {}    ),                                    
            //            +-------------+----------------------------+---------+------------------+---------------+                        
            hsm::transition(Paused {}   , hsm::event<end_pause> {}   , none    , resume_playback  , Playing {}    ),                                                
            hsm::transition(Paused {}   , hsm::event<stop> {}        , none    , stop_playback    , Stopped {}    ),                                                            
            hsm::transition(Paused {}   , hsm::event<open_close> {}  , none    , stop_and_open    , Open {}      ));                                                                      
            //            +-------------+----------------------------+---------+------------------+---------------+                                    
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return Empty {};
    }
};

int main()
{
    hsm::Sm<CdPlayer> CdPlayerSm;

    // Cd player is empty
    assert(CdPlayerSm.is(Empty {}));

    return 0;
}