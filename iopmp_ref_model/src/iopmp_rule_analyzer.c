/***************************************************************************
// Author: Yazan Hussnain (yazan.hussain@10xengineers.ai)
//         Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description:
// This file implements the IOPMP (I/O Physical Memory Protection)
// Permission Check for RISC-V systems. It contains functions to compute
// and validate address ranges, match transaction requests against IOPMP
// entries, and enforce access permissions (Read, Write, Execute) based
// on the IOPMP configuration and hardware restrictions. The IOPMP
// protects memory regions for peripheral accesses, ensuring only
// authorized transactions occur within specified address ranges and
// permissions.
//
// The main functions in this file include:
// - iopmpAddrRange: Computes address ranges for IOPMP entries based on
//   entry configurations, supporting modes such as NA4, TOR, and NAPOT.
// - iopmpMatchAddr: Matches transaction request addresses against an
//   IOPMP entry range, with priority handling.
// - iopmpCheckPerms: Verifies permissions against the IOPMP entry
//   configuration and Requestor Role ID (RRID) to allow or deny access.
// - iopmpRuleAnalyzer: Analyzes IOPMP rules to determine if a transaction
//   matches an entry and has the required permissions, considering
//   priority and configuration-specific conditions.
//
***************************************************************************/

#include "iopmp.h"

/**
  * @brief Computes the address range based on the IOPMP entry configuration.
  *
  * @param startAddr Pointer to store the start address of the range
  * @param endAddr Pointer to store the end address of the range
  * @param prev_iopmpaddr Previous IOPMP address (used in TOR mode)
  * @param iopmpaddr Current IOPMP address
  * @param iopmpcfg IOPMP entry configuration
  * @return 0 on success, 1 if the IOPMP entry is disabled
 **/
int iopmpAddrRange(uint64_t *startAddr, uint64_t *endAddr, uint64_t prev_iopmpaddr, uint64_t iopmpaddr, entry_cfg_t iopmpcfg) {
    uint64_t napot_mask;
    // Check if IOPMP entry is OFF
    if (iopmpcfg.a == IOPMP_OFF) { return 1; }

    switch (iopmpcfg.a) {
        case IOPMP_NA4:  // Address range covers a single 4-byte address
            *startAddr = iopmpaddr;
            *endAddr   = iopmpaddr + 1;
            break;

        case IOPMP_TOR:  // Address range specified by top-of-range mode
            *startAddr = prev_iopmpaddr;
            *endAddr   = iopmpaddr;
            break;

        default:  // Assume NAPOT (Naturally Aligned Power-of-Two) mode
            napot_mask = iopmpaddr ^ (iopmpaddr + 1);
            *startAddr = iopmpaddr & ~napot_mask;
            *endAddr   = *startAddr + napot_mask + 1;
            break;
    }

    return 0;
}

/**
  * @brief Matches transaction request address against IOPMP entry range.
  *
  * @param trans_req Transaction request containing address, size, and length
  * @param lo Lower bound of the IOPMP range
  * @param hi Upper bound of the IOPMP range
  * @param is_priority Flag indicating if the entry has priority
  * @return 0 for a full match, ENTRY_NOTMATCH for no match, or PARTIAL_HIT_ON_PRIORITY for a partial match with priority
 **/
int iopmpMatchAddr(iopmp_trans_req_t trans_req, uint64_t lo, uint64_t hi, int is_priority) {
    // Validate range
    if (hi < lo) { return ENTRY_NOTMATCH; }  // Invalid range, no match

    // Compute the end address of the transaction
    uint64_t trans_end = trans_req.addr + ((int)pow(2, trans_req.size) * (trans_req.length + 1));

    // Check if transaction falls outside the IOPMP entry range
    if (trans_end <= lo || trans_req.addr >= hi) { return ENTRY_NOTMATCH; } // No match, transaction outside range

    // Determine if there's a full match
    if (trans_req.addr >= lo && trans_end <= hi) { return 0; } // Full Match

    // Check for partial match if entry has priority
    return is_priority ? PARTIAL_HIT_ON_PRIORITY : ENTRY_NOTMATCH;
}

