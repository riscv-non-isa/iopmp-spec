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

#include <string.h>

#include "libiopmp.h"
#include "libiopmp_def.h"

#include "iopmp_drv_common.h"
#include "iopmp_util.h"

extern const struct iopmp_driver *const iopmp_drivers[];

#define IS_ALIGNED(x, a)    (((x) & ((a) - 1)) == 0)

#define MAX_MD_ENTRY_NUM    ((1UL << 7) - 1)

#define IOPMP_ADDR_SHIFT    2

int libiopmp_major_version(void)
{
    return LIBIOPMP_VERSION_MAJOR;
}

int libiopmp_minor_version(void)
{
    return LIBIOPMP_VERSION_MINOR;
}

int libiopmp_extra_version(void)
{
    return LIBIOPMP_VERSION_EXTRA;
}

bool libiopmp_check_version(int major, int minor, int extra)
{
    if (major > libiopmp_major_version())
        return true;

    if (major == libiopmp_major_version() && minor > libiopmp_minor_version())
        return true;

    if (major == libiopmp_major_version() &&
        minor == libiopmp_minor_version() && extra > libiopmp_extra_version())
        return true;

    return false;
}

/**
 * \brief Check if the model of IOPMP is Rapid-K, Dynamic-K, or Compact-K
 *
 * \param[in] iopmp             The IOPMP instance to be checked
 *
 * \retval 1 if model is xxx-K
 * \retval 0 if model is not xxx-K
 */
static bool __is_k_model(IOPMP_t *iopmp)
{
    return iopmp->mdcfg_fmt == IOPMP_MDCFG_FMT_1 ||
           iopmp->mdcfg_fmt == IOPMP_MDCFG_FMT_2;
}

enum iopmp_error iopmp_init(IOPMP_t *iopmp, uintptr_t addr, uint8_t srcmd_fmt,
                            uint8_t mdcfg_fmt, uint32_t impid)
{
    const struct iopmp_driver *drv;

    /* Scan each driver to find appropriate one for initialization */
    for (int i = 0; iopmp_drivers[i]; i++) {
        drv = iopmp_drivers[i];
        if (srcmd_fmt == drv->srcmd_fmt && mdcfg_fmt == drv->mdcfg_fmt &&
            impid == drv->impid)
            goto found_driver;
    }

    return IOPMP_ERR_NOT_SUPPORTED;

found_driver:
    memset(iopmp, 0, sizeof(*iopmp));

    assert(drv->init != NULL);
    return drv->init(iopmp, addr);
}

enum iopmp_error iopmp_get_vendor_id(IOPMP_t *iopmp, uint32_t *vendor)
{
    assert(iopmp_is_initialized(iopmp));

    if (!vendor)
        return IOPMP_ERR_INVALID_PARAMETER;

    *vendor = iopmp->vendor;
    return IOPMP_OK;
}

enum iopmp_error iopmp_get_specver(IOPMP_t *iopmp, uint32_t *specver)
{
    assert(iopmp_is_initialized(iopmp));

    if (!specver)
        return IOPMP_ERR_INVALID_PARAMETER;

    *specver = iopmp->specver;
    return IOPMP_OK;
}

enum iopmp_error iopmp_get_impid(IOPMP_t *iopmp, uint32_t *impid)
{
    assert(iopmp_is_initialized(iopmp));

    if (!impid)
        return IOPMP_ERR_INVALID_PARAMETER;

    *impid = iopmp->impid;
    return IOPMP_OK;
}

enum iopmp_error iopmp_lock_prio_entry_num(IOPMP_t *iopmp)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->prient_prog)
        return IOPMP_OK;

    /* If HWCFG0.prient_prog is not wired to 0, this operation is mandatory */
    assert(iopmp->ops_generic->lock_prio_entry_num);
    iopmp->ops_generic->lock_prio_entry_num(iopmp);
    iopmp->prient_prog = false; /* update local cache */

    return IOPMP_OK;
}

enum iopmp_error iopmp_lock_rrid_transl(IOPMP_t *iopmp)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->rrid_transl_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!iopmp->rrid_transl_prog)
        return IOPMP_OK;

    /*
     * If HWCFG0.rrid_transl_prog is not wired to 0, this operation is mandatory
     */
    assert(iopmp->ops_generic->lock_rrid_transl);
    iopmp->ops_generic->lock_rrid_transl(iopmp);
    iopmp->rrid_transl_prog = false;    /* update local cache */

    return IOPMP_OK;
}

enum iopmp_error iopmp_set_enable(IOPMP_t *iopmp)
{
    assert(iopmp_is_initialized(iopmp));

    /* Already enabled. */
    if (iopmp->enable)
        return IOPMP_OK;

    /* HWCFG0.enable is mandatory W1SS bit */
    assert(iopmp->ops_generic->enable);
    iopmp->ops_generic->enable(iopmp);
    iopmp->enable = true;   /* update local cache */

    return IOPMP_OK;
}

enum iopmp_error iopmp_set_prio_entry_num(IOPMP_t *iopmp, uint16_t *num_entry)
{
    enum iopmp_error ret;

    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->prient_prog)
        return IOPMP_ERR_REG_IS_LOCKED;

    if (!num_entry)
        return IOPMP_ERR_INVALID_PARAMETER;

    /*
     * If HWCFG0.rrid_transl_prog is not wired to 0, this operation is mandatory
     */
    assert(iopmp->ops_generic->set_prio_entry_num);
    ret = iopmp->ops_generic->set_prio_entry_num(iopmp, num_entry);
    /* HWCFG2.prio_entry is WARL field. We always update local cache for it */
    iopmp->prio_entry_num = *num_entry;

    return ret;
}

enum iopmp_error iopmp_set_rrid_transl(IOPMP_t *iopmp, uint16_t *rrid_transl)
{
    enum iopmp_error ret;

    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->rrid_transl_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!iopmp->rrid_transl_prog)
        return IOPMP_ERR_REG_IS_LOCKED;

    if (!rrid_transl)
        return IOPMP_ERR_INVALID_PARAMETER;

    /*
     * If HWCFG0.rrid_transl_en is not wired to 0, this operation is mandatory
     */
    assert(iopmp->ops_generic->set_rrid_transl);
    ret = iopmp->ops_generic->set_rrid_transl(iopmp, rrid_transl);
    /* HWCFG2.rrid_transl is WARL field. We always update local cache for it */
    iopmp->rrid_transl = *rrid_transl;

    return ret;
}

enum iopmp_error iopmp_stall_transactions_by_mds(IOPMP_t *iopmp, uint64_t *mds,
                                                 bool exempt, bool polling)
{
    enum iopmp_error ret;

    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->support_stall_by_md)
        return IOPMP_ERR_NOT_SUPPORTED;

    /*
     * According to the specification:
     * MDSTALL can be written at most once and before a resume. Writing a
     * non-zero value to MDSTALL multiple times after a resume leads to RRIDs'
     * stall states being undefined.
     * Thus, we forbid the operation if IOPMP is stalling some transactions.
     */
    if (iopmp->is_stalling)
        return IOPMP_ERR_NOT_ALLOWED;

    assert(iopmp->ops_generic->stall_by_mds);
    ret = iopmp->ops_generic->stall_by_mds(iopmp, mds, exempt, polling);
    if (ret == IOPMP_OK)
        iopmp->is_stalling = true;

    return ret;
}

