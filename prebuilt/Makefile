# SPDX-License-Identifier: GPL-2.0

target_y :=

extra_y := asm-offsets.h

prebuilt/asm-offsets.h: prebuilt/asm-offsets.s
	@echo "generate asm-offsets.h"
	$(call filechk,offsets,__ASM_OFFSETS_H__)
