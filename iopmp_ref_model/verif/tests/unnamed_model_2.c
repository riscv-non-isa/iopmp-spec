/***************************************************************************
// Author:  Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains all the tests that are used to test
// unamed2 model.
// Max Supported RRIDs: 32
// Max Supported MDs: 63
// SRCMD_EN(H) and SPS extension registers are replaced with SRCMD_PERM(H).
// All MDs are associated with the given RRID.
// The MDCFG table is traversed for all MDs.
// Associated IOPMP entries are extracted and traversed for address matching
// and permission checks.
***************************************************************************/

#include "iopmp.h"
#include "config.h"
#include "test_utils.h"

// Declarations
iopmp_trans_req_t iopmp_trans_req;
iopmp_trans_rsp_t iopmp_trans_rsp;
err_info_t err_info_temp;

int main()
{
    uint8_t intrpt;

    FAIL_IF(create_memory(1) < 0)

#if (SRC_ENFORCEMENT_EN == 0)
    START_TEST("Test OFF - Read Access permissions");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 3, 0x30, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x01, 4);
    set_hwcfg0_enable();
    receiver_port(2, 364, 0, 0, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test OFF - Write Access permissions");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 3, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x01, 4);
    set_hwcfg0_enable();
    receiver_port(2, 364, 0, 0, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test OFF - Instruction Fetch permissions");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 3, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x01, 4);
    set_hwcfg0_enable();
    receiver_port(2, 364, 0, 0, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test OFF - UNKNOWN RRID ERROR");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 3, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x01, 4);
    set_hwcfg0_enable();
    receiver_port(70, 364, 0, 0, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, UNKNOWN_RRID);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

#if (IOPMP_TOR_EN)
    START_TEST("Test TOR - Partial hit on a priority rule error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 2, 0x10, 4); // SRCMD_PERM[2] is associated with MD[3]
    configure_mdcfg_n(2, 2, 4);                // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR, 1, (368 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x9, 4);
    set_hwcfg0_enable();
    receiver_port(2, 364, 0, 3, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, PARTIAL_HIT_ON_PRIORITY);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Read Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 2, 0x10, 4);                                      // SRCMD_PERM[2] is associated with MD[3]
    write_register(HWCFG0_OFFSET, read_register(HWCFG0_OFFSET, 4) & 0xFF04FFFF, 4); // md_entry_num set to 2
    configure_mdcfg_n(2, 2, 4);                                                     // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR, 1, (368 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x9, 4);
    set_hwcfg0_enable();
    receiver_port(2, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Write Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 2, 0xB, 4); // SRCMD_PERM[2] is associated with MD[3]
    configure_mdcfg_n(2, 2, 4);               // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR, 1, (368 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0xB, 4); // IOPMP_ENTRY[1] contains ENTRY_CFG - (TOR with read and write permissions)
    set_hwcfg0_enable();
    receiver_port(2, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Non-AMO Write Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 2, 0xB, 4); // SRCMD_PERM[2] is associated with MD[3]
    configure_mdcfg_n(2, 2, 4);               // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR, 1, (368 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0xA, 4); // IOPMP_ENTRY[1] contains ENTRY_CFG - (TOR with read and write permissions)
    set_hwcfg0_enable();
    receiver_port(2, 364, 0, 2, WRITE_ACCESS, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Write Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 2, 0xA, 4); // SRCMD_PERM[2] is associated with MD[3]
    configure_mdcfg_n(2, 2, 4);               // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR, 1, (368 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0xA, 4); // IOPMP_ENTRY[1] contains ENTRY_CFG - (TOR with write permissions)
    set_hwcfg0_enable();
    receiver_port(2, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();
#endif

    START_TEST("Test NA4 - 4Byte Read Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x11, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x11, 4);
    set_hwcfg0_enable();
    receiver_port(30, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Read Access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x10, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x10, 4);
    set_hwcfg0_enable();
    receiver_port(30, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Write Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x13, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x13, 4);
    set_hwcfg0_enable();
    receiver_port(30, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Write Access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x11, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x11, 4);
    set_hwcfg0_enable();
    receiver_port(30, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Execute Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x17, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x17, 4);
    set_hwcfg0_enable();
    receiver_port(30, 364, 0, 2, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Execute Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x13, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x13, 4);
    set_hwcfg0_enable();
    receiver_port(30, 364, 0, 2, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 8Byte Access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x11, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x11, 4);
    set_hwcfg0_enable();
    receiver_port(30, 364, 0, 3, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, PARTIAL_HIT_ON_PRIORITY);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - For exact 4 Byte error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x11, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, (364 >> 2), 4);
    configure_entry_n(ENTRY_CFG, 1, 0x11, 4);
    set_hwcfg0_enable();
    receiver_port(30, 368, 0, 0, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte read access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x19, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x19, 4);
    set_hwcfg0_enable();
    receiver_port(30, 360, 0, 3, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte read access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x18, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x18, 4);
    set_hwcfg0_enable();
    receiver_port(30, 360, 0, 3, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte write access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x18, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x18, 4);
    set_hwcfg0_enable();
    receiver_port(30, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte write access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x1B, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1B, 4);
    set_hwcfg0_enable();
    receiver_port(30, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x18, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x18, 4);
    set_hwcfg0_enable();
    receiver_port(30, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x1C, 4);
    configure_mdcfg_n(30, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(30, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access for non-priority Entry");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 29, 0x1C, 4);
    configure_mdcfg_n(29, 17, 4);
    configure_entry_n(ENTRY_ADDR, 1, 74, 4); // (300 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    configure_srcmd_n(SRCMD_PERMH, 30, 0x18, 4);
    configure_mdcfg_n(30, 25, 4);
    configure_entry_n(ENTRY_ADDR, 18, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 18, 0x18, 4);
    set_hwcfg0_enable();
    configure_entry_n(ENTRY_ADDR, 20, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 20, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(30, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test MDCFG_LCK, updating locked MDCFG field");
    reset_iopmp();
    write_register(MDCFGLCK_OFFSET, 0x8, 4); // MD[0]-MD[3] are locked
    configure_srcmd_n(SRCMD_PERM, 2, 0x1C, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test MDCFG_LCK, updating unlocked MDCFG field");
    reset_iopmp();
    write_register(MDCFGLCK_OFFSET, 0x4, 4); // MD[0]-MD[1] are locked
    configure_srcmd_n(SRCMD_PERM, 2, 0x1C, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK, updating locked ENTRY field");
    reset_iopmp();
    write_register(ENTRYLCK_OFFSET, 0x8, 4); // ENTRY[0]-ENTRY[3] are locked
    configure_srcmd_n(SRCMD_PERM, 2, 0x1C, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK, updating unlocked ENTRY field");
    reset_iopmp();
    write_register(ENTRYLCK_OFFSET, 0x8, 4); // ENTRY[0]-ENTRY[3] are locked
    configure_srcmd_n(SRCMD_PERM, 2, 0x1C, 4);
    configure_mdcfg_n(2, 5, 4);
    configure_entry_n(ENTRY_ADDR, 4, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 4, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test MDLCK register lock bit");
    reset_iopmp();
    write_register(MDLCK_OFFSET, 0x8, 4);  // MD[2] is locked
    write_register(MDLCK_OFFSET, 0x1, 4);  // Locking MDLCK register
    write_register(MDLCK_OFFSET, 0x10, 4); // Trying to lock MD[3] but it shouldn't be locked
    configure_srcmd_n(SRCMD_PERM, 2, 0x1C, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test MDCFG_LCK register lock bit");
    reset_iopmp();
    write_register(MDCFGLCK_OFFSET, 0x8, 4); // MD[0]-MD[3] are locked
    write_register(MDCFGLCK_OFFSET, 0x1, 4); // MDCFGLCK is locked
    write_register(MDCFGLCK_OFFSET, 0x4, 4); // Updating locked MD's MD[0]-MD[1] are locked
    configure_srcmd_n(SRCMD_PERM, 2, 0x1C, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK register lock bit");
    reset_iopmp();
    write_register(ENTRYLCK_OFFSET, 0x8, 4); // ENTRY[0]-ENTRY[3] are locked
    write_register(ENTRYLCK_OFFSET, 0x1, 4); // ENTRYLCK is locked
    write_register(ENTRYLCK_OFFSET, 0x2, 4); // ENTRY[0] is locked
    configure_srcmd_n(SRCMD_PERM, 2, 0x1C, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    END_TEST();

#if (IOPMP_MFR_EN)
    START_TEST("Test MFR Extension");
    write_register(ENTRYLCK_OFFSET, 0x8, 4); // ENTRY[0]-ENTRY[3] are locked
    write_register(ENTRYLCK_OFFSET, 0x1, 4); // ENTRYLCK is locked
    write_register(ENTRYLCK_OFFSET, 0x2, 4); // ENTRY[0] is locked
    configure_srcmd_n(SRCMD_PERM, 2, 0x1C, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    err_mfr_t err_mfr_temp;
    err_mfr_temp.raw = read_register(ERR_MFR_OFFSET, 4);
    FAIL_IF((err_mfr_temp.svw != 0));
    FAIL_IF((err_mfr_temp.svs != 0));
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    err_mfr_temp.raw = read_register(ERR_MFR_OFFSET, 4);
    FAIL_IF((err_mfr_temp.svi != 0));
    FAIL_IF((err_mfr_temp.svs != 1));
    FAIL_IF((err_mfr_temp.svw != 4));
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();
#endif

    START_TEST("Test Interrupt Suppression is Enabled");
    reset_iopmp();
    write_register(ERR_OFFSET, 0x2, 4);
    configure_srcmd_n(SRCMD_PERM, 31, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);  // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x99, 4); // Address Mode is NAPOT, with read permission and exe suppression
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((intrpt == 1)); // Interrupt is suppressed
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Interrupt Suppression is disabled");
    reset_iopmp();
    write_register(ERR_OFFSET, 0x2, 4);
    configure_srcmd_n(SRCMD_PERM, 31, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((intrpt == 0)); // Interrupt is not suppressed
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Error Suppression is Enabled");
    // Receiver Port Signals
    reset_iopmp();
    write_register(ERR_OFFSET, 0x4, 4);
    configure_srcmd_n(SRCMD_PERM, 31, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);                // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (SEXE | NAPOT | R), 4); // Address Mode is NAPOT, with read permission and exe suppression
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360, 1);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Error Suppression is Enabled but rs is zero");
    // Receiver Port Signals
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 2, 0x1, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);                // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (SEXE | NAPOT | R), 4); // Address Mode is NAPOT, with read permission and exe suppression
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360, 1);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Error Suppression is disabled");
    // Receiver Port Signals
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 2, 0x1, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);         // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4); // Address Mode is NAPOT, with read permission and exe suppression
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_ERROR));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != 0));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360, 1);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Interrupt and Error Suppression is Enabled");
    // Receiver Port Signals
    reset_iopmp();
    write_register(ERR_OFFSET, 0x6, 4);
    configure_srcmd_n(SRCMD_PERM, 2, 0x1, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);                       // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (SEXE | SIXE | NAPOT | R), 4); // Address Mode is NAPOT, with read permission and exe suppression
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((intrpt == 1));
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360, 0);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Interrupt and Error Suppression is disabled");
    // Receiver Port Signals
    reset_iopmp();
    write_register(ERR_OFFSET, 0x2, 4);
    configure_srcmd_n(SRCMD_PERM, 2, 0x1, 4);
    configure_mdcfg_n(2, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);         // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4); // Address Mode is NAPOT, with read permission and exe suppression
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((intrpt != 1));
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_ERROR));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360, 1);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

#if (IOPMP_STALL_EN && (STALL_BUF_DEPTH != 0) && (IMP_RRIDSCP))
    START_TEST("Stall MD Feature");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 3, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    write_register(MDSTALL_OFFSET, 0x10, 4);
    write_register(RRISCP_OFFSET, 5, 4);
    receiver_port(5, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.rrid_stalled != 1));
    rridscp_t rridscp_temp;
    rridscp_temp.raw = read_register(RRISCP_OFFSET, 4);
    FAIL_IF((rridscp_temp.stat != 1));
    FAIL_IF((iopmp_trans_rsp.rrid != 5));
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();
#elif (IOPMP_STALL_EN && IMP_RRIDSCP)
    // Set STALL_BUF_DEPTH zero to test this feature
    START_TEST("Faulting Stalled Transactions Feature");
    reset_iopmp();
    write_register(ERR_OFFSET, 0x10, 4);
    configure_srcmd_n(SRCMD_PERM, 3, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x1C, 4);
    set_hwcfg0_enable();
    write_register(MDSTALL_OFFSET, 0x10, 4);
    write_register(RRISCP_OFFSET, 5, 4);
    receiver_port(5, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.rrid_stalled == 1));
    rridscp_t rridscp_temp;
    rridscp_temp.raw = read_register(RRISCP_OFFSET, 4);
    FAIL_IF((rridscp_temp.stat != 1));
    FAIL_IF((iopmp_trans_rsp.rrid != 5));
    CHECK_IOPMP_TRANS(IOPMP_ERROR, STALLED_TRANSACTION);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();
#endif

#if (IOPMP_RRID_TRANSL_EN)
    START_TEST("Test Cascading IOPMP Feature");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERMH, 3, 0xC0000000, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, 0x18, 4);
    set_hwcfg0_enable();
    receiver_port(31, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.rrid_transl != IOPMP_RRID_TRANSL));
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();
#endif

