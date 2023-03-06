# SPDX-License-Identifier: GPL-2.0

.PHONY: rust_build gen_headers rust_target

gen_headers = $(obj)/exports_core_generated.h

rust_target = $(obj)/core.o $(obj)/compiler_builtins.o \
	$(obj)/exports.o

rust_build: $(gen_headers) $(rust_target)

core-cfgs = \
    --cfg no_fp_fmt_parse

alloc-cfgs = \
    --cfg no_global_oom_handling \
    --cfg no_rc \
    --cfg no_sync

# `$(rust_flags)` is passed in case the user added `--sysroot`.
rustc_sysroot := $(shell $(RUSTC) $(rust_flags) --print sysroot)
RUST_LIB_SRC ?= $(rustc_sysroot)/lib/rustlib/src/rust/library

rustc_library = \
	OBJTREE=$(abspath .) \
	$(RUSTC) \
		$(filter-out $(skip_flags),$(rust_flags) $(rustc_target_flags)) \
		--emit=dep-info,obj,metadata --crate-type rlib \
		--out-dir ./$(obj) -L./$(obj) \
		--crate-name $(patsubst %.o,%,$(notdir $@)) $<; \
	mv ./$(obj)/$(patsubst %.o,%,$(notdir $@)).d $(depfile); \
	sed -i '/^\#/d' $(depfile) \
	$(if $(rustc_objcopy),;$(OBJCOPY) $(rustc_objcopy) $@)

$(obj)/core.o: private skip_clippy = 1
$(obj)/core.o: private skip_flags = -Dunreachable_pub
$(obj)/core.o: private rustc_target_flags = $(core-cfgs) -Aunused-imports
$(obj)/core.o: $(RUST_LIB_SRC)/core/src/lib.rs $(obj)/target.json
	@$(rustc_library)

$(obj)/compiler_builtins.o: private rustc_objcopy = -w -W '__*'
$(obj)/compiler_builtins.o: $(obj)/compiler_builtins.rs $(obj)/core.o
	@$(rustc_library)

cmd_exports = \
	$(NM) -p --defined-only $< \
		| grep -E ' (T|R|D) ' | cut -d ' ' -f 3 \
		| xargs -Isymbol \
		echo 'EXPORT_SYMBOL_RUST_GPL(symbol);' > $@

$(obj)/exports_core_generated.h: $(obj)/core.o
	@$(cmd_exports)

clean:
	@rm -f *.o *.d *.rmeta *.so *.ko *.map
