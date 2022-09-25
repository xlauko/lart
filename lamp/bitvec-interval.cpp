#include <lava/bitvec_interval.hpp>
#include <lamp/support/storage.hpp>
#include <lava/support/relational.hpp>

namespace __lamp
{
    using interval = __lava::bitvec_interval< wrapped_storage >;
    using meta_domain = __lava::relational< interval, wrapped_storage >;
} // namespace __lamp

#include "wrapper.hpp"
