# SPDX-License-Identifier: GPL-2.0

target_y := rust_target

obj_y += core.o compiler_builtins.o
obj_y += alloc.o bindings.o kernel.o
obj_y += build_error.o

rust_target: $(obj)/core.o $(obj)/compiler_builtins.o \
	$(obj)/alloc.o $(obj)/bindings.o $(obj)/kernel.o $(obj)/build_error.o
	@:

core-cfgs = \
    --cfg no_fp_fmt_parse

alloc-cfgs = \
    --cfg no_global_oom_handling \
    --cfg no_rc \
    --cfg no_sync

# `$(rust_flags)` is passed in case the user added `--sysroot`.
rustc_sysroot := $(shell $(RUSTC) $(rust_flags) --print sysroot)
rustc_host_target := $(shell $(RUSTC) --version --verbose | grep -F 'host: ' | cut -d' ' -f2)
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

$(obj)/kernel.o: private rustc_target_flags = --extern alloc \
	--extern build_error --extern macros --extern bindings
$(obj)/kernel.o: $(obj)/kernel/lib.rs $(obj)/alloc.o $(obj)/build_error.o \
	$(obj)/libmacros.so $(obj)/bindings.o
	@$(rustc_library)

$(obj)/alloc.o: private skip_clippy = 1
$(obj)/alloc.o: private skip_flags = -Dunreachable_pub
$(obj)/alloc.o: private rustc_target_flags = $(alloc-cfgs)
$(obj)/alloc.o: $(obj)/alloc/lib.rs $(obj)/compiler_builtins.o
	@$(rustc_library)

$(obj)/build_error.o: $(obj)/build_error.rs $(obj)/compiler_builtins.o
	@$(rustc_library)

rustc_procmacro = \
	$(RUSTC) $(rust_common_flags) \
		--emit=dep-info,link --extern proc_macro \
		--crate-type proc-macro --out-dir ./$(obj) \
		--crate-name $(patsubst lib%.so,%,$(notdir $@)) $<; \
	mv ./$(obj)/$(patsubst lib%.so,%,$(notdir $@)).d $(depfile); \
	sed -i '/^\#/d' $(depfile)

# Procedural macros can only be used with the `rustc` that compiled it.
# Therefore, to get `libmacros.so` automatically recompiled when the compiler
# version changes, we add `core.o` as a dependency (even if it is not needed).
$(obj)/libmacros.so: $(obj)/macros/lib.rs $(obj)/core.o
	@$(rustc_procmacro)

# $(obj)/bindings/bindings_generated.rs
# $(obj)/bindings/bindings_helpers_generated.rs
$(obj)/bindings.o: $(obj)/bindings/lib.rs $(obj)/compiler_builtins.o
	@$(rustc_library)

clean:
	@rm -f *.o *.d *.rmeta *.so
