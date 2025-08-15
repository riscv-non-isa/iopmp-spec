/***************************************************************************
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
#define SRCMD_REG_STRIDE 32
#define ENTRY_REG_STRIDE 16
#define MIN_REG_WIDTH    4

// Helper Macros for Register Calculations
#define SRCMD_TABLE_INDEX(offset) (((offset) - SRCMD_TABLE_BASE_OFFSET) / SRCMD_REG_STRIDE)
#define ENTRY_TABLE_INDEX(offset) (((offset) - ENTRY_TABLE_BASE_OFFSET) / ENTRY_REG_STRIDE)
#define SRCMD_REG_INDEX(offset)   ((((offset) - SRCMD_TABLE_BASE_OFFSET) % SRCMD_REG_STRIDE) / MIN_REG_WIDTH)
#define ENTRY_REG_INDEX(offset)   ((((offset) - ENTRY_TABLE_BASE_OFFSET) % ENTRY_REG_STRIDE) / MIN_REG_WIDTH)
#define IS_IN_RANGE(offset, start, end) (((offset) >= (start)) && ((offset) <= (end)))
#define CONCAT32(upr_bits, lwr_bits) (((uint64_t)upr_bits << WORD_BITS) | lwr_bits)
#define IS_MD_ASSOCIATED(md_num, srcmd_en_md, srcmd_enh_mdh) \
    ((md_num < 31) ? ((srcmd_en_md >> md_num) & 1) : ((srcmd_enh_mdh >> (md_num - 31)) & 1))

#define MASK_BIT_POS(BIT_POS) ((1U << BIT_POS) - 1)
#define GET_BIT(VAL, BIT_NUM) ((VAL >> BIT_NUM) & 1)


// Global Variables: Definitions for IOPMP global variables
extern iopmp_regs_t g_reg_file;        // Global register file for IOPMP
extern iopmp_entries_t iopmp_entries;  // IOPMP entry table
extern err_mfrs_t err_svs;             // Error status vector
extern int intrpt_suppress;            // Set when interrupt is suppressed
extern int error_suppress;             // Set when error is suppressed
extern int rrid_stall[IOPMP_RRID_NUM]; // Stall status array for requester IDs
extern int stall_cntr;                 // Counts stalled transactions

extern uint8_t write_memory(uint64_t *data, uint64_t addr, uint32_t size);

// Function Declarations: Core IOPMP operations
int iopmpAddrRange(uint64_t *startAddr, uint64_t *endAddr, uint64_t prev_iopmpaddr, uint64_t iopmpaddr, entry_cfg_t iopmpcfg);
int iopmpMatchAddr(iopmp_trans_req_t trans_req, uint64_t lo, uint64_t hi, int is_priority);
iopmpMatchStatus_t iopmpCheckPerms(uint16_t rrid, perm_type_e req_perm, entry_cfg_t iopmpcfg, uint8_t md,  bool is_amo);
iopmpMatchStatus_t iopmpRuleAnalyzer(iopmp_trans_req_t trans_req, uint64_t prev_iopmpaddr, uint64_t iopmpaddr, entry_cfg_t iopmpcfg, uint8_t md, int is_priority);
extern void iopmp_validate_access(iopmp_trans_req_t *trans_req, iopmp_trans_rsp_t* iopmp_trans_rsp, uint8_t *intrpt);
void setRridSv(uint16_t rrid);
int checkRridSv(uint16_t rrid);
void errorCapture(perm_type_e trans_type, uint8_t error_type, uint16_t rrid, uint16_t entry_id, uint64_t err_addr, uint8_t *intrpt);
void generate_interrupt(uint8_t *intrpt);

#endif // IOPMP_H
