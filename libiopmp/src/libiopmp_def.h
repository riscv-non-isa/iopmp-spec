/*
 * Copyright 2018-2025 Andes Technology Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __LIBIOPMP_DEF_H__
#define __LIBIOPMP_DEF_H__

#include "libiopmp.h"

/******************************************************************************/
/* IOPMP operations                                                           */
/******************************************************************************/
/** Structure represents the generic operations for all the models */
struct iopmp_operations_generic {
    /** Lock the number of priority entries. */
    void (*lock_prio_entry_num)(IOPMP_t *iopmp);

    /** Lock the RRID tagged to outgoing transactions. */
    void (*lock_rrid_transl)(IOPMP_t *iopmp);

    /** Enable this IOPMP protection. */
    void (*enable)(IOPMP_t *iopmp);

    /** Set the maximum number of priority entries. */
    enum iopmp_error (*set_prio_entry_num)(IOPMP_t *iopmp, uint16_t *num_entry);

    /** Set the RRID tagged to outgoing transactions. */
    enum iopmp_error (*set_rrid_transl)(IOPMP_t *iopmp, uint16_t *rrid_transl);

    /** Stall the transactions related to MDs bitmap. */
    enum iopmp_error (*stall_by_mds)(IOPMP_t *iopmp, uint64_t *mds,
                                     bool exempt, bool polling);

    /** Resume the stalled transactions of stall_by_mds(). */
    enum iopmp_error (*resume_transactions)(IOPMP_t *iopmp, bool polling);

    /** Poll until MDSTALL.is_busy == 0. */
    bool (*poll_mdstall)(IOPMP_t *iopmp, bool polling, bool stall_or_resume);

    /** Write RRIDSCP with given op and return the stat. */
    enum iopmp_error (*set_rridscp)(IOPMP_t *iopmp, uint32_t *rrid,
                                    enum iopmp_rridscp_op op,
                                    enum iopmp_rridscp_stat *stat);

    /** Lock entry[0] ~ entry[@entry_num - 1]. Lock ENTRYLCK if @lock is true */
    enum iopmp_error (*lock_entries)(IOPMP_t *iopmp, uint32_t *entry_num,
                                     bool lock);

    /** Lock ERR_CFG/ERRREACT register. */
    void (*lock_err_cfg)(IOPMP_t *iopmp);

    /** Enable/disable global interrupt. */
    void (*set_global_intr)(IOPMP_t *iopmp, bool enable);

    /** Suppress/express global error responses. */
    enum iopmp_error (*set_global_err_resp)(IOPMP_t *iopmp, bool *suppress);

    /** Set MSI enable/disable. */
    enum iopmp_error (*set_msi_en)(IOPMP_t *iopmp, bool *enable);

    /** Set MSI address and data. */
    enum iopmp_error (*set_msi_info)(IOPMP_t *iopmp, uint64_t *msiaddr64,
                                     uint16_t *msidata);

    /** Get MSI write error and clear the flag. */
    void (*get_and_clear_msi_werr)(IOPMP_t *iopmp, bool *msi_werr);

    /** Set stall_violation_en. */
    enum iopmp_error (*set_stall_violation_en)(IOPMP_t *iopmp, bool *enable);

    /** Capture a pending IOPMP error. */
    enum iopmp_error (*capture_error)(IOPMP_t *iopmp,
                                      IOPMP_ERR_REPORT_t *err_report,
                                      bool clear);

    /** Clear error record valid bit. */
    void (*invalidate_error)(IOPMP_t *iopmp);

    /** For MFR extension. Get subsequent violation window. */
    enum iopmp_error (*get_sv_window)(IOPMP_t *iopmp, uint16_t *svi,
                                      uint16_t *svw);

    /**
     * Set the values of entry[@idx_start]~entry[@idx_start+@num_entry-1] from
     * given @entry_array to IOPMP instance.
     */
    enum iopmp_error (*set_entries)(IOPMP_t *iopmp,
                                    const struct iopmp_entry *entry_array,
                                    uint32_t idx_start, uint32_t num_entry);

