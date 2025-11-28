#ifndef __IOPMP_REF_API_H__
#define __IOPMP_REF_API_H__

#include "iopmp_registers.h"
#include "iopmp_req_rsp.h"

extern int reset_iopmp(void);
extern reg_intf_dw read_register(uint64_t offset, uint8_t num_bytes);
extern void write_register(uint64_t offset, reg_intf_dw data, uint8_t num_bytes);
extern void iopmp_validate_access(iopmp_trans_req_t *trans_req, iopmp_trans_rsp_t* iopmp_trans_rsp, uint8_t *intrpt);

#endif
