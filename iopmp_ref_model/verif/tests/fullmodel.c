/***************************************************************************
// Author:  Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains all the tests that are used to test
// full model.
// Max Supported RRIDs: 65536
// Max Supported MDs: 63
***************************************************************************/

#include "iopmp.h"
#include "config.h"
#include "test_utils.h"

// Declarations
// Register offset to size mapping
uint8_t g_offset_to_size[4096]; // Consider initializing this array if needed
int test_num;
iopmp_trans_req_t iopmp_trans_req;
iopmp_trans_rsp_t iopmp_trans_rsp;
err_reqinfo_t err_req_info_temp;
int8_t *memory;
uint64_t bus_error;

int main () {
    uint8_t intrpt;

    FAIL_IF(create_memory(1) < 0)

    START_TEST("Test OFF - Read Access permissions");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (OFF | R), 4);
    receiver_port(2, 364, 0, 0, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test OFF - Write Access permissions");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_W, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (OFF | R), 4);
    receiver_port(2, 364, 0, 0, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test OFF - Instruction Fetch permissions");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (OFF | R), 4);
    receiver_port(2, 364, 0, 0, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test OFF - UNKNOWN RRID ERROR");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (OFF | R), 4);
    receiver_port(70, 364, 0, 0, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, UNKNOWN_RRID);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test TOR - Partial hit on a priority rule error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);       // SRCMD_EN[2] is associated with MD[3]
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4); // SRCMD_R[2] is associated with MD[3]
    configure_mdcfg_n(3, 2, 4);           // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR,1, 368 >> 2, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, 1, (TOR | R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)
    receiver_port(2, 364, 0, 3, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, PARTIAL_HIT_ON_PRIORITY);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Read Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);       // SRCMD_EN[2] is associated with MD[3]
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4); // SRCMD_R[2] is associated with MD[3]
    configure_mdcfg_n(3, 2, 4);           // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR,1, 368 >> 2, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, 1, (TOR | R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)
    receiver_port(2, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS,ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Read Access with SRCMD_R not set");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);       // SRCMD_EN[2] is associated with MD[3]
    configure_mdcfg_n(3, 2, 4);           // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR,1, 368 >> 2, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, 1, (TOR | R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)
    receiver_port(2, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR,ILLEGAL_READ_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

#if (IOPMP_SPS_EN == 0)
    START_TEST("Test TOR - 4Byte Read Access, SRCMD_R not set, SPS disabled");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);                   // SRCMD_EN[2] is associated with MD[3]
    configure_mdcfg_n(3, 2, 4);                       // MDCFG[3].t contains 2
    write_register(HWCFG0_OFFSET, read_register(HWCFG0_OFFSET,   4) & 0xFFFFFFDF, 4);   // Disabling SPS extension
    configure_entry_n(ENTRY_ADDR,1, 368 >> 2, 4);             // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, 1, (TOR | R), 4);              // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)
    receiver_port(2, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS,ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();
#endif

    START_TEST("Test TOR - 4Byte Write Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);        // SRCMD_EN[2] is associated with MD[3]
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);  // SRCMD_R[2] is associated with MD[3]
    configure_srcmd_n(SRCMD_W, 2, 0x10, 4); // SRCMD_W[2] is associated with MD[3]
    configure_mdcfg_n(3, 2, 4);            // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR,1, 368 >> 2, 4);  // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, 1, (TOR | W | R), 4);   // IOPMP_ENTRY[1] contains ENTRY_CFG - (TOR with read and write permissions)
    receiver_port(2, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS,ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Write Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);        // SRCMD_EN[2] is associated with MD[3]
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);  // SRCMD_R[2] is associated with MD[3]
    configure_srcmd_n(SRCMD_W, 2, 0x10, 4); // SRCMD_W[2] is associated with MD[3]
    configure_mdcfg_n(3, 2, 4);            // MDCFG[3].t contains 2
    configure_entry_n(ENTRY_ADDR,1, 368 >> 2, 4);  // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, 1, (TOR | W), 4);   // IOPMP_ENTRY[1] contains ENTRY_CFG - (TOR with write permissions)
    receiver_port(2, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR,ILLEGAL_WRITE_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Read Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | R), 4);
    receiver_port(32, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Read Access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4), 4);
    receiver_port(32, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No SPS Read Access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x00, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | R), 4);
    receiver_port(32, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Write Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_W, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | W | R), 4);
    receiver_port(32, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Write Access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_W, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | R), 4);
    receiver_port(32, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No SPS Write Access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_W, 32, 0x00, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | W | R), 4);
    receiver_port(32, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Execute Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | X | W | R), 4);
    receiver_port(32, 364, 0, 2, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Execute Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | W | R), 4);
    receiver_port(32, 364, 0, 2, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No SPS.R, Execute Access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x00, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | X | W | R), 4);
    receiver_port(32, 364, 0, 2, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 8Byte Access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | R), 4);
    receiver_port(32, 364, 0, 3, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, PARTIAL_HIT_ON_PRIORITY);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - For exact 4 Byte error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR,1, 364 >> 2, 4);
    configure_entry_n(ENTRY_CFG, 1, (NA4 | R), 4);
    receiver_port(32, 368, 0, 0, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte read access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4);
    receiver_port(32, 360, 0, 3, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte read access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT), 4);
    receiver_port(32, 360, 0, 3, READ_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte write access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_W, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT), 4);
    receiver_port(32, 360, 0, 3, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte write access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_W, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | W | R), 4);
    receiver_port(32, 360, 0, 3, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access error");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT), 4);
    receiver_port(32, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(32, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access for non-priority Entry");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 31, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 31, 0x10, 4);
    configure_mdcfg_n(3, 17, 4);
    configure_entry_n(ENTRY_ADDR, 1,  74, 4);    // (300 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    
    configure_srcmd_n(SRCMD_EN, 32, 0x20, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x20, 4);
    configure_mdcfg_n(4, 25, 4);
    configure_entry_n(ENTRY_ADDR, 18,  90, 4);    // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 18, (NAPOT), 4);

    configure_entry_n(ENTRY_ADDR, 20,  90, 4);    // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 20, (NAPOT | X), 4);
    receiver_port(32, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test MDLCK, updating locked srcmd_en field");
    reset_iopmp();
    write_register(MDLCK_OFFSET,   0x10, 4);
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test MDLCK, updating unlocked srcmd_en field");
    reset_iopmp();
    write_register(MDLCK_OFFSET,   0x8, 4);
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test MDCFG_LCK, updating locked MDCFG field");
    reset_iopmp();
    write_register(MDCFGLCK_OFFSET,   0x8, 4);   // MD[0]-MD[3] are locked
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test MDCFG_LCK, updating unlocked MDCFG field");
    reset_iopmp();
    write_register(MDCFGLCK_OFFSET,   0x4, 4);   // MD[0]-MD[1] are locked
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK, updating locked ENTRY field");
    reset_iopmp();
    write_register(ENTRYLCK_OFFSET,   0x8, 4);   // ENTRY[0]-ENTRY[3] are locked
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK, updating unlocked ENTRY field");
    reset_iopmp();
    write_register(ENTRYLCK_OFFSET,   0x8, 4);   // ENTRY[0]-ENTRY[3] are locked
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 5, 4);
    configure_entry_n(ENTRY_ADDR, 4,  90, 4);    // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 4, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test SRCMD_EN lock bit, updating locked SRCMD Table");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 2, 0x1, 4);   // SRCMD_EN[2] lock bit is set
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test SRCMD_EN lock bit, updating unlocked SRCMD Table");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 1, 0x1, 4);   // SRCMD_EN[1] lock bit is set
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 5, 4);
    configure_entry_n(ENTRY_ADDR, 4,  90, 4);    // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 4, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test MDLCK register lock bit");
    reset_iopmp();
    write_register(MDLCK_OFFSET,   0x8, 4);          // MD[2] is locked
    write_register(MDLCK_OFFSET,   0x1, 4);          // Locking MDLCK register
    write_register(MDLCK_OFFSET,   0x10, 4);         // Trying to lock MD[3] but it shouldn't be locked
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();


    START_TEST("Test MDCFG_LCK register lock bit");
    reset_iopmp();
    write_register(MDCFGLCK_OFFSET,   0x8, 4);   // MD[0]-MD[3] are locked
    write_register(MDCFGLCK_OFFSET,   0x1, 4);   // MDCFGLCK is locked
    write_register(MDCFGLCK_OFFSET,   0x4, 4);   // Updating locked MD's MD[0]-MD[1] are locked
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK register lock bit");
    reset_iopmp();
    write_register(ENTRYLCK_OFFSET,   0x8, 4);   // ENTRY[0]-ENTRY[3] are locked
    write_register(ENTRYLCK_OFFSET,   0x1, 4);   // ENTRYLCK is locked
    write_register(ENTRYLCK_OFFSET,   0x2, 4);   // ENTRY[0] is locked
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    END_TEST();


    START_TEST("Test MFR Extension");
    write_register(ENTRYLCK_OFFSET,   0x8, 4);   // ENTRY[0]-ENTRY[3] are locked
    write_register(ENTRYLCK_OFFSET,   0x1, 4);   // ENTRYLCK is locked
    write_register(ENTRYLCK_OFFSET,   0x2, 4);   // ENTRY[0] is locked
    configure_srcmd_n(SRCMD_EN, 2, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 2, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    err_mfr_t err_mfr_temp;
    err_mfr_temp.raw = read_register(ERR_MFR_OFFSET, 4);
    FAIL_IF((err_mfr_temp.svw != 0));
    FAIL_IF((err_mfr_temp.svs != 0));
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    err_mfr_temp.raw = read_register(ERR_MFR_OFFSET, 4);
    FAIL_IF((err_mfr_temp.svi != iopmp_trans_req.rrid));
    FAIL_IF((err_mfr_temp.svs != 1));
    FAIL_IF((err_mfr_temp.svw != 1));
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test MDLCK, updating locked srcmd_enh field");
    reset_iopmp();
    write_register(MDLCKH_OFFSET,   0x1, 4);
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test MDLCK, updating unlocked srcmd_enh field");
    reset_iopmp();
    write_register(MDLCKH_OFFSET,   0x2, 4);
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST(); 

    START_TEST("Test Interrupt Suppression is Enabled");
    reset_iopmp();
    write_register(ERR_OFFSET,   0x2, 4);
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (SIXE | NAPOT | R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((intrpt == 1)); // Interrupt is suppressed
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Interrupt Suppression is disabled");
    reset_iopmp();
    write_register(ERR_OFFSET,   0x2, 4);
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((intrpt == 0)); // Interrupt is not suppressed
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();   

    START_TEST("Test Error Suppression is Enabled");
    // Receiver Port Signals
    reset_iopmp();
    write_register(ERR_OFFSET,   0x4, 4);
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (SEXE | NAPOT | R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360,1);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Error Suppression is Enabled but rs is zero");
    // Receiver Port Signals
    reset_iopmp();
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (SEXE | NAPOT | R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360,1);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Error Suppression is disabled");
    // Receiver Port Signals
    reset_iopmp();
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_ERROR));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != 0));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360,1);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Interrupt and Error Suppression is Enabled");
    // Receiver Port Signals
    reset_iopmp();
    write_register(ERR_OFFSET,   0x6, 4);
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (SEXE | SIXE | NAPOT | R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((intrpt == 1));
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360,0);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Interrupt and Error Suppression is disabled");
    // Receiver Port Signals
    reset_iopmp();
    write_register(ERR_OFFSET,   0x2, 4);
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((intrpt != 1)); 
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_ERROR));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360,1);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();   

    START_TEST("Stall MD Feature");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 5, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 5, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | X), 4);
    write_register(MDSTALL_OFFSET, 0x10, 4);
    write_register(RRISCP_OFFSET,5,4);
    receiver_port(5, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
    
    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((iopmp_trans_rsp.rrid_stalled != 1));
    rridscp_t rridscp_temp;
    rridscp_temp.raw = read_register(RRISCP_OFFSET,4);
    FAIL_IF((rridscp_temp.stat != 1));
    FAIL_IF((iopmp_trans_rsp.rrid != 5));
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST(); 

    START_TEST("Test Cascading IOPMP Feature");
    reset_iopmp();
    configure_srcmd_n(SRCMD_EN, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_R, 32, 0x10, 4);
    configure_srcmd_n(SRCMD_W, 32, 0x10, 4);
    configure_mdcfg_n(3, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);   // (364 >> 2) and keeping lsb 0
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | W | R), 4);
    receiver_port(32, 360, 0, 3, WRITE_ACCESS, &iopmp_trans_req);

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    FAIL_IF((iopmp_trans_rsp.rrid_transl != IOPMP_RRID_TRANSL));
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

#if (MSI_EN)
    START_TEST("Test MSI");
    uint32_t read_data;
    reset_iopmp();
    write_register(ERR_OFFSET, 0x8F0A, 4);
    write_register(ERR_MSIADDR_OFFSET, 0x2000, 4);
    configure_srcmd_n(SRCMD_ENH, 2, 0x1, 4);
    configure_srcmd_n(SRCMD_RH, 2, 0x1, 4);
    configure_mdcfg_n(31, 2, 4);
    configure_entry_n(ENTRY_ADDR, 1, 90, 4);
    configure_entry_n(ENTRY_CFG, 1, (NAPOT | R), 4);
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    read_memory(0x8000, 4, (char *)&read_data);
    FAIL_IF(intrpt == 1);
    FAIL_IF(read_data != 0x8F); // Interrupt is not suppressed
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();
#endif
    free(memory);

    return 0;
}