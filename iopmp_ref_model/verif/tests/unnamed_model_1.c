/***************************************************************************
// Author: Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains all the tests that are used to test
// unamed1 model.
// Max Supported RRIDs: 63
// Max Supported MDs: 63
// There is no physical SRCMD table. RRID i directly maps to MD i.
// There is no physical MDCFG table. Each MD has k associated IOPMP entries.
// Value of k is programmable. Associated IOPMP Entry Ranges:
// (i × k) to ((i + 1) × k - 1) for address matching and permission checks.
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
    receiver_port(2, 364, 0, 0, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (368 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (OFF), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test OFF - Write Access permissions");
    reset_iopmp();
    receiver_port(2, 364, 0, 0, WRITE_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (368 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (OFF|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test OFF - Instruction Fetch permissions");
    reset_iopmp();
    receiver_port(2, 364, 0, 0, INSTR_FETCH, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (368 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (OFF), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test OFF - UNKNOWN RRID ERROR");
    reset_iopmp();
    receiver_port(70, 364, 0, 0, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (368 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (OFF), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, UNKNOWN_RRID);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test TOR - Partial hit on a priority rule error");
    reset_iopmp();
    receiver_port(2, 364, 0, 3, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (368 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (TOR|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, PARTIAL_HIT_ON_PRIORITY);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Read Access");
    reset_iopmp();
    receiver_port(2, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (368 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (TOR|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS,ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Write Access");
    reset_iopmp();
    write_register(HWCFG0_OFFSET, read_register(HWCFG0_OFFSET,   4) & 0xFF04FFFF, 4);   // md_entry_num set to 2
      
    receiver_port(2, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (368 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (TOR|W|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS,ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test TOR - 4Byte Only Write Access");
    reset_iopmp();
         
    receiver_port(2, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (368 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (TOR|W), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR,ILLEGAL_WRITE_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Read Access");
    reset_iopmp();
        
    receiver_port(30, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (364 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NA4|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)
    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Read Access error");
    reset_iopmp();
         
    receiver_port(32, 364, 0, 2, READ_ACCESS, &iopmp_trans_req);
    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (364 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NA4), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Write Access");
    // Reset IOPMP
    reset_iopmp();

    receiver_port(32, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (364 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NA4|W|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Write Access error");
    // Reset
    reset_iopmp();

    receiver_port(32, 364, 0, 2, WRITE_ACCESS, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (364 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NA4|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Execute Access");
    // Reset IOPMP
    reset_iopmp();

    receiver_port(32, 364, 0, 2, INSTR_FETCH, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (364 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NA4|X|W|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Execute Access");
    // Reset IOPMP
    reset_iopmp();

    receiver_port(32, 364, 0, 2, INSTR_FETCH, &iopmp_trans_req);

    // Entry Table CFG  
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (364 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NA4|W|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - 8Byte Access error");
    // Reset
    reset_iopmp();

    receiver_port(2, 364, 0, 3, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG 
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (364 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NA4|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, PARTIAL_HIT_ON_PRIORITY);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NA4 - For exact 4 Byte error");
    // Reset
    reset_iopmp();

    receiver_port(32, 368, 0, 0, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG 
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (364 >> 2), 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NA4|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte read access");
    // Reset IOPMP
    reset_iopmp();
    
    receiver_port(32, 360, 0, 3, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG 
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte read access error");
    // Reset IOPMP
    reset_iopmp();
    
    receiver_port(32, 360, 0, 3, READ_ACCESS, &iopmp_trans_req);

    // Entry Table CFG 
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT|W), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte write access error");
    // Reset IOPMP
    reset_iopmp();

    receiver_port(32, 360, 0, 3, WRITE_ACCESS, &iopmp_trans_req);

    // Entry Table CFG 
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte write access");
    // Reset IOPMP
    reset_iopmp();
   
    receiver_port(32, 360, 0, 3, WRITE_ACCESS, &iopmp_trans_req);

    // Entry Table CFG 
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT|W|R), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access error");
    reset_iopmp();
    receiver_port(32, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Entry Table CFG 
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access");
    // Reset
    reset_iopmp();
    receiver_port(32, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
    // Entry Table CFG 
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT|X), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access for non-priority Entry");
    // Receiver Port Signals
    reset_iopmp();
    receiver_port(32, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 74, 4);  // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 0x1C, 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);  // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 0x18, 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);  // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 0x1C, 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK, updating locked ENTRY field");
    // Reset IOPMP
    reset_iopmp();

    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Entry Table CFG 
    write_register(ENTRYLCK_OFFSET,   0x1000, 4);   // ENTRY[0]-ENTRY[15] are locked
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT|X), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK, updating unlocked ENTRY field");
    
    reset_iopmp();

    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Entry Table CFG 
    write_register(ENTRYLCK_OFFSET,   0x8, 4);   // ENTRY[0]-ENTRY[15] are locked
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT|X), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(ERR_REQINFO_OFFSET,   0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK register lock bit");

    reset_iopmp();
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

    // Entry Table CFG 
    write_register(ENTRYLCK_OFFSET,   0x1000, 4);   // ENTRY[0]-ENTRY[15] are locked
    write_register(ENTRYLCK_OFFSET,   0x1, 4);   // ENTRYLCK is locked
    write_register(ENTRYLCK_OFFSET,   0x2, 4);   // ENTRY[0] is locked
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT|X), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)

    // requestor Port Signals
    iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
    CHECK_IOPMP_TRANS(IOPMP_ERROR, NOT_HIT_ANY_RULE);
    END_TEST();

    START_TEST("Test MFR Extension");
    receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
    // Entry Table CFG 
    write_register(ENTRYLCK_OFFSET,   0x8, 4);   // ENTRY[0]-ENTRY[3] are locked
    write_register(ENTRYLCK_OFFSET,   0x1, 4);   // ENTRYLCK is locked
    write_register(ENTRYLCK_OFFSET,   0x2, 4);   // ENTRY[0] is locked
    configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), 90, 4); // IOPMP_ENTRY[1] contains top range 92
    configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM+1)), (NAPOT|X), 4);  // IOPMP_ENTRY[1] contains ENTRY_CFG - 9 (TOR with read permissions)


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

    START_TEST("Test Interrupt Suppression is Enabled");
   reset_iopmp();
   write_register(ERR_OFFSET,   0x2, 4);
   receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);

   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);   // (364 >> 2) and keeping lsb 0
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 0x99, 4);    // Address Mode is NAPOT, with read permission and exe suppression
   
   // Requestor Port Signals
   iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
   FAIL_IF((intrpt == 1)); // Interrupt is suppressed
   CHECK_IOPMP_TRANS(IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
   write_register(ERR_REQINFO_OFFSET,   0, 4);
   END_TEST();

   START_TEST("Test Interrupt Suppression is disabled");
   reset_iopmp();
   write_register(ERR_OFFSET,   0x2, 4);
   receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), (NAPOT|R), 4);
   

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
   receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);   // (364 >> 2) and keeping lsb 0
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), (SEXE|NAPOT|R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
   

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
   receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);   // (364 >> 2) and keeping lsb 0
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), (SEXE|NAPOT|R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
   

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
   receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);   // (364 >> 2) and keeping lsb 0
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), (NAPOT|R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
   

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
   receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);   // (364 >> 2) and keeping lsb 0
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), (SEXE | SIXE | NAPOT | R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
   

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
   receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);   // (364 >> 2) and keeping lsb 0
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), (NAPOT | R), 4);    // Address Mode is NAPOT, with read permission and exe suppression
   

   // Requestor Port Signals
   iopmp_trans_rsp = iopmp_validate_access(iopmp_trans_req, &intrpt);
   FAIL_IF((intrpt != 1)); 
   FAIL_IF((iopmp_trans_rsp.status != IOPMP_ERROR));
   FAIL_IF((iopmp_trans_rsp.rrid != 2));
   error_record_chk(ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360,1);
   write_register(ERR_REQINFO_OFFSET,   0, 4);
   END_TEST();   

   START_TEST("Test Cascading IOPMP Feature");
   reset_iopmp();
   receiver_port(32, 360, 0, 3, WRITE_ACCESS, &iopmp_trans_req);
   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);   // (364 >> 2) and keeping lsb 0
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 0x1B, 4);

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
   receiver_port(2, 360, 0, 3, INSTR_FETCH, &iopmp_trans_req);
   configure_entry_n(ENTRY_ADDR, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), 90, 4);
   configure_entry_n(ENTRY_CFG, (iopmp_trans_req.rrid * (IOPMP_MD_ENTRY_NUM + 1)), (NAPOT|R), 4);

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