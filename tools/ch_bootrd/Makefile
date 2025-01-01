# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run

CC := gcc

INCLUDES := -I.
CFLAGS := -DCL_TOOLS

TARGET := ch_bootrd

all: $(TARGET)
	@:

$(TARGET): main.c
	@printf "MK\t$@\n"
	@$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $^

clean:
	@rm -rf *.o $(TARGET)

run: $(TARGET)
	@:

.PHONY: $(PHONY)
