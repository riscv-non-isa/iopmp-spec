/***************************************************************************
// Author: Gull Ahmed (gull.ahmed@10xengineers.ai)
//         Yazan Hussnain (yazan.hussain@10xengineers.ai)
// Date: October 21, 2024
// Description:
// This file implements the IOPMP (I/O Physical Memory Protection)
// functions to read/write and reset the MMAP Registers.
//
// The main functions in this file include:
// - reset_iopmp: Resets the I/O Physical Memory Protection (IOPMP)
//   configuration registers to default values.
// - is_access_valid: Checks if the access to a given offset and number
//   of bytes is valid.
// - read_register: Reads a register based on the given offset and byte size.
// - rrid_stall_update: Updates the stall status for each RRID based on
//   memory domain stall conditions.
// - write_register: Writes data to a memory-mapped register identified
//   by the specified offset.
***************************************************************************/

#include "iopmp.h"
#include "config.h"

/* Generate 32-bit mask[h:l] */
#define GENMASK_32(h, l) \
    (((~(uint32_t)0) - ((uint32_t)1 << (l)) + 1) & (~(uint32_t)0 >> (32-1-(h))))

/**
 * @brief Resets the I/O Physical Memory Protection (IOPMP) configuration
 * registers to default values.
 *
 * This function initializes all configuration in the IOPMP module,
 * setting them to their default values. It also conditionally applies
 * certain configurations based on compile-time macros (e.g., `MDCFG_FMT`,
 * `SRCMD_FMT`) to enable or disable specific features, making it
 * adaptable to various IOPMP configurations.
 *
 * This reset function ensures that the IOPMP module is in a known, clean state,
 * ideal for initialization before a new configuration is loaded.
 *
 * @param iopmp The IOPMP instance.
 * @param cfg The hardware configurations of IOPMP instance when reset
 *
 * @return it Returns 0 upon successful reset.
 */

// Function to reset I/O Memory Protection
int reset_iopmp(iopmp_dev_t *iopmp, iopmp_cfg_t *cfg)
{
    // Zeroize all states
    memset(iopmp, 0, sizeof(*iopmp));

    // Reset all IOPMP registers
    iopmp->reg_file.version.vendor          = cfg->vendor;
    iopmp->reg_file.version.specver         = cfg->specver;
    iopmp->reg_file.implementation.raw      = cfg->impid;

    // Hardware Configuration
    iopmp->reg_file.hwcfg0.enable           = cfg->enable;
    iopmp->reg_file.hwcfg0.md_num           = cfg->md_num;
    iopmp->reg_file.hwcfg0.addrh_en         = cfg->addrh_en;
    iopmp->reg_file.hwcfg0.tor_en           = cfg->tor_en;
    iopmp->reg_file.hwcfg1.rrid_num         = cfg->rrid_num;
    iopmp->reg_file.hwcfg1.entry_num        = cfg->entry_num;
    if (cfg->non_prio_en) {
        iopmp->reg_file.hwcfg2.prio_entry       = cfg->prio_entry;
        iopmp->reg_file.hwcfg2.prio_ent_prog    = cfg->prio_ent_prog;
    }
    iopmp->reg_file.hwcfg2.non_prio_en      = cfg->non_prio_en;
    iopmp->reg_file.hwcfg2.chk_x            = cfg->chk_x;
    iopmp->reg_file.hwcfg2.peis             = cfg->peis;
    iopmp->reg_file.hwcfg2.pees             = cfg->pees;
    iopmp->reg_file.hwcfg2.sps_en           = cfg->sps_en;
    iopmp->reg_file.hwcfg2.stall_en         = cfg->stall_en;
    iopmp->reg_file.hwcfg2.mfr_en           = cfg->mfr_en;
    /* Set HWCFG2_en if HWCFG2 is not zero */
    iopmp->reg_file.hwcfg0.HWCFG2_en        = (iopmp->reg_file.hwcfg2.raw) != 0 ? true : false;

    // Set the MDCFG Format - Based on compiled model
#ifdef MDCFG_FMT
    iopmp->reg_file.hwcfg3.mdcfg_fmt        = MDCFG_FMT;
#endif
    // Set the SRCMD Format - Based on compiled model
#ifdef SRCMD_FMT
    iopmp->reg_file.hwcfg3.srcmd_fmt        = SRCMD_FMT;
#endif
#if (MDCFG_FMT == 1 || MDCFG_FMT == 2)
    iopmp->reg_file.hwcfg3.md_entry_num     = IOPMP_MD_ENTRY_NUM;
#endif
    iopmp->reg_file.hwcfg3.no_x             = cfg->no_x;
    iopmp->reg_file.hwcfg3.no_w             = cfg->no_w;
    iopmp->reg_file.hwcfg3.rrid_transl_en   = IOPMP_RRID_TRANSL_EN;
#if (IOPMP_RRID_TRANSL_EN)
    iopmp->reg_file.hwcfg3.rrid_transl_prog = IOPMP_RRID_TRANSL_PROG;
    iopmp->reg_file.hwcfg3.rrid_transl      = IOPMP_RRID_TRANSL;
#endif
    /* Set HWCFG3_en if HWCFG3 is not zero */
    iopmp->reg_file.hwcfg0.HWCFG3_en        = (iopmp->reg_file.hwcfg3.raw) != 0 ? true : false;

    iopmp->reg_file.entryoffset.raw         = ENTRY_OFFSET;

#if (SRCMD_FMT != 1)
    iopmp->reg_file.mdlck.l                 = !IMP_MDLCK;
#endif

    iopmp->reg_file.err_reqid.eid           = IMP_ERROR_REQID ? 0 : 0xFFFF;

    return 0;
}

