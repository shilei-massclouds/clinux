TARGETS := init mount

CC := riscv64-linux-gnu-gcc
STRIP := riscv64-linux-gnu-strip
DST_DIR := build
PREFIX_TARGETS := $(addprefix $(DST_DIR)/,$(TARGETS))

all: $(PREFIX_TARGETS)

$(DST_DIR)/%: %.c
	@mkdir -p $(DST_DIR)
	$(CC) $< -o $@
	$(STRIP) $@

clean:
	@rm -rf ./build

.PHONY: all clean
