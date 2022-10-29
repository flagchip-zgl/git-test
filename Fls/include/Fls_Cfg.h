#ifndef FLS_CFG_H
#define FLS_CFG_H

#include "Fls_Types.h"
#include "Fls_Flash_Types.h"

/**
* @brief          Fls Config Type
* @details        Fls module initialization data structure
* @implements     Fls_ConfigType_typedef
*/
typedef struct
{
    /**
    * @brief pointer to job end notification function
    */
    Fls_JobEndNotificationPtrType jobEndNotificationPtr;
    /**
    * @brief pointer to job error notification function
    */
    Fls_JobErrorNotificationPtrType jobErrorNotificationPtr;
    /**
    * @brief pointer to start flash access notification
    */
    Fls_StartFlashAccessNotifPtrType startFlashAccessNotifPtr;
    /**
    * @brief pointer to finished flash access notification
    */
    Fls_FinishedFlashAccessNotifPtrType finishedFlashAccessNotifPtr;
    /**
    * @brief default FLS device mode after initialization (MEMIF_MODE_FAST, MEMIF_MODE_SLOW)
    */
    MemIf_ModeType eDefaultMode;
    /**
    * @brief max number of bytes to read in one cycle of Fls_MainFunction (fast mode)
    */
    Fls_LengthType u32MaxReadFastMode;
    /**
    * @brief max number of bytes to read in one cycle of  Fls_MainFunction (normal mode)
    */
    Fls_LengthType u32MaxReadNormalMode;
    /**
    * @brief max number of bytes to write in one cycle of Fls_MainFunction (fast mode)
    */
    Fls_LengthType u32MaxWriteFastMode;
    /**
    * @brief max number of bytes to write in one cycle of  Fls_MainFunction (normal mode)
    */
    Fls_LengthType u32MaxWriteNormalMode;
    /**
    * @brief number of configured logical sectors
    */
    Fls_SectorCountType u32SectorCount;
    /**
    * @brief pointer to array containing last logical address of each configured sector
    */
    const Fls_AddressType  (*paSectorEndAddr)[];
    /** 
    * @brief pointer to array containing start logical address of each configured sector 
    */
    const Fls_AddressType (*paSectorStartAddr)[];
    /** 
    * @brief pointer to array containing Sector size of each configured sector 
    */
    const Fls_AddressType (*paSectorSize)[];
      /** 
    * @brief pointer to array containing number of hw sector 
    */
    const uint16_t (*paNumberOfSector)[];
    /**
    * @brief pointer to array containing physical sector ID of each configured sector
    */
    const Fls_Flash_InternalSectorInfoType * const (*pSectorList)[];
    /**
    * @brief pointer to array containing flags set of each configured sector
    */
    const uint8_t (*paSectorFlags)[];
    /**
    * @brief pointer to array containing program size of each configured sector
    */
    const Fls_LengthType (*paSectorProgSize)[];
    /**
    * @brief pointer to array containing page size information of each configured sector
    */
    const Fls_LengthType (*paSectorPageSize)[];
    /**
    * @brief pointer to array containing Unlock information of each configured sector
    */
    const uint8_t (*paSectorUnlock)[];
    /**
    * @brief Pointer to array containing the hardware channel(internal, external_qspi, external_emmc) of each configured sector.
    */
    const Fls_HwChType (*paHwCh)[];
    /**
    * @brief Pointer to array containing the configured hardware start address of each external sector.
    */
    const uint32_t (*paSectorHwAddress)[];

    /**
     * @brief FLS Config Set CRC checksum
     */
    Fls_CrcType u16ConfigCrc;

} Fls_ConfigType;

/*==================================================================================================
                                 GLOBAL CONSTANT DECLARATIONS
==================================================================================================*/



#define P_FLASH_BASE_ADDR   (0x00000000UL)
#define D_FLASH_BASE_ADDR   (0x00040000UL)

#define FLS_SECTOR_ERASE_ASYNCH      0x01U
#define FLS_PAGE_WRITE_ASYNCH        0x02U
#define FLS_SECTOR_IRQ_MODE          0x04U

/* Declaration of post-build configuration set structure */
 /* @violates @ref Fls_Cfg_H_REF_2 Could define variable at block scope */
 extern const Fls_ConfigType Fls_Config;


#endif /* FLS_CFG_H */
