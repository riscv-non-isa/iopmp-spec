/***************************************************************************
// Author: Yazan Hussnain (yazan.hussain@10xengineers.ai)
//         Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains all the configuration parameters
// that a user could change before compilation.
***************************************************************************/

#define IOPMP_RRID_TRANSL_EN    1
#define IOPMP_RRID_TRANSL_PROG  0
#define IOPMP_MD_ENTRY_NUM      3
#define IOPMP_RRID_TRANSL       48

#define USER                    0x80        // User-defined value for error suppression success responses.
#define ERROR_CAPTURE_EN        1           // Indicates if the Error Capture Record feature is implemented.
#define IMP_ERROR_REQID         1           // Indicates if the ERR_REQID register is implemented.
#define IMP_MDLCK               1           // Indicates if the Memory Domain Lock (MDLCK) feature is implemented.
#define MSI_EN                  1           // Indicates if Message-Signal Interrupts are supported.
#define STALL_BUF_DEPTH         32          // Depth of the stall transaction buffer.
#define SRC_ENFORCEMENT_EN      0           // Indicates if source enforcement is enabled.
#define IMP_RRIDSCP             1           // Indicates if the RRIDSCP register is implemented.

#define ENTRY_OFFSET            0x2000      // Offset for the entry table.

#define REG_INTF_BUS_WIDTH      4           // Width (in bytes) of the register interface bus.

#if (MDCFG_FMT == 0)
    // Select the behavior for an MDCFG table improper setting.
    // 0: correct the values to make the table have a proper setting
    #define MDCFG_TABLE_IMPROPER_SETTING_BEHAVIOR   0
#endif
