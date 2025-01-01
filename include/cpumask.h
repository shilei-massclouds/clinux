/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_CPUMASK_H
#define __LINUX_CPUMASK_H

#define nr_cpu_ids 1U

#define nr_cpumask_bits ((unsigned int)NR_CPUS)

/**
 * cpumask_size - size to allocate for a 'struct cpumask' in bytes
 */
static inline unsigned int cpumask_size(void)
{
    return BITS_TO_LONGS(nr_cpumask_bits) * sizeof(long);
}

#endif /* __LINUX_CPUMASK_H */
