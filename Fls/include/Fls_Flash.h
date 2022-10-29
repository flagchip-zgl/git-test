#ifndef FLS_FLASH_H
#define FLS_FLASH_H

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Fls.h"
#include "Fls_Types.h"
#include "Fls_Flash_Types.h"


/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/

/**
* @brief Logical address of data block currently processed by Fls_MainFunction
*/
extern Fls_AddressType Fls_u32JobAddrIt;
/**
* @brief Index of flash sector currently processed by a job
* @details Used by all types of job
*/
extern volatile Fls_SectorIndexType Fls_u32JobSectorIt;
/**
    @brief Result of last flash hardware job
*/
extern volatile MemIf_JobResultType Fls_eLLDJobResult;
/**
    @brief Type of current flash hardware job - used for asynchronous operating mode.
*/
extern Fls_LLDJobType Fls_eLLDJob;
/**
* @brief Pointer to current flash module configuration set
*/
extern const Fls_ConfigType * Fls_pConfigPtr;
/**
    @brief Number of bytes of data left to write.
*/
extern volatile Fls_LengthType Fls_u32LLDRemainingDataToWrite;
/*==================================================================================================
                                       FUNCTION PROTOTYPES
==================================================================================================*/
void Fls_Flash_Init( void );
uint16_t Only_Erase(uint32_t u32Addr, uint32_t u32DataLen);
uint16_t Only_Write(uint32_t u32Addr, uint32_t u32DataLen);
uint16_t Only_Read(uint32_t u32Addr, uint32_t u32DataLen);

void  Fls_Flash_MainFunctionErase( void );
void Fls_Flash_MainFunctionWrite( void );
Fls_LLDReturnType Fls_Flash_SectorErase(const Fls_PhysicalSectorType ePhySector, const boolean bAsynch);
Fls_LLDReturnType Fls_Flash_SectorWrite(const Fls_PhysicalSectorType ePhySector, const Fls_AddressType u32SectorOffset,
		                                const Fls_LengthType  u32DataLength, const uint8_t * const pDataPtr, const boolean bAsynch);
Fls_LLDReturnType  Fls_Flash_SectorRead (const Fls_PhysicalSectorType ePhySector, const Fls_AddressType u32SectorOffset, Fls_LengthType u32DataLength,
										uint8 * pDataPtr);
//Fls_LLDReturnType Fls_Flash_SectorErase(const Fls_PhysicalSectorType ePhySector, const boolean bAsynch);
   
#ifdef __cplusplus
}
#endif

#endif /* FLS_FLASH_H */

/** @}*/
