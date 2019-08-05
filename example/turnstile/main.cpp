
#include "hsm/hsm.h"

#include <cassert>

// States
struct Locked {
};
struct Unlocked {
};

// Events
struct Push {
};
struct Coin {
};

// No events nor guards in this example
const auto none = [](auto event) {};
const auto g = []() {};

struct Turnstile {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Start      , Event              , Guard   , Action , Target
            //             +-----------+--------------------+---------+--------+---------------+
            hsm::transition(Locked {}  , hsm::event<Push> {}, g       , none   , Locked {}    ),
            hsm::transition(Locked {}  , hsm::event<Coin> {}, g       , none   , Unlocked {}  ),
            //            +------------+--------------------+---------+--------+---------------+
            hsm::transition(Unlocked {}, hsm::event<Push> {}, g       , none   , Locked {}    ),
            hsm::transition(Unlocked {}, hsm::event<Coin> {}, g       , none   , Unlocked {} ));
            //            +------------+--------------------+-------+----------+---------------+
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return Locked {};
    }
};

int main()
{
    hsm::Sm<Turnstile> turnstileSm;

    // The turnstile is initially locked
    assert(turnstileSm.is(Locked {}));

    // Inserting a coin unlocks it
    turnstileSm.process_event(Coin {});
    assert(turnstileSm.is(Unlocked {}));

    // Entering the turnstile will lock it again
    turnstileSm.process_event(Push {});
    assert(turnstileSm.is(Locked {}));

    return 0;
}