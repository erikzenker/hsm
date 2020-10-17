#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm::details::utils {

constexpr auto toTypeid
    = [](auto tuple) { return boost::hana::transform(tuple, boost::hana::typeid_); };

}