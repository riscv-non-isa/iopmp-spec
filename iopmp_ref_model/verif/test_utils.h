/***************************************************************************
// Author: Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: 
// This header file defines macros, and function prototypes
// for the Input/Output Physical Memory Protection (IOPMP) Test file. 
***************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "iopmp.h"

#define SRCMD_EN     0x00
#define SRCMD_ENH    0x04
#define SRCMD_R      0x08
#define SRCMD_RH     0x0C
#define SRCMD_W      0x10
#define SRCMD_WH     0x14
#define SRCMD_PERM   0x00
#define SRCMD_PERMH  0x04

#define ENTRY_ADDR      0x00
#define ENTRY_ADDRH     0x04
#define ENTRY_CFG       0x08
#define ENTRY_USER_CFG  0x0C

// Permissions
#define R 0x01
#define W 0x02
#define X 0x04

// Address Mode
#define OFF   0x00
#define TOR   0x08
#define NA4   0x10
#define NAPOT 0x18

// Interrupt Suppression
#define SIRE 0x20
#define SIWE 0x40
#define SIXE 0x80

// Error Suppression
#define SERE 0x100
#define SEWE 0x200
#define SEXE 0x400

extern int test_num;
extern int8_t *memory;
extern uint64_t bus_error;

int create_memory(uint8_t mem_gb);
void configure_srcmd_n(uint8_t srcmd_reg, uint16_t srcmd_idx, reg_intf_dw data, uint8_t num_bytes);
void configure_mdcfg_n(uint8_t md_idx, reg_intf_dw data, uint8_t num_bytes);
void configure_entry_n(uint8_t entry_reg, uint64_t entry_idx, reg_intf_dw data, uint8_t num_bytes);
void receiver_port(uint16_t rrid, uint64_t addr, uint32_t length, uint32_t size, perm_type_e perm, iopmp_trans_req_t *iopmp_trans_req);
int error_record_chk(uint8_t err_type, uint8_t perm, uint64_t addr, bool err_rcd);

// Test Macros: Define macros for IOPMP testing framework
#define START_TEST(TEST_DESC)                           \
    test_num++;                                         \
    printf("Test %02d : %-61s : ", test_num, TEST_DESC)

// Fails test If condition is true.
#define FAIL_IF(CONDITION)                                                        \
    if (CONDITION) {                                                              \
        printf("Test %02d : \x1B[31mFAIL. Line %d\x1B[0m\n", test_num, __LINE__); \
        return -1;                                                                \
    }

// Must be used at the end of test.
#define END_TEST() \
    { printf("\x1B[32mPASS\x1B[0m\n"); }

// Transaction Check Macro: Verifies transaction response against expected values
#define CHECK_IOPMP_TRANS(RSP_STATUS, ERR_TYPE)                                                         \
    FAIL_IF((iopmp_trans_rsp.rrid != iopmp_trans_req.rrid));                                            \
    FAIL_IF((iopmp_trans_rsp.status != (RSP_STATUS)));                                                  \
    err_req_info_temp.raw = read_register(0x0064, 4);                                                   \
    if (iopmp_trans_rsp.status != IOPMP_SUCCESS) {                                                      \
        FAIL_IF((err_req_info_temp.v != 1));                                                            \
        FAIL_IF((err_req_info_temp.ttype != iopmp_trans_req.perm));                                     \
        FAIL_IF((err_req_info_temp.etype != (ERR_TYPE)));                                               \
        FAIL_IF((read_register(0x0068, 4) != (uint32_t)((iopmp_trans_req.addr >> 2) & 0xFFFFFFFF)));    \
        FAIL_IF((read_register(0x006C, 4) != (uint32_t)((iopmp_trans_req.addr >> 34) & 0xFFFFFFFF)));   \
    } else {                                                                                            \
        FAIL_IF((err_req_info_temp.v != 0));                                                            \
    }
