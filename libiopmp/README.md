# libiopmp - A Library to Program RISC-V IOPMP

## Compilation

`libiopmp` can be built by host compiler or RISC-V toolchain. The former one is useful when testing `libiopmp` using the reference model, while the later one is necessary if you want to use `libiopmp` on RISC-V platforms.

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

To compiled `libiopmp` by RISC-V toolchain, you need to add the "path to your RISC-V toolchain" into `$PATH` environment variable, and input the following command:

```shell
~/libiopmp$ export CROSS_COMPILE=riscv32-elf-
~/libiopmp$ make
```
