/*
 * Copyright 2018-2025 Andes Technology Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "libiopmp.h"
#include "libiopmp_def.h"

#include "iopmp_drv_common.h"

/******************************************************************************/
/* Memory-mapped offsets for standard IOPMP                                   */
/******************************************************************************/
/* INFO Registers */
#define IOPMP_VERSION_BASE                      0x0000
    #define IOPMP_VERSION_VENDOR_SHIFT          0
    #define IOPMP_VERSION_VENDOR_MASK           GENMASK_32(23, 0)
    #define IOPMP_VERSION_SPECVER_SHIFT         24
    #define IOPMP_VERSION_SPECVER_MASK          GENMASK_32(31, 24)

#define IOPMP_IMPLEMENTATION_BASE               0x0004
    #define IOPMP_IMPLEMENTATION_IMPID_SHIFT    0
    #define IOPMP_IMPLEMENTATION_IMPID_MASK     GENMASK_32(31, 0)

#define IOPMP_HWCFG0_BASE                       0x0008
    #define IOPMP_HWCFG0_MDCFG_FMT_SHIFT        0
    #define IOPMP_HWCFG0_MDCFG_FMT_MASK         GENMASK_32(1, 0)
    #define IOPMP_HWCFG0_SRCMD_FMT_SHIFT        2
    #define IOPMP_HWCFG0_SRCMD_FMT_MASK         GENMASK_32(3, 2)
    #define IOPMP_HWCFG0_TOR_EN_SHIFT           4
    #define IOPMP_HWCFG0_TOR_EN_MASK            GENMASK_32(4, 4)
    #define IOPMP_HWCFG0_SPS_EN_SHIFT           5
    #define IOPMP_HWCFG0_SPS_EN_MASK            GENMASK_32(5, 5)
    #define IOPMP_HWCFG0_USER_CFG_EN_SHIFT      6
    #define IOPMP_HWCFG0_USER_CFG_EN_MASK       GENMASK_32(6, 6)
    #define IOPMP_HWCFG0_PRIENT_PROG_SHIFT      7
    #define IOPMP_HWCFG0_PRIENT_PROG_MASK       GENMASK_32(7, 7)
    #define IOPMP_HWCFG0_RRID_TRANSL_EN_SHIFT   8
    #define IOPMP_HWCFG0_RRID_TRANSL_EN_MASK    GENMASK_32(8, 8)
    #define IOPMP_HWCFG0_RRID_TRANSL_PROG_SHIFT 9
    #define IOPMP_HWCFG0_RRID_TRANSL_PROG_MASK  GENMASK_32(9, 9)
    #define IOPMP_HWCFG0_CHK_X_SHIFT            10
    #define IOPMP_HWCFG0_CHK_X_MASK             GENMASK_32(10, 10)
    #define IOPMP_HWCFG0_NO_X_SHIFT             11
    #define IOPMP_HWCFG0_NO_X_MASK              GENMASK_32(11, 11)
    #define IOPMP_HWCFG0_NO_W_SHIFT             12
    #define IOPMP_HWCFG0_NO_W_MASK              GENMASK_32(12, 12)
    #define IOPMP_HWCFG0_STALL_EN_SHIFT         13
    #define IOPMP_HWCFG0_STALL_EN_MASK          GENMASK_32(13, 13)
    #define IOPMP_HWCFG0_PEIS_SHIFT             14
    #define IOPMP_HWCFG0_PEIS_MASK              GENMASK_32(14, 14)
    #define IOPMP_HWCFG0_PEES_SHIFT             15
    #define IOPMP_HWCFG0_PEES_MASK              GENMASK_32(15, 15)
    #define IOPMP_HWCFG0_MFR_EN_SHIFT           16
    #define IOPMP_HWCFG0_MFR_EN_MASK            GENMASK_32(16, 16)
    #define IOPMP_HWCFG0_MD_ENTRY_NUM_SHIFT     17
    #define IOPMP_HWCFG0_MD_ENTRY_NUM_MASK      GENMASK_32(23, 17)
    #define IOPMP_HWCFG0_MD_NUM_SHIFT           24
    #define IOPMP_HWCFG0_MD_NUM_MASK            GENMASK_32(29, 24)
    #define IOPMP_HWCFG0_ADDRH_EN_SHIFT         30
    #define IOPMP_HWCFG0_ADDRH_EN_MASK          GENMASK_32(31, 30)
    #define IOPMP_HWCFG0_ENABLE_SHIFT           31
    #define IOPMP_HWCFG0_ENABLE_MASK            GENMASK_32(31, 31)

#define IOPMP_HWCFG1_BASE                       0x000C
    #define IOPMP_HWCFG1_RRID_NUM_SHIFT         0
    #define IOPMP_HWCFG1_RRID_NUM_MASK          GENMASK_32(15, 0)
    #define IOPMP_HWCFG1_ENTRY_NUM_SHIFT        16
    #define IOPMP_HWCFG1_ENTRY_NUM_MASK         GENMASK_32(31, 16)

#define IOPMP_HWCFG2_BASE                       0x0010
    #define IOPMP_HWCFG2_PRIO_ENTRY_SHIFT       0
    #define IOPMP_HWCFG2_PRIO_ENTRY_MASK        GENMASK_32(15, 0)
    #define IOPMP_HWCFG2_RRID_TRANSL_SHIFT      16
    #define IOPMP_HWCFG2_RRID_TRANSL_MASK       GENMASK_32(31, 16)

#define IOPMP_ENTRY_OFFSET_BASE                 0x0014
    #define IOPMP_ENTRYOFFSET_SHIFT             0
    #define IOPMP_ENTRYOFFSET_MASK              GENMASK_32(31, 0)

#define IOPMP_HWCFG_USER_BASE                   0x002C
    #define IOPMP_HWCFG_USER_SHIFT              0
    #define IOPMP_HWCFG_USER_MASK               GENMASK_32(31, 0)

/* Programming Protection Registers */
#define IOPMP_MDSTALL_BASE                      0x0030
    #define IOPMP_MDSTALL_EXEMPT_SHIFT          0
    #define IOPMP_MDSTALL_EXEMPT_MASK           GENMASK_32(0, 0)
    #define IOPMP_MDSTALL_IS_BUSY_SHIFT         0
    #define IOPMP_MDSTALL_IS_BUSY_MASK          IOPMP_MDSTALL_EXEMPT_MASK
    #define IOPMP_MDSTALL_MD_SHIFT              1
    #define IOPMP_MDSTALL_MD_MASK               GENMASK_32(31, 1)

#define IOPMP_MDSTALLH_BASE                     0x0034
    #define IOPMP_MDSTALLH_MDH_SHIFT            0
    #define IOPMP_MDSTALLH_MDH_MASK             GENMASK_32(31, 0)

#define IOPMP_RRIDSCP_BASE                      0x0038
    #define IOPMP_RRIDSCP_RRID_SHIFT            0
    #define IOPMP_RRIDSCP_RRID_MASK             GENMASK_32(15, 0)
    #define IOPMP_RRIDSCP_OP_SHIFT              30
    #define IOPMP_RRIDSCP_OP_MASK               GENMASK_32(31, 30)
    #define IOPMP_RRIDSCP_STAT_SHIFT            IOPMP_RRIDSCP_OP_SHIFT
    #define IOPMP_RRIDSCP_STAT_MASK             IOPMP_RRIDSCP_OP_MASK

/* Configuration Protection Registers */
#define IOPMP_MDLCK_BASE                        0x0040
    #define IOPMP_MDLCK_L_SHIFT                 0
    #define IOPMP_MDLCK_L_MASK                  GENMASK_32(0, 0)
    #define IOPMP_MDLCK_MD_SHIFT                1
    #define IOPMP_MDLCK_MD_MASK                 GENMASK_32(31, 1)

#define IOPMP_MDLCKH_BASE                       0x0044
    #define IOPMP_MDLCKH_MDH_SHIFT              0
    #define IOPMP_MDLCKH_MDH_MASK               GENMASK_32(31, 0)

#define IOPMP_MDCFGLCK_BASE                     0x0048
    #define IOPMP_MDCFGLCK_L_SHIFT              0
    #define IOPMP_MDCFGLCK_L_MASK               GENMASK_32(0, 0)
    #define IOPMP_MDCFGLCK_F_SHIFT              1
    #define IOPMP_MDCFGLCK_F_MASK               GENMASK_32(7, 1)

#define IOPMP_ENTRYLCK_BASE                     0x004C
    #define IOPMP_ENTRYLCK_L_SHIFT              0
    #define IOPMP_ENTRYLCK_L_MASK               GENMASK_32(0, 0)
    #define IOPMP_ENTRYLCK_F_SHIFT              1
    #define IOPMP_ENTRYLCK_F_MASK               GENMASK_32(16, 1)

/* Error Capture Registers */
#define IOPMP_ERR_CFG_BASE                      0x0060
    #define IOPMP_ERR_CFG_L_SHIFT               0
    #define IOPMP_ERR_CFG_L_MASK                GENMASK_32(0, 0)
    #define IOPMP_ERR_CFG_IE_SHIFT              1
    #define IOPMP_ERR_CFG_IE_MASK               GENMASK_32(1, 1)
    #define IOPMP_ERR_CFG_RS_SHIFT              2
    #define IOPMP_ERR_CFG_RS_MASK               GENMASK_32(2, 2)
    #define IOPMP_ERR_CFG_MSI_EN_SHIFT          3
    #define IOPMP_ERR_CFG_MSI_EN_MASK           GENMASK_32(3, 3)
    #define IOPMP_ERR_CFG_STALL_VIO_EN_SHIFT    4
    #define IOPMP_ERR_CFG_STALL_VIO_EN_MASK     GENMASK_32(4, 4)
    #define IOPMP_ERR_CFG_MSIDATA_SHIFT         8
    #define IOPMP_ERR_CFG_MSIDATA_MASK          GENMASK_32(18, 8)

