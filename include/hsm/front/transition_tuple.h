#pragma once

#include "hsm/details/transition.h"

namespace hsm {
constexpr auto transition = details::transition;
constexpr auto internal_transition = details::internal_transition;
}