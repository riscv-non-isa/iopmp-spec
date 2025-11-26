# libiopmp - A Library to Program RISC-V IOPMP

The `libiopmp` is intended to be driver of RISC-V IOPMP which:
* Complies with IOPMP specification **v0.8.1**
* Operates one or multiple IOPMPs
* Supports several IOPMP models and configurations
* Extensible for adding vendor-customized IOPMP driver
* Supports IOPMP with Multi-Faults Record (MFR) extension
* Supports IOPMP with Secondary Permission Setting (SPS) extension
* Supports IOPMP with Message-Signaled Interrupts (MSI) extension

## Adjust `config.mk`

The `libiopmp` has a `config.mk` configuration file which let you modularize
your `libiopmp` to reduce the code size. We describe each of the configurations
here:

* `DEBUG`: Turn on this option to build `libiopmp` without compiler optimization
and assert() macro will be enabled
* `CFG_IOPMP_REF_MODEL`: Turn on this option to enable compiling of register
read/write interface as weak functions. This is useful if the IOPMP you operate
is simulated by the reference model. If you want to control real IOPMP you just
turn off this option.
* `CFG_IOPMP_DRV_FULL`: Turn on this option to enable compiling of driver for
full model
* `CFG_IOPMP_DRV_RAPID_K`: Turn on this option to enable compiling of driver for
rapid-k model
* `CFG_IOPMP_DRV_DYNAMIC_K`: Turn on this option to enable compiling of driver
for dynamic-k model
* `CFG_IOPMP_DRV_ISOLATION`: Turn on this option to enable compiling of driver
for isolation model
* `CFG_IOPMP_DRV_COMPACT_K`: Turn on this option to enable compiling of driver
for compact-k model
* `CFG_IOPMP_DRV_SRCMD_FMT_1_MDCFG_FMT_2`: Turn on this option to enable
compiling of driver for SRCMD_FMT=1 & MDCFG_FMT=2
* `CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_0`: Turn on this option to enable
compiling of driver for SRCMD_FMT=2 & MDCFG_FMT=0
* `CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_1`: Turn on this option to enable
compiling of driver for SRCMD_FMT=2 & MDCFG_FMT=1
* `CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_2`: Turn on this option to enable
compiling of driver for SRCMD_FMT=2 & MDCFG_FMT=2
* `CFG_IOPMP_DRV_SPS_EXTENSION`: Turn on this option to enable compiling of
driver for Secondary Permission Setting (SPS) extension

## Compilation

`libiopmp` can be built by host compiler or RISC-V toolchain. The former one is
useful when testing `libiopmp` using the reference model, while the later one is
necessary if you want to use `libiopmp` on RISC-V platforms.

### Compiled by Host GCC

```shell
~/libiopmp$ make
 CC        libiopmp.o
 CC        iopmp_drv_common.o
 CARRAY    iopmp_drivers.carray.c
 CC        iopmp_drivers.carray.o
 CC        iopmp_drv_full.o
 CC        iopmp_drv_rapid_k.o
 CC        iopmp_drv_dynamic_k.o
 CC        iopmp_drv_isolation.o
 CC        iopmp_drv_compact_k.o
 CC        iopmp_drv_srcmd_fmt_1_mdcfg_fmt_2.o
 CC        iopmp_drv_srcmd_fmt_2_mdcfg_fmt_0.o
 CC        iopmp_drv_srcmd_fmt_2_mdcfg_fmt_1.o
 CC        iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2.o
 AR        lib/libiopmp.a
```

### Compiled by RISC-V toolchain

To compiled `libiopmp` by RISC-V toolchain, you need to add the "path to your
RISC-V toolchain" into `$PATH` environment variable, and input the following
command:

For RV32 target:
```shell
~/libiopmp$ export CROSS_COMPILE=riscv32-unknown-elf-
~/libiopmp$ make
```

For RV64 target:
```shell
~/libiopmp$ export CROSS_COMPILE=riscv64-unknown-elf-
~/libiopmp$ make
```

## Usage

Assume the directory path to libiopmp is `$(LIBIOPMP_DIR)`, the output library
archive will be `$(LIBIOPMP_DIR)/build/lib/libiopmp.a`. All the data structures
and the APIs are declared in `$(LIBIOPMP_DIR)/include/libiopmp.h` header file.

Add the path to the library and header file into your build system. Assume
`CFLAGS` represents the compiler flags and `LDFLAGS` represents the linker
flags, please add the path to `libiopmp.h` into `CFLAGS` and `libiopmp.a` into
`LDFLAGS` accordingly:

```bash
CFLAGS  += -I$(LIBIOPMP_DIR)/include
LDFLAGS += $(LIBIOPMP_DIR)/build/lib/libiopmp.a
```

Then, including the `libiopmp.h` into your program and using the APIs to
operate your IOPMP:

```c
#include "libiopmp.h"
```

## Documentation

Please check the `libiopmp.pdf` under `docs` folder. 
