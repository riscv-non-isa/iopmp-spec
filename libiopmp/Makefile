cur_dir=$(CURDIR)
src_dir=$(cur_dir)/src
inc_dir=$(cur_dir)/include
build_dir=$(cur_dir)/build

include $(cur_dir)/config.mk
include $(src_dir)/objects.mk

# Setup compilation commands
ifneq ($(LLVM),)
ifneq ($(filter %/,$(LLVM)),)
LLVM_PREFIX := $(LLVM)
else ifneq ($(filter -%,$(LLVM)),)
LLVM_SUFFIX := $(LLVM)
endif

CC	= $(LLVM_PREFIX)clang$(LLVM_SUFFIX)
AR	= $(LLVM_PREFIX)llvm-ar$(LLVM_SUFFIX)
LD	= $(LLVM_PREFIX)ld.lld$(LLVM_SUFFIX)
OBJCOPY	= $(LLVM_PREFIX)llvm-objcopy$(LLVM_SUFFIX)
else
ifdef CROSS_COMPILE
CC	= $(CROSS_COMPILE)gcc
AR	= $(CROSS_COMPILE)ar
LD	= $(CROSS_COMPILE)ld
OBJCOPY	= $(CROSS_COMPILE)objcopy
else
CC	?= gcc
AR	?= ar
LD	?= ld
OBJCOPY	?= objcopy
endif
endif
CPP	= $(CC) -E
AS	= $(CC)

CFLAGS	= -g -Wall -Werror -ffreestanding -fno-stack-protector
CFLAGS	+= -fno-strict-aliasing -ffunction-sections -fdata-sections
CFLAGS	+= -fno-omit-frame-pointer -fno-optimize-sibling-calls
CFLAGS	+= -fno-asynchronous-unwind-tables -fno-unwind-tables
CFLAGS	+= -I$(src_dir) -I$(inc_dir)
CFLAGS	+= --coverage

ifeq ($(DEBUG),y)
CFLAGS	+= -O0 -DDEBUG
else
CFLAGS	+= -O2
endif

ifeq ($(CFG_IOPMP_REF_MODEL),y)
CFLAGS	+= -DENABLE_IO_WEAK_FUNCTIONS
endif

ifeq ($(CFG_IOPMP_DRV_SPS_EXTENSION),y)
CFLAGS	+= -DENABLE_SPS
endif

ARFLAGS	= rcs

# Check if verbosity is ON for build process
CMD_PREFIX_DEFAULT := @
ifeq ($(V), 1)
	CMD_PREFIX :=
else
	CMD_PREFIX := $(CMD_PREFIX_DEFAULT)
endif

libiopmp-objs-path-y=$(foreach obj,$(libiopmp-objs-y),$(build_dir)/$(obj))

compile_ar = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " AR        $(subst $(build_dir)/,,$(1))"; \
	     $(AR) $(ARFLAGS) $(1) $(2)
compile_cc = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CC        $(subst $(build_dir)/,,$(1))"; \
	     $(CC) $(CFLAGS) -c $(2) -o $(1)
compile_carray = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CARRAY    $(subst $(build_dir)/,,$(1))"; \
	     $(eval CARRAY_VAR_LIST := $(carray-$(subst .carray.c,,$(shell basename $(1)))-y)) \
	     $(cur_dir)/scripts/gen_iopmp_carray.sh -l "$(CARRAY_VAR_LIST)" > $(1) || rm $(1)

# Preserve all intermediate files
.SECONDARY:

# The default "make all" rule
.PHONY: all
all: $(build_dir)/lib/libiopmp.a

$(build_dir)/lib/libiopmp.a: $(libiopmp-objs-path-y)
	$(call compile_ar,$@,$^)

$(build_dir)/%.o: $(src_dir)/%.c
	$(call compile_cc,$@,$<)

$(build_dir)/%.o: $(build_dir)/%.c
	$(call compile_cc,$@,$<)

$(build_dir)/%.carray.c: $(cur_dir)/scripts/gen_iopmp_carray.sh
	$(call compile_carray,$@,$<)

clean:
	rm -rf $(build_dir)