#define IOPMP_ERR_INFO_BASE                         0x0064
    #define IOPMP_ERR_INFO_V_SHIFT                  0
    #define IOPMP_ERR_INFO_V_MASK                   GENMASK_32(0, 0)
    #define IOPMP_ERR_INFO_TTYPE_SHIFT              1
    #define IOPMP_ERR_INFO_TTYPE_MASK               GENMASK_32(2, 1)
    #define IOPMP_ERR_INFO_MSI_WERR_SHIFT           3
    #define IOPMP_ERR_INFO_MSI_WERR_MASK            GENMASK_32(3, 3)
    #define IOPMP_ERR_INFO_ETYPE_SHIFT              4
    #define IOPMP_ERR_INFO_ETYPE_MASK               GENMASK_32(7, 4)
    #define IOPMP_ERR_INFO_SVC_SHIFT                8
    #define IOPMP_ERR_INFO_SVC_MASK                 GENMASK_32(8, 8)

#define IOPMP_ERR_REQADDR_BASE                      0x0068
    #define IOPMP_ERR_REQADDR_SHIFT                 0
    #define IOPMP_ERR_REQADDR_MASK                  GENMASK_32(31, 0)

#define IOPMP_ERR_REQADDRH_BASE                     0x006C
    #define IOPMP_ERR_REQADDRH_SHIFT                0
    #define IOPMP_ERR_REQADDRH_MASK                 GENMASK_32(31, 0)

#define IOPMP_ERR_REQID_BASE                        0x0070
    #define IOPMP_ERR_REQID_RRID_SHIFT              0
    #define IOPMP_ERR_REQID_RRID_MASK               GENMASK_32(15, 0)
    #define IOPMP_ERR_REQID_EID_SHIFT               16
    #define IOPMP_ERR_REQID_EID_MASK                GENMASK_32(31, 16)

#define IOPMP_ERR_MFR_BASE                          0x0074
    #define IOPMP_ERR_MFR_SVW_SHIFT                 0
    #define IOPMP_ERR_MFR_SVW_MASK                  GENMASK_32(15, 0)
    #define IOPMP_ERR_MFR_SVI_SHIFT                 16
    #define IOPMP_ERR_MFR_SVI_MASK                  GENMASK_32(27, 16)
    #define IOPMP_ERR_MFR_SVS_SHIFT                 31
    #define IOPMP_ERR_MFR_SVS_MASK                  GENMASK_32(31, 31)

#define IOPMP_ERR_MSIADDR_BASE                      0x0078

#define IOPMP_ERR_MSIADDRH_BASE                     0x007C

#define IOPMP_ERR_USER_BASE                         0x0080
    #define IOPMP_ERR_USER_USER_SHIFT               0
    #define IOPMP_ERR_USER_USER_MASK                GENMASK_32(31, 0)

/* MDCFG Table */
#define IOPMP_MDCFG_BASE                            0x0800
    #define IOPMP_MDCFG_T_SHIFT                     0
    #define IOPMP_MDCFG_T_MASK                      GENMASK_32(15, 0)

#define IOPMP_MDCFG_STRIDE                          0x4

/* SRCMD Table Registers */
#define IOPMP_SRCMD_EN_BASE                         0x1000
    #define IOPMP_SRCMD_EN_L_SHIFT                  0
    #define IOPMP_SRCMD_EN_L_MASK                   GENMASK_32(0, 0)
    #define IOPMP_SRCMD_EN_MD_SHIFT                 1
    #define IOPMP_SRCMD_EN_MD_MASK                  GENMASK_32(31, 1)

#define IOPMP_SRCMD_PERM_BASE                       IOPMP_SRCMD_EN_BASE

#define IOPMP_SRCMD_ENH_BASE                        0x1004
    #define IOPMP_SRCMD_ENH_MDH_SHIFT               0
    #define IOPMP_SRCMD_ENH_MDH_MASK                GENMASK_32(31, 0)

#define IOPMP_SRCMD_PERMH_BASE                      IOPMP_SRCMD_ENH_BASE

#define IOPMP_SRCMD_R_BASE                          0x1008
    #define IOPMP_SRCMD_R_MD_SHIFT                  1
    #define IOPMP_SRCMD_R_MD_MASK                   GENMASK_32(31, 1)

#define IOPMP_SRCMD_RH_BASE                         0x100C
    #define IOPMP_SRCMD_RH_MDH_SHIFT                0
    #define IOPMP_SRCMD_RH_MDH_MASK                 GENMASK_32(31, 0)

#define IOPMP_SRCMD_W_BASE                          0x1010
    #define IOPMP_SRCMD_W_MD_SHIFT                  1
    #define IOPMP_SRCMD_W_MD_MASK                   GENMASK_32(31, 1)

#define IOPMP_SRCMD_WH_BASE                         0x1014
    #define IOPMP_SRCMD_WH_MDH_SHIFT                0
    #define IOPMP_SRCMD_WH_MDH_MASK                 GENMASK_32(31, 0)

#define IOPMP_SRCMD_STRIDE                          0x0020

/* Entry Array Registers */
#define IOPMP_ENTRY_ADDR_BASE                       0x0000
#define IOPMP_ENTRY_ADDRH_BASE                      0x0004
#define IOPMP_ENTRY_CFG_BASE                        0x0008
#define IOPMP_ENTRY_USER_CFG_BASE                   0x000C
#define IOPMP_ENTRY_STRIDE                          0x0010

/******************************************************************************/
/* Helper macros                                                              */
/******************************************************************************/
/* Helper functions to get base address of MDCFG registers */
static inline uintptr_t get_addr_of_mdcfg(IOPMP_t *iopmp, uint32_t mdidx)
{
    return iopmp->addr + IOPMP_MDCFG_BASE + (mdidx * IOPMP_MDCFG_STRIDE);
}

/* Helper functions to get base address of SRCMD registers */
#define DECLARE_FUNC_GET_ADDR_OF_SRCMD(name, base)                      \
static inline uintptr_t get_addr_of_ ## name(IOPMP_t *iopmp,            \
                                             uint32_t idx)              \
{                                                                       \
    return iopmp->addr + base + (idx * IOPMP_SRCMD_STRIDE);             \
}
DECLARE_FUNC_GET_ADDR_OF_SRCMD(srcmd_en,    IOPMP_SRCMD_EN_BASE);
DECLARE_FUNC_GET_ADDR_OF_SRCMD(srcmd_enh,   IOPMP_SRCMD_ENH_BASE);
DECLARE_FUNC_GET_ADDR_OF_SRCMD(srcmd_perm,  IOPMP_SRCMD_PERM_BASE);
DECLARE_FUNC_GET_ADDR_OF_SRCMD(srcmd_permh, IOPMP_SRCMD_PERMH_BASE);
#ifdef ENABLE_SPS
DECLARE_FUNC_GET_ADDR_OF_SRCMD(srcmd_r,     IOPMP_SRCMD_R_BASE);
DECLARE_FUNC_GET_ADDR_OF_SRCMD(srcmd_rh,    IOPMP_SRCMD_RH_BASE);
DECLARE_FUNC_GET_ADDR_OF_SRCMD(srcmd_w,     IOPMP_SRCMD_W_BASE);
DECLARE_FUNC_GET_ADDR_OF_SRCMD(srcmd_wh,    IOPMP_SRCMD_WH_BASE);
#endif

