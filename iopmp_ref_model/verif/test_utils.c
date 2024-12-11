/***************************************************************************
// Author: Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains all the functions that could be used
// while testing any IOPMP Model.
***************************************************************************/
#include "test_utils.h"
#include "config.h"
#include "iopmp.h"

/**
  * @brief Allocates memory of the specified size in gigabytes.
  *
  * @param mem_gb - The size of the memory to allocate in gigabytes.
  * @return 0 if the memory allocation was successful, -1 otherwise.
 **/
int create_memory(uint8_t mem_gb) {
    // Calculate the total memory size in bytes
    size_t total_size = (size_t)mem_gb * 1024UL * 1024UL * 1024UL;

    // Allocate memory and check for allocation failure
    memory = malloc(total_size);
    if (memory == NULL) {
        return -1; // Memory allocation failed
    }

    return 0; // Memory allocation successful
}

/**
  * @brief Read data from a specific memory address.
  *
  * @param addr - The memory address from where the data should be read.
  * @param size - The size of the data in bytes.
  * @param data - Pointer to the data for read.
  * @return 0 if the read was successful, BUS_ERROR if the address corresponds to a bus error.
 **/
uint8_t read_memory(uint64_t addr, uint8_t size, char *data) {
    // Validate address against bus_error
    if (addr == bus_error) { return BUS_ERROR; }

    // Perform memory read
    memcpy(data, &memory[addr], size);
    return 0; // Read successfult
}

/**
  * @brief Writes data to a specific memory address.
  *
  * @param data - Pointer to the data to write.
  * @param addr - The memory address where the data should be written.
  * @param size - The size of the data in bytes.
  * @return 0 if the write was successful, BUS_ERROR if the address corresponds to a bus error.
 **/
uint8_t write_memory(char *data, uint64_t addr, uint32_t size) {
    // Validate address against bus_error
    if (addr == bus_error) { return BUS_ERROR; }

    // Perform memory write
    memcpy(&memory[addr], data, size);

    return 0; // Write successful
}

/**
  * @brief SRCMD Table Configurations
  *
  * @param srcmd_reg It could be SRCMD_EN, SRCMD_ENH, SRCMD_R, SRCMD_RH, SRCMD_W, SRCMD_WH.
  * @param srcmd_idx It could be any legal SRCMD Table Index.
  * @param data Value that you want to write in this register.
  * @param num_bytes It could be 4-Byte write or 8-Byte Write.
 **/
void configure_srcmd_n(uint8_t srcmd_reg, uint16_t srcmd_idx, reg_intf_dw data, uint8_t num_bytes){
    write_register(SRCMD_TABLE_BASE_OFFSET + srcmd_reg + (srcmd_idx * 32), data, num_bytes);
}
/**
  * @brief MDCFG Table Configurations
  *
  * @param md_idx It could be any legal MDCFG Table Index.
  * @param data Value that you want to write in this register.
  * @param num_bytes It could be 4-Byte write or 8-Byte Write.
 **/
void configure_mdcfg_n(uint8_t md_idx, reg_intf_dw data, uint8_t num_bytes){
    write_register(MDCFG_TABLE_BASE_OFFSET + (md_idx * 4),   data, num_bytes);
}

/**
  * @brief Entry Table Configurations
  *
  * @param entry_reg It could be ENTRY_ADDR, ENTRY_ADDRH, ENTRY_CFG, ENTRY_USER_CFG.
  * @param entry_idx It could be any legal Entry Table Index.
  * @param data Value that you want to write in this register.
  * @param num_bytes It could be 4-Byte write or 8-Byte Write.
 **/
void configure_entry_n(uint8_t entry_reg, uint64_t entry_idx, reg_intf_dw data, uint8_t num_bytes){
    write_register(ENTRY_TABLE_BASE_OFFSET + entry_reg + (entry_idx * 16), data, num_bytes);    // (364 >> 2) and keeping lsb 0
}

/**
  * @brief Receiver Port Signals
  *
  * @param rrid RRID Of the Bus Initiator
  * @param addr Address to be checked
  * @param length Number of transfers
  * @param size It should be 0 for 1-byte, 1 for 2-byte, 2 for 4-byte access.   
  * @param perm The permissions required for this transcation.
  * @param iopmp_trans_req This is pointer, pass it as it is.
 **/
void receiver_port(uint16_t rrid, uint64_t addr, uint32_t length, uint32_t size, perm_type_e perm, iopmp_trans_req_t *iopmp_trans_req){
    iopmp_trans_req->rrid    = rrid;
    iopmp_trans_req->addr    = addr;
    iopmp_trans_req->length  = length;
    iopmp_trans_req->size    = size;
    iopmp_trans_req->perm    = perm;
}

/**
  * @brief error_record_check
  *
  * @param err_type RRID Of the Bus Initiator
  * @param req_perm Address to be checked
  * @param req_addr Errored Address
  * @param err_rcd Set if error should be recorded
 **/
int error_record_chk(uint8_t err_type, uint8_t req_perm, uint64_t req_addr, bool err_rcd){
    err_reqinfo_t err_req_info_temp;
    err_req_info_temp.raw = read_register(0x0064, 4);
    if (err_rcd){
        FAIL_IF((err_req_info_temp.v != 1));                                                            
        FAIL_IF((err_req_info_temp.ttype != req_perm));                                     
        FAIL_IF((err_req_info_temp.etype != (err_type)));                                               
        FAIL_IF((read_register(0x0068, 4) != (uint32_t)((req_addr >> 2) & 0xFFFFFFFF)));    
        FAIL_IF((read_register(0x006C, 4) != (uint32_t)((req_addr >> 34) & 0xFFFFFFFF)));
    }
    else {
        FAIL_IF((err_req_info_temp.v == 1)); 
    }                                                   
    
    return 0;                                                            
}