enum iopmp_error iopmp_resume_transactions(IOPMP_t *iopmp, bool polling)
{
    enum iopmp_error ret;

    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->support_stall_by_md)
        return IOPMP_ERR_NOT_SUPPORTED;

    /* We forbid the operation if IOPMP was not stalling any transactions */
    if (!iopmp->is_stalling)
        return IOPMP_ERR_NOT_ALLOWED;

    assert(iopmp->ops_generic->resume_transactions);
    ret = iopmp->ops_generic->resume_transactions(iopmp, polling);
    if (ret == IOPMP_OK)
        iopmp->is_stalling = false;

    return ret;
}

static enum iopmp_error __iopmp_poll_mdstall(IOPMP_t *iopmp,
                                             bool polling,
                                             bool stall_or_resume)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->support_stall_by_md)
        return IOPMP_ERR_NOT_SUPPORTED;

    assert(iopmp->ops_generic->poll_mdstall);
    return iopmp->ops_generic->poll_mdstall(iopmp, polling, stall_or_resume);
}

enum iopmp_error iopmp_transactions_are_stalled(IOPMP_t *iopmp, bool polling)
{
    if (!iopmp->is_stalling)
        return IOPMP_ERR_NOT_EXIST;

    return __iopmp_poll_mdstall(iopmp, polling, true);
}

enum iopmp_error iopmp_transactions_are_resumed(IOPMP_t *iopmp, bool polling)
{
    if (iopmp->is_stalling)
        return IOPMP_ERR_NOT_EXIST;

    return __iopmp_poll_mdstall(iopmp, polling, false);
}

/**
 * \brief Write RRIDSCP with given op and return the stat
 *
 * \param[in] iopmp             The IOPMP instance to be set
 * \param[in,out] rrid          Input the RRID to be stalled. Output WARL value
 * \param[in] op                The desired operation of RRIDSCP
 * \param[out] stat             The pointer to store enum iopmp_rridscp_stat
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_INVALID_PARAMETER if given \p rrid is NULL or invalid
 * \retval IOPMP_ERR_OUT_OF_BOUNDS if given \p rrid is out of bounds
 * \retval IOPMP_ERR_NOT_SUPPORTED if \p iopmp does not support stall by RRID
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p rrid does not match the
 *         actual value. The actual value is output via \p rrid
 */
static enum iopmp_error __iopmp_set_rridscp(IOPMP_t *iopmp, uint32_t *rrid,
                                            enum iopmp_rridscp_op op,
                                            enum iopmp_rridscp_stat *stat)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->support_stall_by_rrid)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!rrid || !stat)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (*rrid >= iopmp->rrid_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    assert(iopmp->ops_generic->set_rridscp);
    return iopmp->ops_generic->set_rridscp(iopmp, rrid, op, stat);
}

enum iopmp_error iopmp_stall_cherry_pick_rrid(IOPMP_t *iopmp, uint32_t *rrid,
                                              bool select,
                                              enum iopmp_rridscp_stat *stat)
{
    enum iopmp_rridscp_op op;

    op = select ? IOPMP_RRIDSCP_OP_STALL : IOPMP_RRIDSCP_OP_DONT_STALL;
    return __iopmp_set_rridscp(iopmp, rrid, op, stat);
}

enum iopmp_error iopmp_query_stall_stat_by_rrid(IOPMP_t *iopmp, uint32_t *rrid,
                                                enum iopmp_rridscp_stat *stat)
{
    return __iopmp_set_rridscp(iopmp, rrid, IOPMP_RRIDSCP_OP_QUERY, stat);
}

enum iopmp_error iopmp_get_locked_md(IOPMP_t *iopmp, uint64_t *mds,
                                     bool *mdlck_lock)
{
    assert(iopmp_is_initialized(iopmp));

    if (!mds || !mdlck_lock)
        return IOPMP_ERR_INVALID_PARAMETER;

    *mds = iopmp->mdlck_md;
    *mdlck_lock = iopmp->mdlck_lock;

    return IOPMP_OK;
}

enum iopmp_error iopmp_lock_md(IOPMP_t *iopmp, uint64_t *mds, bool mdlck_lock)
{
    enum iopmp_error ret;
    uint64_t valid_mdlck_md_mask;
    uint64_t __mds;

    assert(iopmp_is_initialized(iopmp));

    if (!mds)
        return IOPMP_ERR_INVALID_PARAMETER;

    __mds = *mds;

    /* Early return for both zero input */
    if (!__mds && !mdlck_lock)
        return IOPMP_OK;

    /* Check if given 'mds' contains unsupported MD bits */
    valid_mdlck_md_mask = ((uint64_t)1 << iopmp->md_num) - 1;
    if (__mds > valid_mdlck_md_mask)
        return IOPMP_ERR_NOT_SUPPORTED;

    /* Already locked */
    if (iopmp->mdlck_lock) {
        if ((__mds & iopmp->mdlck_md) == __mds &&
            mdlck_lock == iopmp->mdlck_lock)
            return IOPMP_OK;
        else
            return IOPMP_ERR_REG_IS_LOCKED;
    }

    assert(iopmp->ops_specific->set_md_lock);
    ret = iopmp->ops_specific->set_md_lock(iopmp, mds, mdlck_lock);
    /*
     * MDLCK.md and MDLCKH.mdh are WARL fields. We always update local data
     * cache for them.
     */
    iopmp->mdlck_lock = mdlck_lock;
    iopmp->mdlck_md = *mds;

    return ret;
}

enum iopmp_error iopmp_lock_mdcfg(IOPMP_t *iopmp, uint32_t *md_num, bool lock)
{
    enum iopmp_error ret;
    uint32_t mdcfglck_f;

    assert(iopmp_is_initialized(iopmp));

    if (iopmp->mdcfg_fmt != IOPMP_MDCFG_FMT_0)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!md_num)
        return IOPMP_ERR_INVALID_PARAMETER;

    mdcfglck_f = *md_num;

    if (iopmp->mdcfglck_f == mdcfglck_f && iopmp->mdcfglck_lock == lock)
        return IOPMP_OK;                /* MDCFGLCK is already what we want */

    if (mdcfglck_f > iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (iopmp->mdcfglck_lock)
        return IOPMP_ERR_REG_IS_LOCKED; /* Already locked */

    if (iopmp->mdcfglck_f > mdcfglck_f)
        return IOPMP_ERR_NOT_ALLOWED;   /* Should be monotonically increased */

    /* MDCFGLCK is mandatory register in MDCFG_FMT=0 */
    assert(iopmp->ops_specific->lock_mdcfg);

    ret = iopmp->ops_specific->lock_mdcfg(iopmp, md_num, lock);
    /* Update local data cache */
    iopmp->mdcfglck_lock = lock;
    iopmp->mdcfglck_f = *md_num;

    return ret;
}

enum iopmp_error iopmp_is_mdcfglck_locked(IOPMP_t *iopmp, bool *locked)
{
    assert(iopmp_is_initialized(iopmp));

    if (iopmp->mdcfg_fmt != IOPMP_MDCFG_FMT_0)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!locked)
        return IOPMP_ERR_INVALID_PARAMETER;

    *locked = iopmp->mdcfglck_lock;
    return IOPMP_OK;
}

enum iopmp_error iopmp_get_locked_mdcfg_num(IOPMP_t *iopmp, uint32_t *md_num)
{
    assert(iopmp_is_initialized(iopmp));

    if (iopmp->mdcfg_fmt != IOPMP_MDCFG_FMT_0)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!md_num)
        return IOPMP_ERR_INVALID_PARAMETER;

    *md_num = iopmp->mdcfglck_f;
    return IOPMP_OK;
}

