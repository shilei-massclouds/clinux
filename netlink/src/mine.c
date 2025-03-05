#include <linux/types.h>
#include "../../booter/src/booter.h"

CL_MINE(dev_mc_add_excl)
CL_MINE(dev_uc_add_excl)
CL_MINE(dev_uc_del)
CL_MINE(refcount_dec_and_mutex_lock)

CL_MINE(__scm_destroy)
CL_MINE(scm_detach_fds)
CL_MINE(__scm_send)
CL_MINE(yield)

CL_MINE(netlink_policy_dump_loop)
CL_MINE(netlink_policy_dump_start)
CL_MINE(netlink_policy_dump_free)
CL_MINE(netlink_policy_dump_write)
