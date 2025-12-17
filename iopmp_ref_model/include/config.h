/***************************************************************************
// Copyright (c) 2025 by 10xEngineers.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Author: Yazan Hussnain (yazan.hussain@10xengineers.ai)
//         Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains all the configuration parameters
// that a user could change before compilation.
***************************************************************************/

#define USER                    0x80        // User-defined value for error suppression success responses.
#define STALL_BUF_DEPTH         32          // Depth of the stall transaction buffer.
#define SRC_ENFORCEMENT_EN      0           // Indicates if source enforcement is enabled.

#define REG_INTF_BUS_WIDTH      4           // Width (in bytes) of the register interface bus.

// Select the behavior for an MDCFG table improper setting.
// 0: correct the values to make the table have a proper setting
#define MDCFG_TABLE_IMPROPER_SETTING_BEHAVIOR   0
