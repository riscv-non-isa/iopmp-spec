/***************************************************************************
// Copyright (c) 2025 by 10xEngineers.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Author: Yazan Hussnain (yazan.hussain@10xengineers.ai)
//         Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: IOPMP Request Response Structure
// This header file defines data structures and enumerations used in the
// Input/Output Physical Memory Protection (IOPMP) for handling request
// and response transactions. It includes the enumeration of permission
// types (read, write, and instruction fetch) and status codes, as well
// as detailed request and response structures for transactions. These
// definitions help to manage access control, track transaction details,
// and capture any errors that occur during access attempts.
***************************************************************************/

#include "iopmp.h"

#ifndef __IOPMP_REQ_RSP_H__
#define __IOPMP_REQ_RSP_H__

// Enumerates transaction types (read, write, instruction fetch)
typedef enum {
    READ_ACCESS  = 1,  // Read permission
    WRITE_ACCESS = 2,  // Write permission
    INSTR_FETCH  = 3   // Instruction fetch permission
} perm_type_e;

// Structure for IOPMP transaction requests
typedef struct __attribute__((__packed__)) {
    uint16_t    rrid;      // Requester ID
    uint64_t    addr;      // Target address for the transaction
    uint32_t    length;    // Length of the transaction
    uint32_t    size;      // Size of each access in the transaction
    perm_type_e perm;      // Type of permission requested
    bool        is_amo;    // Indicates the AMO Access
} iopmp_trans_req_t;

// Enumerates status results for IOPMP transactions
typedef enum {
    IOPMP_SUCCESS = 0,  // Transaction successful
    IOPMP_ERROR   = 1   // Transaction encountered an error
} status_e;

// Enumerates specific match and error statuses for transactions
typedef enum {
    ILLEGAL_READ_ACCESS    = 0x01,  // Illegal read access attempted
    ILLEGAL_WRITE_ACCESS   = 0x02,  // Illegal write access attempted
    ILLEGAL_INSTR_FETCH    = 0x03,  // Illegal instruction fetch attempted
    PARTIAL_HIT_ON_PRIORITY= 0x04,  // Partial hit on a priority entry
    NOT_HIT_ANY_RULE       = 0x05,  // No rule matched the transaction
    UNKNOWN_RRID           = 0x06,  // Unknown requester ID in transaction
    STALLED_TRANSACTION    = 0x07,  // Error due to a stalled transaction
    ENTRY_MATCH            = 0x10,  // Entry matched in access control
    ENTRY_NOTMATCH         = 0x11   // No matching entry found
} iopmpMatchStatus_t;

// Structure for IOPMP transaction responses
typedef struct __attribute__((__packed__)) {
    uint32_t rrid;          // Requester ID
    uint8_t  user;          // User mode indicator
    uint8_t  rrid_stalled;  // Requester ID stall status
    uint16_t rrid_transl;
    status_e status;        // Transaction status (success or error)
} iopmp_trans_rsp_t;

#endif
