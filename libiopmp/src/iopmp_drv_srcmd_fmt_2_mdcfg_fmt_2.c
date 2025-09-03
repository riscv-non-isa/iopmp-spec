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

#include "libiopmp_def.h"

#include "iopmp_drv_common.h"

static
struct iopmp_operations_specific iopmp_operations_srcmd_fmt_2_mdcfg_fmt_2 = {
    .set_md_lock = srcmd_fmt_0_2_set_md_lock,
    .lock_mdcfg = NULL,
    .get_md_entry_top = mdcfg_fmt_1_2_get_md_entry_top,
    .set_md_entry_top = NULL,
    .set_md_entry_num = mdcfg_fmt_2_set_md_entry_num,
    .get_association_rrid_md = NULL,
    .set_association_rrid_md = NULL,
    .set_md_permission = srcmd_fmt_2_set_md_permission,
    .set_md_permission_multi = srcmd_fmt_2_set_md_permission_multi,
    .lock_srcmd_table = srcmd_fmt_2_lock_srcmd_table,
};

const struct iopmp_driver iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2;

static enum iopmp_error
iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2_init(IOPMP_t *iopmp, uintptr_t addr)
{
    return iopmp_drv_init_common(iopmp, addr,
                                 iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2.srcmd_fmt,
                                 iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2.mdcfg_fmt,
                                 &iopmp_operations_srcmd_fmt_2_mdcfg_fmt_2);
}

const struct iopmp_driver iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2 = {
    .srcmd_fmt = IOPMP_SRCMD_FMT_2,
    .mdcfg_fmt = IOPMP_MDCFG_FMT_2,
    .impid = IOPMP_IMPID_NOT_SPECIFIED,
    .init = iopmp_drv_srcmd_fmt_2_mdcfg_fmt_2_init,
};