/**
  * @brief Checks if the access to a given offset and number of bytes is valid.
  *
  * @param iopmp The IOPMP instance.
  * @param offset     The offset within the memory map to check for access.
  * @param num_bytes  The number of bytes requested for the access.
  * @return uint8_t   Returns 1 if access is valid, 0 if invalid.
 **/
uint8_t is_access_valid(iopmp_dev_t *iopmp, uint64_t offset, uint8_t num_bytes) {
    // Check if the offset falls within the allowed IOPMP rule range
    bool iopmpRule_range;
    iopmpRule_range = (offset >= ENTRY_OFFSET) &
                      (offset < ((ENTRY_OFFSET + 0xC) + (iopmp->reg_file.hwcfg1.entry_num * ENTRY_REG_STRIDE)));

    // Validate the access by checking:
    // 1. If the requested byte size is either 4 or 8.
    // 2. If the offset is within the allowable range for valid RRIDs,
    //    and if it is outside the range, ensure it falls within the IOPMP rule range.
    // 3. If the offset is aligned with the requested byte size.
    if ((num_bytes != 4 && num_bytes != 8) || (num_bytes > REG_INTF_BUS_WIDTH) ||
        (offset > (SRCMD_WH_OFFSET + ((iopmp->reg_file.hwcfg1.rrid_num - 1) * SRCMD_REG_STRIDE)) && !iopmpRule_range) ||
        ((offset & (num_bytes - 1)) != 0)) {
        return 0; // Access is invalid
    }

    return 1; // Access is valid
}

/**
 * @brief Reads a register based on the given offset and byte size.
 *
 * This function handles special cases for specific offsets (e.g., error registers)
 * and returns the corresponding register value, either 4 or 8 bytes, based on the
 * provided number of bytes.
 *
 * @param iopmp The IOPMP instance.
 * @param offset The offset of the register to be read.
 * @param num_bytes The number of bytes to read (either 4 or 8 bytes).
 *
 * @return The value of the register in the appropriate size (4 or 8 bytes).
 */
