
#ifndef FLS_IPW_H
#define FLS_IPW_H

/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Fls.h"

/** 
* @brief Index of flash sector currently processed by a job
* @details Used by all types of job
*/
/*
* @violates @ref fls_ipw_h_REF_5 An external object or function shall be declared in one and only
* one file.
*/
extern volatile Fls_SectorIndexType Fls_u32JobSectorIt;

/** 
* @brief Index of last flash sector by current job
* @details Used to check status of all external flash chips before start jobs
*/
/*
* @violates @ref fls_ipw_h_REF_5 An external object or function shall be declared in one and only
* one file.
*/
extern Fls_SectorIndexType Fls_u32JobSectorEnd;

/** 
    @brief Result of last flash hardware job
*/
/*
* @violates @ref fls_ipw_h_REF_5 An external object or function shall be declared in one and only
* one file.
*/
extern volatile MemIf_JobResultType Fls_eLLDJobResult;
/** 
    @brief Type of current flash hardware job - used for asynchronous operating mode.
*/
/*
* @violates @ref fls_ipw_h_REF_5 An external object or function shall be declared in one and only
* one file.
*/
extern Fls_LLDJobType Fls_eLLDJob;
/** 
* @brief Pointer to current flash module configuration set
*/
/*
* @violates @ref fls_ipw_h_REF_5 An external object or function shall be declared in one and only
* one file.
*/
extern const Fls_ConfigType * Fls_pConfigPtr;

/*
* @violates @ref fls_ipw_h_REF_5 An external object or function shall be declared in one and only
* one file.
*/
extern volatile Fls_LLDIrqJobType Fls_eLLDIrqJob;

void Fls_IPW_Init(void);
Fls_LLDReturnType Fls_IPW_SectorErase (const boolean bAsynch);
Fls_LLDReturnType Fls_IPW_SectorWrite (const Fls_AddressType u32SectorOffset, const Fls_AddressType u32Length,  const uint8 * pJobDataSrcPtr, const boolean bAsynch );
Fls_LLDReturnType Fls_IPW_SectorRead ( const Fls_AddressType u32SectorOffset, const Fls_AddressType u32Length, uint8 * pJobDataDestPtr);
void Fls_IPW_LLDMainFunction( void );

#endif /* FLS_IPW_H */
