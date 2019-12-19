#pragma once

#include "flatten_transition_table.h"
#include "remove_duplicates.h"
#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
constexpr auto collectEventTypeids = [](auto transition) { return bh::at_c<2>(transition).typeid_; };
}

constexpr auto collect_event_typeids_recursive
    = [](auto state) { 
        return bh::transform(flatten_transition_table(state), collectEventTypeids);    
};
}