enum iopmp_error iopmp_lock_entries(IOPMP_t *iopmp, uint32_t *entry_num,
                                    bool lock)
{
    enum iopmp_error ret;
    uint32_t __entry_num;

    assert(iopmp_is_initialized(iopmp));

    if (!entry_num)
        return IOPMP_ERR_INVALID_PARAMETER;

    __entry_num = *entry_num;

    if (iopmp->entrylck_f == __entry_num && iopmp->entrylck_lock == lock)
        return IOPMP_OK;                /* ENTRYLCK is already what we want */

    if (__entry_num > iopmp->entry_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (iopmp->entrylck_lock)
        return IOPMP_ERR_REG_IS_LOCKED; /* Already locked */

    if (iopmp->entrylck_f > __entry_num)
        return IOPMP_ERR_NOT_ALLOWED;   /* Should be monotonically increased */

    /* ENTRYLCK is mandatory register */
    assert(iopmp->ops_generic->lock_entries);

    ret = iopmp->ops_generic->lock_entries(iopmp, entry_num, lock);
    iopmp->entrylck_lock = lock;
    iopmp->entrylck_f = *entry_num;

    return ret;
}

enum iopmp_error iopmp_lock_err_cfg(IOPMP_t *iopmp)
{
    assert(iopmp_is_initialized(iopmp));

    /* Already locked? */
    if (iopmp->err_cfg_lock)
        return IOPMP_OK;

    /* ERR_CFG.l is mandatory W1SS bit */
    assert(iopmp->ops_generic->lock_err_cfg);

    iopmp->ops_generic->lock_err_cfg(iopmp);
    iopmp->err_cfg_lock = true; /* update local cache */

    return IOPMP_OK;
}

enum iopmp_error iopmp_set_global_intr(IOPMP_t *iopmp, bool enable)
{
    assert(iopmp_is_initialized(iopmp));

    /* Already enabled or disabled? */
    if (iopmp->intr_enable == enable)
        return IOPMP_OK;

    /* Already locked? */
    if (iopmp->err_cfg_lock)
        return IOPMP_ERR_REG_IS_LOCKED;

    /* ERR_CFG.ie is mandatory RW bit */
    assert(iopmp->ops_generic->set_global_intr);

    iopmp->ops_generic->set_global_intr(iopmp, enable);
    iopmp->intr_enable = enable;    /* update local cache */

    return IOPMP_OK;
}

enum iopmp_error iopmp_set_global_err_resp(IOPMP_t *iopmp, bool *suppress)
{
    enum iopmp_error ret;
    bool __suppress;

    assert(iopmp_is_initialized(iopmp));

    if (!suppress)
        return IOPMP_ERR_INVALID_PARAMETER;

    __suppress = *suppress;

    /* Already suppressed or expressed? */
    if (iopmp->err_resp_suppress == __suppress)
        return IOPMP_OK;

    /* Already locked? */
    if (iopmp->err_cfg_lock)
        return IOPMP_ERR_REG_IS_LOCKED;

    /* ERR_CFG.rs is optional */
    if (!iopmp->ops_generic->set_global_err_resp)
        return IOPMP_ERR_NOT_SUPPORTED;

    ret = iopmp->ops_generic->set_global_err_resp(iopmp, suppress);
    iopmp->err_resp_suppress = *suppress;   /* update local cache */

    return ret;
}

enum iopmp_error iopmp_set_msi_en(IOPMP_t *iopmp, bool *enable)
{
    enum iopmp_error ret;
    bool __enable;

    assert(iopmp_is_initialized(iopmp));

    if (!enable)
        return IOPMP_ERR_INVALID_PARAMETER;

    __enable = *enable;

    /* Already enabled or disabled? */
    if (iopmp->msi_en == __enable)
        return IOPMP_OK;

    /* Already locked? */
    if (iopmp->err_cfg_lock)
        return IOPMP_ERR_REG_IS_LOCKED;

    /* ERR_CFG.msi_en is optional */
    if (!iopmp->ops_generic->set_msi_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    ret = iopmp->ops_generic->set_msi_en(iopmp, enable);
    iopmp->msi_en = *enable;    /* update local cache */

    return ret;
}

enum iopmp_error iopmp_get_msi_addr(IOPMP_t *iopmp, uint64_t *msiaddr64)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->msi_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!msiaddr64)
        return IOPMP_ERR_INVALID_PARAMETER;

    *msiaddr64 = iopmp->msiaddr64;

    return IOPMP_OK;
}

enum iopmp_error iopmp_get_msi_data(IOPMP_t *iopmp, uint16_t *msidata)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->msi_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!msidata)
        return IOPMP_ERR_INVALID_PARAMETER;

    *msidata = iopmp->msidata;

    return IOPMP_OK;
}

enum iopmp_error iopmp_set_msi_info(IOPMP_t *iopmp, uint64_t *msiaddr64,
                                    uint16_t *msidata)
{
    enum iopmp_error ret;
    uint64_t __msiaddr64;
    uint16_t __msidata;

    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->msi_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!msiaddr64 || !msidata)
        return IOPMP_ERR_INVALID_PARAMETER;

    __msiaddr64 = *msiaddr64;
    __msidata = *msidata;

    /* Already set? */
    if (iopmp->msiaddr64 == __msiaddr64 && iopmp->msidata == __msidata)
        return IOPMP_OK;

    /*
     * If HWCFG0.addrh_en=0, IOPMP only implements ERR_MSIADDR which contains
     * bits 33 to 2 of the address.
     */
    if (!iopmp->addrh_en && __msiaddr64 > 0x3FFFFFFFF)
        return IOPMP_ERR_NOT_SUPPORTED;

    /* ERR_CFG.msidata only supports maximum to 11 bits */
    if (__msidata > 0x7FF)
        return IOPMP_ERR_NOT_SUPPORTED;

    /* Already locked? */
    if (iopmp->err_cfg_lock)
        return IOPMP_ERR_REG_IS_LOCKED;

    assert(iopmp->ops_generic->set_msi_info);
    ret = iopmp->ops_generic->set_msi_info(iopmp, msiaddr64, msidata);
    /*
     * ERR_CFG.msidata, ERR_MSIADDR and ERR_MSIADDRH are WARL registers.
     * We need to always update local data cache for them.
     */
    iopmp->msiaddr64 = *msiaddr64;
    iopmp->msidata = *msidata;

    return ret;
}

enum iopmp_error iopmp_get_and_clear_msi_werr(IOPMP_t *iopmp, bool *msi_werr)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->msi_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!msi_werr)
        return IOPMP_ERR_INVALID_PARAMETER;

    /* If ERR_CFG.msi_en=1, this operation is mandatory */
    assert(iopmp->ops_generic->get_and_clear_msi_werr);
    iopmp->ops_generic->get_and_clear_msi_werr(iopmp, msi_werr);

    return IOPMP_OK;
}

enum iopmp_error iopmp_set_stall_violation_en(IOPMP_t *iopmp, bool *enable)
{
    enum iopmp_error ret;

    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->stall_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!enable)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (iopmp->stall_violation_en == *enable)   /* Already set */
        return IOPMP_OK;

    /* If HWCFG0.stall_en=1, this operation is mandatory */
    assert(iopmp->ops_generic->set_stall_violation_en);
    ret = iopmp->ops_generic->set_stall_violation_en(iopmp, enable);
    iopmp->stall_violation_en = *enable;

    return ret;
}

enum iopmp_error iopmp_invalidate_error(IOPMP_t *iopmp)
{
    assert(iopmp_is_initialized(iopmp));

    /*
     * The error capture record is optional.
     * If it is not implemented, ERR_INFO.v should be wired to zero.
     */
    if (!iopmp->ops_generic->invalidate_error)
        return IOPMP_ERR_NOT_SUPPORTED;

    iopmp->ops_generic->invalidate_error(iopmp);

    return IOPMP_OK;
}

