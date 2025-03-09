// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/bpf.h>
#include <net/protocol.h>
#include <cl_hook.h>

#include "../../booter/src/booter.h"

int
cl_filter_init(void)
{
    sbi_puts("module[filter]: init begin ...\n");
    sbi_puts("module[filter]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_filter_init);

DEFINE_ENABLE_FUNC(filter);

const struct bpf_func_proto bpf_get_current_uid_gid_proto;
const struct bpf_func_proto bpf_get_current_comm_proto;
const struct bpf_func_proto bpf_get_current_cgroup_id_proto;
const struct bpf_func_proto bpf_get_smp_processor_id_proto;
const struct bpf_func_proto bpf_get_current_pid_tgid_proto;
const struct bpf_func_proto bpf_get_current_ancestor_cgroup_id_proto;
const struct bpf_func_proto bpf_get_local_storage_proto;
struct proto tcpv6_prot;
