/***************************************************************************
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
  * @brief Processes the IOPMP transaction request, traversing the SRCMD and MDCFG tables
  *        and entry array to match address and permissions.
  *
  * @param iopmp The IOPMP instance.
  * @param trans_req The transaction request with required address, permissions, etc.
  * @param intrpt Pointer to the interrupt flag.
  * @return iopmp_trans_rsp_t Response structure with transaction status.
 **/
void iopmp_validate_access(iopmp_dev_t *iopmp, iopmp_trans_req_t *trans_req, iopmp_trans_rsp_t* iopmp_trans_rsp, uint8_t *intrpt) {
    iopmp_trans_rsp->rrid         = trans_req->rrid;
    iopmp_trans_rsp->rrid_stalled = 0;
    iopmp_trans_rsp->user         = 0;
    iopmp_trans_rsp->status       = IOPMP_ERROR;
    #if (IOPMP_RRID_TRANSL_EN)
        iopmp_trans_rsp->rrid_transl = iopmp->reg_file.hwcfg3.rrid_transl;
    #endif

    // Check to block invalid combination
    if (trans_req->perm == INSTR_FETCH && trans_req->is_amo) {
        fprintf(stderr, "Instruction Fetch transaction cannot be an Atomic Memory Operation (AMO)\n");
        assert(trans_req->is_amo == 0);
    }

    iopmp->intrpt_suppress = 0;
    iopmp->error_suppress  = 0;
    int lwr_entry, upr_entry;

    #if (SRCMD_FMT == 0)
        srcmd_en_t  srcmd_en;
        srcmd_enh_t srcmd_enh;
    #endif

    iopmpMatchStatus_t iopmpMatchStatus;
    int nonPrioErrorSup    = 0;
    int nonPrioIntrSup     = 0;
    int firstIllegalAccess = 1;

    #if (ERROR_CAPTURE_EN)
        iopmpMatchStatus_t nonPrioRuleStatus;
        int nontPrioRuleNum = 0;
        nonPrioRuleStatus = NOT_HIT_ANY_RULE;
    #endif
    // IOPMP always allow the transaction when enable = 0
    if (!iopmp->reg_file.hwcfg0.enable) {
        iopmp_trans_rsp->status = IOPMP_SUCCESS;
        return ;
    }
    // Check for valid RRID; if invalid, capture error and return
    if (trans_req->rrid >= IOPMP_RRID_NUM) {
        // Initially, check for global error suppression
        iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
        #if (ERROR_CAPTURE_EN)
            errorCapture(iopmp, trans_req->perm, UNKNOWN_RRID, trans_req->rrid, 0, trans_req->addr, intrpt);
        #endif
        // In case of error suppression, success response is returned, with user defined value on initiator port
        // NOTE: You can change the `user` value
        if (iopmp->error_suppress) { iopmp_trans_rsp->status = IOPMP_SUCCESS; iopmp_trans_rsp->user = USER; }
        return ;
    }

    if (iopmp->rrid_stall[trans_req->rrid]) {
        if (iopmp->stall_cntr != STALL_BUF_DEPTH){
            iopmp_trans_rsp->rrid_stalled = 1;
            iopmp->stall_cntr++;
            return ;
        }
        else if (iopmp->reg_file.err_cfg.stall_violation_en) {
            iopmp->error_suppress = iopmp->reg_file.err_cfg.rs;
            #if (ERROR_CAPTURE_EN)
                errorCapture(iopmp, trans_req->perm, STALLED_TRANSACTION, trans_req->rrid, 0, trans_req->addr, intrpt);
            #endif
            if (iopmp->error_suppress) { iopmp_trans_rsp->status = IOPMP_SUCCESS; iopmp_trans_rsp->user = USER; }
            return ;
        }
    }

    // Read SRCMD table based on `rrid`
    #if (SRCMD_FMT == 0)
        #if (SRC_ENFORCEMENT_EN == 1)
            srcmd_en  = iopmp->reg_file.srcmd_table[0].srcmd_en;
            srcmd_enh = iopmp->reg_file.srcmd_table[0].srcmd_enh;
        #else
            srcmd_en  = iopmp->reg_file.srcmd_table[trans_req->rrid].srcmd_en;
            srcmd_enh = iopmp->reg_file.srcmd_table[trans_req->rrid].srcmd_enh;
        #endif
    #endif

    // Determine MDCFG table range for entries
    #if (SRCMD_FMT != 1)
        int start_md_num = 0;
        int end_md_num   = IOPMP_MD_NUM;
    #else
        int start_md_num = SRC_ENFORCEMENT_EN ? 0 : trans_req->rrid;
        int end_md_num   = SRC_ENFORCEMENT_EN ? 1 : trans_req->rrid + 1;
    #endif

    // Traverse each MD entry and perform address/permission checks
    for (int cur_md = start_md_num; cur_md < end_md_num; ++cur_md) {
        #if (SRCMD_FMT == 0)
            if (!IS_MD_ASSOCIATED(cur_md, srcmd_en.md, srcmd_enh.mdh)) continue;
        #endif

        #if (MDCFG_FMT == 0)
            lwr_entry = (cur_md == 0) ? 0 : iopmp->reg_file.mdcfg[cur_md - 1].t;
            upr_entry = iopmp->reg_file.mdcfg[cur_md].t;
        #else
            lwr_entry = cur_md * (iopmp->reg_file.hwcfg3.md_entry_num + 1);
            upr_entry = ((cur_md + 1) * (iopmp->reg_file.hwcfg3.md_entry_num + 1));
        #endif

        for (int cur_entry = lwr_entry; cur_entry < upr_entry; cur_entry++) {
            uint64_t prev_addr     = (cur_entry == 0) ? 0 : CONCAT32(iopmp->iopmp_entries.entry_table[cur_entry - 1].entry_addrh.addrh, iopmp->iopmp_entries.entry_table[cur_entry - 1].entry_addr.addr);
            uint64_t curr_addr     = CONCAT32(iopmp->iopmp_entries.entry_table[cur_entry].entry_addrh.addrh, iopmp->iopmp_entries.entry_table[cur_entry].entry_addr.addr);
            entry_cfg_t entry_cfg  = iopmp->iopmp_entries.entry_table[cur_entry].entry_cfg;
            #if (IOPMP_NON_PRIO_EN)
                bool is_priority_entry = (cur_entry < iopmp->reg_file.hwcfg2.prio_entry);
            #else
                bool is_priority_entry = true;
            #endif

            // Analyze entry for match
            iopmpMatchStatus = iopmpRuleAnalyzer(iopmp, *trans_req, prev_addr, curr_addr, entry_cfg, cur_md, is_priority_entry);

            if (iopmpMatchStatus == ENTRY_MATCH) {
                iopmp_trans_rsp->status = IOPMP_SUCCESS;
                return ;  // Return on successful match
            } else if (iopmpMatchStatus != ENTRY_NOTMATCH) {
                if (!is_priority_entry) {
                    #if (ERROR_CAPTURE_EN)
                        nonPrioErrorSup |= iopmp->error_suppress;
                        nonPrioIntrSup  |= iopmp->intrpt_suppress;
                        if (firstIllegalAccess) {
                            nonPrioRuleStatus  = iopmpMatchStatus;
                            nontPrioRuleNum    = cur_entry;
                            firstIllegalAccess = 0;
                        }
                    #endif
                    continue;
                }
                #if (ERROR_CAPTURE_EN)
                    errorCapture(iopmp, trans_req->perm, iopmpMatchStatus, trans_req->rrid, cur_entry, trans_req->addr, intrpt);
                #endif
                // In case of error suppression, success response is returned, with user defined value on initiator port
                // NOTE: You can change the `user` value
                if (iopmp->error_suppress) { iopmp_trans_rsp->status = IOPMP_SUCCESS; iopmp_trans_rsp->user = USER; }
                return ;  // Error found, capture and return response
            }
        }
    }

    // If No rule hits, enable error suppression based on global error suppression bit
    if (nonPrioRuleStatus == NOT_HIT_ANY_RULE) { iopmp->error_suppress = iopmp->reg_file.err_cfg.rs; }
    else { iopmp->error_suppress = nonPrioErrorSup; iopmp->intrpt_suppress = nonPrioIntrSup; }

    #if (ERROR_CAPTURE_EN)
        errorCapture(iopmp, trans_req->perm, nonPrioRuleStatus, trans_req->rrid, nontPrioRuleNum, trans_req->addr, intrpt);
    #endif
    // Return response with default status if no match/error occurs
    // In case of error suppression, success response is returned, with user defined value on initiator port
    // NOTE: You can change the `user` value
    if (iopmp->error_suppress) { iopmp_trans_rsp->status = IOPMP_SUCCESS; iopmp_trans_rsp->user = USER; }
}
