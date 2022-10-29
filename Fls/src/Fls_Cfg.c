
/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Fls_Cfg.h"

extern void Fee_JobEndNotification( void );

extern void Fee_JobErrorNotification( void );

/* aFlsSectorFlags[] (FlsConfigSet) */
static const uint8 FlsConfigSet_aFlsSectorFlags[4] =
{
    FLS_SECTOR_ERASE_ASYNCH, /* (FlsSector_0) */
    FLS_SECTOR_ERASE_ASYNCH, /* (FlsSector_1) */
    FLS_SECTOR_ERASE_ASYNCH, /* (FlsSector_2) */
    FLS_SECTOR_ERASE_ASYNCH /* (FlsSector_3) */
};
/* aFlsSectorUnlock[] (FlsConfigSet) */
static const uint8 FlsConfigSet_aFlsSectorUnlock[4] =
{
    1U, /* (FlsSector_0) */
    1U, /* (FlsSector_1) */
    1U, /* (FlsSector_2) */
    1U /* (FlsSector_3) */
};

/* aFlsSectorEndAddr[] (FlsConfigSet) */
static const Fls_AddressType FlsConfigSet_aFlsSectorEndAddr[4] =
{
    (Fls_AddressType)1023U, /* FlsSectorEndAddr (FlsSector_0)*/
    (Fls_AddressType)2047U, /* FlsSectorEndAddr (FlsSector_1)*/
    (Fls_AddressType)3071U, /* FlsSectorEndAddr (FlsSector_2)*/
    (Fls_AddressType)4095U /* FlsSectorEndAddr (FlsSector_3)*/
};

/* aFlsSectorSize[] (FlsConfigSet) */
static const Fls_AddressType FlsConfigSet_aFlsSectorSize[4] =
{
    (Fls_AddressType)1024U, /* FlsSectorSize (FlsSector_0)*/
    (Fls_AddressType)1024U, /* FlsSectorSize (FlsSector_1)*/
    (Fls_AddressType)1024U, /* FlsSectorSize (FlsSector_2)*/
    (Fls_AddressType)1024U /* FlsSectorSize (FlsSector_3)*/
};

/* FlsNumberOfSector[] (FlsConfigSet) */
static const uint16 FlsConfigSet_aFlsNumberOfSector[4] =
{
    1U, /* FlsNumberOfSector (FlsSector_0)*/
    1U, /* FlsNumberOfSector (FlsSector_1)*/
    1U, /* FlsNumberOfSector (FlsSector_2)*/
    1U /* FlsNumberOfSector (FlsSector_3)*/
};

/* aFlsSectorStartAddr[] (FlsConfigSet) */
static const Fls_AddressType FlsConfigSet_aFlsSectorStartAddr[4] =
{
    (Fls_AddressType)0U, /* FlsSectorStartAddr (FlsSector_0)*/
    (Fls_AddressType)1024U, /* FlsSectorStartAddr (FlsSector_1)*/
    (Fls_AddressType)2048U, /* FlsSectorStartAddr (FlsSector_2)*/
    (Fls_AddressType)3072U /* FlsSectorStartAddr (FlsSector_3)*/
};

/* paSectorProgSize[] (FlsConfigSet) */
static const Fls_LengthType FlsConfigSet_aFlsProgSize[4] =
{
    (Fls_LengthType)FLS_WRITE_DOUBLE_WORD, /* FlsProgrammingSize (FlsSector_0) */
    (Fls_LengthType)FLS_WRITE_DOUBLE_WORD, /* FlsProgrammingSize (FlsSector_1) */
    (Fls_LengthType)FLS_WRITE_DOUBLE_WORD, /* FlsProgrammingSize (FlsSector_2) */
    (Fls_LengthType)FLS_WRITE_DOUBLE_WORD /* FlsProgrammingSize (FlsSector_3) */
};


/* paSectorPageSize[] (FlsConfigSet) */
static const Fls_LengthType FlsConfigSet_aFlsSectorPageSize[4] =
{
    (Fls_LengthType)8U, /* FlsPageSize (FlsSector_0) */
    (Fls_LengthType)8U, /* FlsPageSize (FlsSector_1) */
    (Fls_LengthType)8U, /* FlsPageSize (FlsSector_2) */
    (Fls_LengthType)8U /* FlsPageSize (FlsSector_3) */
};