/**
  * @brief Checks IOPMP permissions based on request and configuration.
  *
  * @param rrid Requestor Role ID
  * @param req_perm Requested permission type (Read, Write, Execute)
  * @param iopmpcfg IOPMP configuration for the entry
  * @param md Respective Memory Domain
  * @param is_amo Indicates the AMO Access
  * @return ENTRY_MATCH if permission is granted, specific ILLEGAL_* code if denied
 **/
iopmpMatchStatus_t iopmpCheckPerms(uint16_t rrid, perm_type_e req_perm, entry_cfg_t iopmpcfg, uint8_t md, bool is_amo) {

    #if (SRCMD_FMT == 0)
        uint64_t srcmd_r, srcmd_w;
        uint8_t  srcmd_r_bit, srcmd_w_bit;
        srcmd_r     = CONCAT32(g_reg_file.srcmd_table[rrid].srcmd_rh.raw, g_reg_file.srcmd_table[rrid].srcmd_r.raw);
        srcmd_w     = CONCAT32(g_reg_file.srcmd_table[rrid].srcmd_wh.raw, g_reg_file.srcmd_table[rrid].srcmd_w.raw);
        srcmd_r_bit = GET_BIT(srcmd_r, (md + 1));
        srcmd_w_bit = GET_BIT(srcmd_w, (md + 1));
    #elif (SRCMD_FMT == 2)
        uint64_t srcmd_perm;
        uint8_t  srcmd_perm_r, srcmd_perm_w;
        srcmd_perm   = CONCAT32(g_reg_file.srcmd_table[md].srcmd_permh.raw, g_reg_file.srcmd_table[md].srcmd_perm.raw);
        srcmd_perm_r = GET_BIT(srcmd_perm, (rrid * 2));
        srcmd_perm_w = GET_BIT(srcmd_perm, ((rrid * 2) + 1));
    #endif

    // Extract hardware configuration flags
#if (SRCMD_FMT == 0)
    bool sps_en = g_reg_file.hwcfg2.sps_en; // Software privilege separation enable
#endif
    bool chk_x  = g_reg_file.hwcfg2.chk_x;  // Execute permission check enable

    // Common permission checks
    bool read_allowed    = false;
    bool write_allowed   = false;
    bool execute_allowed = false;

    #if (SRCMD_FMT == 0)
        read_allowed    = sps_en ? (iopmpcfg.r & srcmd_r_bit) : iopmpcfg.r;
        write_allowed   = sps_en ? (iopmpcfg.w & srcmd_w_bit & ((iopmpcfg.r & srcmd_r_bit) | !is_amo)) : (iopmpcfg.w & (iopmpcfg.r | !is_amo));
        execute_allowed = sps_en ? (iopmpcfg.x & srcmd_r_bit) : iopmpcfg.x;
    #elif (SRCMD_FMT == 1)
        read_allowed    = iopmpcfg.r;
        write_allowed   = (iopmpcfg.w & (iopmpcfg.r | !is_amo));
        execute_allowed = iopmpcfg.x;
    #elif (SRCMD_FMT == 2)
        read_allowed    = iopmpcfg.r || srcmd_perm_r;
        write_allowed   = ((iopmpcfg.w || srcmd_perm_w) & ((iopmpcfg.r || srcmd_perm_r) | !is_amo));
        execute_allowed = (iopmpcfg.x || srcmd_perm_r);
    #endif

    // Handle requested permission type
    switch (req_perm) {
        case READ_ACCESS:
            if (!read_allowed) {
                intrpt_suppress = iopmpcfg.sire;
                error_suppress  = iopmpcfg.sere | g_reg_file.err_cfg.rs;
            }
            return read_allowed ? ENTRY_MATCH : ILLEGAL_READ_ACCESS;

        case WRITE_ACCESS:
            if (!write_allowed) {
                intrpt_suppress = iopmpcfg.siwe;
                error_suppress  = iopmpcfg.sewe | g_reg_file.err_cfg.rs;
            }
            return write_allowed ? ENTRY_MATCH : ILLEGAL_WRITE_ACCESS;

        case INSTR_FETCH:
            if (chk_x) {
                if (!execute_allowed) {
                    intrpt_suppress = iopmpcfg.sixe;
                    error_suppress  = iopmpcfg.sexe | g_reg_file.err_cfg.rs;
                }
                return execute_allowed ? ENTRY_MATCH : ILLEGAL_INSTR_FETCH;
            } else if (read_allowed) {
                return ENTRY_MATCH;  // Grant Execute permission via Read fallback
            }
            intrpt_suppress = iopmpcfg.sixe;
            error_suppress  = iopmpcfg.sexe | g_reg_file.err_cfg.rs;
            return ILLEGAL_INSTR_FETCH;

        default:
            return ILLEGAL_READ_ACCESS;  // Default case for invalid permission request
    }
}

