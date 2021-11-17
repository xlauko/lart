#include <lava/interval.hpp>
#include <lamp/support/storage.hpp>

namespace __lamp
{
    using interval = __lava::interval< wrapped_storage >;
    using meta_domain = __lava::relational< interval, wrapped_storage >;
} // namespace __lamp


#include "wrapper.hpp"