/***************************************************************************
// Author: Yazan Hussnain (yazan.hussain@10xengineers.ai)
// Date: October 21, 2024
// Description: This file contains all the configuration parameters
// that a user could change before compilation.
***************************************************************************/

#define IOPMP_TOR_EN            1
#define IOPMP_SPS_EN            1
#define IOPMP_PARIENT_PROG      0
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
#define IOPMP_MD_NUM            63      // Max 63 MD is supported
#define IOPMP_ADDRH_EN          1
#define IOPMP_ENABLE            0
#define IOPMP_ENTRY_NUM         512
#define IOPMP_PRIO_ENTRY        16
#define IOPMP_RRID_TRANSL       48
#define USER                    0x80      // It could be any user defined value, incase of error suppression
#define ERROR_CAPTURE_EN        1
#define IMP_ERROR_REQID         1
#define IMP_MDLCK               1
#define MSI_EN                  1
#define STALL_BUF_DEPTH         32
#define SRC_ENFORCEMENT_EN      0

#define ENTRY_OFFSET            0x2000

#define REG_INTF_BUS_WIDTH      4

#if (SRCMD_FMT == 0)
    #define IOPMP_RRID_NUM      64
#elif (SRCMD_FMT == 1)
    #define IOPMP_RRID_NUM      63      // Max RRID Num could be 63 when SRCMD_FMT is 1, because RRID is directly mapped to MD's
#else
    #define IOPMP_RRID_NUM      32      // Max RRID Num could be 32 when SRCMD_FMT is 2, because SRCMD_PERM is 32 bits
#endif
