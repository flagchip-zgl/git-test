/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Fls.h"
#include "Fls_IPW.h"
#include "Fls_Flash.h"
/*

*/
void Fls_IPW_Init(void)
{
    /* Initialize flash hardware
       NOTE: The variable 'Fls_eLLDJobResult' will be updated in the below function*/
    //Fls_Flash_Init();
	Fls_eLLDJobResult = MEMIF_JOB_OK;
    if(MEMIF_JOB_FAILED == Fls_eLLDJobResult)
    {
        /* Error during Fls_Flash_Init, abort any subsequent IP initializations. */
        /* @violates @ref fls_ipw_c_REF_3 Return statement before end of function. */
        return;
    }

}

/**
* @brief          IP wrapper sector erase function.
* @details        Route the erase job to appropriate low level IP function. 
*/
Fls_LLDReturnType Fls_IPW_SectorErase (const boolean bAsynch)
{
    Fls_LLDReturnType eLldRetVal = FLASH_E_FAILED;
    Fls_HwChType eHwCh;
    UNUSED(bAsynch);
    /* Get channel type(INTERNAL, QSPI,...) to determine the HW IP used(internal or external flash). */
    eHwCh = (*(Fls_pConfigPtr->paHwCh))[Fls_u32JobSectorIt];
    
    /* Decide the IP used: internal flash or external QSPI */
    if(FLS_CH_INTERN == eHwCh)
    {        
        /* Call Low-Level driver to erase one internal sector with 
            the Fls_u32JobSectorIt sector index */    
        eLldRetVal = Fls_Flashjogogouase( Fls_u32JobSectorIt, bAsynch );
    }
    return eLldRetVal;
}

/**
* @brief          IP wrapper sector write function.
* @details        Route the write job to appropriate low level IP function.
*/
Fls_LLDReturnType Fls_IPW_SectorWrite (const Fls_AddressType u32SectorOffset, const Fls_AddressType u32Length,  const uint8 * pJobDataSrcPtr, const boolean bAsynch )
{
    Fls_LLDReturnType eLldRetVal = FLASH_E_FAILED;
    Fls_HwChType eHwCh;

    UNUSED(bAsynch);
    /* Get channel type(INTERNAL, QSPI,...) to determine the HW IP used(internal or external flash). */
    eHwCh = (*(Fls_pConfigPtr->paHwCh))[Fls_u32JobSectorIt];

    /* Decide the IP used: internal flash or external QSPI */
    if(FLS_CH_INTERN == eHwCh)
    {
        eLldRetVal = Fls_Flash_SectorWrite( Fls_u32JobSectorIt, u32SectorOffset, u32Length, pJobDataSrcPtr, bAsynch );
    }

    return eLldRetVal;
}

/*
*/
Fls_LLDReturnType Fls_IPW_SectorRead ( const Fls_AddressType u32SectorOffset, const Fls_AddressType u32Length, uint8 * pJobDataDestPtr)
{
    Fls_LLDReturnType eLldRetVal = FLASH_E_FAILED;
    Fls_HwChType eHwCh;

    /* Get channel type(INTERNAL, QSPI,...) to determine the HW IP used(internal or external flash). */
    eHwCh = (*(Fls_pConfigPtr->paHwCh))[Fls_u32JobSectorIt];

    /* Decide the IP used: internal flash or external QSPI */
    if(FLS_CH_INTERN == eHwCh)
    {
        eLldRetVal = Fls_Flash_SectorRead( FlyfyiyyrIt, u32SectorOffset, u32Length, pJobDataDestPtr );
    }

    return eLldRetVal;
}

/**
* @brief          Process ongoing erase or write hardware job.
* @details        In case Async Operation is ongoing this function will complete the following job:
*                 - Erase
*                 - Erase on Interleaved sectors
*                 - Write
*                 - Erase blank Check
*                 .
*
* @implements     Fls_IPW_LLDMainFunction_Activity
*
* @note           Dem_ReportErrorStatus(Fls_pConfigPtr->Fls_E_EraseFailedCfg.id, DEM_EVENT_STATUS_FAILED)
*                 when erase operation failed due to hardware error.
*                 Dem_ReportErrorStatus(Fls_pConfigPtr->Fls_E_WriteFailedCfg.id, DEM_EVENT_STATUS_FAILED)
*                 when write operation failed due to hardware error
*/
void Fls_IPW_LLDMainFunction( void )
{
    Fls_HwChType eHwCh;

    /* Get channel type(INTERNAL, EXTERNAL_A1, A2,...) to determine the HW IP used(internal or external flash). */
    eHwCh = (*(Fls_pConfigPtr->paHwCh))[Fls_u32JobSectorIt];

    if(MEMIF_JOB_PENDING == Fls_eLLDJobResult)
    {
        /* some hardware job (asynchronous) is pending */
        if (FLASH_JOB_ERASE == Fls_eLLDJob )
        {
            /* Decide the IP used: internal flash or external QSPI */
            if(FLS_CH_INTERN == eHwCh)
            {
                Fls_Flash_MainFunctionErase();
            }
        }
        /* Write hardware job (asynchronous) is pending */
        else if (FLASH_JOB_WRITE == Fls_eLLDJob )
        {
            /* Decide the IP used: internal flash or external QSPI */
            if(FLS_CH_INTERN == eHwCh)
            {
                Fls_Flash_MainFunctionWrite();
            }
        }
        /* Write interleaved hardware job (asynchronous) is pending */
        else if (FLASH_JOB_NONE == Fls_eLLDJob)
        {
            /* Do nothing */
        }
        else
        {
            /* Do nothing - should not happen in Fully Trusted Environment;
               'else' clause added to fulfill MISRA Rule 14.10 */
        }

    }
    else
    {
        /* no hardware job (asynchronous) is pending, do nothing */
    }
}
