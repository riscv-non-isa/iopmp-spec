
/***************************************************************************
// Author: Yazan Hussnain (yazan.hussain@10xengineers.ai)
// Date: October 24, 2024
// Description: IOPMP Interrupt Generation
// When interrupt generation is valid, this function is used to generate
// WSI or MSI based on the configuration in ERR_CFG register. This also
// check for the interrrupt suppression.
***************************************************************************/

#include "iopmp.h"

/**
 * @brief Generate an interrupt based on configuration and suppress flag.
 *
 * This function checks if interrupts are enabled and not suppressed. If MSI (Message Signaled Interrupt)
 * is disabled, it generates a wired signal interrupt. When MSI is enabled, it constructs the MSI address
 * and data, writes them to memory, and handles any potential bus errors during the write operation.
 *
 * @param intrpt Pointer to a variable to store the interrupt flag.
 */
void generate_interrupt(uint8_t *intrpt) {
    // Extract configuration values for clarity
    const uint8_t msi_enabled = g_reg_file.err_cfg.msi_en;
    const uint8_t interrupt_enabled = g_reg_file.err_cfg.ie;
    *intrpt = 0;
    // Check if interrupts are not enabled
    if (interrupt_enabled) {
        // Check if interrupts are not suppressed
        if (!intrpt_suppress) {
            *intrpt = !msi_enabled;
            #if (MSI_EN)
                if (msi_enabled) {
                    // Construct MSI address and data for enabled MSI.
                    // {MSI_ADDRH[64:34], MSI_ADDR[33:2], 2'b00}
                    uint64_t msi_addr = CONCAT32(g_reg_file.err_msiaddrh.raw,g_reg_file.err_msiaddr.raw) << 2;
                    uint32_t msi_data = g_reg_file.err_cfg.msidata;
                    // Write MSI data to memory
                    uint8_t status = write_memory((char *)&msi_data, msi_addr, MSI_DATA_BYTE);

                    // Handle bus errors during MSI write
                    if (status & BUS_ERROR) {
                        // TODO: Add specific error-handling logic here
                        return;
                    }
                }
            #endif
        }
    }
}