enum iopmp_error iopmp_capture_error(IOPMP_t *iopmp,
                                     IOPMP_ERR_REPORT_t *err_report,
                                     bool invalidate)
{
    assert(iopmp_is_initialized(iopmp));

    if (!err_report)
        return IOPMP_ERR_INVALID_PARAMETER;

    /* The error capture record is optional */
    if (!iopmp->ops_generic->capture_error)
        return IOPMP_ERR_NOT_SUPPORTED;

    return iopmp->ops_generic->capture_error(iopmp, err_report, invalidate);
}

enum iopmp_error iopmp_mfr_get_sv_window(IOPMP_t *iopmp, uint16_t *svi,
                                         uint16_t *svw)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp->mfr_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!svi || !svw)
        return IOPMP_ERR_INVALID_PARAMETER;

    /* If HWCFG0.mfr_en=1, this operation is mandatory */
    assert(iopmp->ops_generic->get_sv_window);
    return iopmp->ops_generic->get_sv_window(iopmp, svi, svw);
}

enum iopmp_error iopmp_lock_srcmd_table_fmt_0(IOPMP_t *iopmp, uint32_t rrid)
{
    assert(iopmp_is_initialized(iopmp));

    if (iopmp->srcmd_fmt != IOPMP_SRCMD_FMT_0)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (rrid >= iopmp->rrid_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    assert(iopmp->ops_specific->lock_srcmd_table);
    return iopmp->ops_specific->lock_srcmd_table(iopmp, rrid, 0);
}

enum iopmp_error iopmp_is_srcmd_table_fmt_0_locked(IOPMP_t *iopmp,
                                                   uint32_t rrid,
                                                   bool *locked)
{
    uint64_t mds;

    assert(iopmp_is_initialized(iopmp));

    if (iopmp->srcmd_fmt != IOPMP_SRCMD_FMT_0)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (rrid >= iopmp->rrid_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!locked)
        return IOPMP_ERR_INVALID_PARAMETER;

    /* Get current SRCMD_EN(rrid) */
    assert(iopmp->ops_specific->get_association_rrid_md);
    iopmp->ops_specific->get_association_rrid_md(iopmp, rrid, &mds, locked);

    return IOPMP_OK;
}

enum iopmp_error iopmp_lock_srcmd_table_fmt_2(IOPMP_t *iopmp, uint32_t mdidx)
{
    enum iopmp_error ret;
    uint64_t mds;

    assert(iopmp_is_initialized(iopmp));

    if (iopmp->srcmd_fmt != IOPMP_SRCMD_FMT_2)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (mdidx >= iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    mds = (uint64_t)1 << mdidx;
    if ((iopmp->mdlck_md & mds) == mds)
        return IOPMP_OK;

    if (iopmp->mdlck_lock)
        return IOPMP_ERR_REG_IS_LOCKED;

    assert(iopmp->ops_specific->lock_srcmd_table);
    ret = iopmp->ops_specific->lock_srcmd_table(iopmp, 0, mdidx);
    if (ret == IOPMP_OK)
        iopmp->mdlck_md |= mds;

    return ret;
}

enum iopmp_error iopmp_is_srcmd_table_fmt_2_locked(IOPMP_t *iopmp,
                                                   uint32_t mdidx,
                                                   bool *locked)
{
    uint64_t mds, checked_md;

    assert(iopmp_is_initialized(iopmp));

    if (iopmp->srcmd_fmt != IOPMP_SRCMD_FMT_2)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (mdidx >= iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!locked)
        return IOPMP_ERR_INVALID_PARAMETER;

    mds = iopmp->mdlck_md;
    checked_md = (uint64_t)1 << mdidx;
    *locked = (checked_md & mds) == checked_md;

    return IOPMP_OK;
}

enum iopmp_error iopmp_get_rrid_md_association(IOPMP_t *iopmp, uint32_t rrid,
                                               uint64_t *mds, bool *lock)
{
    assert(iopmp_is_initialized(iopmp));

    if (rrid >= iopmp->rrid_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!lock || !mds)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (iopmp->srcmd_fmt == IOPMP_SRCMD_FMT_0) {
        /* Check SRCMD table */
        assert(iopmp->ops_specific->get_association_rrid_md);
        iopmp->ops_specific->get_association_rrid_md(iopmp, rrid, mds, lock);
    } else if (iopmp->srcmd_fmt == IOPMP_SRCMD_FMT_1) {
        /* Each RRID is exactly associated with a single MD */
        *mds = (uint64_t)1 << rrid;
        *lock = true;
    } else if (iopmp->srcmd_fmt == IOPMP_SRCMD_FMT_2) {
        /* Every RRID implicitly associates all implemented memory domains */
        *mds = GENMASK_64((iopmp->md_num - 1), 0);
        *lock = true;
    }

    return IOPMP_OK;
}

enum iopmp_error iopmp_set_rrid_md_association(IOPMP_t *iopmp, uint32_t rrid,
                                               uint64_t mds_set,
                                               uint64_t mds_clr,
                                               uint64_t *mds,
                                               bool lock)
{
    uint64_t valid_mds;
    bool is_srcmd_en_locked;

    assert(iopmp_is_initialized(iopmp));

    if (iopmp->srcmd_fmt != IOPMP_SRCMD_FMT_0)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (rrid >= iopmp->rrid_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!mds)
        return IOPMP_ERR_INVALID_PARAMETER;

    valid_mds = GENMASK_64((iopmp->md_num - 1), 0);
    if (mds_set > valid_mds || mds_clr > valid_mds)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* One of selected MDs has been locked by MDLCK */
    if (iopmp->mdlck_md & (mds_set | mds_clr))
        return IOPMP_ERR_REG_IS_LOCKED;

    /* Get current SRCMD_EN(rrid) */
    assert(iopmp->ops_specific->get_association_rrid_md);
    iopmp->ops_specific->get_association_rrid_md(iopmp, rrid, mds,
                                                 &is_srcmd_en_locked);

    if (is_srcmd_en_locked)
        return IOPMP_ERR_REG_IS_LOCKED;

    /* Set new MD bitmap */
    *mds |= mds_set;
    /* Clear new MD bitmap */
    *mds &= ~mds_clr;

    assert(iopmp->ops_specific->set_association_rrid_md);
    return iopmp->ops_specific->set_association_rrid_md(iopmp, rrid, mds, lock);
}

enum iopmp_error iopmp_set_md_permission(IOPMP_t *iopmp, uint32_t rrid,
                                         uint32_t mdidx, bool *r, bool *w)
{
    assert(iopmp_is_initialized(iopmp));

    if (iopmp->srcmd_fmt != IOPMP_SRCMD_FMT_2)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (rrid >= iopmp->rrid_num || mdidx >= iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (iopmp->mdlck_md & ((uint64_t)1 << mdidx))
        return IOPMP_ERR_REG_IS_LOCKED;

    /* This operation is mandatory for SRCMD_FMT_2 */
    assert(iopmp->ops_specific->set_md_permission);
    return iopmp->ops_specific->set_md_permission(iopmp, rrid, mdidx, r, w);
}

enum iopmp_error iopmp_set_md_permission_multi(IOPMP_t *iopmp, uint32_t mdidx,
                                               IOPMP_SRCMD_PERM_CFG_t *cfg)
{
    assert(iopmp_is_initialized(iopmp));

    if (iopmp->srcmd_fmt != IOPMP_SRCMD_FMT_2)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (mdidx >= iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!cfg)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (iopmp->mdlck_md & ((uint64_t)1 << mdidx))
        return IOPMP_ERR_REG_IS_LOCKED;

    /* This operation is mandatory for SRCMD_FMT_2 */
    assert(iopmp->ops_specific->set_md_permission_multi);
    return iopmp->ops_specific->set_md_permission_multi(iopmp, mdidx, cfg);
}

void iopmp_set_srcmd_perm_cfg_nocheck(IOPMP_SRCMD_PERM_CFG_t *cfg,
                                      uint32_t rrid, bool r, bool w)
{
    uint64_t shift, mask, val;

    shift = (rrid << 1);
    mask  = (uint64_t)IOPMP_SRCMD_PERM_MASK << shift;
    val   = (((uint64_t)w << 1) | ((uint64_t)r << 0)) << shift;
    cfg->srcmd_perm_mask |= mask;
    cfg->srcmd_perm_val   = (cfg->srcmd_perm_val & ~mask) | (val & mask);
}

enum iopmp_error iopmp_set_srcmd_perm_cfg(IOPMP_SRCMD_PERM_CFG_t *cfg,
                                          uint32_t rrid, bool r, bool w)
{
    if (!cfg)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (rrid >= IOPMP_MAX_RRID_SRCMD_FMT_2)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    iopmp_set_srcmd_perm_cfg_nocheck(cfg, rrid, r, w);
    return IOPMP_OK;
}

/**
 * \brief (SPS only) Set RRID's read/write permission to MDs
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[in] rrid              The RRID to be set
 * \param[in] mds_set           The desired MDs to set permission to \p rrid
 * \param[in] mds_clr           The desired MDs to clear permission to \p rrid
 * \param[out] mds              The pointer to an integer to store WARL value of
 *                              SRCMD_{R|W}.md after setting
 * \param[in] fp_get_srcmd_rw_64    The function pointer to SPS operation to get
 *                                  value of SRCMD_{R|W}
 * \param[in] fp_set_srcmd_rw_64    The function pointer to SPS operation to set
 *                                  value of SRCMD_{R|W}
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_OUT_OF_BOUNDS if given \p rrid or \p mds is out of bounds
 * \retval IOPMP_ERR_REG_IS_LOCKED if register has been locked by SRCMD_EN.l
 * \retval IOPMP_ERR_ILLEGAL_VALUE if the written \p mds does not match the
 *         actual values
 */
static enum iopmp_error __sps_set(
    IOPMP_t *iopmp, uint32_t rrid, uint64_t mds_set, uint64_t mds_clr,
    uint64_t *mds, uint64_t (*fp_get_srcmd_rw_64)(IOPMP_t *, uint32_t),
    enum iopmp_error (*fp_set_srcmd_rw_64)(IOPMP_t *, uint32_t , uint64_t *))
{
    uint64_t valid_mds, srcmd_mds;
    bool is_srcmd_en_locked;

    if (rrid >= iopmp->rrid_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!mds)
        return IOPMP_ERR_INVALID_PARAMETER;

    valid_mds = GENMASK_64((iopmp->md_num - 1), 0);
    if (mds_set > valid_mds || mds_clr > valid_mds)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* One of selected MDs has been locked by MDLCK */
    if (iopmp->mdlck_md & (mds_set | mds_clr))
        return IOPMP_ERR_REG_IS_LOCKED;

    /* Get current SRCMD_EN(rrid) */
    assert(iopmp->ops_specific->get_association_rrid_md);
    iopmp->ops_specific->get_association_rrid_md(iopmp, rrid, &srcmd_mds,
                                                 &is_srcmd_en_locked);

    if (is_srcmd_en_locked)
        return IOPMP_ERR_REG_IS_LOCKED;

    assert(fp_get_srcmd_rw_64);
    *mds = fp_get_srcmd_rw_64(iopmp, rrid);

    /* Set new MD bitmap */
    *mds |= mds_set;
    /* Clear new MD bitmap */
    *mds &= ~mds_clr;

    assert(fp_set_srcmd_rw_64);
    return fp_set_srcmd_rw_64(iopmp, rrid, mds);
}

/**
 * \brief (SPS only) Get RRID's read/write permission to MDs
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[in] rrid              The RRID to be checked
 * \param[out] mds              Pointer to variable to output permission
 * \param[in] fp_get_srcmd_rw_64    The function pointer to SPS operation to get
 *                                  value of SRCMD_{R|W}
 *
 * \retval IOPMP_OK if successes
 * \retval IOPMP_ERR_OUT_OF_BOUNDS if given \p rrid is out of bounds
 * \retval IOPMP_ERR_INVALID_PARAMETER if given \p mds is NULL
 */
static
enum iopmp_error __sps_get(IOPMP_t *iopmp, uint32_t rrid, uint64_t *mds,
                           uint64_t (*fp_get_srcmd_rw_64)(IOPMP_t *, uint32_t))
{
    if (rrid >= iopmp->rrid_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!mds)
        return IOPMP_ERR_INVALID_PARAMETER;

    assert(fp_get_srcmd_rw_64);
    *mds = fp_get_srcmd_rw_64(iopmp, rrid);

    return IOPMP_OK;
}

enum iopmp_error iopmp_sps_set_rrid_md_read(IOPMP_t *iopmp, uint32_t rrid,
                                            uint64_t mds_set,
                                            uint64_t mds_clr,
                                            uint64_t *mds)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp_get_support_sps(iopmp))
        return IOPMP_ERR_NOT_SUPPORTED;

