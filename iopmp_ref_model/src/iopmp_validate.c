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

iopmp_regs_t    g_reg_file;
iopmp_entries_t iopmp_entries;
#if (IOPMP_MFR_EN)
err_mfrs_t      err_svs;
#endif
int             intrpt_suppress;
int             error_suppress;
int             stall_cntr;

/**
  * @brief Processes the IOPMP transaction request, traversing the SRCMD and MDCFG tables
  *        and entry array to match address and permissions.
  *
  * @param trans_req The transaction request with required address, permissions, etc.
  * @param intrpt Pointer to the interrupt flag.
  * @return iopmp_trans_rsp_t Response structure with transaction status.
 **/
void iopmp_validate_access(iopmp_trans_req_t *trans_req, iopmp_trans_rsp_t* iopmp_trans_rsp, uint8_t *intrpt) {
    iopmp_trans_rsp->rrid         = trans_req->rrid;
    iopmp_trans_rsp->rrid_stalled = 0;
    iopmp_trans_rsp->user         = 0;
    iopmp_trans_rsp->status       = IOPMP_ERROR;
    #if (IOPMP_RRID_TRANSL_EN)
        iopmp_trans_rsp->rrid_transl = g_reg_file.hwcfg3.rrid_transl;
    #endif

    // Check to block invalid combination
    if (trans_req->perm == INSTR_FETCH && trans_req->is_amo) {
        fprintf(stderr, "Instruction Fetch transaction cannot be an Atomic Memory Operation (AMO)\n");
        assert(trans_req->is_amo == 0);
    }

    intrpt_suppress = 0;
    error_suppress  = 0;
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
    if (!g_reg_file.hwcfg0.enable) {
        iopmp_trans_rsp->status = IOPMP_SUCCESS;
        return ;
    }
    // Check for valid RRID; if invalid, capture error and return
    if (trans_req->rrid >= IOPMP_RRID_NUM) {
        // Initially, check for global error suppression
        error_suppress = g_reg_file.err_cfg.rs;
        #if (ERROR_CAPTURE_EN)
            errorCapture(trans_req->perm, UNKNOWN_RRID, trans_req->rrid, 0, trans_req->addr, intrpt);
        #endif
        // In case of error suppression, success response is returned, with user defined value on initiator port
        // NOTE: You can change the `user` value
        if (error_suppress) { iopmp_trans_rsp->status = IOPMP_SUCCESS; iopmp_trans_rsp->user = USER; }
        return ;
    }

    if (rrid_stall[trans_req->rrid]) {
        if (stall_cntr != STALL_BUF_DEPTH){
            iopmp_trans_rsp->rrid_stalled = 1;
            stall_cntr++;
            return ;
        }
        else if (g_reg_file.err_cfg.stall_violation_en) {
            error_suppress = g_reg_file.err_cfg.rs;
            #if (ERROR_CAPTURE_EN)
                errorCapture(trans_req->perm, STALLED_TRANSACTION, trans_req->rrid, 0, trans_req->addr, intrpt);
            #endif
            if (error_suppress) { iopmp_trans_rsp->status = IOPMP_SUCCESS; iopmp_trans_rsp->user = USER; }
            return ;
        }
    }

    // Read SRCMD table based on `rrid`
    #if (SRCMD_FMT == 0)
        #if (SRC_ENFORCEMENT_EN == 1)
            srcmd_en  = g_reg_file.srcmd_table[0].srcmd_en;
            srcmd_enh = g_reg_file.srcmd_table[0].srcmd_enh;
        #else
            srcmd_en  = g_reg_file.srcmd_table[trans_req->rrid].srcmd_en;
            srcmd_enh = g_reg_file.srcmd_table[trans_req->rrid].srcmd_enh;
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
            lwr_entry = (cur_md == 0) ? 0 : g_reg_file.mdcfg[cur_md - 1].t;
            upr_entry = g_reg_file.mdcfg[cur_md].t;
        #else
            lwr_entry = cur_md * (g_reg_file.hwcfg3.md_entry_num + 1);
            upr_entry = ((cur_md + 1) * (g_reg_file.hwcfg3.md_entry_num + 1));
        #endif

        for (int cur_entry = lwr_entry; cur_entry <= upr_entry; cur_entry++) {
            uint64_t prev_addr     = (cur_entry == 0) ? 0 : CONCAT32(iopmp_entries.entry_table[cur_entry - 1].entry_addrh.addrh, iopmp_entries.entry_table[cur_entry - 1].entry_addr.addr);
            uint64_t curr_addr     = CONCAT32(iopmp_entries.entry_table[cur_entry].entry_addrh.addrh, iopmp_entries.entry_table[cur_entry].entry_addr.addr);
            entry_cfg_t entry_cfg  = iopmp_entries.entry_table[cur_entry].entry_cfg;
            #if (IOPMP_NON_PRIO_EN)
                bool is_priority_entry = (cur_entry < g_reg_file.hwcfg2.prio_entry);
            #else
                bool is_priority_entry = true;
            #endif

            // Analyze entry for match
            iopmpMatchStatus = iopmpRuleAnalyzer(*trans_req, prev_addr, curr_addr, entry_cfg, cur_md, is_priority_entry);

            if (iopmpMatchStatus == ENTRY_MATCH) {
                iopmp_trans_rsp->status = IOPMP_SUCCESS;
                return ;  // Return on successful match
            } else if (iopmpMatchStatus != ENTRY_NOTMATCH) {
                if (!is_priority_entry) {
                    #if (ERROR_CAPTURE_EN)
                        nonPrioErrorSup    |= error_suppress;
                        nonPrioIntrSup     |= intrpt_suppress;
                        if (firstIllegalAccess) {
                            nonPrioRuleStatus  = iopmpMatchStatus;
                            nontPrioRuleNum    = cur_entry;
                            firstIllegalAccess = 0;
                        }
                    #endif
                    continue;
                }
                #if (ERROR_CAPTURE_EN)
                    errorCapture(trans_req->perm, iopmpMatchStatus, trans_req->rrid, cur_entry, trans_req->addr, intrpt);
                #endif
                // In case of error suppression, success response is returned, with user defined value on initiator port
                // NOTE: You can change the `user` value
                if (error_suppress) { iopmp_trans_rsp->status = IOPMP_SUCCESS; iopmp_trans_rsp->user = USER; }
                return ;  // Error found, capture and return response
            }
        }
    }

    // If No rule hits, enable error suppression based on global error suppression bit
    if (nonPrioRuleStatus == NOT_HIT_ANY_RULE) { error_suppress = g_reg_file.err_cfg.rs; }
    else { error_suppress = nonPrioErrorSup; intrpt_suppress = nonPrioIntrSup; }

    #if (ERROR_CAPTURE_EN)
        errorCapture(trans_req->perm, nonPrioRuleStatus, trans_req->rrid, nontPrioRuleNum, trans_req->addr, intrpt);
    #endif
    // Return response with default status if no match/error occurs
    // In case of error suppression, success response is returned, with user defined value on initiator port
    // NOTE: You can change the `user` value
    if (error_suppress) { iopmp_trans_rsp->status = IOPMP_SUCCESS; iopmp_trans_rsp->user = USER; }
}
