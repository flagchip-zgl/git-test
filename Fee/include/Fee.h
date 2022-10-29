#ifndef FEE_H
#define FEE_H

#ifdef __cplusplus
extern "C"{
#endif

#include "Fls_Types.h"
#include "Std_Types.h"
#include "Fee_Cfg.h"
#include "Fee_InternalTypes.h"
/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Fee_Cfg.h"

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/



/**
* @brief        All service IDs (passed to DET)
*/
/** @implements Fee_interface */
#define FEE_INIT_ID                    0x00U
/** @implements Fee_interface */
#define FEE_SETMODE_ID                 0x01U
/** @implements Fee_interface */
#define FEE_READ_ID                    0x02U
/** @implements Fee_interface */
#define FEE_WRITE_ID                   0x03U
/** @implements Fee_interface */
#define FEE_CANCEL_ID                  0x04U
/** @implements Fee_interface */
#define FEE_GETSTATUS_ID               0x05U
/** @implements Fee_interface */
#define FEE_GETJOBRESULT_ID            0x06U
/** @implements Fee_interface */
#define FEE_INVALIDATEBLOCK_ID         0x07U
/** @implements Fee_interface */
#define FEE_GETVERSIONINFO_ID          0x08U
/** @implements Fee_interface */
#define FEE_ERASEIMMEDIATEBLOCK_ID     0x09U
/** @implements Fee_interface */
#define FEE_JOBENDNOTIFICATION_ID      0x10U
/** @implements Fee_interface */
#define FEE_JOBERRORNOTIFICATION_ID    0x11U
/** @implements Fee_interface */
#define FEE_MAINFUNCTION_ID            0x12U
/* Vendor specific API */
/** @implements Fee_interface */
#define FEE_GETRUNTIMEINFO_ID          0x13U
/** @implements Fee_interface */
#define FEE_FORCESWAPONNEXTWRITE_ID    0x14U


/**
* @brief        Development error codes (passed to DET)
*
*/
/** @brief API called when module was not initialized */
#define FEE_E_UNINIT                  0x01U
/** @brief API called with invalid block number */
#define FEE_E_INVALID_BLOCK_NO        0x02U
/** @brief API called with invalid block offset */
#define FEE_E_INVALID_BLOCK_OFS       0x03U
/** @brief API called with invalid data pointer */
#define FEE_E_PARAM_POINTER           0x04U
/** @brief API called with invalid length information */
#define FEE_E_INVALID_BLOCK_LEN       0x05U
/** @brief API called while module is busy processing a user request */
#define FEE_E_BUSY                    0x06U
/** @brief API called while module is busy doing internal management operations */
#define FEE_E_BUSY_INTERNAL           0x07U
/** @brief API called while module is not busy because there is no job to cancel */
#define FEE_E_INVALID_CANCEL          0x08U
/** @brief API Fee_init failed */
#define FEE_E_INIT_FAILED             0x09U
/* Vendor Specific Define */
/** @brief API called when underlying driver has cancel API disabled */
#define FEE_E_CANCEL_API              0x0AU
/** @brief API called with invalid cluster group index */
#define FEE_E_CLUSTER_GROUP_IDX       0x0BU
/** @brief API number of foreign blocks from data flash exceeds the total number
    of blocks allowed which is FEE_MAX_NR_OF_BLOCKS */
#define FEE_E_FOREIGN_BLOCKS_OVF      0x0CU


/*==================================================================================================
*                                     FUNCTION PROTOTYPES
==================================================================================================*/

void Fee_Init(void);

Std_ReturnType Fee_Read
    (
        uint16 uBlockNumber,
        uint16 uBlockOffset,
        uint8 * pDataBufferPtr,
        uint16 uLength
    );

Std_ReturnType Fee_Write
   (
       uint16 uBlockNumber,
       const uint8 * pDataBufferPtr
   );


MemIf_StatusType Fee_GetStatus( void );

MemIf_JobResultType Fee_GetJobResult( void );

Std_ReturnType Fee_InvalidateBlock
    (
        uint16 uBlockNumber
    );

Std_ReturnType Fee_EraseImmediateBlock
    (
        uint16 uBlockNumber
    );

void Fee_MainFunction( void );

void Fee_GetRunTimeInfo
    (
        uint8 uClrGrpIndex,
        const Fee_ClusterGroupRuntimeInfoType * pClrGrpRTInfo
    );

Std_ReturnType Fee_ForceSwapOnNextWrite
    (
        uint8 uClrGrpIndex
    );

extern Fee_BlockInfoType Fee_aBlockInfo[];
extern Fee_ClusterGroupInfoType  Fee_aClrGrpInfo[];
extern boolean Job_finish;

#ifdef __cplusplus
}
#endif

#endif /* FEE_H */
/** @} */