static const Fls_Flash_InternalSectorInfoType FlsSector_0_FlsConfigSet_sInternalSectorInfo =
{
    /* 
    * @violates @ref fls_pbcfg_c_REF_2 Conversions involing function pointers must be to/from integral types 
    * @violates @ref fls_pbcfg_c_REF_8 A cast should not be performed between a pointer type and an integral type.
    */
    (volatile uint32_t*)(D_FLASH_BASE_ADDR + 0x000000UL),    /* sectorStartAddressPtr */
    0U  /* Sector location to calculate cfgCRC */              
};
static const Fls_Flash_InternalSectorInfoType FlsSector_1_FlsConfigSet_sInternalSectorInfo =
{
    /* 
    * @violates @ref fls_pbcfg_c_REF_2 Conversions involing function pointers must be to/from integral types 
    * @violates @ref fls_pbcfg_c_REF_8 A cast should not be performed between a pointer type and an integral type.
    */
    (volatile uint32_t*)(D_FLASH_BASE_ADDR + 0x000400UL),    /* sectorStartAddressPtr */
    1U  /* Sector location to calculate cfgCRC */              
};
static const Fls_Flash_InternalSectorInfoType FlsSector_2_FlsConfigSet_sInternalSectorInfo =
{
    /* 
    * @violates @ref fls_pbcfg_c_REF_2 Conversions involing function pointers must be to/from integral types 
    * @violates @ref fls_pbcfg_c_REF_8 A cast should not be performed between a pointer type and an integral type.
    */
    (volatile uint32_t*)(D_FLASH_BASE_ADDR + 0x000800UL),    /* sectorStartAddressPtr */
    2U  /* Sector location to calculate cfgCRC */              
};
static const Fls_Flash_InternalSectorInfoType FlsSector_3_FlsConfigSet_sInternalSectorInfo =
{
    /* 
    * @violates @ref fls_pbcfg_c_REF_2 Conversions involing function pointers must be to/from integral types 
    * @violates @ref fls_pbcfg_c_REF_8 A cast should not be performed between a pointer type and an integral type.
    */
    (volatile uint32_t*)(D_FLASH_BASE_ADDR + 0x000C00UL),    /* sectorStartAddressPtr */
    3U  /* Sector location to calculate cfgCRC */              
};

/*  FLASH physical sectorization description */
static const Fls_Flash_InternalSectorInfoType * const FlsConfigSet_aSectorList[4] =
{
    /* FLS_DATA_ARRAY_0_BLOCK_1_S000 */
    &FlsSector_0_FlsConfigSet_sInternalSectorInfo
    ,    /* FLS_DATA_ARRAY_0_BLOCK_1_S001 */
    &FlsSector_1_FlsConfigSet_sInternalSectorInfo
    ,    /* FLS_DATA_ARRAY_0_BLOCK_1_S002 */
    &FlsSector_2_FlsConfigSet_sInternalSectorInfo
    ,    /* FLS_DATA_ARRAY_0_BLOCK_1_S003 */
    &FlsSector_3_FlsConfigSet_sInternalSectorInfo
};


/* External QSPI flash parameters. */

/* paHwCh[] (FlsConfigSet) */
static const Fls_HwChType FlsConfigSet_paHwCh[4] =
{
    FLS_CH_INTERN, /* (FlsSector_0) */
    FLS_CH_INTERN, /* (FlsSector_1) */
    FLS_CH_INTERN, /* (FlsSector_2) */
    FLS_CH_INTERN /* (FlsSector_3) */
};





/**
* @brief        Structure used to set function pointers notification, working mode
*/
/* Fls module initialization data (FlsConfigSet)*/
/* @violates @ref fls_pbcfg_c_REF_5 Could be made static */

const Fls_ConfigType Fls_Config=
{
    &Fee_JobEndNotification, /* FlsJobEndNotification */
    &Fee_JobErrorNotification, /* FlsJobErrorNotification */  
    NULL_PTR, /* FlsStartFlashAccessNotif */
    NULL_PTR, /* FlsFinishedFlashAccessNotif */
    MEMIF_MODE_SLOW, /* FlsDefaultMode */
    1048576U, /* FlsMaxReadFastMode */
    1024U, /* FlsMaxReadNormalMode */
    256U, /* FlsMaxWriteFastMode */
    32U, /* FlsMaxWriteNormalMode */
    4U, /* FlsSectorCount */
    &FlsConfigSet_aFlsSectorEndAddr,                /* (*paSectorEndAddr)[]  */
    &FlsConfigSet_aFlsSectorStartAddr,              /* (*paSectorStartAddr)[]  */
    &FlsConfigSet_aFlsSectorSize,                   /* (*paSectorSize)[]  */
    &FlsConfigSet_aFlsNumberOfSector,               /* (*paNumberOfSector)[]  */
    &FlsConfigSet_aSectorList,                      /* (*pSectorList)[] */
    &FlsConfigSet_aFlsSectorFlags,                  /* (*paSectorFlags)[] */
    &FlsConfigSet_aFlsProgSize,                     /* (*paSectorProgSize)[] */
    &FlsConfigSet_aFlsSectorPageSize,               /* (*paSectorPageSize)[] */    
    &FlsConfigSet_aFlsSectorUnlock,                 /* (*paSectorUnlock)[] */
    &FlsConfigSet_paHwCh,                           /* (*paHwCh)[] */
    NULL_PTR, /* pFlsQspiCfgConfig */
    50513U /* configCrc */
};


/** @}*/