/* Helper functions to read low 32-bit value from SRCMD registers */
#define DECLARE_FUNC_READ_SRCMD_L(name)                                 \
static uint32_t read_ ## name(IOPMP_t *iopmp, uint32_t idx)             \
{                                                                       \
    return io_read32(get_addr_of_##name(iopmp, idx));                   \
}
DECLARE_FUNC_READ_SRCMD_L(srcmd_en);
DECLARE_FUNC_READ_SRCMD_L(srcmd_perm);
#ifdef ENABLE_SPS
DECLARE_FUNC_READ_SRCMD_L(srcmd_r);
DECLARE_FUNC_READ_SRCMD_L(srcmd_w);
#endif

/* Helper functions to read high 32-bit value from SRCMD registers */
#define DECLARE_FUNC_READ_SRCMD_H(name, condition)                      \
static uint32_t read_ ## name(IOPMP_t *iopmp, uint32_t idx)             \
{                                                                       \
    if (condition)                                                      \
        return io_read32(get_addr_of_##name(iopmp, idx));               \
    return 0;                                                           \
}
DECLARE_FUNC_READ_SRCMD_H(srcmd_enh,   (iopmp->md_num > 31));
DECLARE_FUNC_READ_SRCMD_H(srcmd_permh, (iopmp->rrid_num > 16));
#ifdef ENABLE_SPS
DECLARE_FUNC_READ_SRCMD_H(srcmd_rh,    (iopmp->md_num > 31));
DECLARE_FUNC_READ_SRCMD_H(srcmd_wh,    (iopmp->md_num > 31));
#endif

/* Helper functions to read 64-bit value from SRCMD registers */
#define DECLARE_FUNC_READ_SRCMD_64(name)                                \
static uint64_t read_ ## name ## _64(IOPMP_t *iopmp, uint32_t idx)      \
{                                                                       \
    uint32_t val_##name    = read_##name(iopmp, idx);                   \
    uint32_t val_##name##h = read_##name##h(iopmp, idx);                \
    return ((uint64_t)val_##name##h << 32) | val_##name;                \
}
DECLARE_FUNC_READ_SRCMD_64(srcmd_en);
DECLARE_FUNC_READ_SRCMD_64(srcmd_perm);
#ifdef ENABLE_SPS
DECLARE_FUNC_READ_SRCMD_64(srcmd_r);
DECLARE_FUNC_READ_SRCMD_64(srcmd_w);
#endif

/* Helper functions to write low 32-bit value into SRCMD registers */
#define DECLARE_FUNC_WRITE_SRCMD_L(name)                                \
static void write_ ## name(IOPMP_t *iopmp, uint32_t idx, uint32_t val)  \
{                                                                       \
    io_write32(get_addr_of_##name(iopmp, idx), val);                    \
}
DECLARE_FUNC_WRITE_SRCMD_L(srcmd_en);
DECLARE_FUNC_WRITE_SRCMD_L(srcmd_perm);
#ifdef ENABLE_SPS
DECLARE_FUNC_WRITE_SRCMD_L(srcmd_r);
DECLARE_FUNC_WRITE_SRCMD_L(srcmd_w);
#endif

/* Helper functions to write high 32-bit value into SRCMD registers */
#define DECLARE_FUNC_WRITE_SRCMD_H(name, condition)                     \
static void write_ ## name(IOPMP_t *iopmp, uint32_t idx, uint32_t val)  \
{                                                                       \
    if (condition)                                                      \
        io_write32(get_addr_of_##name(iopmp, idx), val);                \
}
DECLARE_FUNC_WRITE_SRCMD_H(srcmd_enh,   (iopmp->md_num > 31));
DECLARE_FUNC_WRITE_SRCMD_H(srcmd_permh, (iopmp->rrid_num > 16));
#ifdef ENABLE_SPS
DECLARE_FUNC_WRITE_SRCMD_H(srcmd_rh,    (iopmp->md_num > 31));
DECLARE_FUNC_WRITE_SRCMD_H(srcmd_wh,    (iopmp->md_num > 31));
#endif

/* Helper functions to write 64-bit value into SRCMD registers */
#define DECLARE_FUNC_WRITE_SRCMD_64(name)                               \
static void write_ ## name ## _64(IOPMP_t *iopmp, uint32_t idx,         \
                                  uint64_t val)                         \
{                                                                       \
    write_##name##h(iopmp, idx, val >> 32);                             \
    write_##name(iopmp, idx, val & 0xFFFFFFFF);                         \
}
DECLARE_FUNC_WRITE_SRCMD_64(srcmd_en);
DECLARE_FUNC_WRITE_SRCMD_64(srcmd_perm);
#ifdef ENABLE_SPS
DECLARE_FUNC_WRITE_SRCMD_64(srcmd_r);
DECLARE_FUNC_WRITE_SRCMD_64(srcmd_w);
#endif

/* Helper functions to get base address of ENTRY registers */
#define DECLARE_FUNC_GET_ADDR_OF_ENTRY(name, base)                      \
static inline uintptr_t get_addr_of_ ## name(IOPMP_t *iopmp,            \
                                             uint32_t idx)              \
{                                                                       \
    return iopmp->addr_entry_array + base +	(idx * IOPMP_ENTRY_STRIDE); \
}
DECLARE_FUNC_GET_ADDR_OF_ENTRY(entry,       IOPMP_ENTRY_ADDR_BASE);
DECLARE_FUNC_GET_ADDR_OF_ENTRY(entry_addr,  IOPMP_ENTRY_ADDR_BASE);
DECLARE_FUNC_GET_ADDR_OF_ENTRY(entry_addrh, IOPMP_ENTRY_ADDRH_BASE);
DECLARE_FUNC_GET_ADDR_OF_ENTRY(entry_cfg,   IOPMP_ENTRY_CFG_BASE);

/******************************************************************************/
/* Internal functions to read/write value from/to IOPMP instance              */
/******************************************************************************/
static void write_hwcfg0(IOPMP_t *iopmp, uint32_t mask, uint32_t val)
{
    uint32_t hwcfg0;

    hwcfg0 = io_read32(iopmp->addr + IOPMP_HWCFG0_BASE);
    /* Clear W1C fields: prient_prog & rrid_transl_prog */
    hwcfg0 &= ~(IOPMP_HWCFG0_PRIENT_PROG_MASK |
                IOPMP_HWCFG0_RRID_TRANSL_PROG_MASK);
    hwcfg0 = (hwcfg0 & ~mask) | (val & mask);
    io_write32(iopmp->addr + IOPMP_HWCFG0_BASE, hwcfg0);
}

static void write_hwcfg2(IOPMP_t *iopmp, uint32_t mask, uint32_t val)
{
    uint32_t hwcfg2;

    hwcfg2 = io_read32(iopmp->addr + IOPMP_HWCFG2_BASE);
    hwcfg2 = (hwcfg2 & ~mask) | (val & mask);
    io_write32(iopmp->addr + IOPMP_HWCFG2_BASE, hwcfg2);
}

static void write_err_cfg(IOPMP_t *iopmp, uint32_t mask, uint32_t val)
{
    uint32_t err_cfg;

    err_cfg = io_read32(iopmp->addr + IOPMP_ERR_CFG_BASE);
    err_cfg = (err_cfg & ~mask) | (val & mask);
    io_write32(iopmp->addr + IOPMP_ERR_CFG_BASE, err_cfg);
}

static void detect_stall_function(IOPMP_t *iopmp)
{
    /*
     * HWCFG0.stall_en indicates if the IOPMP implements stall-related features,
     * which are MDSTALL, MDSTALLH, and RRIDSCP registers.
     */
    if (iopmp->stall_en) {
        iopmp->support_stall_by_md = true;
        /*
         * If RRIDSCP is not implemented, it always returns zero. One can test
         * if it is implemented by writing a zero and then reading it back. Any
         * IOPMP implementing RRIDSCP should not return a zero in RRIDSCP.stat
         * in this case.
         */
        io_write32(iopmp->addr + IOPMP_RRIDSCP_BASE, 0);
        if (io_read32(iopmp->addr + IOPMP_RRIDSCP_BASE))
            iopmp->support_stall_by_rrid = true;
        else
            iopmp->support_stall_by_rrid = false;
    } else {
        iopmp->support_stall_by_md = false;
        iopmp->support_stall_by_rrid = false;
    }
}

void detect_entry_addr_bits(IOPMP_t *iopmp)
{
    uint64_t val;
    uintptr_t addr_entry_0 = iopmp->addr_entry_array;

    io_write32(addr_entry_0 + IOPMP_ENTRY_CFG_BASE, 0);
    io_write32(addr_entry_0 + IOPMP_ENTRY_ADDR_BASE, 0xFFFFFFFF);
    val = io_read32(addr_entry_0 + IOPMP_ENTRY_ADDR_BASE);
    io_write32(addr_entry_0 + IOPMP_ENTRY_ADDR_BASE, 0);    /* Clear */
    if (iopmp->addrh_en) {
        io_write32(addr_entry_0 + IOPMP_ENTRY_ADDRH_BASE, 0xFFFFFFFF);
        val |= (uint64_t)io_read32(addr_entry_0 + IOPMP_ENTRY_ADDRH_BASE) << 32;
        io_write32(addr_entry_0 + IOPMP_ENTRY_ADDRH_BASE, 0);   /* Clear */
    }

    iopmp->entry_addr_bits = val;

    /*
     * From Privileged Architecture Spec.:
     * Software may determine the PMP granularity by writing zero to pmp0cfg,
     * then writing all ones to pmpaddr0, then reading back pmpaddr0. If G is
     * the index of the least significant bit set, the PMP granularity is
     * 2^(G+2) bytes.
     */
    iopmp->granularity = (uint32_t)1 << (iopmp_ctzll(val) + 2);
}

/******************************************************************************/
/* Implementation of libiopmp APIs                                            */
/******************************************************************************/
/**
 * \brief Set the IOPMP HWCFG0.enable
 *
 * \param[in] iopmp             The IOPMP instance to be set
 */
static void generic_enable(IOPMP_t *iopmp)
{
    write_hwcfg0(iopmp, IOPMP_HWCFG0_ENABLE_MASK, IOPMP_HWCFG0_ENABLE_MASK);
}

/**
 * \brief Lock number of priority entry
 *
 * \param[in] iopmp             The IOPMP instance
 */
static void generic_lock_prio_entry_num(IOPMP_t *iopmp)
{
    write_hwcfg0(iopmp, IOPMP_HWCFG0_PRIENT_PROG_MASK,
                 IOPMP_HWCFG0_PRIENT_PROG_MASK);
}

/**
 * \brief Lock the RRID tagged to outgoing transactions
 *
 * \param[in] iopmp             The IOPMP instance
 */
static void generic_lock_rrid_transl(IOPMP_t *iopmp)
{
    write_hwcfg0(iopmp, IOPMP_HWCFG0_RRID_TRANSL_PROG_MASK,
                 IOPMP_HWCFG0_RRID_TRANSL_PROG_MASK);
}

/**
 * \brief Set IOPMP HWCFG2.prio_entry
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] num_entry     Input the number of entries to be matched with
 *                              priority. Output WARL value.
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p num_entry does not match
 *         the actual value. The actual value is output via \p num_entry
 */
static enum iopmp_error generic_set_prio_entry_num(IOPMP_t *iopmp,
                                                   uint16_t *num_entry)
{
    uint16_t __prio_entry = *num_entry;
    uint32_t hwcfg2;

    write_hwcfg2(iopmp, IOPMP_HWCFG2_PRIO_ENTRY_MASK,
                 (__prio_entry << IOPMP_HWCFG2_PRIO_ENTRY_SHIFT));

    /* HWCFG2.prio_entry is WARL field. Read it back to check the value */
    hwcfg2 = io_read32(iopmp->addr + IOPMP_HWCFG2_BASE);
    *num_entry = EXTRACT_FIELD(hwcfg2, IOPMP_HWCFG2_PRIO_ENTRY);

    return (__prio_entry == *num_entry) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Set IOPMP HWCFG2.rrid_transl
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] rrid_transl   Input the value of rrid_transl to be set. Output
 *                              WARL value
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p rrid_transl does not
 *         match the actual value. The actual value is output via \p rrid_transl
 */
static enum iopmp_error generic_set_rrid_transl(IOPMP_t *iopmp,
                                                uint16_t *rrid_transl)
{
    uint16_t __rrid_transl = *rrid_transl;
    uint32_t hwcfg2;

    write_hwcfg2(iopmp, IOPMP_HWCFG2_RRID_TRANSL_MASK,
                 (__rrid_transl << IOPMP_HWCFG2_RRID_TRANSL_SHIFT));

    /* HWCFG2.rrid_transl is WARL field. Read it back to check the value */
    hwcfg2 = io_read32(iopmp->addr + IOPMP_HWCFG2_BASE);
    *rrid_transl = EXTRACT_FIELD(hwcfg2, IOPMP_HWCFG2_RRID_TRANSL);

    return (__rrid_transl == *rrid_transl) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Clear MDSTALL to resume the stalled transactions
 *
 * \param[in] iopmp             The IOPMP instance to be resumed
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p mds does not match the
 *         actual value
 */
static enum iopmp_error __resume_transactions(IOPMP_t *iopmp)
{
    uint32_t rb_mdstallh, rb_mdstall_md;

    if (iopmp->md_num > 31)
        io_write32(iopmp->addr + IOPMP_MDSTALLH_BASE, 0);
    io_write32(iopmp->addr + IOPMP_MDSTALL_BASE, 0);
    /*
     * MDSTALL.md and MDSTALLH.mdh are WARL fields. Read them back to check the
     * values.
     */
    if (iopmp->md_num > 31)
        rb_mdstallh = io_read32(iopmp->addr + IOPMP_MDSTALLH_BASE);
    else
        rb_mdstallh = 0;
    rb_mdstall_md = io_read32(iopmp->addr + IOPMP_MDSTALL_BASE);
    rb_mdstall_md = EXTRACT_FIELD(rb_mdstall_md, IOPMP_MDSTALL_MD);
    if (rb_mdstallh != 0 || rb_mdstall_md != 0)
        return IOPMP_ERR_ILLEGAL_VALUE;

    return IOPMP_OK;
}

/**
 * \brief Set MDSTALL to stall the transactions related to MDs bitmap
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] mds           Input the MD bitmap to be stalled. Output WARL
 *                              value
 * \param[in] exempt            Stall transactions with exempt selected MDs
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p mds does not match the
 *         actual value. The actual value is output via \p mds
 */
static enum iopmp_error __stall_by_mds_common(IOPMP_t *iopmp, uint64_t *mds,
                                              bool exempt)
{
    uint64_t __mds = *mds;
    uint32_t mdstall, mdstall_md, mdstallh;
    uint32_t rb_mdstall_md, rb_mdstallh;

    mdstallh = __mds >> 31;           /* mds[62:31] */
    mdstall_md = __mds & 0x7FFFFFFF;  /* mds[30: 0] */
    mdstall = MAKE_FIELD_32(mdstall_md, IOPMP_MDSTALL_MD) |
              MAKE_FIELD_32(exempt, IOPMP_MDSTALL_EXEMPT);
    /* Write MD_STALLH first then MD_STALL to take effect. */
    if (mdstallh)
        io_write32(iopmp->addr + IOPMP_MDSTALLH_BASE, mdstallh);
    io_write32(iopmp->addr + IOPMP_MDSTALL_BASE, mdstall);

    /*
     * MDSTALL.md and MDSTALLH.mdh are WARL fields. Read them back to check the
     * values.
     */
    if (mdstallh)
        rb_mdstallh = io_read32(iopmp->addr + IOPMP_MDSTALLH_BASE);
    else
        rb_mdstallh = 0;
    rb_mdstall_md = io_read32(iopmp->addr + IOPMP_MDSTALL_BASE);
    rb_mdstall_md = EXTRACT_FIELD(rb_mdstall_md, IOPMP_MDSTALL_MD);
    *mds = ((uint64_t)rb_mdstallh << 31) | rb_mdstall_md;
    if (rb_mdstallh != mdstallh || rb_mdstall_md != mdstall_md) {
        /* Resume transactions due to error */
        __resume_transactions(iopmp);
        return IOPMP_ERR_ILLEGAL_VALUE;
    }

    return IOPMP_OK;
}

/**
 * \brief Poll MDSTALL.is_busy until its value is zero
 */
static void __polling_mdstall(IOPMP_t *iopmp)
{
    uint32_t mdstall;

    do {
        mdstall = io_read32(iopmp->addr + IOPMP_MDSTALL_BASE);
    } while(EXTRACT_FIELD(mdstall, IOPMP_MDSTALL_IS_BUSY));
}

/**
 * \brief Set MDSTALL to stall the transactions related to MDs bitmap, and poll
 * the stall status until stall takes effect if necessary
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] mds           Input the MD bitmap to be stalled. Output WARL
 *                              value
 * \param[in] exempt            Stall transactions with exempt selected MDs
 * \param[in] polling           Set true to poll the stall status until stalling
 *                              takes effect
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p mds does not match the
 *         actual value. The actual value is output via \p mds
 */
static enum iopmp_error generic_stall_by_mds(IOPMP_t *iopmp, uint64_t *mds,
                                             bool exempt, bool polling)
{
    enum iopmp_error ret;

    ret = __stall_by_mds_common(iopmp, mds, exempt);
    if (ret != IOPMP_OK)
        return ret;

    if (polling)
        __polling_mdstall(iopmp);

    return IOPMP_OK;
}

/**
 * \brief Resume the stalled transactions previously stalled, and poll the
 * resume status until resuming takes effect if necessary
 *
 * \param[in] iopmp             The IOPMP instance to be resumed
 * \param[in] polling           Set true to poll the resume status until
 *                              resuming takes effect
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p mds does not match the
 *         actual value
 */
static enum iopmp_error generic_resume_transactions(IOPMP_t *iopmp,
                                                    bool polling)
{
    enum iopmp_error ret;

    ret = __resume_transactions(iopmp);
    if (ret != IOPMP_OK)
        return ret;

    if (polling)
        __polling_mdstall(iopmp);

    return IOPMP_OK;
}

/**
 * \brief Poll until MDSTALL.is_busy == 0
 *
 * \param[in] iopmp             The IOPMP instance to be checked
 * \param[in] polling           Set true to poll the status until takes effect
 * \param[in] stall_or_resume   Set true to poll for stall status or set false
 *                              to poll for resume status
 *
 * \retval 1 if the previous operation has taken effect
 * \retval 0 if the previous operation has not taken effect yet
 */
static bool generic_poll_mdstall(IOPMP_t *iopmp, bool polling,
                                 bool stall_or_resume)
{
    uint32_t mdstall;

    if (polling) {
        __polling_mdstall(iopmp);
        return true;
    }

    mdstall = io_read32(iopmp->addr + IOPMP_MDSTALL_BASE);
    return EXTRACT_FIELD(mdstall, IOPMP_MDSTALL_IS_BUSY) == false;
}

/**
 * \brief Write RRIDSCP with given RRID and operation
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] rrid          Input the RRID to be stalled. Output WARL value
 * \param[in] op                The operation of RRIDSCP
 * \param[out] stat             The pointer to store enum iopmp_rridscp_stat
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p rrid does not match the
 *         actual value. The actual value is output via \p rrid
 */
static enum iopmp_error generic_set_rridscp(IOPMP_t *iopmp, uint32_t *rrid,
                                            enum iopmp_rridscp_op op,
                                            enum iopmp_rridscp_stat *stat)
{
    uint32_t rridscp;
    uint32_t __rrid = *rrid;

    rridscp  = MAKE_FIELD_32(__rrid, IOPMP_RRIDSCP_RRID);
    rridscp |= MAKE_FIELD_32(op, IOPMP_RRIDSCP_OP);

    /* Write to query. */
    io_write32(iopmp->addr + IOPMP_RRIDSCP_BASE, rridscp);
    /* RRIDSCP.rrid is WARL field. Read it back to check the value */
    rridscp = io_read32(iopmp->addr + IOPMP_RRIDSCP_BASE);
    *rrid = EXTRACT_FIELD(rridscp, IOPMP_RRIDSCP_RRID);
    *stat = EXTRACT_FIELD(rridscp, IOPMP_RRIDSCP_STAT);

    return (__rrid == *rrid) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Lock ENTRY(0) ~ ENTRY(entry_num - 1)
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] entry_num     Input the number of entry to be locked. Output
 *                              WARL value
 * \param[in] lock              Lock ENTRYLCK register or not
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p entry_num does not match
 *         the actual value. The actual value is output via \p entry_num
 */
static enum iopmp_error generic_lock_entries(IOPMP_t *iopmp,
                                             uint32_t *entry_num,
                                             bool lock)
{
    uint32_t entrylck;
    uint32_t __entry_num = *entry_num;

    entrylck = MAKE_FIELD_32(lock, IOPMP_ENTRYLCK_L) |
               MAKE_FIELD_32(__entry_num, IOPMP_ENTRYLCK_F);
    io_write32(iopmp->addr + IOPMP_ENTRYLCK_BASE, entrylck);

    /* ENTRYLCK.f is WARL. Read the value back to check */
    entrylck = io_read32(iopmp->addr + IOPMP_ENTRYLCK_BASE);
    *entry_num = EXTRACT_FIELD(entrylck, IOPMP_ENTRYLCK_F);

    return __entry_num == *entry_num ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Set IOPMP ERR_CFG.l to lock ERR_CFG register
 *
 * \param[in] iopmp             The IOPMP instance to be set
 */
static void generic_lock_err_cfg(IOPMP_t *iopmp)
{
    write_err_cfg(iopmp, IOPMP_ERR_CFG_L_MASK, IOPMP_ERR_CFG_L_MASK);
}

/**
 * \brief Set IOPMP ERR_CFG.ie to enable/disable IOPMP global interrupt
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in] enable            True to enable or false to disable
 */
static void generic_set_global_intr(IOPMP_t *iopmp, bool enable)
{
    write_err_cfg(iopmp, IOPMP_ERR_CFG_IE_MASK,
                  (enable << IOPMP_ERR_CFG_IE_SHIFT));
}

/**
 * \brief Set IOPMP ERR_CFG.rs to suppress/express error response
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] suppress      True to suppress or false to express
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p suppress does not match
 *         the actual value. The actual value is output via \p suppress
 */
static enum iopmp_error generic_set_global_err_resp(IOPMP_t *iopmp,
                                                    bool *suppress)
{
    uint32_t err_cfg;
    bool __suppress = *suppress;

    write_err_cfg(iopmp, IOPMP_ERR_CFG_RS_MASK,
                  (*suppress << IOPMP_ERR_CFG_RS_SHIFT));
    /* ERR_CFG.rs is WARL. Read it back to check the value */
    err_cfg = io_read32(iopmp->addr + IOPMP_ERR_CFG_BASE);
    *suppress = EXTRACT_FIELD(err_cfg, IOPMP_ERR_CFG_RS);

    return __suppress == *suppress ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Set IOPMP message-signaled interrupts (MSI) enable/disable
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] enable        True to enable or false to disable
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if \p enable can not be written into \p iopmp
 */
static enum iopmp_error generic_set_msi_en(IOPMP_t *iopmp, bool *enable)
{
    uint32_t err_cfg;
    bool __enable = *enable;

    write_err_cfg(iopmp, IOPMP_ERR_CFG_MSI_EN_MASK,
                  (__enable << IOPMP_ERR_CFG_MSI_EN_SHIFT));
    /* ERR_CFG.msi_en is WARL. Read it back to check the value */
    err_cfg = io_read32(iopmp->addr + IOPMP_ERR_CFG_BASE);
    *enable = EXTRACT_FIELD(err_cfg, IOPMP_ERR_CFG_MSI_EN);

    return __enable == *enable ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Set IOPMP message-signaled interrupts (MSI) information
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] msiaddr64     Input 64-bit MSI address. Output WARL value
 * \param[in,out] msidata       Input 11-bit MSI data. Output WARL value
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if \p msiaddr64 or \p msidata can not be
 *         written into \p iopmp
 */
static enum iopmp_error generic_set_msi_info(IOPMP_t *iopmp,
                                             uint64_t *msiaddr64,
                                             uint16_t *msidata)
{
    uint32_t err_cfg, err_msiaddr, err_msiaddrh;
    uint32_t rb_err_msiaddr, rb_err_msiaddrh;
    uint64_t __msiaddr64 = *msiaddr64;
    uint16_t __msidata = *msidata;

    write_err_cfg(iopmp, IOPMP_ERR_CFG_MSIDATA_MASK,
                  (__msidata << IOPMP_ERR_CFG_MSIDATA_SHIFT));
    /* ERR_CFG.msidata is WARL field. Read it back to check the value */
    err_cfg = io_read32(iopmp->addr + IOPMP_ERR_CFG_BASE);
    *msidata = EXTRACT_FIELD(err_cfg, IOPMP_ERR_CFG_MSIDATA);

    if (!iopmp->addrh_en) {
        /* Write bits 33 to 2 of the address into ERR_MSIADDR */
        err_msiaddr = __msiaddr64 >> 2;
        io_write32(iopmp->addr + IOPMP_ERR_MSIADDR_BASE, err_msiaddr);
        /* ERR_MSIADDR.msiaddr is WARL field. Read it back to check the value */
        *msiaddr64 = io_read32(iopmp->addr + IOPMP_ERR_MSIADDR_BASE) << 2;
    } else {
        /* Write bits 31 to 0 of the address into ERR_MSIADDR */
        err_msiaddr = __msiaddr64 & UINT32_MAX;
        io_write32(iopmp->addr + IOPMP_ERR_MSIADDR_BASE, err_msiaddr);
        /* Write bits 63 to 32 of the address into ERR_MSIADDRH */
        err_msiaddrh = __msiaddr64 >> 32;
        io_write32(iopmp->addr + IOPMP_ERR_MSIADDRH_BASE, err_msiaddrh);

        /*
         * ERR_MSIADDR.msiaddr abd ERR_MSIADDRH.msiaddrh are WARL fields.
         * Read them back to check the values.
         */
        rb_err_msiaddrh = io_read32(iopmp->addr + IOPMP_ERR_MSIADDRH_BASE);
        rb_err_msiaddr = io_read32(iopmp->addr + IOPMP_ERR_MSIADDR_BASE);
        *msiaddr64 = reg_pair_to_64(rb_err_msiaddrh, rb_err_msiaddr);
    }

    return (__msidata == *msidata) && (__msiaddr64 == *msiaddr64) ?
           IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Check if there is an MSI write error and clear the flag
 *
 * \param[in] iopmp             The IOPMP instance to be checked
 * \param[out] msi_werr         The pointer to flag
 */
static void generic_get_and_clear_msi_werr(IOPMP_t *iopmp, bool *msi_werr)
{
    uint32_t err_info;

    err_info = io_read32(iopmp->addr + IOPMP_ERR_INFO_BASE);
    *msi_werr = EXTRACT_FIELD(err_info, IOPMP_ERR_INFO_MSI_WERR);

    /* ERR_INFO.msi_werr is W1C */
    io_write32(iopmp->addr + IOPMP_ERR_INFO_BASE, IOPMP_ERR_INFO_MSI_WERR_MASK);
}

/**
 * \brief Set IOPMP ERR_CFG.stall_violation_en
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] enable        Input 1 to enable, 0 to disable. Output WARL
 *                              value
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if \p enable can't be written into \p iopmp
 */
static
enum iopmp_error generic_set_stall_violation_en(IOPMP_t *iopmp, bool *enable)
{
    uint32_t err_cfg;
    bool __enable = *enable;

    write_err_cfg(iopmp, IOPMP_ERR_CFG_STALL_VIO_EN_MASK,
                  (*enable << IOPMP_ERR_CFG_STALL_VIO_EN_SHIFT));
    /* ERR_CFG.stall_violation_en is WARL. Read it back to check the value */
    err_cfg = io_read32(iopmp->addr + IOPMP_ERR_CFG_BASE);
    *enable = EXTRACT_FIELD(err_cfg, IOPMP_ERR_CFG_STALL_VIO_EN);

    return __enable == *enable ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Invalidate the error record by clearing ERR_INFO.v bit
 *
 * \param[in] iopmp             The IOPMP instance to be invalidated
 */
static void generic_invalidate_error(IOPMP_t *iopmp)
{
    /* Only ERR_INFO.ip is writable. Write 1 clear */
    io_write32(iopmp->addr + IOPMP_ERR_INFO_BASE, IOPMP_ERR_INFO_V_MASK);
}

/**
 * \brief Capture latest IOPMP error report
 *
 * \param[in] iopmp             The IOPMP instance to be captured
 * \param[out] err_report       The pointer to IOPMP error report structure
 * \param[in] invalidate        Flag to clear V bit after reading error report
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_NOT_EXIST if there is no an pending error
 */
static enum iopmp_error generic_capture_error(IOPMP_t *iopmp,
                                              IOPMP_ERR_REPORT_t *err_report,
                                              bool invalidate)
{
    uint32_t err_reqaddr, err_reqaddrh, err_reqid, err_info;

    /* Check ERR_INFO.v first */
    err_info = io_read32(iopmp->addr + IOPMP_ERR_INFO_BASE);
    if ((err_info & IOPMP_ERR_INFO_V_MASK) == 0)
        return IOPMP_ERR_NOT_EXIST; /* No pending error */

    /* Read ERR_REQADDR, ERR_REQADDRH, and ERR_REQID from IOPMP */
    err_reqaddr  = io_read32(iopmp->addr + IOPMP_ERR_REQADDR_BASE);
    if (iopmp->addrh_en)
        err_reqaddrh = io_read32(iopmp->addr + IOPMP_ERR_REQADDRH_BASE);
    else
        err_reqaddrh = 0;
    /* Read ERR_REQID from IOPMP */
    err_reqid = io_read32(iopmp->addr + IOPMP_ERR_REQID_BASE);

    /* Record into given structure */
    err_report->addr = reg_pair_to_64(err_reqaddrh, err_reqaddr);
    err_report->rrid = EXTRACT_FIELD(err_reqid, IOPMP_ERR_REQID_RRID);
    err_report->eid = EXTRACT_FIELD(err_reqid, IOPMP_ERR_REQID_EID);
    err_report->ttype = EXTRACT_FIELD(err_info, IOPMP_ERR_INFO_TTYPE);
    err_report->msi_werr = EXTRACT_FIELD(err_info, IOPMP_ERR_INFO_MSI_WERR);
    err_report->etype = EXTRACT_FIELD(err_info, IOPMP_ERR_INFO_ETYPE);
    err_report->svc = EXTRACT_FIELD(err_info, IOPMP_ERR_INFO_SVC);

    if (invalidate)
        generic_invalidate_error(iopmp);

    return IOPMP_OK;
}

/**
 * \brief Get subsequent violation window
 *
 * \param[in] iopmp             The IOPMP instance to be allocated
 * \param[in,out] svi           When calling, user can specify start index of
 *                              search windows. When this function returns with
 *                              IOPMP_OK, svi indicates the index of window
 *                              which has subsequent violation
 * \param[out] svw              When this function returns with IOPMP_OK, svw
 *                              indicates the content of window which has
 *                              subsequent violation
 *
 * \retval IOPMP_OK if at least one subsequent violation is found
 * \retval IOPMP_ERR_NOT_EXIST if there is no any subsequent violation
 *
 * \note Expected to be called after iopmp_capture_error() to get ERR_INFO.svc
 */
static enum iopmp_error generic_get_sv_window(IOPMP_t *iopmp, uint16_t *svi,
                                              uint16_t *svw)
{
    uint32_t err_info;
    uint32_t err_mfr;
    bool svc;

    err_info = io_read32(iopmp->addr + IOPMP_ERR_INFO_BASE);
    svc = EXTRACT_FIELD(err_info, IOPMP_ERR_INFO_SVC);
    if (!svc)
        return IOPMP_ERR_NOT_EXIST;

    /* Set original position */
    err_mfr = MAKE_FIELD_32(*svi, IOPMP_ERR_MFR_SVI);
    io_write32(iopmp->addr + IOPMP_ERR_MFR_BASE, err_mfr);

    err_mfr = io_read32(iopmp->addr + IOPMP_ERR_MFR_BASE);
    if (err_mfr & IOPMP_ERR_MFR_SVS_MASK) {
        /* Subsequent violation found */
        *svi = EXTRACT_FIELD(err_mfr, IOPMP_ERR_MFR_SVI);
        *svw = EXTRACT_FIELD(err_mfr, IOPMP_ERR_MFR_SVW);
        return IOPMP_OK;
    }

    return IOPMP_ERR_NOT_EXIST;
}

enum iopmp_error generic_set_entries(IOPMP_t *iopmp,
                                     const struct iopmp_entry *entry_array,
                                     uint32_t idx_start, uint32_t num_entry)
{
    uintptr_t e = get_addr_of_entry(iopmp, idx_start);
    int i;

    for (i = 0; i < num_entry; i++) {
        io_write32(e + IOPMP_ENTRY_CFG_BASE, 0);
        io_write32(e + IOPMP_ENTRY_ADDR_BASE, entry_array[i].addr & UINT32_MAX);
        if (iopmp->addrh_en)
            io_write32(e + IOPMP_ENTRY_ADDRH_BASE, entry_array[i].addr >> 32);
        io_write32(e + IOPMP_ENTRY_CFG_BASE, entry_array[i].cfg);

        e += IOPMP_ENTRY_STRIDE;
    }

    return IOPMP_OK;
}

void generic_get_entries(IOPMP_t *iopmp, struct iopmp_entry *entry_array,
                         uint32_t idx_start, uint32_t num_entry)
{
    uintptr_t e = get_addr_of_entry(iopmp, idx_start);
    uint32_t addr, addrh = 0, cfg;
    int i;

    for (i = 0; i < num_entry; i++) {
        /* Read ENTRY_ADDR(idx), ENTRY_ADDRH(idx), and ENTRY_CFG(idx) from IP */
        if (iopmp->addrh_en)
            addrh = io_read32(e + IOPMP_ENTRY_ADDRH_BASE);
        addr = io_read32(e + IOPMP_ENTRY_ADDR_BASE);
        cfg  = io_read32(e + IOPMP_ENTRY_CFG_BASE);
        /* Store into data structure */
        entry_array[i].addr = reg_pair_to_64(addrh, addr);
        entry_array[i].cfg = cfg;

        e += IOPMP_ENTRY_STRIDE;
    }
}

void generic_clear_entries(IOPMP_t *iopmp, uint32_t idx_start,
                           uint32_t num_entry)
{
    uintptr_t e = get_addr_of_entry(iopmp, idx_start);
    int i;

    for (i = 0; i < num_entry; i++) {
        io_write32(e + IOPMP_ENTRY_CFG_BASE, 0);
        io_write32(e + IOPMP_ENTRY_ADDR_BASE, 0);
        if (iopmp->addrh_en)
            io_write32(e + IOPMP_ENTRY_ADDRH_BASE, 0);

        e += IOPMP_ENTRY_STRIDE;
    }
}

/******************************************************************************/
/* Functions specific to some models                                          */
/******************************************************************************/
void srcmd_fmt_0_get_association_rrid_md(IOPMP_t *iopmp, uint32_t rrid,
                                         uint64_t *mds, bool *lock)
{
    uint64_t srcmd_en_64 = read_srcmd_en_64(iopmp, rrid);

    *mds = srcmd_en_64 >> IOPMP_SRCMD_EN_MD_SHIFT;
    *lock = EXTRACT_FIELD(srcmd_en_64, IOPMP_SRCMD_EN_L);
}

enum iopmp_error srcmd_fmt_0_set_association_rrid_md(IOPMP_t *iopmp,
                                                     uint32_t rrid,
                                                     uint64_t *mds,
                                                     bool lock)
{
    uint64_t srcmd_en_64;
    uint64_t __mds = *mds;

    srcmd_en_64 = MAKE_FIELD_64(lock, IOPMP_SRCMD_EN_L) |
                  (__mds << IOPMP_SRCMD_EN_MD_SHIFT);
    write_srcmd_en_64(iopmp, rrid, srcmd_en_64);

    /* SRCMD_EN.md and SRCMD_ENH.mdh are WARL. Read them back to check. */
    srcmd_en_64 = read_srcmd_en_64(iopmp, rrid);
    *mds = srcmd_en_64 >> IOPMP_SRCMD_EN_MD_SHIFT;

    return (*mds == __mds) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

enum iopmp_error srcmd_fmt_0_2_set_md_lock(IOPMP_t *iopmp, uint64_t *mds,
                                           bool lock_mdlck)
{
    uint32_t mdlck, mdlckh;
    uint64_t mdlck_64;
    uint64_t __mds = *mds;

    /* Set bits of new MDs and MDLCK.l */
    mdlck_64 = (__mds << IOPMP_MDLCK_MD_SHIFT);
    if (lock_mdlck)
        mdlck_64 |= IOPMP_MDLCK_L_MASK;
    mdlckh = mdlck_64 >> 32;
    mdlck  = mdlck_64 & UINT32_MAX;

    /* Write MDLCKH first */
    if (mdlckh) {
        io_write32(iopmp->addr + IOPMP_MDLCKH_BASE, mdlckh);
        /* MDLCKH.mdh is WARL. Read the value back to check */
        mdlckh = io_read32(iopmp->addr + IOPMP_MDLCKH_BASE);
    }
    /* Write MDLCK */
    io_write32(iopmp->addr + IOPMP_MDLCK_BASE, mdlck);
    /* MDLCK is WARL. Read the value back to check */
    mdlck = io_read32(iopmp->addr + IOPMP_MDLCK_BASE);

    mdlck_64 = reg_pair_to_64(mdlckh, mdlck);
    *mds = mdlck_64 >> IOPMP_MDLCK_MD_SHIFT;

    return ((__mds & *mds) == __mds) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

enum iopmp_error srcmd_fmt_0_lock_srcmd_table(IOPMP_t *iopmp, uint32_t rrid,
                                              uint32_t mdidx)
{
    uint64_t srcmd_en_64;

    srcmd_en_64  = read_srcmd_en_64(iopmp, rrid);
    srcmd_en_64 |= MAKE_FIELD_64(1, IOPMP_SRCMD_EN_L);
    write_srcmd_en_64(iopmp, rrid, srcmd_en_64);

    return IOPMP_OK;
}

enum iopmp_error srcmd_fmt_2_lock_srcmd_table(IOPMP_t *iopmp, uint32_t rrid,
                                              uint32_t mdidx)
{
    uint64_t mds = (uint64_t)1 << mdidx;

    return srcmd_fmt_0_2_set_md_lock(iopmp, &mds, false);
}

enum iopmp_error mdcfg_fmt_0_lock_mdcfg(IOPMP_t *iopmp, uint32_t *md_num,
                                        bool lock)
{
    uint32_t mdcfglck;
    uint64_t __md_num = *md_num;

    mdcfglck = MAKE_FIELD_32(lock, IOPMP_MDCFGLCK_L) |
               MAKE_FIELD_32(__md_num, IOPMP_MDCFGLCK_F);
    io_write32(iopmp->addr + IOPMP_MDCFGLCK_BASE, mdcfglck);
    /* MDCFGLCK.f is WARL field. Read the value back to check */
    mdcfglck = io_read32(iopmp->addr + IOPMP_MDCFGLCK_BASE);
    *md_num = EXTRACT_FIELD(mdcfglck, IOPMP_MDCFGLCK_F);

    return (__md_num == *md_num) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

void mdcfg_fmt_0_get_md_entry_top(IOPMP_t *iopmp, uint32_t mdidx,
                                  uint32_t *entry_top)
{
    uint32_t mdcfg;

    mdcfg = io_read32(get_addr_of_mdcfg(iopmp, mdidx));
    *entry_top = EXTRACT_FIELD(mdcfg, IOPMP_MDCFG_T);
}

void mdcfg_fmt_1_2_get_md_entry_top(IOPMP_t *iopmp, uint32_t mdidx,
                                    uint32_t *entry_top)
{
    *entry_top = (iopmp->md_entry_num + 1) * (mdidx + 1);
}

enum iopmp_error mdcfg_fmt_0_set_md_entry_top(IOPMP_t *iopmp, uint32_t mdidx,
                                              uint32_t *entry_top)
{
    uint32_t __entry_top = *entry_top;
    uintptr_t addr_mdcfg;
    uint32_t mdcfg;

    addr_mdcfg = get_addr_of_mdcfg(iopmp, mdidx);
    mdcfg = MAKE_FIELD_32(__entry_top, IOPMP_MDCFG_T);
    io_write32(addr_mdcfg, mdcfg);
    /* MDCFG.t is WARL field. Read it back to check it */
    mdcfg = io_read32(addr_mdcfg);
    *entry_top = EXTRACT_FIELD(mdcfg, IOPMP_MDCFG_T);

    return (__entry_top == *entry_top) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

enum iopmp_error mdcfg_fmt_2_set_md_entry_num(IOPMP_t *iopmp,
                                              uint32_t *md_entry_num)
{
    uint32_t __md_entry_num = *md_entry_num;
    uint32_t hwcfg0;

    hwcfg0 = MAKE_FIELD_32(__md_entry_num, IOPMP_HWCFG0_MD_ENTRY_NUM);
    write_hwcfg0(iopmp, IOPMP_HWCFG0_MD_ENTRY_NUM_MASK, hwcfg0);
    /* HWCFG0.md_entry_num is WARL field. Read it back to check it */
    hwcfg0 = io_read32(iopmp->addr + IOPMP_HWCFG0_BASE);
    *md_entry_num = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_MD_ENTRY_NUM);

    return (__md_entry_num == *md_entry_num) ? IOPMP_OK :
                                               IOPMP_ERR_ILLEGAL_VALUE;
}

enum iopmp_error srcmd_fmt_2_set_md_permission(IOPMP_t *iopmp, uint32_t rrid,
                                               uint32_t mdidx, bool *r, bool *w)
{
    uint32_t srcmd_perm, shift, mask, val;
    bool __r = *r, __w = *w;

    val  = ((uint32_t)__r << 0);
    val |= ((uint32_t)__w << 1);
    /* Set SRCMD_PERM if RRID < 16, otherwise set SRCMD_PERMH */
    if (rrid < 16) {
        srcmd_perm = read_srcmd_perm(iopmp, mdidx);
        shift = rrid << 1;
        mask = IOPMP_SRCMD_PERM_MASK << shift;
        val  = val << shift;
        srcmd_perm = (srcmd_perm & ~mask) | (val & mask);
        write_srcmd_perm(iopmp, mdidx, srcmd_perm);
        /* SRCMD_PERM.perm is WARL field. Read it back to check value */
        srcmd_perm = read_srcmd_perm(iopmp, mdidx);
        *r = ((srcmd_perm & mask) >> shift) & 0b1;
        *w = ((srcmd_perm & mask) >> (shift + 1)) & 0b1;
        if (__r != *r || __w != *w)
            return IOPMP_ERR_ILLEGAL_VALUE;
    } else {
        srcmd_perm = read_srcmd_permh(iopmp, mdidx);
        shift = (rrid - 16) << 1;
        mask = IOPMP_SRCMD_PERM_MASK << shift;
        val  = val << shift;
        srcmd_perm = (srcmd_perm & ~mask) | (val & mask);
        write_srcmd_permh(iopmp, mdidx, srcmd_perm);
        /* SRCMD_PERMH.permh is WARL field. Read it back to check value */
        srcmd_perm = read_srcmd_permh(iopmp, mdidx);
        *r = ((srcmd_perm & mask) >> shift) & 0b1;
        *w = ((srcmd_perm & mask) >> (shift + 1)) & 0b1;
        if (__r != *r || __w != *w)
            return IOPMP_ERR_ILLEGAL_VALUE;
    }

    return IOPMP_OK;
}

enum iopmp_error
srcmd_fmt_2_set_md_permission_multi(IOPMP_t *iopmp, uint32_t mdidx,
                                    IOPMP_SRCMD_PERM_CFG_t *cfg)
{
    uint64_t srcmd_perm_64, rb_srcmd_perm_64, mask, val;

    srcmd_perm_64 = read_srcmd_perm_64(iopmp, mdidx);
    mask = cfg->srcmd_perm_mask;
    val  = cfg->srcmd_perm_val;
    srcmd_perm_64 = (srcmd_perm_64 & ~mask) | (val & mask);
    write_srcmd_perm_64(iopmp, mdidx, srcmd_perm_64);

    /* SRCMD_PERM(H).perm is WARL field. Read it back to check value */
    rb_srcmd_perm_64 = read_srcmd_perm_64(iopmp, mdidx);
    if (rb_srcmd_perm_64 != srcmd_perm_64) {
        /* Set the value into cfg structure to let user check it */
        cfg->srcmd_perm_val = (rb_srcmd_perm_64 & cfg->srcmd_perm_mask);
        return IOPMP_ERR_ILLEGAL_VALUE;
    }

    return IOPMP_OK;
}

/**
 * \brief Set a global entry into IOPMP for SRCMD_FMT=2 and MDCFG_FMT=1 and
 * HWCFG0.md_entry_num=0 (K=1)
 *
 * \param[in] iopmp             The IOPMP instance to be written
 * \param[in] entry             The pointer to the entry
 * \param[in] entry_idx         The global index of target entry
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written SRCMD_PERM(H) does not match
 *         the actual value
 *
 * \note This operation is only supported by SRCMD_FMT=2 and MDCFG_FMT=1 and
 *       HWCFG0.md_entry_num=0 (K=1)
 */
static enum iopmp_error srcmd_fmt_2_mdcfg_fmt_1_md_entry_num_0_set_entry(
    IOPMP_t *iopmp, const struct iopmp_entry *entry, uint32_t entry_idx)
{
    /* The "private_data" member in the entry encodes SRCMD_PERM(H) */
    write_srcmd_perm_64(iopmp, entry_idx, entry->private_data);
    /* SRCMD_PERM(H).perm is WARL field. Read it back to check value */
    if (read_srcmd_perm_64(iopmp, entry_idx) != entry->private_data)
        return IOPMP_ERR_ILLEGAL_VALUE;

    return generic_set_entries(iopmp, entry, entry_idx, 1);
}

enum iopmp_error srcmd_fmt_2_mdcfg_fmt_1_md_entry_num_0_set_entries(
    IOPMP_t *iopmp, const struct iopmp_entry *entry_array,
    uint32_t idx_start, uint32_t num_entry)
{
    enum iopmp_error ret;

    for (int i = 0; i < num_entry; i++) {
        ret = srcmd_fmt_2_mdcfg_fmt_1_md_entry_num_0_set_entry(
                iopmp, &entry_array[i], idx_start + i);
        if (ret != IOPMP_OK)
            return ret;
    }

    return IOPMP_OK;
}

#ifdef ENABLE_SPS
/******************************************************************************/
/* Functions specific to IOPMP/SPS (Secondary Permission Setting) extension   */
/******************************************************************************/
/**
 * \brief Get RRID's read permission to MD(0) ~ MD(62)
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[in] rrid              The RRID to be got
 *
 * \return SRCMD_RH(rrid).mdh | SRCMD_R(rrid).md
 *
 * \note This operation is only supported by IOPMP/SPS extension
 */
static uint64_t sps_get_srcmd_r_64_md(IOPMP_t *iopmp, uint32_t rrid)
{
    return read_srcmd_r_64(iopmp, rrid) >> IOPMP_SRCMD_R_MD_SHIFT;
}

/**
 * \brief Set RRID's read permission to MD(0) ~ MD(62)
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[in] rrid              The RRID to be set
 * \param[in,out] mds           Input the read permission bitmap associated with
 *                              \p rrid. Output WARL value
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p mds does not match the
 *         actual values
 *
 * \note This operation is only supported by IOPMP/SPS extension
 */
static enum iopmp_error sps_set_srcmd_r_64_md(IOPMP_t *iopmp, uint32_t rrid,
                                              uint64_t *mds)
{
    uint64_t srcmd_r_64;
    uint64_t __mds = *mds;

    srcmd_r_64 = __mds << IOPMP_SRCMD_R_MD_SHIFT;
    write_srcmd_r_64(iopmp, rrid, srcmd_r_64);
    /* SRCMD_R.md and SRCMD_RH.mdh are WARL. Read them back to check. */
    *mds = sps_get_srcmd_r_64_md(iopmp, rrid);

    return (*mds == __mds) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}

/**
 * \brief Get RRID's write permission to MD(0) ~ MD(62)
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[in] rrid              The RRID to be got
 *
 * \return SRCMD_WH(rrid).mdh | SRCMD_W(rrid).md
 *
 * \note This operation is only supported by IOPMP/SPS extension
 */
static uint64_t sps_get_srcmd_w_64_md(IOPMP_t *iopmp, uint32_t rrid)
{
    return read_srcmd_w_64(iopmp, rrid) >> IOPMP_SRCMD_W_MD_SHIFT;
}

/**
 * \brief Set RRID's write permission to MD(0) ~ MD(62)
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[in] rrid              The RRID to be set
 * \param[in,out] mds           Input the write permission bitmap associated
 *                              with \p rrid. Output WARL value
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p mds does not match the
 *         actual values
 *
 * \note This operation is only supported by IOPMP/SPS extension
 */
static enum iopmp_error sps_set_srcmd_w_64_md(IOPMP_t *iopmp, uint32_t rrid,
                                              uint64_t *mds)
{
    uint64_t srcmd_w_64;
    uint64_t __mds = *mds;

    srcmd_w_64 = __mds << IOPMP_SRCMD_W_MD_SHIFT;
    write_srcmd_w_64(iopmp, rrid, srcmd_w_64);
    /* SRCMD_W.md and SRCMD_WH.mdh are WARL. Read them back to check. */
    *mds = sps_get_srcmd_w_64_md(iopmp, rrid);

    return (*mds == __mds) ? IOPMP_OK : IOPMP_ERR_ILLEGAL_VALUE;
}
#endif

/******************************************************************************/
/* IOPMP operations for all well-defined models in IOPMP specification        */
/******************************************************************************/
/* Generic operations for all IOPMP models */
static struct iopmp_operations_generic iopmp_operations_generic = {
    .enable = generic_enable,
    .lock_prio_entry_num = generic_lock_prio_entry_num,
    .lock_rrid_transl = generic_lock_rrid_transl,
    .set_prio_entry_num = generic_set_prio_entry_num,
    .set_rrid_transl = generic_set_rrid_transl,
    .stall_by_mds = generic_stall_by_mds,
    .resume_transactions = generic_resume_transactions,
    .poll_mdstall = generic_poll_mdstall,
    .set_rridscp = generic_set_rridscp,
    .lock_entries = generic_lock_entries,
    .lock_err_cfg = generic_lock_err_cfg,
    .set_global_intr = generic_set_global_intr,
    .set_global_err_resp = generic_set_global_err_resp,
    .set_msi_en = generic_set_msi_en,
    .set_msi_info = generic_set_msi_info,
    .get_and_clear_msi_werr = generic_get_and_clear_msi_werr,
    .set_stall_violation_en = generic_set_stall_violation_en,
    .capture_error = generic_capture_error,
    .invalidate_error = generic_invalidate_error,
    .get_sv_window = generic_get_sv_window,
    .set_entries = generic_set_entries,
    .get_entries = generic_get_entries,
    .clear_entries = generic_clear_entries,
};

#ifdef ENABLE_SPS
/* Operations specific to IOPMP/SPS extension */
static struct iopmp_operations_sps iopmp_ops_sps = {
    .sps_get_srcmd_r_64_md = sps_get_srcmd_r_64_md,
    .sps_set_srcmd_r_64_md = sps_set_srcmd_r_64_md,
    .sps_get_srcmd_w_64_md = sps_get_srcmd_w_64_md,
    .sps_set_srcmd_w_64_md = sps_set_srcmd_w_64_md,
};
#endif

/******************************************************************************/
/* IOPMP common initialization for standard IOPMP implementation              */
/******************************************************************************/
static enum iopmp_error
__init_common(IOPMP_t *iopmp, uintptr_t addr,
              uint8_t srcmd_fmt, uint8_t mdcfg_fmt,
              struct iopmp_operations_specific *ops_specific)
{
    uint32_t data, hwcfg0;
    uint8_t hwcfg0_srcmd_fmt, hwcfg0_mdcfg_fmt;

    /* Read HWCFG0 first to check srcmd_fmt and mdcfg_fmt */
    hwcfg0 = io_read32(addr + IOPMP_HWCFG0_BASE);
    hwcfg0_mdcfg_fmt = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_MDCFG_FMT);
    hwcfg0_srcmd_fmt = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_SRCMD_FMT);
    if (srcmd_fmt != hwcfg0_srcmd_fmt || mdcfg_fmt != hwcfg0_mdcfg_fmt)
        return IOPMP_ERR_NOT_SUPPORTED;

    /* Set base address and address width of IOPMP */
    iopmp->addr = addr;

    /* Read VERSION */
    data = io_read32(iopmp->addr + IOPMP_VERSION_BASE);
    /* Record into local data structure */
    iopmp->vendor = EXTRACT_FIELD(data, IOPMP_VERSION_VENDOR);
    iopmp->specver = EXTRACT_FIELD(data, IOPMP_VERSION_SPECVER);

    /* Read IMPLEMENTATION */
    data = io_read32(iopmp->addr + IOPMP_IMPLEMENTATION_BASE);
    /* Record into local data structure */
    iopmp->impid = EXTRACT_FIELD(data, IOPMP_IMPLEMENTATION_IMPID);

    /* Record HWCFG0 into local data structure */
    iopmp->mdcfg_fmt = hwcfg0_mdcfg_fmt;
    iopmp->srcmd_fmt = hwcfg0_srcmd_fmt;
    iopmp->tor_en = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_TOR_EN);
    iopmp->sps_en = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_SPS_EN);
    iopmp->user_cfg_en = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_USER_CFG_EN);
    iopmp->prient_prog = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_PRIENT_PROG);
    iopmp->rrid_transl_en = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_RRID_TRANSL_EN);
    iopmp->rrid_transl_prog = EXTRACT_FIELD(hwcfg0,
                                            IOPMP_HWCFG0_RRID_TRANSL_PROG);
    iopmp->chk_x = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_CHK_X);
    iopmp->no_x = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_NO_X);
    iopmp->no_w = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_NO_W);
    iopmp->stall_en = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_STALL_EN);
    iopmp->peis = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_PEIS);
    iopmp->pees = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_PEES);
    iopmp->mfr_en = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_MFR_EN);
    iopmp->md_entry_num = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_MD_ENTRY_NUM);
    iopmp->md_num = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_MD_NUM);
    iopmp->addrh_en = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_ADDRH_EN);
    iopmp->enable = EXTRACT_FIELD(hwcfg0, IOPMP_HWCFG0_ENABLE);

    /* Read HWCFG1 */
    data = io_read32(iopmp->addr + IOPMP_HWCFG1_BASE);
    /* Record into local data structure */
    iopmp->rrid_num = EXTRACT_FIELD(data, IOPMP_HWCFG1_RRID_NUM);
    iopmp->entry_num = EXTRACT_FIELD(data, IOPMP_HWCFG1_ENTRY_NUM);

    /* Read HWCFG2 */
    data = io_read32(iopmp->addr + IOPMP_HWCFG2_BASE);
    /* Record into local data structure */
    iopmp->prio_entry_num = EXTRACT_FIELD(data, IOPMP_HWCFG2_PRIO_ENTRY);
    iopmp->rrid_transl = EXTRACT_FIELD(data, IOPMP_HWCFG2_RRID_TRANSL);

    /* Read ENTRY_OFFSET */
    data = io_read32(iopmp->addr + IOPMP_ENTRY_OFFSET_BASE);
    /* Record into local data structure */
    iopmp->addr_entry_array = iopmp->addr + (int32_t)data;

    /* Read ERR_CFG */
    data = io_read32(iopmp->addr + IOPMP_ERR_CFG_BASE);
    /* Record into local data structure */
    iopmp->err_cfg_lock = EXTRACT_FIELD(data, IOPMP_ERR_CFG_L);
    iopmp->intr_enable = EXTRACT_FIELD(data, IOPMP_ERR_CFG_IE);
    iopmp->msi_en = EXTRACT_FIELD(data, IOPMP_ERR_CFG_MSI_EN);
    iopmp->stall_violation_en = EXTRACT_FIELD(data, IOPMP_ERR_CFG_STALL_VIO_EN);
    iopmp->msidata = EXTRACT_FIELD(data, IOPMP_ERR_CFG_MSIDATA);

    /* Read ERR_MSIADDR and ERR_MSIADDRH */
    if (iopmp->msi_en) {
        uint32_t msiaddr, msiaddrh = 0;
        msiaddr = io_read32(iopmp->addr + IOPMP_ERR_MSIADDR_BASE);
        if (iopmp->addrh_en)
            msiaddrh = io_read32(iopmp->addr + IOPMP_ERR_MSIADDRH_BASE);
        else
            msiaddr = msiaddr << 2; /* ERR_MSIADDR contains address[33:2] */
        iopmp->msiaddr64 = (uint64_t)msiaddrh << 32 | msiaddr;
    }

    /* Read MDLCK(H) */
    if (iopmp->srcmd_fmt == IOPMP_SRCMD_FMT_1) {
        iopmp->mdlck_lock = true;
        iopmp->mdlck_md = ((uint64_t)1 << iopmp->md_num) - 1;
    } else {
        uint32_t mdlck, mdlckh;
        uint64_t mdlck_64;

        mdlck = io_read32(iopmp->addr + IOPMP_MDLCK_BASE);
        if (iopmp->md_num > 31)
            mdlckh = io_read32(iopmp->addr + IOPMP_MDLCKH_BASE);
        else
            mdlckh = 0;
        mdlck_64 = ((uint64_t)mdlckh << 32) | mdlck;

        iopmp->mdlck_lock = EXTRACT_FIELD(mdlck, IOPMP_MDLCK_L);
        iopmp->mdlck_md = mdlck_64 >> IOPMP_MDLCK_MD_SHIFT;
    }

    /* Read MDCFGLCK when MDCFG is in Format 0 */
    if (iopmp->mdcfg_fmt == IOPMP_MDCFG_FMT_0) {
        data = io_read32(iopmp->addr + IOPMP_MDCFGLCK_BASE);
        /* Record into local data structure */
        iopmp->mdcfglck_lock = EXTRACT_FIELD(data, IOPMP_MDCFGLCK_L);
        iopmp->mdcfglck_f = EXTRACT_FIELD(data, IOPMP_MDCFGLCK_F);
    }

    /* Read ENTRYLCK */
    data = io_read32(iopmp->addr + IOPMP_ENTRYLCK_BASE);
    /* Record into local data structure */
    iopmp->entrylck_lock = EXTRACT_FIELD(data, IOPMP_ENTRYLCK_L);
    iopmp->entrylck_f = EXTRACT_FIELD(data, IOPMP_ENTRYLCK_F);

    /* Detect if this IOPMP supports stall transactions */
    detect_stall_function(iopmp);

    /* Detect implemented bits of ENTRY_ADDR(H) */
    detect_entry_addr_bits(iopmp);

    /* Setup operations */
    iopmp->ops_generic = &iopmp_operations_generic;
    iopmp->ops_specific = ops_specific;
    if (!iopmp->ops_specific)
        return IOPMP_ERR_NOT_SUPPORTED;
    iopmp->ops_sps = NULL;

    iopmp->init = true;
    return IOPMP_OK;
}

#ifdef ENABLE_SPS
enum iopmp_error
iopmp_drv_init_common(IOPMP_t *iopmp, uintptr_t addr,
                      uint8_t srcmd_fmt, uint8_t mdcfg_fmt,
                      struct iopmp_operations_specific *ops_specific)
{
    enum iopmp_error ret;

    ret = __init_common(iopmp, addr, srcmd_fmt, mdcfg_fmt, ops_specific);
    if (ret != IOPMP_OK)
        return ret;

    /* Assign IOPMP/SPS operations */
    if (iopmp->sps_en)
        iopmp->ops_sps = &iopmp_ops_sps;

    return IOPMP_OK;
}
#else
enum iopmp_error
iopmp_drv_init_common(IOPMP_t *iopmp, uintptr_t addr,
                      uint8_t srcmd_fmt, uint8_t mdcfg_fmt,
                      struct iopmp_operations_specific *ops_specific)
{
    return __init_common(iopmp, addr, srcmd_fmt, mdcfg_fmt, ops_specific);
}
#endif