/**
  * @brief Matches the transaction request to an IOPMP entry, handling priority and permissions.
  *
  * @param trans_req Transaction request containing address, permissions, etc.
  * @param prev_iopmpaddr Previous IOPMP address (used in TOR mode)
  * @param iopmpaddr Current IOPMP address
  * @param iopmpcfg IOPMP entry configuration
  * @param md Respective Memory Domain
  * @param is_priority Flag indicating if the entry has priority
  * @return iopmpMatchStatus_t Status of the match:
  *         - ENTRY_MATCH: Full match, access granted.
  *         - ENTRY_NOTMATCH: No match or access denied.
  *         - PARTIAL_HIT_ON_PRIORITY: Partial match found with priority.
  *         - ILLEGAL_* status: Access denied based on permission type (e.g., read/write/execute).
 **/
iopmpMatchStatus_t iopmpRuleAnalyzer(iopmp_trans_req_t trans_req, uint64_t prev_iopmpaddr, uint64_t iopmpaddr, entry_cfg_t iopmpcfg, uint8_t md, int is_priority) {
    iopmpMatchStatus_t match_status = ENTRY_MATCH;  // Default to full match
    uint64_t start_addr, end_addr;
    bool no_w  = g_reg_file.hwcfg3.no_w;      // Write restriction
    bool no_x  = g_reg_file.hwcfg3.no_x;      // Execute restriction
    bool chk_x = g_reg_file.hwcfg2.chk_x;     // Execute permission check enable
    if ((no_w && (trans_req.perm == WRITE_ACCESS)) || (no_x && (trans_req.perm == INSTR_FETCH) && chk_x)) { return ENTRY_NOTMATCH; }

    // Set up the address range; if range setup fails, return not matched
    if (iopmpAddrRange(&start_addr, &end_addr, prev_iopmpaddr, iopmpaddr, iopmpcfg)) { return ENTRY_NOTMATCH; }

    // Match transaction address to the IOPMP address range
    // Multiply the addresses with 4 is equivalent to (Address << 2)
    int addr_match_status = iopmpMatchAddr(trans_req, (start_addr * 4), (end_addr * 4), is_priority);
    if (addr_match_status == PARTIAL_HIT_ON_PRIORITY) {
        error_suppress = g_reg_file.err_cfg.rs;
        return PARTIAL_HIT_ON_PRIORITY;  // Priority entry partial match
    }
    if (addr_match_status) { return ENTRY_NOTMATCH; } // No match found

    #if (SRC_ENFORCEMENT_EN)
        match_status = iopmpCheckPerms(0, trans_req.perm, iopmpcfg, md, trans_req.is_amo);
    #else
    // Check access permissions
        match_status = iopmpCheckPerms(trans_req.rrid, trans_req.perm, iopmpcfg, md, trans_req.is_amo);
    #endif

    // If all checks pass, return full match status
    return match_status;
}