    return __sps_set(iopmp, rrid, mds_set, mds_clr, mds,
                     iopmp->ops_sps->sps_get_srcmd_r_64_md,
                     iopmp->ops_sps->sps_set_srcmd_r_64_md);
}

enum iopmp_error iopmp_sps_get_rrid_md_read(IOPMP_t *iopmp, uint32_t rrid,
                                            uint64_t *mds)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp_get_support_sps(iopmp))
        return IOPMP_ERR_NOT_SUPPORTED;

    return __sps_get(iopmp, rrid, mds, iopmp->ops_sps->sps_get_srcmd_r_64_md);
}

enum iopmp_error iopmp_sps_set_rrid_md_write(IOPMP_t *iopmp, uint32_t rrid,
                                             uint64_t mds_set,
                                             uint64_t mds_clr,
                                             uint64_t *mds)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp_get_support_sps(iopmp))
        return IOPMP_ERR_NOT_SUPPORTED;

    return __sps_set(iopmp, rrid, mds_set, mds_clr, mds,
                     iopmp->ops_sps->sps_get_srcmd_w_64_md,
                     iopmp->ops_sps->sps_set_srcmd_w_64_md);
}

enum iopmp_error iopmp_sps_get_rrid_md_write(IOPMP_t *iopmp, uint32_t rrid,
                                             uint64_t *mds)
{
    assert(iopmp_is_initialized(iopmp));

    if (!iopmp_get_support_sps(iopmp))
        return IOPMP_ERR_NOT_SUPPORTED;

    return __sps_get(iopmp, rrid, mds, iopmp->ops_sps->sps_get_srcmd_w_64_md);
}

enum iopmp_error iopmp_sps_set_rrid_md_rw(IOPMP_t *iopmp, uint32_t rrid,
                                          uint64_t mds_set_r,
                                          uint64_t mds_clr_r,
                                          uint64_t mds_set_w,
                                          uint64_t mds_clr_w,
                                          uint64_t *mds_r,
                                          uint64_t *mds_w)
{
    enum iopmp_error ret;

    assert(iopmp_is_initialized(iopmp));

    if (!iopmp_get_support_sps(iopmp))
        return IOPMP_ERR_NOT_SUPPORTED;

    ret = __sps_set(iopmp, rrid, mds_set_r, mds_clr_r, mds_r,
                     iopmp->ops_sps->sps_get_srcmd_r_64_md,
                     iopmp->ops_sps->sps_set_srcmd_r_64_md);
    if (ret != IOPMP_OK)
        return ret;

    return __sps_set(iopmp, rrid, mds_set_w, mds_clr_w, mds_w,
                     iopmp->ops_sps->sps_get_srcmd_w_64_md,
                     iopmp->ops_sps->sps_set_srcmd_w_64_md);
}

