libiopmp-objs-y =
libiopmp-objs-y += libiopmp.o
libiopmp-objs-y += iopmp_drv_common.o
libiopmp-objs-y += iopmp_drivers.carray.o

libiopmp-objs-$(CFG_IOPMP_DRV_FULL) += iopmp_drv_full.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_FULL) += iopmp_drv_full

libiopmp-objs-$(CFG_IOPMP_DRV_RAPID_K) += iopmp_drv_rapid_k.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_RAPID_K) += iopmp_drv_rapid_k

libiopmp-objs-$(CFG_IOPMP_DRV_DYNAMIC_K) += iopmp_drv_dynamic_k.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_DYNAMIC_K) += iopmp_drv_dynamic_k

libiopmp-objs-$(CFG_IOPMP_DRV_ISOLATION) += iopmp_drv_isolation.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_ISOLATION) += iopmp_drv_isolation

libiopmp-objs-$(CFG_IOPMP_DRV_COMPACT_K) += iopmp_drv_compact_k.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_COMPACT_K) += iopmp_drv_compact_k

libiopmp-objs-$(CFG_IOPMP_DRV_SRCMD_FMT_1_MDCFG_FMT_2) += iopmp_drv_srcmd_fmt_1_mdcfg_fmt_2.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_SRCMD_FMT_1_MDCFG_FMT_2) += iopmp_drv_srcmd_fmt_1_mdcfg_fmt_2

libiopmp-objs-$(CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_0) += iopmp_drv_srcmd_fmt_2_mdcfg_fmt_0.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_0) += iopmp_drv_srcmd_fmt_2_mdcfg_fmt_0

libiopmp-objs-$(CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_1) += iopmp_drv_srcmd_fmt_2_mdcfg_fmt_1.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_1) += iopmp_drv_srcmd_fmt_2_mdcfg_fmt_1

libiopmp-objs-$(CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_2) += iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2.o
carray-iopmp_drivers-$(CFG_IOPMP_DRV_SRCMD_FMT_2_MDCFG_FMT_2) += iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2
