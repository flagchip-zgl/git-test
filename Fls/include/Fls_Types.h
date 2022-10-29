#ifndef FLS_TYPES_H
#define FLS_TYPES_H

#ifdef __cplusplus
extern "C"{
#endif

/**
* @page misra_violations MISRA-C:2004 violations
*
* @section [global]
* Violates MISRA 2004 Required Rule 5.1,
* Identifiers (internal and external) shall not rely on the significance of more than 31 characters.
* The used compilers use more than 31 chars for identifiers.
*
* @section [global]
* Violates MISRA 2004 Required Rule 1.4,
* The compiler/linker shall be checked to ensure that 31 character significance and
* case sensitivity are supported for external identifiers.
* The used compilers use more than 31 chars for identifiers.
*
*/

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "typedef.h"

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
/**
    @brief Physical flash sectors type, unique identifier for each configured sector.
*/
typedef uint32_t Fls_PhysicalSectorType;

/**
    @brief Flash sector channel type.
*/
typedef enum
{
    FLS_CH_INTERN = 0,
    FLS_CH_QSPI = 1
} Fls_HwChType;

/**
* @brief          Type of job currently executed by Fls_MainFunction.
*/
typedef enum
{
    /**
    * @brief erase one or more complete flash sectors
    */
    FLS_JOB_ERASE = 0,
    /**
    * @brief write one or more complete flash pages
    */
    FLS_JOB_WRITE,
    /**
    * @brief read one or more bytes from flash memory
    */
    FLS_JOB_READ,
    /**
    * @brief compare data buffer with content of flash memory
    */
    FLS_JOB_COMPARE
    /**
    * @brief check content of erased flash memory area
    */
    ,FLS_JOB_BLANK_CHECK

} Fls_JobType;


/**
* @brief          Result of low-level flash operation.
*/
typedef enum
{
    FLASH_E_OK = 0,                 /**< @brief operation succeeded */
    FLASH_E_FAILED,                 /**< @brief operation failed due to hardware error */
    FLASH_E_BLOCK_INCONSISTENT,     /**< @brief data buffer doesn't match with content of flash memory */
    FLASH_E_PENDING,                /**< @brief operation is pending */
    FLASH_E_PARTITION_ERR           /**< @brief FlexNVM partition ratio error */
} Fls_LLDReturnType;

/**
* @brief          Type of job currently executed by Fls_LLDMainFunction.
*/
typedef enum
{
    /**
    * @brief no job executed by Fls_LLDMainFunction
    */
    FLASH_JOB_NONE = 0,
    /**
    * @brief erase one flash sector
    */
    FLASH_JOB_ERASE,
    /**
    * @brief complete erase and start an interleaved erase flash sector
    */
    FLASH_JOB_ERASE_TEMP,
    /**
    * @brief write one or more complete flash pages
    */
    FLASH_JOB_WRITE,
    /**
    * @brief erase blank check of flash sector
    */
    FLASH_JOB_ERASE_BLANK_CHECK

} Fls_LLDJobType;

/**
* @brief    Type of job executed in interrupt context.
*/
typedef enum
{
    /**
    * @brief no job scheduled to be executed in interrupt context
    */
    FLASH_IRQ_JOB_NONE = 0,
    /**
    * @brief read one or more flash bytes, in interrupt context.
    */
    FLASH_IRQ_JOB_READ,
    /**
    * @brief read and compare one or more flash bytes, in interrupt context.
    */
    FLASH_IRQ_JOB_COMPARE,
    /**
    * @brief read and compare against erased value one or more flash bytes, in interrupt context.
    */
    FLASH_IRQ_JOB_BLANK_CHECK,
    /**
    * @brief erase one flash sector, in interrupt context.
    */
    FLASH_IRQ_JOB_ERASE,
    /**
    * @brief read the external memory status, in interrupt context.
    */
    FLASH_IRQ_JOB_GET_STATUS,
    /**
    * @brief read the external memory status for the parallel device, in interrupt context.
    */
    FLASH_IRQ_JOB_GET_STATUS_PAR,
    /**
    * @brief set the write enable bit, in interrupt context.
    */
    FLASH_IRQ_JOB_WRITE_EN,
    /**
    * @brief write one or more complete flash pages, in interrupt context.
    */
    FLASH_IRQ_JOB_WRITE

} Fls_LLDIrqJobType;

/**
* @brief          Size of data to be processeed by CRC.
*
* @implements     Fls_CrcDataSizeType_enumeration
*/
typedef enum
{
    FLS_CRC_8_BITS = 0,
    FLS_CRC_16_BITS
} Fls_CrcDataSizeType;

/**
    @brief Enumeration of physical flash sectors program size
*/
typedef enum
{
    FLS_WRITE_DOUBLE_WORD = 8U,
    FLS_WRITE_PAGE = 32U,
    FLS_WRITE_QUAD_PAGE = 128U,
    FLS_WRITE_128BYTES_PAGE = 128U,
    FLS_WRITE_256BYTES_PAGE = 256U,
    FLS_WRITE_512BYTES_PAGE = 512U
} Fls_ProgSizeType;
/**
* @brief the number of bytes uses to compare.
*
*/
typedef enum
{
    FLS_SIZE_1BYTE = 1U,
    FLS_SIZE_4BYTE = 4U
} Fls_DataBytesType;

/**
* @brief          Logical sector index.
*/
typedef uint32_t Fls_SectorIndexType;

/**
* @brief          Fls CRC Type.
* @details        CRC computed over config set.
* @implements     Fls_CrcType_typedef
*/
typedef uint16_t Fls_CrcType;

/**
* @brief          Fls Address Type.
* @details        Address offset from the configured flash base address to access a certain flash
*                 memory area.
* @implements     Fls_AddressType_typedef
*/
typedef uint32_t Fls_AddressType;

/**
* @brief          Fls Length Type.
* @details        Number of bytes to read,write,erase,compare
* @implements     Fls_LengthType_typedef
*/
typedef uint32_t Fls_LengthType;

/**
* @brief          Fls Sector Count Type
* @details        Number of configured sectors
*/
typedef uint32_t Fls_SectorCountType;

/**
 * @brief Pointer type of Fls_AC_Callback function
*/
typedef void (*Fls_ACCallbackPtrType )  ( void );


/**
* @brief          Fls Job End Notification Pointer Type
* @details        Pointer type of Fls_JobEndNotification function
* @implements     Fls_JobEndNotificationPtrType_typedef
*/
typedef void (*Fls_JobEndNotificationPtrType)(void);

/**
* @brief          Fls Job Error Notification Pointer Type
* @details        Pointer type of Fls_JobErrorNotification function
* @implements     Fls_JobErrorNotificationPtrType_typedef
*/
typedef void (*Fls_JobErrorNotificationPtrType)(void);


/**
* @brief          Fls Start Flash Access Notification Pointer Type
* @details        Pointer type of Fls_StartFlashAccessNotif function
*
*/
typedef void (*Fls_StartFlashAccessNotifPtrType)(void);

/**
* @brief          Fls Finished Flash Access Notification Pointer Type
* @details        Pointer type of Fls_FinishedFlashAccessNotif function
*
*/
typedef void (*Fls_FinishedFlashAccessNotifPtrType)(void);

/**
    @brief Define pointer type of erase access code function
*/
typedef void (*Fls_AcErasePtrTOType )    (
                                                                    const uint32_t u32RegBaseAddr,
																	void (*CallBack)( void ),
																	uint32_t * pTimerCounterAC
                                                                );
typedef void (*Fls_AcErasePtrNoTOType )  (
																	const uint32_t u32RegBaseAddr,
																	void (*CallBack)( void )
                                                                );

/**
    @brief Define pointer type of write access code function
*/
typedef void (*Fls_AcWritePtrTOType )    (
																	const uint32_t u32RegBaseAddr,
																	void (*CallBack)( void ),
                                                                    uint32_t * pTimerCounterAC
                                                                );
typedef void (*Fls_AcWritePtrNoTOType )  (
																	const uint32_t u32RegBaseAddr,
																	void (*CallBack)( void )
                                                                );

typedef enum
{
    MEMIF_UNINIT = 0,           /**< @brief The underlying abstraction module or device driver has not been initialized (yet)*/
    MEMIF_IDLE,                 /**< @brief The underlying abstraction module or device driver is currently idle */
    MEMIF_BUSY,                 /**< @brief The underlying abstraction module or device driver is currently busy */
    MEMIF_BUSY_INTERNAL         /**< @brief The underlying abstraction module is busy with internal management operations.
                                            The underlying device driver can be busy or idle */
}MemIf_StatusType;


typedef enum
{
    MEMIF_JOB_OK = 0,               /**< @brief The job has been finished succesfully */
    MEMIF_JOB_FAILED,               /**< @brief The job has not been finished succesfully */
    MEMIF_JOB_PENDING,              /**< @brief The job has not yet been finished */
    MEMIF_JOB_CANCELED,             /**< @brief The job has been canceled */
    MEMIF_BLOCK_INCONSISTENT,       /**< @brief The requested block is inconsistent, it may contain corrupted data */
    MEMIF_BLOCK_INVALID             /**< @brief The requested block has been marked as invalid, the requested operation can not be performed */
}MemIf_JobResultType;

typedef enum
{
    MEMIF_MODE_SLOW = 0,        /**< @brief The underlying memory abstraction modules and drivers are working in slow mode */
    MEMIF_MODE_FAST             /**< @brief The underlying memory abstraction modules and drivers are working in fast mode */
}MemIf_ModeType;

#ifdef __cplusplus
}
#endif

#endif /* FLS_TYPES_H */

/** @}*/
