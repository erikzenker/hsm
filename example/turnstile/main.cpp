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

// Guards
const auto noError = [](auto /*event*/){return true;};

// Actions
const auto beep = [](auto /*event*/){ std::cout << "beep!" << std::endl;};
const auto blink = [](auto /*event*/){ std::cout << "blink, blink, blink!" << std::endl;};

struct Turnstile {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Start                + Event               [Guard]   / Action = Target
            // +--------------------+---------------------+---------+--------+------------------------+
            hsm::state<Locked> {}   + hsm::event<Push> {} [noError] / beep   = hsm::state<Locked> {}  ,
            hsm::state<Locked> {}   + hsm::event<Coin> {} [noError] / blink  = hsm::state<Unlocked> {},
            // +--------------------+---------------------+---------+------------------------+
            hsm::state<Unlocked> {} + hsm::event<Push> {} [noError]          = hsm::state<Locked> {}  ,
            hsm::state<Unlocked> {} + hsm::event<Coin> {} [noError] / blink  = hsm::state<Unlocked> {}
            // +--------------------+---------------------+---------+------------------------+                        
            );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(Locked {});
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