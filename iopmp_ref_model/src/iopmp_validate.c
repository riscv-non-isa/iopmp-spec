/***************************************************************************
// Copyright (c) 2025 by 10xEngineers.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Author: Gull Ahmed (gull.ahmed@10xengineers.ai)
//         Yazan Hussnain (yazan.hussain@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains the iopmp_validate_access function that
// Processes the IOPMP transaction request, traversing the SRCMD and MDCFG tables
// and entry array to match address and permissions and return the response
// structure based upon the transaction status.
***************************************************************************/

#include "iopmp.h"

/**
* @brief Translate the type of requested permission by transaction to IOPMP error type
*
* @param perm Type of permission requested by transaction
* @return iopmpErrorType_t enum representing the IOPMP error type
 */
static iopmpErrorType_t perm_to_etype(perm_type_e perm)
{
    if (perm == WRITE_ACCESS)
        return ILLEGAL_WRITE_ACCESS;
    else if (perm == INSTR_FETCH)
        return ILLEGAL_INSTR_FETCH;
    return ILLEGAL_READ_ACCESS;
}

/**
  * @brief Processes the IOPMP transaction request, traversing the SRCMD and MDCFG tables
  *        and entry array to match address and permissions.
  *
  * @param iopmp The IOPMP instance.
  * @param trans_req The transaction request with required address, permissions, etc.
  * @param intrpt Pointer to the variable to store wired interrupt flag.
  *               This flag is set to 1 if the following conditions are true:
  *                 - the transaction fails
  *                 - a primary error capture occurs
  *                 - the interrupts are not suppressed
  *                 - IOPMP doesn't implement MSI extension, or MSI is not enabled
  *               This flag is set to 0 if the following conditions are true:
  *                 - this transaction fails
  *                 - a primary error capture occurs
  *                 - the interrupts are suppressed, or IOPMP implements MSI extension
  *                   and triggers MSI instead of wired interrupt
  * @return iopmp_trans_rsp_t Response structure with transaction status.
 **/
