/***************************************************************************
// Author: Yazan Hussnain (yazan.hussain@10xengineers.ai)
// Date: October 24, 2024
// Description: IOPMP Error Capture Module
// This file contains the implementation for error capturing in the
// Input/Output Physical Memory Protection (IOPMP). The `errorCapture`
// function is responsible for logging error information when a transaction
// request encounters an access violation or permission-related issue. It
// stores details such as transaction type, error type, request ID, entry ID,
// and the address where the error occurred. Additionally, the function
// detects and flags subsequent violations and can trigger an interrupt if
// necessary.
***************************************************************************/

#include "iopmp.h"

/**
  * @brief Captures and logs error information for a transaction request.
  *
  * @param trans_type Type of the transaction request (read/write permissions).
  * @param error_type Specific error type encountered during the transaction.
  * @param rrid Requester ID associated with the transaction.
  * @param entry_id IOPMP entry ID where the error was encountered.
  * @param err_addr Address at which the error occurred.
  * @param intrpt Pointer to an interrupt flag, which is set if an error is captured.
 **/
void errorCapture(perm_type_e trans_type, uint8_t error_type, uint16_t rrid, uint16_t entry_id, uint64_t err_addr, uint8_t *intrpt) {
    int err_reqinfo_v = g_reg_file.err_info.v;
    // If no error has been logged and interrupt and error both are not suppressed, capture error details
    if (!g_reg_file.err_info.v && (!error_suppress | !intrpt_suppress)) {
        g_reg_file.err_info.v     = 1;               // Mark error as captured
        // Set error status and transaction details
        g_reg_file.err_info.ttype = trans_type;      // Transaction type (read/write)
        g_reg_file.err_info.etype = error_type;      // Specific error type

        // Capture lower and upper parts of error address
        g_reg_file.err_reqaddr.addr   = (uint32_t)((err_addr >> 2) & UINT32_MAX);         // Error address [33:2]
        g_reg_file.err_reqaddrh.addrh = (uint32_t)((err_addr >> 34) & UINT32_MAX);        // Error address [65:34]

        // Record Request ID and Entry ID details
        g_reg_file.err_reqid.rrid = rrid;
        g_reg_file.err_reqid.eid  = entry_id;

    // If an error was previously logged, handle a subsequent violation
    } else if ((err_svs.sv[rrid].svw != MAX_SVS) && g_reg_file.hwcfg0.mfr_en && (!error_suppress | !intrpt_suppress)) {
        // Update violation window counter
        err_svs.sv[rrid].svw |= (err_svs.sv[rrid].svw + 1);  // Increment violation count
    }

    // Check for any subsequent violation and set err_info.svc
    for (int i = 0; i < IOPMP_RRID_NUM; i++) {
        if (err_svs.sv[i].svw) {
            g_reg_file.err_info.svc = 1;
            break;
        }
    }
    
    // Generate Interrupt
    if (!err_reqinfo_v)
        generate_interrupt(intrpt);
}
