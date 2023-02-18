# SPDX-License-Identifier: GPL-2.0

target_y += modpost
hostprogs_y += modpost

modpost_objs := modpost.c
modpost_objs += file2alias.c
modpost_objs += sumversion.c

modpost_objs := $(addprefix $(obj)/,$(modpost_objs))

$(obj)/modpost: $(modpost_objs)
	gcc -Wall -o $@ $^