enum iopmp_error iopmp_sps_get_rrid_md_rw(IOPMP_t *iopmp, uint32_t rrid,
                                          uint64_t *mds_r, uint64_t *mds_w)
{
    enum iopmp_error ret;

    assert(iopmp_is_initialized(iopmp));

    if (!iopmp_get_support_sps(iopmp))
        return IOPMP_ERR_NOT_SUPPORTED;

    ret = __sps_get(iopmp, rrid, mds_r, iopmp->ops_sps->sps_get_srcmd_r_64_md);
    if (ret != IOPMP_OK)
        return ret;

    return __sps_get(iopmp, rrid, mds_w, iopmp->ops_sps->sps_get_srcmd_w_64_md);
}

static inline void __get_md_entry_association_nocheck(IOPMP_t *iopmp,
                                                      uint32_t mdidx,
                                                      uint32_t *entry_idx_start,
                                                      uint32_t *num_entry)
{
    uint32_t md_entry_top_prev, md_entry_top;

    assert(iopmp->ops_specific->get_md_entry_top);
    if (mdidx) {
        iopmp->ops_specific->get_md_entry_top(iopmp, mdidx - 1,
                                              &md_entry_top_prev);
    } else {
        md_entry_top_prev = 0;
    }
    iopmp->ops_specific->get_md_entry_top(iopmp, mdidx, &md_entry_top);

    *entry_idx_start = md_entry_top_prev;
    *num_entry = md_entry_top - md_entry_top_prev;
}

enum iopmp_error iopmp_get_md_entry_association(IOPMP_t *iopmp, uint32_t mdidx,
                                                uint32_t *entry_idx_start,
                                                uint32_t *num_entry)
{
    assert(iopmp_is_initialized(iopmp));

    if (mdidx >= iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!entry_idx_start || !num_entry)
        return IOPMP_ERR_INVALID_PARAMETER;

    __get_md_entry_association_nocheck(iopmp, mdidx, entry_idx_start,
                                       num_entry);

    return IOPMP_OK;
}

/**
 * \brief Check if given MD(mdidx_start)~MD(mdidx_start + md_num) are valid MDs
 *
 * \retval 1 if MD(mdidx_start) ~ MD(mdidx_start + md_num) are valid MDs
 * \retval 0 if MD(mdidx_start) ~ MD(mdidx_start + md_num) are not valid MDs
 *
 * \note This function avoids unsigned integer overflow
 */
static bool __check_md_idx_range(IOPMP_t *iopmp, uint32_t mdidx_start,
                                 uint32_t md_num)
{
    return mdidx_start < iopmp->md_num &&
           md_num <= (iopmp->md_num - mdidx_start);
}

enum iopmp_error iopmp_set_md_entry_association_multi(IOPMP_t *iopmp,
                                                      uint32_t mdidx_start,
                                                      uint32_t *num_entries,
                                                      uint32_t md_num)
{
    enum iopmp_error ret;
    uint32_t prev_top, this_top;

    assert(iopmp_is_initialized(iopmp));

    if (iopmp->mdcfg_fmt != IOPMP_MDCFG_FMT_0)
        return IOPMP_ERR_NOT_ALLOWED;   /* Must call iopmp_set_md_entry_num() */

    if (!num_entries)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (!__check_md_idx_range(iopmp, mdidx_start, md_num))
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* Check if MDCFG(mdidx) has been locked by MDCFGLCK.f */
    if (mdidx_start < iopmp->mdcfglck_f)
        return IOPMP_ERR_REG_IS_LOCKED;

    if (mdidx_start) {
        assert(iopmp->ops_specific->get_md_entry_top);
        iopmp->ops_specific->get_md_entry_top(iopmp, mdidx_start - 1,
                                              &prev_top);
    } else {
        prev_top = 0;
    }

    for (int m = 0; m < md_num; m++) {
        this_top = prev_top + num_entries[m];
        if (this_top > iopmp->entry_num)
            return IOPMP_ERR_OUT_OF_BOUNDS;
        /* This operation is mandatory for MDCFG_FMT_0 */
        assert(iopmp->ops_specific->set_md_entry_top);
        ret = iopmp->ops_specific->set_md_entry_top(iopmp, mdidx_start + m,
                                                    &this_top);
        /* Return actual number of entries */
        num_entries[m] = this_top - prev_top;
        if (ret != IOPMP_OK)
            return ret;

        /* Update previous top */
        prev_top = this_top;
    }

    return ret;
}

enum iopmp_error iopmp_get_md_entry_num(IOPMP_t *iopmp, uint32_t *md_entry_num)
{
    assert(iopmp_is_initialized(iopmp));

    if (!__is_k_model(iopmp))
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!md_entry_num)
        return IOPMP_ERR_INVALID_PARAMETER;

    *md_entry_num = iopmp->md_entry_num;

    return IOPMP_OK;
}

enum iopmp_error iopmp_set_md_entry_num(IOPMP_t *iopmp, uint32_t *md_entry_num)
{
    enum iopmp_error ret;

    assert(iopmp_is_initialized(iopmp));

    if (iopmp->mdcfg_fmt != IOPMP_MDCFG_FMT_2)
        return IOPMP_ERR_NOT_ALLOWED;

    if (!md_entry_num)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (iopmp->md_entry_num == *md_entry_num)
        return IOPMP_OK;

    /* HWCFG0.md_entry_num is locked if HWCFG0.enable is 1 */
    if (iopmp->enable)
        return IOPMP_ERR_REG_IS_LOCKED;

    /* HWCFG0.md_entry_num only has 7 bits */
    if (*md_entry_num > MAX_MD_ENTRY_NUM)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* Try to write new md_entry_num into IOPMP */
    assert(iopmp->ops_specific->set_md_entry_num);
    ret = iopmp->ops_specific->set_md_entry_num(iopmp, md_entry_num);
    iopmp->md_entry_num = *md_entry_num;    /* Update local cache */

    return ret;
}

/**
 * \brief Encode IOPMP NAPOT entry from given memory region and flags
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[out] entry            The entry to be output
 * \param[in] addr              Address of the memory region
 * \param[in] size              Size of the memory region
 * \param[in] hw_flags          HW flags of the entry for this memory region
 * \param[in] sw_flags          SW flags of the entry for this memory region
 * \param[in] private_data      Private data that can be used in specific model
 *
 * \return 1
 */
static int __encode_entry_pow2(IOPMP_t *iopmp, struct iopmp_entry *entry,
                               uint64_t addr, uint64_t size,
                               enum iopmp_entry_flags hw_flags,
                               enum iopmp_entry_flags sw_flags,
                               uint64_t private_data)
{
    /* Encode entry_cfg */
    enum iopmp_entry_flags match;

    if (sw_flags & IOPMP_ENTRY_FORCE_OFF)
        match = IOPMP_ENTRY_A_OFF;
    else
        match = (size == 4) ? IOPMP_ENTRY_A_NA4 : IOPMP_ENTRY_A_NAPOT;

    uint32_t entry_cfg = (hw_flags | match);

    /* Encode entry_addr */
    uint64_t mask = ((uint64_t)1 << (iopmp_ctzll(size) - IOPMP_ADDR_SHIFT)) - 1;
    uint64_t entry_addr = ((addr >> IOPMP_ADDR_SHIFT) & ~mask) | (mask >> 1);

    entry->cfg = entry_cfg;
    entry->addr = entry_addr;
    entry->prient_flag =
        (sw_flags & IOPMP_ENTRY_PRIO) ? IOPMP_PRIENT_PRIORITY :
        (sw_flags & IOPMP_ENTRY_NON_PRIO) ? IOPMP_PRIENT_NON_PRIORITY :
        IOPMP_PRIENT_ANY;
    entry->private_data = private_data;

    return 1;
}

