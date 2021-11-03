# Hana State Machine (HSM)
![Linux CI](https://github.com/erikzenker/hsm/workflows/Linux%20CI/badge.svg) ![MacOs CI](https://github.com/erikzenker/hsm/workflows/MacOs%20CI/badge.svg) ![Windows CI](https://github.com/erikzenker/hsm/workflows/Windows%20CI/badge.svg)

[![codecov](https://codecov.io/gh/erikzenker/hsm/branch/master/graph/badge.svg)](https://codecov.io/gh/erikzenker/hsm) [![GitHub license](https://img.shields.io/github/license/erikzenker/hsm.svg)](https://github.com/erikzenker/hsm/blob/master/LICENSE) [![GitHub contributors](https://img.shields.io/github/contributors/erikzenker/hsm.svg)](https://GitHub.com/erikzenker/hsm/graphs/contributors/) [![GitHub release](https://img.shields.io/github/release/erikzenker/hsm.svg)](https://GitHub.com/erikzenker/hsm/releases/) [![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://paypal.me/erikzenker) [![Join the chat at https://gitter.im/hsm-gitter/community](https://badges.gitter.im/hsm-gitter/community.svg)](https://gitter.im/hsm-gitter/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
=

The **hana state machine** (hsm) is a [finite state machine](https://en.wikipedia.org/wiki/Finite-state_machine) library based on the [boost hana](https://www.boost.org/doc/libs/1_61_0/libs/hana/doc/html/index.html) meta programming library. It follows the principles of the [boost msm](https://www.boost.org/doc/libs/1_64_0/libs/msm/doc/HTML/index.html) and [boost sml](https://boost-ext.github.io/sml/index.html) libraries, but tries to reduce own complex meta programming code to a minimum.

The following table compares features among popular c++ state machine libraries.
A click on a particular feature check mark will forward to the feature documentation.

 <table style="width:100%">
  <tr>
    <th>Feature</th>
    <th><a href="https://github.com/erikzenker/hsm">Hsm</a></th>
    <th><a href="https://boost-ext.github.io/sml/index.html">Sml</a></th>
    <th><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/index.html">Msm</a></th>
    <th><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/index.html">Statechart</a></th>
  </tr>
  <tr>
    <td>External transition</td>
    <td><center><a href="test/integration/basic_transitions.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#transitions">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e358">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/uml_mapping.html#Transition">✓</a></center></td>
  </tr>
  <tr>
    <td>Anonymous transition (Completion)</td>
    <td><center><a href="test/integration/anonymous_transition.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#transitions">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e713">✓</a></center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>Internal transition</td>
    <td><center><a href="test/integration/internal_transition.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#transitions">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e740">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/uml_mapping.html#InternalTransition">✓</a></center></td>
  </tr>
  <tr>
    <td>Direct transition</td>
    <td><center><a href="test/integration/direct_transition.cpp">✓</a></center></td>
    <td><center>✗</center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e875">✓</a></center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>Guards / actions</td>
    <td><center><a href="test/integration/guards_actions.cpp">✓</a></center></td>
    <td><center><a href="test/integration/guards_actions.cpp">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e358">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/uml_mapping.html#Reactions">✓</a></center></td>
  </tr>
  <tr>
    <td>Entry / exit actions</td>
    <td><center><a href="test/integration/entry_exit_actions.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#transitions">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e406">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/uml_mapping.html#Actions">✓</a></center></td>
  </tr>  
  <tr>
    <td>Orthogonal regions</td>
    <td><center><a href="test/integration/orthogonal_regions.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#orthogonal-regions">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e577">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/tutorial.html#OrthogonalStates">✓</a></center></td>
  </tr> 
  <tr>
    <td>Hierachies / sub state machines</td>
    <td><center><a href="test/integration/basic_transitions.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#composite">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e529">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/tutorial.html#BasicTopicsAStopWatch">✓</a></center></td>
  </tr>
  <tr>
    <td>Event defering</td>
    <td><center><a href="test/integration/defer_events.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#deferprocess">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/tutorial.html#DeferringEvents">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/tutorial.html#DeferringEvents">✓</a></center></td>
  </tr>  
  <tr>
    <td>Transition logging</td>
    <td><center><a href="test/integration/transition_logging.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#logging">✓</a></center></td>
    <td><center>✓</center></td>
    <td><center>?</center></td>
  </tr>
  <tr>
    <td>Initial pseudo state</td>
    <td><center><a href="test/integration/basic_transitions.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/uml_vs_sml.html">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e471">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/tutorial.html#DefiningStatesAndEvents">✓</a></center></td>
  </tr>
  <tr>
    <td>History pseudo state</td>
    <td><center><a href="test/integration/history_pseudo_state.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#history">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e668">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/tutorial.html#History">✓</a></center></td>
  </tr>
  <tr>
    <td>eUml postfix frontend</td>
    <td><center><a href="test/integration/transition_postfix_dsl.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/uml_vs_sml.html">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s04.html">✓</a></center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>eUml prefix frontend</td>
    <td><center><a href="test/integration/transition_prefix_dsl.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/uml_vs_sml.html">✓</a></center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s04.html">✓</a></center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>Entry / exit pseudo state</td>
    <td><center><a href="test/integration/entry_exit_pseudo_states.cpp">✓</a></center></td>
    <td><center>✗</center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e875">✓</a></center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>State data members</td>
    <td><center><a href="test/integration/state_data_members.cpp">✓</a></center></td>
    <td><center><a href="https://github.com/boost-ext/sml/blob/master/example/data.cpp">✓</a></center></td>
    <td><center>✓</center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/tutorial.html#StateLocalStorage">✓</a></center></td>
  </tr>
  <tr>
    <td>Unexpected event / no transition handler</td>
    <td><center><a href="test/integration/unexpected_transition_handler.cpp">✓</a></center></td>
    <td><center>✗</center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/ch03s02.html#d0e471">✓</a></center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>Dependency injection</td>
    <td><center><a href="test/integration/dependency_injection.cpp">✓</a></center></td>
    <td><center><a href="https://boost-ext.github.io/sml/examples.html#dependency-injection">✓</a></center></td>
    <td><center>✗</center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>Single amalgamation header</td>
    <td><center><a href="src/include/hsm_gen.h">✓</a></center></td>
    <td><center><a href="https://raw.githubusercontent.com/boost-ext/sml/master/include/boost/sml.hpp">✓</a></center></td>
    <td><center>✗</center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>Custom target state construction</td>
    <td><center><a href="test/integration/custom_targets.cpp">✓</a></center></td>
    <td><center>✗</center></td>
    <td><center>✗</center></td>
    <td><center>✗</center></td>
  </tr>
  <tr>
    <td>Chain actions</td>
    <td><center><a href="test/integration/chain_actions.cpp">✓</a></center></td>
    <td><center><a href="https://github.com/boost-ext/sml/blob/master/example/actions_guards.cpp">✓</a><center></td>
    <td><center><a href="https://www.boost.org/doc/libs/1_63_0/libs/msm/doc/HTML/ch03s02.html">✓</a></center></td>
    <td><center>?</center></td>
  </tr>
</table> 

## Example ([Run](https://godbolt.org/z/1YYc9fo8a))
```c++
#include "hsm/hsm.h"

#include <iostream>
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
const auto noError = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };

// Actions
constexpr auto beep
    = [](auto /*event*/, auto /*source*/, auto /*target*/) { std::cout << "beep!" << std::endl; };
constexpr auto blink = [](auto /*event*/, auto /*source*/, auto /*target*/) {
    std::cout << "blink, blink, blink!" << std::endl;
};

struct Turnstile {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            // Source              + Event            [Guard]   / Action = Target
            // +-------------------+-----------------+---------+--------+----------------------+
            * hsm::state<Locked>   + hsm::event<Push>           / beep   = hsm::state<Locked>  ,
              hsm::state<Locked>   + hsm::event<Coin> [noError] / blink  = hsm::state<Unlocked>,
            // +--------------------+---------------------+---------+--------+------------------------+
              hsm::state<Unlocked> + hsm::event<Push> [noError]          = hsm::state<Locked>  ,
              hsm::state<Unlocked> + hsm::event<Coin>           / blink  = hsm::state<Unlocked>
            // +--------------------+---------------------+---------+--------+------------------------+                        
            );
        // clang-format on
    }
};

auto main() -> int
{
    hsm::sm<Turnstile> turnstileSm;

    // The turnstile is initially locked
    assert(turnstileSm.is(hsm::state<Locked>));

    // Inserting a coin unlocks it
    turnstileSm.process_event(Coin {});
    assert(turnstileSm.is(hsm::state<Unlocked>));

    // Entering the turnstile will lock it again
    turnstileSm.process_event(Push {});
    assert(turnstileSm.is(hsm::state<Locked>));

    return 0;
}
```

## Play with it Online
* Follow the link to the compiler explorer: [https://godbolt.org/z/r9sTrMfqE](https://godbolt.org/z/r9sTrMfqE)


## Runtime Benchmark Results

The benchmark result are taken from the [state machine benchmark repository](https://github.com/erikzenker/state-machine-benchmark).

 <table style="width:100%">
  <tr>
    <th>Benchmark</th>
    <th><a href="https://github.com/erikzenker/hsm">Hsm</a></th>
    <th><a href="https://boost-experimental.github.io/sml/index.html">Sml</a></th>
    <th><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/index.html">Msm</a></th>
    <th><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/index.html">Statechart</a></th>
  </tr>
  <tr>
    <td><a href="benchmark/simple/">Simple state machine</a></td>
    <td><center>99 ms</center></td>
    <td><center>17 ms</center></td>
    <td><center>18 ms</center></td>
    <td><center>443 ms</center></td>
  </tr>
  <tr>
    <td><a href="benchmark/complex/">Complex state machine</a></td>
    <td><center>818 ms</center></td>
    <td><center>978 ms</center></td>
    <td><center>881 ms</center></td>
    <td><center>1374 ms</center></td>
  </tr>
</table> 

## Compiletime Benchmark Results

 <table style="width:100%">
  <tr>
    <th>Benchmark</th>
    <th><a href="https://github.com/erikzenker/hsm">Hsm</a></th>
    <th><a href="https://boost-experimental.github.io/sml/index.html">Sml</a></th>
    <th><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/index.html">Msm</a></th>
    <th><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/index.html">Statechart</a></th>
  </tr>
  <tr>
    <td><a href="benchmark/simple/">Simple state machine</a></td>
    <td><center>6.41 s</center></td>
    <td><center>0.62 s</center></td>
    <td><center>5.17 s </center></td>
    <td><center>1.52 s</center></td>
  </tr>
  <tr>
    <td><a href="benchmark/complex/">Complex state machine</a></td>
    <td><center>41.99 s</center></td>
    <td><center>3.01 s</center></td>
    <td><center>25.54 s</center></td>
    <td><center>4.27 s</center></td>
  </tr>
</table>

## Compilememory Benchmark Results

 <table style="width:100%">
  <tr>
    <th>Benchmark</th>
    <th><a href="https://github.com/erikzenker/hsm">Hsm</a></th>
    <th><a href="https://boost-experimental.github.io/sml/index.html">Sml</a></th>
    <th><a href="https://www.boost.org/doc/libs/1_73_0/libs/msm/doc/HTML/index.html">Msm</a></th>
    <th><a href="https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/index.html">Statechart</a></th>
  </tr>
  <tr>
    <td><a href="benchmark/simple/">Simple state machine</a></td>
    <td><center>174.649 MB</center></td>
    <td><center>28.474 MB</center></td>
    <td><center>404.621 MB</center></td>
    <td><center>70.976 MB</center></td>
  </tr>
  <tr>
    <td><a href="benchmark/complex/">Complex state machine</a></td>
    <td><center>815.720 MB</center></td>
    <td><center>188.333 MB</center></td>
    <td><center>1323.477 MB</center></td>
    <td><center>122.720 MB</center></td>
  </tr>
</table>

## Dependencies
* Boost 1.72
* C++17
* \>= g++-8
* \>= clang-8
* Cmake 3.14

## Dev Dependencies
* Gtest

## Integration
### Usage as Single Header
* Download [amalgamation header](https://raw.githubusercontent.com/erikzenker/hsm/master/include/hsm/gen/hsm.h) and put it into your project src folder
* Include amalgamation header:
  ```c++
  #include "path/to/amalgamation/header/hsm.h"
  ```

### CMake
To use this library from a CMake project, you can locate it directly with find_package() and use the namespaced imported target from the generated package configuration:

```cmake
# CMakeLists.txt
find_package(hsm 1.3.5 REQUIRED)
...
add_library(foo ...)
...
target_link_libraries(foo PRIVATE hsm::hsm)
```

Since CMake v3.11, [FetchContent](https://cmake.org/cmake/help/v3.11/module/FetchContent.html) can be used to automatically download the repository as a dependency at configure time. You can follow this [example](integration/fetch_content) and 
implement the following snippet:

```cmake
include(FetchContent)

FetchContent_Declare(hsm
  GIT_REPOSITORY https://github.com/erikzenker/hsm.git
  GIT_TAG v1.4.7)

FetchContent_GetProperties(hsm)
if(NOT hsm_POPULATED)
  FetchContent_Populate(hsm)
  add_subdirectory(${hsm_SOURCE_DIR} ${hsm_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(foo PRIVATE hsm::hsm)
```

If you are using [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake), you can follow this [example](integration/cpm). Implement the following snippet:

```cmake
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME hsm
    GITHUB_REPOSITORY erikzenker/hsm
    VERSION 1.4.7)

target_link_libraries(foo PRIVATE hsm::hsm)
```

### Package Managers

If you are using [Conan](https://conan.io/) to manage your dependencies, merely add hsm/x.y.z to your `conanfile`'s requires, where `x.y.z` is the release version you want to use. Please file issues here if you experience problems with the packages.

## Install 

### CMake
``` bash
cmake -S . -B build
cmake --install build/ --prefix /tmp/
```

### Conan/Cmake
``` bash
mkdir -p build/dependencies/conan
conan install . -if build/dependencies/conan -s compiler.libcxx=libstdc++11 --build missing
cmake -S . -B build
cmake --install build/ --prefix /tmp/ -D "CMAKE_MODULE_PATH=${PWD}/build/dependencies/conan"
```
### Conan [![Download](https://api.bintray.com/packages/erikzenker/conan-erikzenker/hsm%3Aerikzenker/images/download.svg) ](https://bintray.com/erikzenker/conan-erikzenker/hsm%3Aerikzenker/_latestVersion)
``` bash
conan remote add conan-erikzenker https://api.bintray.com/conan/erikzenker/conan-erikzenker
conan install hsm/1.0@erikzenker/testing --build missing
```

### Install from Arch Linux AUR
``` bash
pacaur -S hsm-git
```

## Compile and Run the Tests Using the Installed Library
``` bash
cmake -S test -B build
cmake --build build/test
cd build/test
ctest --output-on-failure
```

## Author
* erikzenker(at)hotmail.com
