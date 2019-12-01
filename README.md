# Hana State Machine (HSM)
[![Build Status](https://travis-ci.org/erikzenker/cmake-project-template.svg?branch=master)](https://travis-ci.org/erikzenker/hsm) [![codecov](https://codecov.io/gh/erikzenker/hsm/branch/master/graph/badge.svg)](https://codecov.io/gh/erikzenker/hsm) [![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://paypal.me/erikzenker) [![Join the chat at https://gitter.im/hsm-gitter/community](https://badges.gitter.im/hsm-gitter/community.svg)](https://gitter.im/hsm-gitter/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
=

The **hana state machine** (hsm) is a [finite state machine](https://en.wikipedia.org/wiki/Finite-state_machine) library based on the [boost hana](https://www.boost.org/doc/libs/1_61_0/libs/hana/doc/html/index.html) meta programming library. It follows the principles of the [boost msm](https://www.boost.org/doc/libs/1_64_0/libs/msm/doc/HTML/index.html) and [boost sml](https://boost-experimental.github.io/sml/index.html) libraries, but tries to reduce own complex meta programming code to a minimum.

Currently the following features are supported:
* [Initial pseudo state](test/integration/basic_transitions.cpp)
* [External transitions with guards and actions](test/integration/guards_actions.cpp)
* [Anonymous transitions](test/integration/anonymous_transition.cpp)
* [Submachines](test/integration/basic_transitions.cpp)
* [Entry/exit pseudo states](test/integration/entry_exit_pseudo_states.cpp)
* [History pseudo state](test/integration/history_pseudo_state.cpp)
* [Entry/exit actions](test/integration/entry_exit_actions.cpp)
* [Direct transitions](test/integration/direct_transition.cpp)
* [Orthogonal regions](test/integration/orthogonal_regions.cpp)
* [Internal transitions](test/integration/internal_transition.cpp)
* [Unexpected event handler](test/integration/unexpected_transition_handler.cpp)
* [Dependency injection](test/integration/dependency_injection.cpp)

## Simple Example ([Turnstile](example/turnstile/main.cpp))
![Turnstile fsm](doc/example/turnstile_example.svg "Turnstile fsm")

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
```


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