/**
 * \brief Encode IOPMP TOR entries from given memory region and flags
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[out] entries          The array of entry to be output
 * \param[in] num_entry         Number of entries in \p entries
 * \param[in] addr              Address of the memory region
 * \param[in] size              Size of the memory region
 * \param[in] hw_flags          HW flags of the entry for this memory region
 * \param[in] sw_flags          SW flags of the entry for this memory region
 * \param[in] private_data      Private data that can be used in specific model
 *
 * \retval 1 if successes and the memory region is encoded as first TOR entry
 * \retval 2 if successes and the memory region is encoded as two TOR entries
 */
static int __encode_entry_tor(IOPMP_t *iopmp, struct iopmp_entry *entries,
                              uint64_t addr, uint64_t size,
                              enum iopmp_entry_flags hw_flags,
                              enum iopmp_entry_flags sw_flags,
                              uint64_t private_data)
{
    uint32_t entry_cfg0;
    uint32_t entry_cfg1;
    uint64_t entry_addr0;
    uint64_t entry_addr1;

    entry_cfg0 = entry_cfg1 = hw_flags;
    if (sw_flags & IOPMP_ENTRY_FIRST_TOR) {
        entry_cfg0 |= IOPMP_ENTRY_A_TOR;
        entry_addr0 = (addr + size) >> IOPMP_ADDR_SHIFT;

        entries[0].cfg  = entry_cfg0;
        entries[0].addr = entry_addr0;
        entries[0].prient_flag =
            (sw_flags & IOPMP_ENTRY_PRIO) ? IOPMP_PRIENT_PRIORITY :
            (sw_flags & IOPMP_ENTRY_NON_PRIO) ? IOPMP_PRIENT_NON_PRIORITY :
            IOPMP_PRIENT_ANY;
        entries[0].private_data = private_data;

        return 1;
    }

    entry_cfg0 |= IOPMP_ENTRY_A_OFF;
    entry_cfg1 |= (sw_flags & IOPMP_ENTRY_FORCE_OFF) ? IOPMP_ENTRY_A_OFF :
                                                       IOPMP_ENTRY_A_TOR;
    entry_addr0 = addr >> IOPMP_ADDR_SHIFT;
    entry_addr1 = (addr + size) >> IOPMP_ADDR_SHIFT;

    entries[0].cfg  = entry_cfg0;
    entries[0].addr = entry_addr0;
    entries[0].prient_flag =
        (sw_flags & IOPMP_ENTRY_PRIO) ? IOPMP_PRIENT_PRIORITY :
        (sw_flags & IOPMP_ENTRY_NON_PRIO) ? IOPMP_PRIENT_NON_PRIORITY :
        IOPMP_PRIENT_ANY;
    entries[0].private_data = private_data;
    entries[1].cfg  = entry_cfg1;
    entries[1].addr = entry_addr1;
    entries[1].prient_flag =
        (sw_flags & IOPMP_ENTRY_PRIO) ? IOPMP_PRIENT_PRIORITY :
        (sw_flags & IOPMP_ENTRY_NON_PRIO) ? IOPMP_PRIENT_NON_PRIORITY :
        IOPMP_PRIENT_ANY;
    entries[1].private_data = private_data;

    return 2;
}

/**
 * \brief Check if given value is power of 2
 *
 * \param[in] val               The value to be checked
 *
 * \retval 1 if it is power of 2
 * \retval 0 if it is not power of 2
 */
static inline bool ispow2(uint64_t val)
{
    return val && !(val & (val - 1));
}

/**
 * \brief Check if given memory region is NAPOT
 *
 * \param[in] addr              The base address of the memory region
 * \param[in] size              The size(in bytes) of the memory region
 *
 * \retval 1 if it is NAPOT
 * \retval 0 if it is not NAPOT
 */
static inline bool is_napot(uint64_t addr, uint64_t size)
{
    uint64_t max_size;

    assert(size >= 4);

    if (ispow2(size) == false)
        return false;

    if (addr == 0)
        max_size = UINT64_MAX;
    else
        max_size = addr & -addr;

    return size <= max_size;
}

enum iopmp_error iopmp_encode_entry(IOPMP_t *iopmp, struct iopmp_entry *entries,
                                    uint32_t num_entry, uint64_t addr,
                                    uint64_t size, enum iopmp_entry_flags flags,
                                    uint64_t private_data)
{
    enum iopmp_entry_flags hw_flags, sw_flags;

    assert(iopmp_is_initialized(iopmp));

    if (!entries || !num_entry)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (size == 0)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (!IS_ALIGNED(addr, iopmp->granularity) ||
        !IS_ALIGNED(size, iopmp->granularity))
        return IOPMP_ERR_INVALID_PARAMETER;

    if ((addr >> IOPMP_ADDR_SHIFT) > iopmp->entry_addr_bits)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    hw_flags = flags & (IOPMP_ENTRY_RWX | IOPMP_ENTRY_SIE_MASK |
                        IOPMP_ENTRY_SEE_MASK);
    sw_flags = flags & IOPMP_ENTRY_SW_FLAGS_MASK;
    if (!iopmp->peis && ((hw_flags & IOPMP_ENTRY_SIE_MASK) != 0))
        return IOPMP_ERR_NOT_SUPPORTED;
    if (!iopmp->pees && ((hw_flags & IOPMP_ENTRY_SEE_MASK) != 0))
        return IOPMP_ERR_NOT_SUPPORTED;

    /* NA4 or NAPOT region */
    if (is_napot(addr, size) && !(sw_flags & IOPMP_ENTRY_FORCE_TOR))
        return __encode_entry_pow2(iopmp, entries, addr, size, hw_flags,
                                   sw_flags, private_data);

    /* TOR region */
    if (!iopmp->tor_en)
        return IOPMP_ERR_NOT_SUPPORTED;

    if (!(sw_flags & IOPMP_ENTRY_FIRST_TOR) && (num_entry < 2))
        return IOPMP_ERR_NOT_ALLOWED;

    /* The lower bound of TOR entry 0 must be 0 */
    if ((sw_flags & IOPMP_ENTRY_FIRST_TOR) && (addr != 0))
        return IOPMP_ERR_NOT_ALLOWED;

    return __encode_entry_tor(iopmp, entries, addr, size, hw_flags, sw_flags,
                              private_data);
}

/**
 * \brief Sanity check on priority of entries
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[in] entry_array       The array of entries
 * \param[in] idx_start         The global start index of target entries
 * \param[in] num_entry         The number of entries to be checked
 *
 * \retval 1 on success
 * \retval 0 on error
 */
static bool __check_entry_priority(IOPMP_t *iopmp,
                                   const struct iopmp_entry *entry_array,
                                   uint32_t idx_start, uint32_t num_entry)
{
    uint32_t num_prient;

    num_prient = iopmp_get_prio_entry_num(iopmp);
    /*
     * Assume IOPMP has N entries.
     * The priority entries are indexed from 0 ~ (num_prient - 1).
     * The non-priority entries are indexed from num_prient ~ (N - 1).
     */
    for (int i = 0; i < num_entry; i++) {
        switch (entry_array[i].prient_flag) {
        case IOPMP_PRIENT_ANY:
            break;
        case IOPMP_PRIENT_PRIORITY:
            if ((idx_start + i) >= num_prient)
                return false;
            break;
        case IOPMP_PRIENT_NON_PRIORITY:
            if ((idx_start + i) < num_prient)
                return false;
            break;
        default:    /* GCOV_EXCL_LINE */
            break;  /* GCOV_EXCL_LINE */
        }
    }

    return true;
}