#if (MSI_EN)
    START_TEST("Test MSI Write error");
    uint64_t read_data;
    reset_iopmp();
    bus_error = 0x8000;
    write_register(ERR_OFFSET, 0x8F0A, 4);
#if (IOPMP_ADDRH_EN)
    write_register(ERR_MSIADDR_OFFSET, 0x8000, 4);
#else
    write_register(ERR_MSIADDR_OFFSET, (0x8000 >> 2), 4);
#endif
    configure_srcmd_n(SRCMD_PERM, 31, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    bus_error = 0;
    read_memory(0x8000, 4, &read_data);
    FAIL_IF(intrpt == 1);
    FAIL_IF(read_data == 0x8F); // Interrupt is not suppressed
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test MSI");
    reset_iopmp();
    write_register(ERR_OFFSET, 0x8F0A, 4);
#if (IOPMP_ADDRH_EN)
    write_register(ERR_MSIADDR_OFFSET, 0x8000, 4);
#else
    write_register(ERR_MSIADDR_OFFSET, (0x8000 >> 2), 4);
#endif
    configure_srcmd_n(SRCMD_PERM, 31, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4);
    set_hwcfg0_enable();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // Requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    read_memory(0x8000, 4, &read_data);
    FAIL_IF(intrpt == 1);
    FAIL_IF(read_data != 0x8F); // Interrupt is not suppressed
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();
#endif
#endif

    free(memory);

#if (SRC_ENFORCEMENT_EN)
    START_TEST("Test SourceEnforcement Enable Feature");
    reset_iopmp();
    configure_srcmd_n(SRCMD_PERM, 0, 0x03, 4);
    configure_mdcfg_n(0, 2, 4);
    configure_entry_n(ENTRY_ADDR, 0, 90, 4); // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 0, (NAPOT | W | R), 4);
    set_hwcfg0_enable();
    receiver_port(31, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    receiver_port(12, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_INFO_OFFSET, 0, 4);
    END_TEST();
#endif

    return 0;
}
