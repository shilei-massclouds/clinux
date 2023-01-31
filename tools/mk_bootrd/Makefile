# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run

CC := gcc

INCLUDES := -I.
CFLAGS := $(shell pkg-config --cflags json-c) -DCL_TOOLS
LDFLAGS := $(shell pkg-config --libs json-c)
BOOTRD_FILE := ../../output/bootrd.disk

TARGET := mk_bootrd

all: $(TARGET)
	@:

$(TARGET): main.c
	@printf "MK\t$@\n"
	@$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $^ $(LDFLAGS)

clean:
	@rm -rf *.o $(TARGET)

run: $(TARGET)
	@rm -f $(BOOTRD_FILE)
	@dd if=/dev/zero of=$(BOOTRD_FILE) bs=1M count=32
	./$(TARGET)

.PHONY: $(PHONY)
