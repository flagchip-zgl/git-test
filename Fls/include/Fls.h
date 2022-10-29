#ifndef FLS_H
#define FLS_H

#ifdef __cplusplus
extern "C"{
#endif

#include "Fls_Cfg.h"
#include "Std_Types.h"

/**
 * @brief Index of flash sector in group currently processed by erase job
 * @details Used by Erase job
 */
extern Fls_SectorIndexType Fls_u32JobSectorItPos;
/**
* @brief Pointer to current flash module configuration set
*/
extern const Fls_ConfigType * Fls_pConfigPtr;
/**
 * @brief Index of last flash sector by current job
 * @details Used to check status of all external flash chips before start jobs
 * or is the last sector in Erase job
 */
extern Fls_SectorIndexType Fls_u32JobSectorEndPos;

void Fls_Init(void);
Std_ReturnType Fls_Erase(Fls_AddressType u32TargetAddress,Fls_LengthType u32Length);
Std_ReturnType Fls_Write (Fls_AddressType u32TargetAddress,	const uint8 * pSourceAddressPtr, Fls_LengthType u32Length);
MemIf_JobResultType Fls_GetJobResult( void );
Std_ReturnType Fls_Read (Fls_AddressType u32SourceAddress, uint8 * pTargetAddressPtr, Fls_LengthType u32Length);
void Fls_MainFunction( void );

#ifdef __cplusplus
}
#endif

#endif /* FLS_H */

/** @} */

