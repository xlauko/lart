#include <lava/interval.hpp>
#include <lamp/support/storage.hpp>
#include <lamp/support/tracing.hpp>

namespace __lamp
{
    using interval = __lava::interval< wrapped_storage >;
    using meta_domain = tracing< __lava::relational< interval, wrapped_storage > >;
}

#include "wrapper.hpp"