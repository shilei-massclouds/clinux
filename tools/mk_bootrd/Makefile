# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run

CC := gcc

INCLUDES := -I.
CFLAGS := $(shell pkg-config --cflags json-c) -DCL_TOOLS
LDFLAGS := $(shell pkg-config --libs json-c)

TARGET := mk_bootrd

all: $(TARGET)
	@:

$(TARGET): main.c
	@printf "MK\t$@\n"
	@$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $^ $(LDFLAGS)

clean:
	@rm -rf *.o *.json $(TARGET)

run:
	@:

.PHONY: $(PHONY)
