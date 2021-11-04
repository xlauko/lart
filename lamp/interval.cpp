#include <lava/interval.hpp>
#include <lamp/support/storage.hpp>
#include <lava/support/relational.hpp>

namespace __lamp
{
<<<<<<< HEAD
    using interval = __lava::interval< wrapped_storage >;
=======
    using interval = __lava::interval< wrapped_storage >;    
>>>>>>> 782e922 (lamp: make interval domain relational)
    using meta_domain = __lava::relational< interval, wrapped_storage >;
} // namespace __lamp


#include "wrapper.hpp"