    /**
     * Get the values of entry[@index_start]~entry[@idx_start+@num_entry-1] to
     * given @entry_array from IOPMP instance.
     */
    void (*get_entries)(IOPMP_t *iopmp, struct iopmp_entry *entry_array,
                        uint32_t idx_start, uint32_t num_entry);

    /** Clear the values of entry[@idx_start]~entry[@idx_start+@num_entry-1] */
    void (*clear_entries)(IOPMP_t *iopmp, uint32_t idx_start,
                          uint32_t num_entry);
};

/** Structure represents the operations for specific model */
struct iopmp_operations_specific {
    /** Lock the MDs based on given bitmap @mds. Lock MDLCK if @lock is true. */
    enum iopmp_error (*set_md_lock)(IOPMP_t *iopmp, uint64_t *mds, bool lock);

    /** Lock MDCFG(0) ~ MDCFG(@md_num - 1). Lock MDCFGLCK if @lock is true. */
    enum iopmp_error (*lock_mdcfg)(IOPMP_t *iopmp, uint32_t *md_num, bool lock);

    /** Get association of RRID and multiple MDs from IOPMP */
    void (*get_association_rrid_md)(IOPMP_t *iopmp, uint32_t rrid,
                                    uint64_t *mds, bool *lock);

    /** Set association of RRID and multiple MDs into IOPMP */
    enum iopmp_error (*set_association_rrid_md)(IOPMP_t *iopmp, uint32_t rrid,
                                                uint64_t *mds, bool lock);

    /** Set single RRID's read, write and instruction permissions to MD */
    enum iopmp_error (*set_md_permission)(IOPMP_t *iopmp, uint32_t rrid,
                                          uint32_t mdidx, bool *r, bool *w);

    /** Set multiple RRIDs' read, write and instruction permissions to MD */
    enum iopmp_error (*set_md_permission_multi)(IOPMP_t *iopmp, uint32_t mdidx,
                                                IOPMP_SRCMD_PERM_CFG_t *cfg);

    /** Lock SRCMD table for SRCMD_FMT=0 or SRCMD_FMT=2*/
    enum iopmp_error (*lock_srcmd_table)(IOPMP_t *iopmp, uint32_t rrid,
                                         uint32_t mdidx);

    /** For mdcfg_fmt=0. Get the top range (index of entry) of IOPMP MD */
    void (*get_md_entry_top)(IOPMP_t *iopmp, uint32_t mdidx,
                             uint32_t *entry_top);

    /** For mdcfg_fmt=0. Set the top range (index of entry) of IOPMP MD */
    enum iopmp_error (*set_md_entry_top)(IOPMP_t *iopmp, uint32_t mdidx,
                                         uint32_t *entry_top);

    /** For mdcfg_fmt=2 (Dynamic-K model). Set md_entry_num */
    enum iopmp_error (*set_md_entry_num)(IOPMP_t *iopmp,
                                         uint32_t *md_entry_num);
};

/** Structure represents the operations for SPS extension */
struct iopmp_operations_sps {
    /** Get 64-bit value of {SRCMD_RH(rrid), SRCMD_R(rrid)}.md */
    uint64_t (*sps_get_srcmd_r_64_md)(IOPMP_t *iopmp, uint32_t rrid);
    /** Set 64-bit value of {SRCMD_RH(rrid), SRCMD_R(rrid)}.md */
    enum iopmp_error (*sps_set_srcmd_r_64_md)(IOPMP_t *iopmp, uint32_t rrid,
                                              uint64_t *mds);
    /** Get 64-bit value of {SRCMD_WH(rrid), SRCMD_W(rrid)}.md */
    uint64_t (*sps_get_srcmd_w_64_md)(IOPMP_t *iopmp, uint32_t rrid);
    /** Set 64-bit value of {SRCMD_WH(rrid), SRCMD_W(rrid)}.md */
    enum iopmp_error (*sps_set_srcmd_w_64_md)(IOPMP_t *iopmp, uint32_t rrid,
                                              uint64_t *mds);
};

#endif  /* __LIBIOPMP_DEF_H__ */