reg_intf_dw read_register(iopmp_dev_t *iopmp, uint64_t offset, uint8_t num_bytes) {

    if (!is_access_valid(iopmp, offset, num_bytes)) return 0;

    // If the requested offset corresponds to the error MFR (ERR_MFR_OFFSET)
    // handle reading from the error register.
    if (offset == ERR_MFR_OFFSET && iopmp->reg_file.hwcfg2.mfr_en) {
        // ERR_INFO.svc=0 indicates there is no subsequent violation.
        if (iopmp->reg_file.err_info.svc == 0)
            return 0;

        // Clear the error flags for error register.
        iopmp->reg_file.err_mfr.svs = 0;
        iopmp->reg_file.err_mfr.svw = 0;

        // Start searching for errors from the current error index.
        int start_index = iopmp->reg_file.err_mfr.svi;

        // Loop over the RRIDs to find any error state.
        for (int i = 0; i < NUM_SVW; i++) {
            // Calculate the current index, with wrap-around using modulo.
            int current_index = (start_index + i) % NUM_SVW;

            // If an error is found (svw is non-zero), update the error status.
            if (iopmp->err_svs.sv[current_index].svw) {
                iopmp->reg_file.err_mfr.svw = iopmp->err_svs.sv[current_index].svw;   // Subsequent violation window
                iopmp->reg_file.err_mfr.svi = current_index;                          // Update the error index.
                iopmp->reg_file.err_mfr.svs = 1;                                      // Subsequent Violation Status

                // Clear the error flag after processing.
                iopmp->err_svs.sv[current_index].svw = 0;
                break;
            }
        }

        // Clear ERR_INFO.svc if there is no subsequent violation.
        iopmp->reg_file.err_info.svc = 0;
        for (int i = 0; i < NUM_SVW; i++) {
            if (iopmp->err_svs.sv[i].svw) {
                iopmp->reg_file.err_info.svc = 1;
                break;
            }
        }

        return iopmp->reg_file.err_mfr.raw;
    }

    // If the offset is within the valid range for entry registers, return the appropriate value.
    if ((offset >= ENTRY_OFFSET) && (offset < (ENTRY_OFFSET + 0xC + (iopmp->reg_file.hwcfg1.entry_num * ENTRY_REG_STRIDE) + 4))) {
        // Return 4-byte or 8-byte register value based on num_bytes.
        return iopmp->iopmp_entries.regs4[(offset - ENTRY_OFFSET) / num_bytes];
    }

    // For all other offsets, return the corresponding register value.
    // If num_bytes is 4, return a 4-byte value, otherwise return an 8-byte value.
    return iopmp->reg_file.regs4[offset / num_bytes];
}

/**
 * @brief Updates the stall status for each RRID based on memory domain stall conditions.
 *
 * @param iopmp The IOPMP instance.
 * @param exempt A flag indicating whether the RRID stall status should be exempted.
 */
void rrid_stall_update(iopmp_dev_t *iopmp, uint8_t exempt) {
    uint64_t stall_by_md;

    // Combine the high and low parts of the 'mdstall' register to create a full 64-bit stall mask.
    stall_by_md = ((uint64_t)iopmp->reg_file.mdstallh.mdh << 31) | iopmp->reg_file.mdstall.md;

    // Iterate through all RRIDs to update the stall status.
    for (int i = 0; i < iopmp->reg_file.hwcfg1.rrid_num; i++) {

        #if (SRCMD_FMT == 0)
            uint64_t srcmd_md;
            // Format 0: Combine srcmd_enh and srcmd_en fields to evaluate stall conditions.
            // This forms a 64-bit value representing memory domain stall conditions.
            srcmd_md = ((uint64_t)iopmp->reg_file.srcmd_table[i].srcmd_enh.mdh << 31) | iopmp->reg_file.srcmd_table[i].srcmd_en.md;

            // Update the rrid_stall array based on the combined stall conditions, considering the exempt flag.
            iopmp->rrid_stall[i] = exempt ^ ((srcmd_md & stall_by_md) != 0);

        #elif (SRCMD_FMT == 1)
            // Format 1: Directly use the bit at position `i` in the `stall_by_md` mask for the RRID stall condition.
            // Because in format 1. RRID i is directly mapped with MD i.
            iopmp->rrid_stall[i] = exempt ^ (((stall_by_md >> i) & 1) != 0);

        #elif (SRCMD_FMT == 2)
            uint64_t srcmd_md;
            srcmd_md = (1ULL << iopmp->reg_file.hwcfg0.md_num) - 1;
            // Update rrid_stall based on the accumulated permissions and the stall conditions.
            iopmp->rrid_stall[i] = exempt ^ ((srcmd_md & stall_by_md) != 0);
        #endif
    }
}

