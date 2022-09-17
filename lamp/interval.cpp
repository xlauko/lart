#include <lava/interval.hpp>
#include <lamp/support/storage.hpp>
#include <lava/support/relational.hpp>

namespace __lamp
{
    using interval = __lava::interval< wrapped_storage >;
    using meta_domain = __lava::relational< interval, wrapped_storage >;
}

#include "wrapper.hpp"