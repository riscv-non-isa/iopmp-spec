/***************************************************************************
// Copyright (c) 2025 by 10xEngineers.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Authors: Mazhar Ali (mazhar.ali@10xengineers.ai)
//          Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: IOPMP Header File
// This header file defines structures, macros, and function prototypes
// for the Input/Output Physical Memory Protection (IOPMP). It includes
// the primary modes of operation for the IOPMP (off, TOR, NA4, NAPOT) and
// provides declarations for the primary functions used to process access
// requests, match addresses, check permissions, analyze rules, and handle
// error capture. Additionally, macros for testing and transaction checks
// are defined to support validation and debugging.
***************************************************************************/

#ifndef IOPMP_H
#define IOPMP_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "iopmp_ref_api.h"
#include "iopmp_registers.h"
#include "iopmp_req_rsp.h"

// IOPMP Mode Types: Define operational modes for IOPMP
#define IOPMP_OFF   0  // IOPMP is disabled
#define IOPMP_TOR   1  // Top-of-Range mode
#define IOPMP_NA4   2  // Naturally aligned 4-byte regions
#define IOPMP_NAPOT 3  // Naturally aligned power of two regions

#define BUS_ERROR     0xC
#define MSI_DATA_BYTE 0x4

#define WORD_BITS        32
#define MIN_REG_WIDTH    4

// Helper Macros for Register Calculations
#define MDCFG_TABLE_INDEX(offset)           (((offset) - MDCFG_TABLE_BASE_OFFSET) / 4)
#define SRCMD_TABLE_INDEX(offset)           (((offset) - SRCMD_TABLE_BASE_OFFSET) / SRCMD_REG_STRIDE)
#define ENTRY_TABLE_INDEX(iopmp, offset)    (((offset) - (iopmp->reg_file.entryoffset.offset)) / ENTRY_REG_STRIDE)
#define SRCMD_REG_INDEX(offset)             ((((offset) - SRCMD_TABLE_BASE_OFFSET) % SRCMD_REG_STRIDE) / MIN_REG_WIDTH)
#define ENTRY_REG_INDEX(iopmp, offset)      ((((offset) - (iopmp->reg_file.entryoffset.offset)) % ENTRY_REG_STRIDE) / MIN_REG_WIDTH)
#define IS_IN_RANGE(offset, start, end) (((offset) >= (start)) && ((offset) <= (end)))
#define CONCAT32(upr_bits, lwr_bits) (((uint64_t)upr_bits << WORD_BITS) | lwr_bits)
#define IS_MD_ASSOCIATED(md_num, srcmd_en_md, srcmd_enh_mdh) \
    ((md_num < 31) ? ((srcmd_en_md >> md_num) & 1) : ((srcmd_enh_mdh >> (md_num - 31)) & 1))

#define MASK_BIT_POS(BIT_POS) ((1U << BIT_POS) - 1)
#define GET_BIT(VAL, BIT_NUM) ((VAL >> BIT_NUM) & 1)

typedef struct iopmp_dev_t {
    iopmp_regs_t reg_file;              // Register file for IOPMP
    iopmp_entries_t iopmp_entries;      // IOPMP entry table
    err_mfrs_t err_svs;                 // Error status vector
    int intrpt_suppress;                // Set when interrupt is suppressed
    int error_suppress;                 // Set when error is suppressed
    int rrid_stall[IOPMP_MAX_RRID_NUM]; // Stall status array for requester IDs
    int stall_cntr;                     // Counts stalled transactions
    bool imp_mdlck;                     // IOPMP implements the Memory Domain Lock (MDLCK) feature
    bool imp_error_capture;             // IOPMP implements the error capture record
    bool imp_err_reqid_eid;             // IOPMP implements ERR_REQID.eid
    bool imp_rridscp;                   // IOPMP implements RRIDSCP-related features
    bool imp_msi;                       // IOPMP implements message-signaled interrupts (MSI)
} iopmp_dev_t;

// Configurations of IOPMP when reset
typedef struct iopmp_cfg_t {
    uint32_t vendor;                    // The JEDEC manufacturer ID
    uint8_t specver;                    // The specification version
    uint32_t impid;                     // The user-defined implementation ID
    bool enable;                        // IOPMP checks transactions by default
    uint8_t md_num;                     // The supported number of MD in the IOPMP
    bool addrh_en;                      // IOPMP has ENTRY_ADDRH(i) and ERR_REQADDRH
    bool tor_en;                        // IOPMP supports TOR
    uint16_t rrid_num;                  // The supported number of RRID in the IOPMP
    uint16_t entry_num;                 // The supported number of entries in the IOPMP
    uint16_t prio_entry;                // The supported number of priority entries in the IOPMP
    bool prio_ent_prog;                 // HWCFG2.prio_entry is programmable
    bool non_prio_en;                   // IOPMP supports non-priority entries
    bool chk_x;                         // IOPMP implements the check of an instruction fetch
    bool peis;                          // IOPMP implements interrupt suppression per entry
    bool pees;                          // IOPMP implements the error suppression per entry
    bool sps_en;                        // IOPMP supports secondary permission settings
    bool stall_en;                      // IOPMP implements stall-related features
    bool mfr_en;                        // IOPMP implements Multi Faults Record
    uint8_t mdcfg_fmt;                  // MDCFG Table format
    uint8_t srcmd_fmt;                  // SRCMD Table format
    uint8_t md_entry_num;               // For MDCFG format 1 and 2. Each memory domain has exactly (md_entry_num + 1) entries
    bool no_x;                          // IOPMP denies all instruction fetch transactions
    bool no_w;                          // IOPMP denies all write accesses transactions
    bool rrid_transl_en;                // IOPMP supports tag a new RRID on the initiator port
    bool rrid_transl_prog;              // HWCFG3.rrid_transl field is programmable
    uint16_t rrid_transl;               // The RRID tagged to outgoing transactions
    uint64_t entryoffset;               // The offset address of the IOPMP array from the base of an IOPMP instance
    bool imp_mdlck;                     // IOPMP implements the Memory Domain Lock (MDLCK) feature
    bool imp_error_capture;             // IOPMP implements the error capture record
    bool imp_err_reqid_eid;             // IOPMP implements ERR_REQID.eid
    bool imp_rridscp;                   // IOPMP implements RRIDSCP-related features
    bool imp_msi;                       // IOPMP implements message-signaled interrupts (MSI)
} iopmp_cfg_t;

uint8_t write_memory(uint64_t *data, uint64_t addr, uint32_t size);

// Function Declarations: Core IOPMP operations
iopmpMatchStatus_t iopmpRuleAnalyzer(iopmp_dev_t *iopmp, iopmp_trans_req_t trans_req, uint64_t prev_iopmpaddr, uint64_t iopmpaddr, entry_cfg_t iopmpcfg, uint8_t md, int is_priority);
void errorCapture(iopmp_dev_t *iopmp, perm_type_e trans_type, uint8_t error_type, uint16_t rrid, uint16_t entry_id, uint64_t err_addr, uint8_t *intrpt);
void generate_interrupt(iopmp_dev_t *iopmp, uint8_t *intrpt);

#endif // IOPMP_H
