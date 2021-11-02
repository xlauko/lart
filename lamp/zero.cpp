#include <lava/zero.hpp>
#include <lamp/support/storage.hpp>
#include <lava/support/relational.hpp>

namespace __lamp
{
    using zero = __lava::zero< wrapped_storage >;    
    using meta_domain = __lava::relational< zero, wrapped_storage >;
} // namespace __lamp

#include "wrapper.hpp"