/**
 * \brief Check if given ENTRY(idx_start) ~ ENTRY(idx_start + num_entry) are
 * valid entries
 *
 * \retval 1 if ENTRY(idx_start) ~ ENTRY(idx_start + num_entry) are valid
 * \retval 0 if ENTRY(idx_start) ~ ENTRY(idx_start + num_entry) are not valid
 *
 * \note This function avoids unsigned integer overflow
 */
static bool __check_entry_idx_range(IOPMP_t *iopmp, uint32_t idx_start,
                                    uint32_t num_entry)
{
    return idx_start < iopmp->entry_num &&
           num_entry <= (iopmp->entry_num - idx_start);
}

enum iopmp_error iopmp_set_entries(IOPMP_t *iopmp,
                                   const struct iopmp_entry *entry_array,
                                   uint32_t idx_start, uint32_t num_entry)
{
    assert(iopmp_is_initialized(iopmp));

    if (!entry_array || !num_entry)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (!__check_entry_idx_range(iopmp, idx_start, num_entry))
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* Sanity check priority entries */
    if (!__check_entry_priority(iopmp, entry_array, idx_start, num_entry))
        return IOPMP_ERR_INVALID_PRIORITY;

    /* Check if desired entries have been locked by ENTRYLCK.f */
    if (idx_start < iopmp->entrylck_f)
        return IOPMP_ERR_REG_IS_LOCKED;

    assert(iopmp->ops_generic->set_entries);
    return iopmp->ops_generic->set_entries(iopmp, entry_array, idx_start,
                                           num_entry);
}

enum iopmp_error iopmp_set_entries_to_md(IOPMP_t *iopmp, uint32_t mdidx,
                                         const struct iopmp_entry *entry_array,
                                         uint32_t idx_start,
                                         uint32_t num_entry)
{
    uint32_t md_entry_idx_start, md_num_entry;

    assert(iopmp_is_initialized(iopmp));

    if (mdidx >= iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* Get start index and number of entries this MD has */
    __get_md_entry_association_nocheck(iopmp, mdidx, &md_entry_idx_start,
                                       &md_num_entry);

    if (num_entry > md_num_entry)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (idx_start >= md_num_entry)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    return iopmp_set_entries(iopmp, entry_array, md_entry_idx_start + idx_start,
                             num_entry);
}

enum iopmp_error iopmp_get_entries(IOPMP_t *iopmp,
                                   struct iopmp_entry *entry_array,
                                   uint32_t idx_start, uint32_t num_entry)
{
    assert(iopmp_is_initialized(iopmp));

    if (!entry_array || !num_entry)
        return IOPMP_ERR_INVALID_PARAMETER;

    if (!__check_entry_idx_range(iopmp, idx_start, num_entry))
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* Read IOPMP entries from IOPMP registers */
    assert(iopmp->ops_generic->get_entries);
    iopmp->ops_generic->get_entries(iopmp, entry_array, idx_start, num_entry);

    return IOPMP_OK;
}

enum iopmp_error iopmp_get_entries_from_md(IOPMP_t *iopmp, uint32_t mdidx,
                                           struct iopmp_entry *entry_array,
                                           uint32_t idx_start,
                                           uint32_t num_entry)
{
    uint32_t md_entry_idx_start, md_num_entry;

    assert(iopmp_is_initialized(iopmp));

    if (mdidx >= iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* Get start index and number of entries this MD has */
    __get_md_entry_association_nocheck(iopmp, mdidx, &md_entry_idx_start,
                                       &md_num_entry);

    if (num_entry > md_num_entry)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (idx_start >= md_num_entry)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    return iopmp_get_entries(iopmp, entry_array, md_entry_idx_start + idx_start,
                             num_entry);
}

enum iopmp_error iopmp_clear_entries(IOPMP_t *iopmp, uint32_t idx_start,
                                     uint32_t num_entry)
{
    assert(iopmp_is_initialized(iopmp));

    if (num_entry > iopmp->entry_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!__check_entry_idx_range(iopmp, idx_start, num_entry))
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* Check if desired entries have been locked by ENTRYLCK.f */
    if (idx_start < iopmp->entrylck_f)
        return IOPMP_ERR_REG_IS_LOCKED;

    assert(iopmp->ops_generic->clear_entries);
    iopmp->ops_generic->clear_entries(iopmp, idx_start, num_entry);

    return IOPMP_OK;
}

enum iopmp_error iopmp_clear_entries_in_md(IOPMP_t *iopmp, uint32_t mdidx)
{
    uint32_t md_entry_idx_start, md_num_entry;

    assert(iopmp_is_initialized(iopmp));

    if (mdidx >= iopmp->md_num)
        return IOPMP_ERR_OUT_OF_BOUNDS;

    /* Get start index and number of entries this MD has */
    __get_md_entry_association_nocheck(iopmp, mdidx, &md_entry_idx_start,
                                       &md_num_entry);

    return iopmp_clear_entries(iopmp, md_entry_idx_start, md_num_entry);
}

/**
 * \brief Check if given index range of IOPMP entries belong to the MD(mdidx)
 *
 * \param[in] iopmp             The IOPMP instance
 * \param[in] mdidx             The index of target MD
 * \param[in] idx_start         The global start index of target entries
 * \param[in] num_entry         The number of entries to be checked
 *
 * \retval 1 if some of given IOPMP entries belong to the MD(mdidx)
 * \retval 0 if the given IOPMP entries do not belong to the MD(mdidx)
 */
static bool __entries_intersect_with_md(IOPMP_t *iopmp, uint32_t mdidx,
                                        uint32_t idx_start, uint32_t num_entry)
{
    uint32_t md_entry_idx_start, md_num_entry;

    /* Get start index and number of entries this MD has */
    __get_md_entry_association_nocheck(iopmp, mdidx, &md_entry_idx_start,
                                       &md_num_entry);

    uint32_t rangeA_start = md_entry_idx_start;
    uint32_t rangeA_end = md_entry_idx_start + md_num_entry;
    uint32_t rangeB_start = idx_start;
    uint32_t rangeB_end = idx_start + num_entry;

    if ((rangeB_start <= rangeA_start) &&
        (rangeA_start < rangeB_end) &&
        (rangeB_start < rangeA_end) &&
        (rangeA_end <= rangeB_end))
        return true;

    if ((rangeA_start <= rangeB_start) &&
        (rangeA_end <= rangeB_start))
        return false;
    if ((rangeB_end <= rangeA_start) &&
        (rangeB_end <= rangeA_end))
        return false;

    return true;
}

enum iopmp_error iopmp_entries_get_belong_md(IOPMP_t *iopmp, uint32_t idx_start,
                                             uint32_t num_entry, uint64_t *mds)
{
    uint64_t __mds;

    assert(iopmp_is_initialized(iopmp));

    if (!__check_entry_idx_range(iopmp, idx_start, num_entry))
        return IOPMP_ERR_OUT_OF_BOUNDS;

    if (!mds)
        return IOPMP_ERR_INVALID_PARAMETER;

    __mds = 0;
    for (int i = 0; i < iopmp->md_num; i++) {
        if (__entries_intersect_with_md(iopmp, i, idx_start, num_entry))
            __mds |= (uint64_t)1 << i;
    }

    *mds = __mds;
    return IOPMP_OK;
}
