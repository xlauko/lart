#include <lava/interval.hpp>
#include <lamp/support/storage.hpp>
#include <lamp/support/tracing.hpp>

namespace __lamp
{
    using meta_domain = tracing< __lava::interval< wrapped_storage > >;
}

#include "wrapper.hpp"