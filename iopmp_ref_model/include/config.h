/***************************************************************************
// Author: Yazan Hussnain (yazan.hussain@10xengineers.ai)
//         Gull Ahmed (gull.ahmed@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains all the configuration parameters
// that a user could change before compilation.
***************************************************************************/

#define IOPMP_TOR_EN            1
#define IOPMP_SPS_EN            1
#define IOPMP_RRID_TRANSL_EN    1
#define IOPMP_RRID_TRANSL_PROG  0
#define IOPMP_CHK_X             1
#define IOPMP_NO_X              0
#define IOPMP_NO_W              0
#define IOPMP_STALL_EN          1
#define IOPMP_PEIS              1
#define IOPMP_PEES              1
#define IOPMP_MFR_EN            1
#define IOPMP_MD_ENTRY_NUM      3
#define IOPMP_MD_NUM            63          // Max 63 MD is supported
#define IOPMP_ADDRH_EN          1
#define IOPMP_ENABLE            0
#define IOPMP_ENTRY_NUM         512
#define IOPMP_PRIO_ENTRY        16          // Depends on IOPMP_NON_PRIO_EN. If IOPMP_NON_PRIO_EN=0 the value should equal to IOPMP_ENTRY_NUM.
#define IOPMP_PRIO_ENT_PROG     0           // Depends on IOPMP_NON_PRIO_EN.
#define IOPMP_NON_PRIO_EN       1
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

#if (SRCMD_FMT == 0)
    #define IOPMP_RRID_NUM      64
#elif (SRCMD_FMT == 1)
    #define IOPMP_RRID_NUM      63          // Maximum RRID number when SRCMD_FMT is 1, as RRID is directly mapped to MDs.
#else
    #define IOPMP_RRID_NUM      32          // Maximum RRID number when SRCMD_FMT is 2, as SRCMD_PERM is 32 bits.
#endif

#if (MDCFG_FMT == 0)
    // Select the behavior for an MDCFG table improper setting.
    // 0: correct the values to make the table have a proper setting
    #define MDCFG_TABLE_IMPROPER_SETTING_BEHAVIOR   0
#endif
