# SPDX-License-Identifier: GPL-2.0

PHONY := all clean

CC := gcc

INCLUDES := -I.
CFLAGS := $(shell pkg-config --cflags json-c)
LDFLAGS := $(shell pkg-config --libs json-c)

TARGET := mk_bootrd

all: $(TARGET)
	@:

$(TARGET): main.c
	@printf "MK\t$@\n"
	@$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $^ $(LDFLAGS)

clean:
	@rm -rf *.o $(TARGET)

.PHONY: $(PHONY)