/**
 * @brief Writes data to a memory-mapped register identified by the specified offset.
 *
 * The data type `reg_intf_dw` depends on the configuration in the `config.h` file
 *  (e.g., `uint32_t` for 4-byte width, `uint64_t` for 8-byte width).
 *
 * @param iopmp The IOPMP instance.
 * @param offset The offset of the register to be written.
 * @param data It contains the data that need to be written.
 * @param num_bytes The number of bytes to write (either 4 or 8 bytes).
 *
 */
void write_register(iopmp_dev_t *iopmp, uint64_t offset, reg_intf_dw data, uint8_t num_bytes) {

  // Extract lower and upper 32-bits of data based on bus width
    uint32_t lwr_data4, upr_data4;
#if (REG_INTF_BUS_WIDTH == 8)
    lwr_data4 = data & UINT32_MAX;
    upr_data4 = (data >> 32) & UINT32_MAX;  // Using 32 bits for upper part
#else
    lwr_data4 = data;
    upr_data4 = data;         // Upper part is same as lower part
#endif

  // Initialize temporary registers
    hwcfg0_t         hwcfg0_temp         = { .raw = lwr_data4 };
    hwcfg2_t         hwcfg2_temp         = { .raw = lwr_data4 };
    hwcfg3_t         hwcfg3_temp         = { .raw = lwr_data4 };
    entrylck_t       entrylck_temp       = { .raw = upr_data4 };
    err_cfg_t        err_cfg_temp        = { .raw = lwr_data4 };
    entry_addr_t     entry_addr_temp     = { .raw = lwr_data4 };
    entry_addrh_t    entry_addrh_temp    = { .raw = upr_data4 };
    entry_cfg_t      entry_cfg_temp      = { .raw = lwr_data4 };
    entry_user_cfg_t entry_user_cfg_temp = { .raw = upr_data4 };

// Conditional block for error capture
#if (ERROR_CAPTURE_EN)
    err_info_t err_info_temp = { .raw = upr_data4 };
#endif

// Conditional block for msi addr
#if (MSI_EN)
    err_msiaddr_t    err_msiaddr_temp    = { .raw = lwr_data4 };
    err_msiaddrh_t   err_msiaddrh_temp   = { .raw = upr_data4 };
#endif

// Conditional block for SRCMD format
#if (SRCMD_FMT != 1)
    mdlck_t  mdlck_temp  = { .raw = lwr_data4 };
    mdlckh_t mdlckh_temp = { .raw = upr_data4 };
#endif

// MDCFG format check
#if (MDCFG_FMT == 0)
    mdcfglck_t mdcfglck_temp = { .raw = lwr_data4 };
    mdcfg_t    mdcfg_temp    = { .raw = data };
#endif

    uint32_t md_num = iopmp->reg_file.hwcfg0.md_num;

// SRCMD format handling
#if (SRCMD_FMT == 0)
    srcmd_en_t  srcmd_en_temp  = { .raw = lwr_data4 & ((md_num >= 31) ? UINT32_MAX : GENMASK_32(md_num, 0)) };
    srcmd_enh_t srcmd_enh_temp = { .raw = (md_num < 32) ? 0 : upr_data4 & GENMASK_32(md_num - 32, 0) };
    srcmd_r_t   srcmd_r_temp   = { .raw = lwr_data4 & ((md_num >= 31) ? UINT32_MAX : GENMASK_32(md_num, 0)) };
    srcmd_rh_t  srcmd_rh_temp  = { .raw = (md_num < 32) ? 0 : upr_data4 & GENMASK_32(md_num - 32, 0) };
    srcmd_w_t   srcmd_w_temp   = { .raw = lwr_data4 & ((md_num >= 31) ? UINT32_MAX : GENMASK_32(md_num, 0)) };
    srcmd_wh_t  srcmd_wh_temp  = { .raw = (md_num < 32) ? 0 : upr_data4 & GENMASK_32(md_num - 32, 0) };
#elif (SRCMD_FMT == 2)
    srcmd_perm_t  srcmd_perm_temp  = { .raw = lwr_data4 };
    srcmd_permh_t srcmd_permh_temp = { .raw = upr_data4 };
#endif

// IOPMP Stall configuration
    mdstall_t  mdstall_temp  = { .raw = lwr_data4 & ((md_num >= 31) ? UINT32_MAX : GENMASK_32(md_num, 0)) };
    mdstallh_t mdstallh_temp = { .raw = (md_num < 32) ? 0 : upr_data4 & GENMASK_32(md_num - 32, 0) };
    #if (IMP_RRIDSCP)
        rridscp_t  rridscp_temp  = { .raw = lwr_data4 };
        rridscp_temp.op          = (lwr_data4 >> 30) & MASK_BIT_POS(2);
    #endif
    mdstall_temp.md          = (lwr_data4 >> 1) & ((md_num >= 31) ? UINT32_MAX : GENMASK_32(md_num - 1, 0));
    mdstall_temp.exempt      = GET_BIT(lwr_data4, 0);

// IOPMP MFR configuration
    err_mfr_t err_mfr_temp = { .raw = upr_data4 };

  if (!is_access_valid(iopmp, offset, num_bytes)) return;

  switch (offset) {
    case VERSION_OFFSET:
      // This register is read only
      return;

    case IMPLEMENTATION_OFFSET:
      // This register is read only
      return;

    case HWCFG0_OFFSET:
        iopmp->reg_file.hwcfg0.enable           |= hwcfg0_temp.enable;
        break;

    case HWCFG1_OFFSET:
        // This register is read only
        return;

    case HWCFG2_OFFSET:
        if (iopmp->reg_file.hwcfg0.HWCFG2_en) {
            if (iopmp->reg_file.hwcfg2.non_prio_en) {
                if (iopmp->reg_file.hwcfg2.prio_ent_prog) {
                    iopmp->reg_file.hwcfg2.prio_entry = hwcfg2_temp.prio_entry;
                }
                iopmp->reg_file.hwcfg2.prio_ent_prog &= ~hwcfg2_temp.prio_ent_prog;
            }
        }
        break;

    case HWCFG3_OFFSET:
        if (iopmp->reg_file.hwcfg0.HWCFG3_en) {
        #if (MDCFG_FMT == 2)
            if (!iopmp->reg_file.hwcfg0.enable) {
                iopmp->reg_file.hwcfg3.md_entry_num = hwcfg3_temp.md_entry_num;
            }
        #endif
        #if (IOPMP_RRID_TRANSL_EN)
            if (iopmp->reg_file.hwcfg3.rrid_transl_prog) {
                iopmp->reg_file.hwcfg3.rrid_transl = hwcfg3_temp.rrid_transl;
            }
            iopmp->reg_file.hwcfg3.rrid_transl_prog &= ~hwcfg3_temp.rrid_transl_prog;
        #endif
        }
        break;

    case ENTRYOFFSET_OFFSET:
        // This register is read only
        return;

    case MDSTALL_OFFSET:
        if (iopmp->reg_file.hwcfg2.stall_en) {
            iopmp->reg_file.mdstall.exempt = mdstall_temp.exempt;
            iopmp->reg_file.mdstall.md     = mdstall_temp.md;
            rrid_stall_update(iopmp, iopmp->reg_file.mdstall.exempt);
            if ((mdstall_temp.raw == 0) && (iopmp->reg_file.mdstall.raw == 0)) {
                iopmp->stall_cntr = 0;
            }
        }
        if (num_bytes == 4) break;

    case MDSTALLH_OFFSET:
        if (iopmp->reg_file.hwcfg2.stall_en) {
            iopmp->reg_file.mdstallh.mdh = mdstallh_temp.mdh;
        }
        break;

#if (IMP_RRIDSCP)
    case RRISCP_OFFSET:
        iopmp->reg_file.rridscp.rsv  = 0;
        iopmp->reg_file.rridscp.op   = rridscp_temp.op;
        if (rridscp_temp.rrid < iopmp->reg_file.hwcfg1.rrid_num) {
            iopmp->reg_file.rridscp.rrid = rridscp_temp.rrid;
        }
        else if (iopmp->reg_file.rridscp.op == 0) {
            iopmp->reg_file.rridscp.stat = 3;
            break;
        }

        if (iopmp->reg_file.rridscp.op == 0) {
            iopmp->reg_file.rridscp.stat = 2 - iopmp->rrid_stall[iopmp->reg_file.rridscp.rrid];
        }
        else if (iopmp->reg_file.rridscp.op == 1) { iopmp->rrid_stall[rridscp_temp.rrid] = 1; }
        else if (iopmp->reg_file.rridscp.op == 2) { iopmp->rrid_stall[rridscp_temp.rrid] = 0; }
        break;
#endif

#if (SRCMD_FMT != 1) & (IMP_MDLCK)
    case MDLCK_OFFSET:
        if (!iopmp->reg_file.mdlck.l) {
            iopmp->reg_file.mdlck.l  |= mdlck_temp.l;
            iopmp->reg_file.mdlck.md |= mdlck_temp.md;
        }
        if (num_bytes) break;

    case MDLCKH_OFFSET:
        if (!iopmp->reg_file.mdlck.l) {
            iopmp->reg_file.mdlckh.mdh |= mdlckh_temp.mdh;
        }
        break;
#endif

#if (MDCFG_FMT == 0)
    case MDCFGLCK_OFFSET:
        if (!iopmp->reg_file.mdcfglck.l) {
            iopmp->reg_file.mdcfglck.l |= mdcfglck_temp.l;
            if (mdcfglck_temp.f > iopmp->reg_file.mdcfglck.f) {
                iopmp->reg_file.mdcfglck.f = mdcfglck_temp.f;
            }
            iopmp->reg_file.mdcfglck.rsv = 0;
        }
        break;
#endif

    case ENTRYLCK_OFFSET:
        if (!iopmp->reg_file.entrylck.l) {
            iopmp->reg_file.entrylck.l |= entrylck_temp.l;
            if (entrylck_temp.f > iopmp->reg_file.entrylck.f) {
                iopmp->reg_file.entrylck.f   = entrylck_temp.f;
            }
            iopmp->reg_file.entrylck.rsv = 0;
        }
        iopmp->reg_file.entrylck.rsv = 0;
        break;

    case ERR_CFG_OFFSET:
        if (!iopmp->reg_file.err_cfg.l) {
            iopmp->reg_file.err_cfg.l                 |= err_cfg_temp.l;
            iopmp->reg_file.err_cfg.ie                 = err_cfg_temp.ie;
            iopmp->reg_file.err_cfg.rs                 = err_cfg_temp.rs;
            iopmp->reg_file.err_cfg.msi_en             = err_cfg_temp.msi_en & MSI_EN;
            iopmp->reg_file.err_cfg.stall_violation_en = err_cfg_temp.stall_violation_en;
            iopmp->reg_file.err_cfg.msidata            = err_cfg_temp.msidata;
            iopmp->reg_file.err_cfg.rsv1               = 0;
            iopmp->reg_file.err_cfg.rsv2               = 0;
        }
        break;

#if (ERROR_CAPTURE_EN)
    case ERR_INFO_OFFSET:
        iopmp->reg_file.err_info.v        &= ~err_info_temp.v;
        iopmp->reg_file.err_info.msi_werr &= ~err_info_temp.msi_werr;
        iopmp->reg_file.err_info.rsv       = 0;
        break;

    case ERR_REQADDR_OFFSET:
        return;

    case ERR_REQADDRH_OFFSET:
        return;
#endif

#if (IMP_ERROR_REQID)
    case ERR_REQID_OFFSET:
        return;
#endif

    case ERR_MFR_OFFSET:
        if (iopmp->reg_file.hwcfg2.mfr_en) {
            iopmp->reg_file.err_mfr.svi = err_mfr_temp.svi;
        }
        break;

#if (MSI_EN)
    case ERR_MSIADDR_OFFSET:
        iopmp->reg_file.err_msiaddr.raw = (!iopmp->reg_file.err_cfg.l) ?
                                          err_msiaddr_temp.raw :
                                          iopmp->reg_file.err_msiaddr.raw;
        break;

    case ERR_MSIADDRH_OFFSET:
        if (iopmp->reg_file.hwcfg0.addrh_en) {
            iopmp->reg_file.err_msiaddrh.raw = (!iopmp->reg_file.err_cfg.l) ?
                                               err_msiaddrh_temp.raw :
                                               iopmp->reg_file.err_msiaddrh.raw;
        }
        break;
#endif

    case ERR_USER0_OFFSET:
    case ERR_USER1_OFFSET:
    case ERR_USER2_OFFSET:
    case ERR_USER3_OFFSET:
    case ERR_USER4_OFFSET:
    case ERR_USER5_OFFSET:
    case ERR_USER6_OFFSET:
    case ERR_USER7_OFFSET:
        break;

    default:
        break;
  }

#if (MDCFG_FMT == 0)
    if ((((offset-MDCFG_TABLE_BASE_OFFSET)/4) >= iopmp->reg_file.mdcfglck.f) & IS_IN_RANGE(offset, MDCFG_TABLE_BASE_OFFSET, (MDCFG_TABLE_BASE_OFFSET + (md_num*4)))){
        if (mdcfg_temp.t < iopmp->reg_file.hwcfg1.entry_num) {
            iopmp->reg_file.mdcfg[(offset-MDCFG_TABLE_BASE_OFFSET)/4].t = mdcfg_temp.t;
        }
        iopmp->reg_file.mdcfg[(offset-MDCFG_TABLE_BASE_OFFSET)/4].rsv = 0;

#if (MDCFG_TABLE_IMPROPER_SETTING_BEHAVIOR == 0)
        /*
         * MDCFG table must be monotonically incremental. Some reference
         * behaviors for an improper setting are given in the specification,
         * e.g., "correct the values to make the table have a proper setting".
         * The reference model authomatically fixes it if current MDCFG table
         * violates the monotonically incremental rule. Programmer can check
         * this register after programming done.
         *
         * The MDCFG look up table is implemented in the following way:
         * - For any m >= 1, if (MDCFG(m).t < MDCFG(m-1).t):
         *                       MDCFG(m).t = MDCFG(m-1).t
         */
        for (int m = 1; m < iopmp->reg_file.hwcfg0.md_num; m++) {
            if (iopmp->reg_file.mdcfg[m].t < iopmp->reg_file.mdcfg[m - 1].t) {
                iopmp->reg_file.mdcfg[m].t = iopmp->reg_file.mdcfg[m - 1].t;
            }
        }
#endif
    }
#endif

// Code block for handling SRCMD table accesses based on format type
#if (SRCMD_FMT != 1)
    int srcmd_tbl_access;
    int is_srcmd_locked = 0;  // Initialize as unlocked

    // Pre-compute access range and lock status based on format type
    #if (SRCMD_FMT == 0)
        srcmd_tbl_access = IS_IN_RANGE(offset, SRCMD_TABLE_BASE_OFFSET, SRCMD_TABLE_BASE_OFFSET + (iopmp->reg_file.hwcfg1.rrid_num * SRCMD_REG_STRIDE) + 28);
        if (srcmd_tbl_access) {
            is_srcmd_locked = iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_en.l;
        }

    #elif (SRCMD_FMT == 2)
        srcmd_tbl_access = IS_IN_RANGE(offset, SRCMD_TABLE_BASE_OFFSET, SRCMD_TABLE_BASE_OFFSET + (md_num * SRCMD_REG_STRIDE) + 8);
        if (srcmd_tbl_access) {
            int table_index = SRCMD_TABLE_INDEX(offset);

            if (table_index < 31) {
                is_srcmd_locked = (iopmp->reg_file.mdlck.md >> table_index) & 1;
            } else {
                is_srcmd_locked = (iopmp->reg_file.mdlckh.mdh >> (table_index - 31)) & 1;
            }
        }
    #endif

    // Proceed only if within access range and not locked
    if (srcmd_tbl_access && !is_srcmd_locked) {
        uint32_t srcmd_reg = SRCMD_REG_INDEX(offset);

        switch (srcmd_reg) {
            #if (SRCMD_FMT == 0)
                // SRCMD_EN Register
                case 0:
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_en.l |= srcmd_en_temp.l;
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_en.md =
                        (srcmd_en_temp.md & ~iopmp->reg_file.mdlck.md) |
                        (iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_en.md & iopmp->reg_file.mdlck.md);
                    if (num_bytes == 4) break;

                // SRCMD_ENH Register
                case 1:
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_enh.mdh =
                        ((srcmd_enh_temp.mdh & ~iopmp->reg_file.mdlckh.mdh) |
                         (iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_enh.mdh & iopmp->reg_file.mdlckh.mdh));
                    break;

                // SRCMD_R Register
                case 2:
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_r.rsv = 0;
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_r.md =
                        (srcmd_r_temp.md & ~iopmp->reg_file.mdlck.md) |
                        (iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_r.md & iopmp->reg_file.mdlck.md);
                    if (num_bytes == 4) break;

                // SRCMD_RH Register
                case 3:
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_rh.mdh =
                        ((srcmd_rh_temp.mdh & ~iopmp->reg_file.mdlckh.mdh) |
                         (iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_rh.mdh & iopmp->reg_file.mdlckh.mdh));
                    break;

                // SRCMD_W Register
                case 4:
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_w.rsv = 0;
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_w.md =
                        (srcmd_w_temp.md & ~iopmp->reg_file.mdlck.md) |
                        (iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_w.md & iopmp->reg_file.mdlck.md);
                    if (num_bytes == 4) break;

                // SRCMD_WH Register
                case 5:
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_wh.mdh =
                        ((srcmd_wh_temp.mdh & ~iopmp->reg_file.mdlckh.mdh) |
                         (iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_wh.mdh & iopmp->reg_file.mdlckh.mdh));
                    break;

            #elif (SRCMD_FMT == 2)
                // SRCMD_PERM Register
                case 0:
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_perm.perm = srcmd_perm_temp.perm;
                    if (num_bytes == 4) break;

                // SRCMD_PERMH Register
                case 1:
                    iopmp->reg_file.srcmd_table[SRCMD_TABLE_INDEX(offset)].srcmd_permh.permh = srcmd_permh_temp.permh;
                    break;
            #endif

            default:
                break;
        }
    }
#endif

    if (IS_IN_RANGE(offset, ENTRY_TABLE_BASE_OFFSET, ENTRY_TABLE_BASE_OFFSET + (iopmp->reg_file.hwcfg1.entry_num * ENTRY_REG_STRIDE) + 12)) {
        uint32_t entry_reg = ENTRY_REG_INDEX(offset);
        if (ENTRY_TABLE_INDEX(offset) >= iopmp->reg_file.entrylck.f) {
            switch (entry_reg)
            {
                // Entry Addr Register
                case 0:
                    iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_addr.addr = entry_addr_temp.addr;
                    if ((num_bytes == 4) || !iopmp->reg_file.hwcfg0.addrh_en) break;

                // Entry Addrh Register
                case 1:
                    if (iopmp->reg_file.hwcfg0.addrh_en) {
                        iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_addrh.addrh = entry_addrh_temp.addrh;
                    }
                    break;

                // Entry Cfg Register
                case 2:
                    iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.r = entry_cfg_temp.r;
                    iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.w = entry_cfg_temp.w;
                    iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.x = entry_cfg_temp.x;
                    if (entry_cfg_temp.a == IOPMP_TOR) {
                        // ENTRY_CFG.A is WARL, check for TOR Enable before, writing.
                        if (iopmp->reg_file.hwcfg0.tor_en) {
                            iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.a = entry_cfg_temp.a;
                        }
                    }
                    else {
                        iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.a = entry_cfg_temp.a;
                    }

                    // Interrupt suppression bits are writeable, only if interrupt suppression is supported
                    if (iopmp->reg_file.hwcfg2.peis){
                        iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.sire = entry_cfg_temp.sire;
                        iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.siwe = entry_cfg_temp.siwe;
                        iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.sixe = entry_cfg_temp.sixe;
                    }

                    // Error suppression bits are writeable, only if error suppression is supported
                    if (iopmp->reg_file.hwcfg2.pees) {
                        iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.sere = entry_cfg_temp.sere;
                        iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.sewe = entry_cfg_temp.sewe;
                        iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.sexe = entry_cfg_temp.sexe;
                    }
                    iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_cfg.rsv = 0;
                    break;

                case 3:
                    iopmp->iopmp_entries.entry_table[ENTRY_TABLE_INDEX(offset)].entry_user_cfg.im = entry_user_cfg_temp.im;
                    break;
                default:
                    break;
            }
        }
    }
}