void iopmp_validate_access(iopmp_dev_t *iopmp, iopmp_trans_req_t *trans_req, iopmp_trans_rsp_t* iopmp_trans_rsp, uint8_t *intrpt) {
    iopmp_trans_rsp->rrid         = trans_req->rrid;
    iopmp_trans_rsp->rrid_stalled = 0;
    iopmp_trans_rsp->user         = 0;
    iopmp_trans_rsp->status       = IOPMP_ERROR;
    iopmp_trans_rsp->rrid_transl  = trans_req->rrid;

    // Check to block invalid combination
    if (trans_req->perm == INSTR_FETCH && trans_req->is_amo) {
        fprintf(stderr, "Instruction Fetch transaction cannot be an Atomic Memory Operation (AMO)\n");
        assert(trans_req->is_amo == 0);
    }

    iopmpErrorType_t error_type = NO_ERROR;
    uint16_t error_eid = 0;

    iopmp->intrpt_suppress = 0;
    iopmp->error_suppress  = 0;
    int lwr_entry, upr_entry;

    srcmd_en_t  srcmd_en;
    srcmd_enh_t srcmd_enh;

    iopmpMatchStatus_t iopmpMatchStatus;
    int nonPrioErrorSup    = 0;
    int nonPrioIntrSup     = 0;
    int firstIllegalAccess = 1;
    iopmpErrorType_t nonPrioRuleStatus = NOT_HIT_ANY_RULE;
    int nonPrioRuleNum = 0;

    // IOPMP always allow the transaction when enable = 0
    if (!iopmp->reg_file.hwcfg0.enable) {
        goto pass_checks;
    }

    // Tag a new RRID which represents that the transaction has been checked.
    // The RRID translation takes effect when IOPMP checker is enabled.
    if (iopmp->reg_file.hwcfg3.rrid_transl_en) {
        iopmp_trans_rsp->rrid_transl = iopmp->reg_file.hwcfg3.rrid_transl;
    }

    // Check for valid RRID; if invalid, capture error and return
    if (trans_req->rrid >= iopmp->reg_file.hwcfg1.rrid_num) {
        // Initially, check for global error suppression
        iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
        error_type = UNKNOWN_RRID;
        goto stop_and_report_fault;
    }

    if (iopmp->rrid_stall[trans_req->rrid]) {
        if (iopmp->stall_cntr != STALL_BUF_DEPTH){
            iopmp_trans_rsp->rrid_stalled = 1;
            iopmp->stall_cntr++;
            return ;
        }
        else if (iopmp->reg_file.err_cfg.stall_violation_en) {
            iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
            error_type = STALLED_TRANSACTION;
            goto stop_and_report_fault;
        }
    }

    // When no_w is set to 1, the IOPMP denies all write transactions regardless
    // of entry rule configurations, reporting them with error type
    // "not hit any rule" (0x05).
    if (trans_req->perm == WRITE_ACCESS && iopmp->reg_file.hwcfg3.no_w) {
        iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
        error_type = NOT_HIT_ANY_RULE;
        goto stop_and_report_fault;
    }

    if (trans_req->perm == INSTR_FETCH) {
        // When chk_x and no_x are set to 1, the IOPMP denies all instruction
        // fetch transactions regardless of entry rule configurations, reporting
        // them with error type "not hit any rule" (0x05).
        if (iopmp->reg_file.hwcfg2.chk_x && iopmp->reg_file.hwcfg3.no_x) {
            iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
            error_type = NOT_HIT_ANY_RULE;
            goto stop_and_report_fault;
        }
    }

    // Read SRCMD table based on `rrid`
    if (iopmp->reg_file.hwcfg3.srcmd_fmt == 0) {
        #if (SRC_ENFORCEMENT_EN == 1)
            srcmd_en  = iopmp->reg_file.srcmd_table[0].srcmd_en;
            srcmd_enh = iopmp->reg_file.srcmd_table[0].srcmd_enh;
        #else
            srcmd_en  = iopmp->reg_file.srcmd_table[trans_req->rrid].srcmd_en;
            srcmd_enh = iopmp->reg_file.srcmd_table[trans_req->rrid].srcmd_enh;
        #endif
    }

    int start_md_num;
    int end_md_num;
    // Determine MDCFG table range for entries
    if (iopmp->reg_file.hwcfg3.srcmd_fmt == 0 ||
        iopmp->reg_file.hwcfg3.srcmd_fmt == 2) {
        start_md_num = 0;
        end_md_num   = iopmp->reg_file.hwcfg0.md_num;
    } else if (iopmp->reg_file.hwcfg3.srcmd_fmt == 1) {
        start_md_num = SRC_ENFORCEMENT_EN ? 0 : trans_req->rrid;
        end_md_num   = SRC_ENFORCEMENT_EN ? 1 : trans_req->rrid + 1;
    }

    // Traverse each MD entry and perform address/permission checks
    for (int cur_md = start_md_num; cur_md < end_md_num; ++cur_md) {
        if (iopmp->reg_file.hwcfg3.srcmd_fmt == 0) {
            if (!IS_MD_ASSOCIATED(cur_md, srcmd_en.md, srcmd_enh.mdh)) continue;
        }

        if (iopmp->reg_file.hwcfg3.mdcfg_fmt == 0) {
            lwr_entry = (cur_md == 0) ? 0 : iopmp->reg_file.mdcfg[cur_md - 1].t;
            upr_entry = iopmp->reg_file.mdcfg[cur_md].t;
        } else {
            lwr_entry = cur_md * (iopmp->reg_file.hwcfg3.md_entry_num + 1);
            upr_entry = ((cur_md + 1) * (iopmp->reg_file.hwcfg3.md_entry_num + 1));
        }

        for (int cur_entry = lwr_entry; cur_entry < upr_entry; cur_entry++) {
            uint64_t prev_addr     = (cur_entry == 0) ? 0 : CONCAT32(iopmp->iopmp_entries.entry_table[cur_entry - 1].entry_addrh.addrh, iopmp->iopmp_entries.entry_table[cur_entry - 1].entry_addr.addr);
            uint64_t curr_addr     = CONCAT32(iopmp->iopmp_entries.entry_table[cur_entry].entry_addrh.addrh, iopmp->iopmp_entries.entry_table[cur_entry].entry_addr.addr);
            entry_cfg_t entry_cfg  = iopmp->iopmp_entries.entry_table[cur_entry].entry_cfg;

            // Analyze entry for match
            iopmpMatchStatus = iopmpRuleAnalyzer(iopmp, *trans_req, prev_addr, curr_addr, entry_cfg, cur_md);
            if (iopmpMatchStatus == ENTRY_MATCH_GRANT) {
                // If the entry matches all bytes of the transaction and grants
                // transaction permission to operate, the transaction is legal.
                goto pass_checks;
            } else if (iopmpMatchStatus == ENTRY_PARTIAL_MATCH) {
                // If the partial matching entry is non-priority entry, just
                // keep checking next entry.
                if (iopmp->reg_file.hwcfg2.non_prio_en && cur_entry >= iopmp->reg_file.hwcfg2.prio_entry)
                    goto check_next_entry;

                // If the partial matching entry is priority entry, check fails.
                // The priority entry must match all bytes of a transaction, or
                // transaction is illegal with error type =
                // "partial hit on a priority rule" (0x04).
                iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
                error_type = PARTIAL_HIT_ON_PRIORITY;
                error_eid  = cur_entry;
                goto stop_and_report_fault;
            } else if (iopmpMatchStatus == ENTRY_MATCH_NOTGRANT) {
                // If the matching entry is non-priority entry but doesn't grant
                // transaction permission to operate, the model records this
                // access as "first illegal access". This "first illegal access"
                // will be reported if the subsequent entries still fail the
                // checks. If no matching entry permits, the transaction is
                // illegal.
                if (iopmp->reg_file.hwcfg2.non_prio_en && cur_entry >= iopmp->reg_file.hwcfg2.prio_entry) {
                    nonPrioErrorSup |= iopmp->error_suppress;
                    nonPrioIntrSup  |= iopmp->intrpt_suppress;
                    if (firstIllegalAccess) {
                        nonPrioRuleStatus = perm_to_etype(trans_req->perm);
                        nonPrioRuleNum    = cur_entry;
                        firstIllegalAccess = 0;
                    }
                    goto check_next_entry;
                }

                // If the matching entry is priority entry but doesn't grant
                // transaction permission to operate, the transaction is illegal
                // with error type = "illegal read access" (0x01) for read
                // access transaction, "illegal write access/AMO" (0x02) for
                // write access/atomic memory operation (AMO) transaction.
                error_type = perm_to_etype(trans_req->perm);
                error_eid  = cur_entry;
                goto stop_and_report_fault;
            }

            // ENTRY_NOTMATCH: Keep checking next entry
check_next_entry:
        }
    }

    // If No rule hits, enable error suppression based on global error suppression bit
    if (iopmp->reg_file.hwcfg2.non_prio_en) {
        if (nonPrioRuleStatus == NOT_HIT_ANY_RULE) {
            // None of the non-priority entries fully matches the transaction
            iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
            error_type = NOT_HIT_ANY_RULE;
        } else {
            // At least one non-priority entry fully matches the transaction but
            // doesn't grant transaction permission.
            // The IOPMP specification says:
            // If no matching entry permits, the transaction is illegal with
            // error type = "illegal read access" (0x01) for read access
            // transaction or "illegal write access/AMO" (0x02) for write
            // access/AMO transaction.
            iopmp->error_suppress = nonPrioErrorSup;
            iopmp->intrpt_suppress = nonPrioIntrSup;
            error_type = nonPrioRuleStatus;
            error_eid  = nonPrioRuleNum;
        }
    } else {
        iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
        error_type = NOT_HIT_ANY_RULE;
    }
    goto stop_and_report_fault;

pass_checks:
    iopmp_trans_rsp->status = IOPMP_SUCCESS;
    return;

stop_and_report_fault:
    // If IOPMP implements error capture feature, IOPMP triggers error capture
    // to log the error information into the registers.
    if (iopmp->imp_error_capture) {
        errorCapture(iopmp, trans_req->perm, error_type, trans_req->rrid, error_eid, trans_req->addr, intrpt);
    }
    // Return response with default status if no match/error occurs
    // In case of error suppression, success response is returned, with user defined value on initiator port
    // NOTE: You can change the `user` value
    if (iopmp->error_suppress) {
        iopmp_trans_rsp->status = IOPMP_SUCCESS;
        iopmp_trans_rsp->user = USER;
    }
}
