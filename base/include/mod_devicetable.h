/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LINUX_MOD_DEVICETABLE_H
#define LINUX_MOD_DEVICETABLE_H

struct of_device_id {
    char        name[32];
    char        type[32];
    char        compatible[128];
    const void  *data;
};

#endif /* LINUX_MOD_DEVICETABLE_H */
