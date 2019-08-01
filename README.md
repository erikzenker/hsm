# Hana State Machine (HSM)
[![Build Status](https://travis-ci.org/erikzenker/cmake-project-template.svg?branch=master)](https://travis-ci.org/erikzenker/hsm)
=

The hana state machine (hsm) is a [finite state machine](https://en.wikipedia.org/wiki/Finite-state_machine) library based on the [boost hana](https://www.boost.org/doc/libs/1_61_0/libs/hana/doc/html/index.html) meta programming library. It follows the principles of the [boost msm](https://www.boost.org/doc/libs/1_64_0/libs/msm/doc/HTML/index.html) and [boost sml](https://boost-experimental.github.io/sml/index.html) libraries, but tries to reduce complex meta programming library code to a minimum. The library is in an very early developement phase.

Currently the following features are supported:
* Initial pseudo state
* External transitions with guards and actions
* Anonymous transitions
* Submachines
* Pseudo exit states

What is missing:
* Pseudo entry states
* Orthogonal regions
* Explixit terminate state
* Entry/exit state behavior
* Internal transitions
* Direct transitions
* Fork Entry
* History

```c++
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
const auto none = []() {};

struct Turnstile {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Start      , Event              , Guard   , Action , Target
            //             +-----------+--------------------+---------+--------+---------------+
            hsm::transition(Locked {}  , hsm::event<Push> {}, none    , none   , Locked {}    ),
            hsm::transition(Locked {}  , hsm::event<Coin> {}, none    , none   , Unlocked {}  ),
            //            +------------+--------------------+---------+--------+---------------+
            hsm::transition(Unlocked {}, hsm::event<Push> {}, none    , none   , Locked {}    ),
            hsm::transition(Unlocked {}, hsm::event<Coin> {}, none    , none   , Unlocked {} ));
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
```
## Roadmap


## Create and Install the Package
``` bash
mkdir src/build
cd src/build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/tmp/
cmake --build . --target install
```

## Compile and Run the Tests Using the Installed Library
``` bash
mkdir test/build/
cd test/build/
cmake .. -DCMAkE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/tmp/
cmake --build . --target hsmTests
ctest -VV
```

## Author
* erikzenker(at)hotmail.com