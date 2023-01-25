# SPDX-License-Identifier: GPL-2.0

PHONY := all clean

TARGET := rootfs.ext2

all: $(TARGET)
	@:

$(TARGET):
	@printf "TOOL\t$@\n"
	@ ./mk.sh

clean:
	@ ./clr.sh

.PHONY: $(PHONY)
