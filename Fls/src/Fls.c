#include "Fls.h"
#include "Fls_IPW.h"
/**
* @brief Result of last flash module job
* @implements Fls_eJobResult_Object
*/
static MemIf_JobResultType Fls_eJobResult = MEMIF_JOB_OK;
/**
* @brief Pointer to current position in source data buffer
* @details Used by both write and compare jobs
*/
static const uint8 * Fls_pJobDataSrcPtr = NULL_PTR;
/**
* @brief Logical address of data block currently processed by Fls_MainFunction
*/
Fls_AddressType Fls_u32JobAddrIt = 0UL;
/**
* @brief Last logical address to be processed by a job
*/
static Fls_AddressType Fls_u32JobAddrEnd = 0UL;
/**
* @brief Index of flash sector currently processed by a job
* @details Used by all types of job
*/
volatile Fls_SectorIndexType Fls_u32JobSectorIt = 0UL;
/**
* @brief Index of last flash sector by current job
* @details Used to check status of all external flash chips before start jobs
* or is the last sector in Erease job
*/
Fls_SectorIndexType Fls_u32JobSectorEnd = 0UL;
/**
* @brief Pointer to current position in target data buffer
* @details Used only by read job
*/
static uint8 * Fls_pJobDataDestPtr = NULL_PTR;
/**
* @brief Indicates that new job has been accepted
* @details Used by all types of job
*/
static uint8_t Fls_u8JobStart = 0U;
/**
* @brief Type of currently executed job (erase, write, read, or compare)
*/
Fls_JobType Fls_eJob = FLS_JOB_ERASE;
/**
* @brief Pointer to current flash module configuration set
*/
const Fls_ConfigType * Fls_pConfigPtr = NULL_PTR;
/**
* @brief Maximum number of bytes to read or compare in one cycle of Fls_MainFunction
*/
static Fls_LengthType Fls_u32MaxRead = 0UL;
/**
* @brief Maximum number of bytes to write in one cycle of Fls_MainFunction
*/
static Fls_LengthType Fls_u32MaxWrite = 0UL;
/**
    @brief Result of last flash hardware job
*/
volatile MemIf_JobResultType Fls_eLLDJobResult = MEMIF_JOB_OK;
/**
    @brief Type of current flash hardware job - used for asynchronous operating mode.
*/
Fls_LLDJobType Fls_eLLDJob = FLASH_JOB_NONE;
/**
    @brief Type of current flash hardware job - used for interrupt operating mode.
*/
volatile Fls_LLDIrqJobType Fls_eLLDIrqJob = FLASH_IRQ_JOB_NONE;

/**
    @brief Number of bytes of data left to write.
*/
volatile Fls_LengthType Fls_u32LLDRemainingDataToWrite=0UL;

/*==================================================================================================
                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static MemIf_JobResultType Fls_LLDGetJobResult( void );
static void Fls_LLDClrJobResult( void );
/**
* @brief        Maps u32TargetAddress to sector index
*
* @details      Calculate index of logical sector u32TargetAddress belongs to
*
* @param[in]    u32TargetAddress Target address in flash memory
*
* @return       Zero based index of logical sector
* @retval       0 .. (Fls_pConfigPtr->u32SectorCount - 1)
*
* @pre          The module must be initialized
* @pre          u32TargetAddress must be in configured flash boundary
*
*/
static Fls_SectorIndexType Fls_GetSectorIndexByAddr( const Fls_AddressType u32TargetAddress )
{
    Fls_SectorIndexType u32SectorIndex = 0UL;

    for( u32SectorIndex = 0U; u32SectorIndex < Fls_pConfigPtr->u32SectorCount;
         u32SectorIndex++
       )
    {
        if( u32TargetAddress <= (*(Fls_pConfigPtr->paSectorEndAddr))[u32SectorIndex] )
        {
            /* u32TargetAddress belongs to sector with index u32SectorIndex */
            break;
        }
        else
        {
            /* Check next sector */
        }
    }
    return u32SectorIndex;
}

/**
* @brief        Return start address of given sector
*
* @details      Calculate start address (in linear space) of logical sector
*               specified by the u32SectorIndex parameter
*
* @param[in]    u32SectorIndex Index of logical sector
*
* @return       Start address of u32SectorIndex sector
* @retval       0 .. (FLS_TOTAL_SIZE - 1)
*
* @pre          The module must be initialized
* @pre          u32SectorIndex must be in range 0 .. (Fls_pConfigPtr->u32SectorCount - 1)
*
*/
static Fls_AddressType Fls_GetSectorStartAddr ( const Fls_SectorIndexType u32SectorIndex )
{
    Fls_AddressType u32TargetAddress = 0U;

    if( 0U != u32SectorIndex )
    {
        u32TargetAddress = (*(Fls_pConfigPtr->paSectorEndAddr))[u32SectorIndex - 1U] + 1U;
    }
    else
    {
        /* First sector starts with address 0 */
    }

    return u32TargetAddress + 1;
}

/*==================================================================================================
                                       LOCAL FUNCTIONS
==================================================================================================*/
/**
* @brief        Erase one complete flash sector
*
* @details      Call low level flash driver service
*               to erase one complete flash sector specified by the Fls_u32JobSectorIt
*               internal job variable
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_OK erase operation succeeded and there
*               is no more sectors to erase
* @retval       MEMIF_JOB_PENDING erase operation succeeded and there
*               is still one or more sectors to erase
* @retval       MEMIF_JOB_FAILED operation failed due to hardware error
*
* @param[in]    Job Type of access code to unload. Can be either
*               FLS_JOB_ERASE or FLS_JOB_WRITE
*
* @pre          The module must be initialized, the Fls_u32JobSectorIt internal
*               job variable must contain index of logical sector to be erased, and
*               Fls_u32JobSectorEnd must contain index of last logical sector to be erased
*
* @post         increments the Fls_u32JobSectorIt internal job variable
*
*
* @implements     Fls_DoJobErase_Activity
*
*/
static MemIf_JobResultType  Fls_DoJobErase( void )
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_PENDING;
    uint32 u32Datastore = 0UL;

    /*
    * @violates @ref fls_c_REF_4 No reliance shall be placed on
    * undefined or unspecified behaviour
    * @violates @ref fls_c_REF_9 The value of an expression shall be the same under any order of
    * evaluation that the standard permits.
    */
    if( Fls_u32JobSectorIt > Fls_u32JobSectorEnd )
    {
        /* No more sectors to erase */
        eRetVal = MEMIF_JOB_OK;
    }
    else
    {
        boolean bAsynch = (boolean)0;
        Fls_LLDReturnType eLldRetVal = FLASH_E_OK;

        /* Get the Fls_IPW_SectorErase function operation mode */
        if( (*(Fls_pConfigPtr->paSectorFlags))[Fls_u32JobSectorIt] &
            (FLS_SECTOR_ERASE_ASYNCH | FLS_SECTOR_IRQ_MODE)
          )
        {
            /* The sector write is performed in an asynchronous manner, it is ASYNC or IRQ mode. */
            bAsynch = (boolean)TRUE;
        }
        else
        {
            /* The sector write is SYNC, neither ASYNC or IRQ mode. */
            bAsynch = (boolean)FALSE;
        }

        eLldRetVal = Fls_IPW_SectorErase( bAsynch );

        /*
         * @violates @ref fls_c_REF_4 No reliance shall be placed on
         * undefined or unspecified behaviour
         * @violates @ref fls_c_REF_3 Side effects on right hand of logical operator: '||' or '&&'.
         * @violates @ref fls_c_REF_9 The value of an expression shall be the same under any order of
         * evaluation that the standard permits.
         */
        if(( FLASH_E_OK == eLldRetVal ) && ( Fls_u32JobSectorIt == Fls_u32JobSectorEnd ))
        {
            eRetVal = MEMIF_JOB_OK;
        }
        else if( FLASH_E_FAILED == eLldRetVal )
        {
            eRetVal = MEMIF_JOB_FAILED;
        }
        else
        {
            /* The erase operation is either:
                1. For ASYNCHRONOUS and IRQ operation: pending (FLASH_E_PENDING) in hardware, or
                2. For SYNCHRONOUS operation: finished (FLASH_E_OK) and there are more sectors to erase.

                1. If the operation is Async or Irq erase, than the sector is increased in
                   Fls_IPW_LLDMainFunction()(for Async erase) or in the IRQ routine(for Irq mode).
                   when completing the job.
                2. If the operation is Sync erase, than the sector is increased below because the job is complete.
            */
            u32Datastore = (*(Fls_pConfigPtr->paSectorFlags))[Fls_u32JobSectorIt];
            if( ( 0U == ( u32Datastore & FLS_SECTOR_ERASE_ASYNCH) ) && ( 0U == ( u32Datastore & FLS_SECTOR_IRQ_MODE) )
              )
            {
                Fls_u32JobSectorIt++;
            }
        }

    }

    return( eRetVal );
}

/**
* @brief       Calculate last logical address to read, write, or compare
*              from in current cycle of Fls_MainFunction
*
* @details      Calculate last logical address of Fls_DoJobDataTransfer
*               loop to limit number of bytes transferred to/from flash memory
*               in one cycle of Fls_MainFunction
*
* @return       Fls_AddressType
* @retval       0 .. (FLS_TOTAL_SIZE - 1)
*
* @param[in]    u32MaxTransfer Maximum number of bytes to read, write, or compare
*               in current cycle of Fls_MainFunction
*
* @pre          Fls_u32JobAddrIt must contain current, and Fls_u32JobAddrEnd last
*               logical address of current job
*
*/
static Fls_AddressType Fls_CalcMaxTransferAddrEnd ( const Fls_LengthType u32MaxTransfer )
{
    Fls_AddressType u32MaxTransferEndAddr = Fls_u32JobAddrEnd;

    /* Adjust the u32MaxTransferEndAddr address to transfer only
        the u32MaxTransfer bytes in one Fls_MainFunction() call */
    if( u32MaxTransferEndAddr >= (Fls_u32JobAddrIt + u32MaxTransfer) )
    {
        u32MaxTransferEndAddr = (Fls_u32JobAddrIt + u32MaxTransfer) - 1U;
    }
    else
    {
        /* No adjustment is necessary. Job will finish in this cycle
            of Fls_MainFunction */
    }
    MemIf_JobResultType eRetVal = MEMIF_JOB_PENDING;
    uint32 u32Datastore = 0UL;
    return( u32MaxTransferEndAddr );
}

/**
* @brief       Calculate sector offset to read, write, or compare
*              from current sector
*
* @details      Calculate sector offset in bytes for low-level driver
*               services
*
* @return       Fls_AddressType
* @retval      0 .. (FLS_SECTOR_SIZE - 1)
*
* @pre         The module must be initialized, Fls_u32JobSectorIt internal job
*              variable must contain index of current logical sector,
*              Fls_u32JobAddrIt must contain current logical address
*
*/
static Fls_AddressType Fls_CalcSectorTransferOffset( void )
{
    Fls_AddressType u32SectorAddrStart = 0UL;

    /* Get first address of Fls_u32JobSectorIt sector */
    u32SectorAddrStart = Fls_GetSectorStartAddr( Fls_u32JobSectorIt );

    /* Calculate the sector offset */
    return( Fls_u32JobAddrIt - u32SectorAddrStart );
}


/**
* @brief       Calculate number of bytes to read, write, or compare
*              from current sector
*
* @details      Calculate sector transfer length for low-level driver
*               services
*
* @param[in]    u32MaxTransferEndAddr Last address to read, write,
*               or compare data from in this cycle of Fls_MainFunction
*
* @return      Fls_AddressType
* @retval      0 .. (FLS_SECTOR_SIZE - 1)
*
* @pre        The module must be initialized, Fls_u32JobSectorIt internal job
*             variable must contain index of current logical sector,
*             Fls_u32JobAddrIt must contain current logical address, and
*             u32MaxTransferEndAddr must be >= Fls_u32JobAddrIt and lie within
*             the specified lower and upper flash address boundaries
*
*/
static Fls_AddressType Fls_CalcSectorTransferLength ( const Fls_AddressType u32MaxTransferEndAddr )
{
    Fls_AddressType u32SectorEndAddr = 0UL;

    /* Get last address of Fls_u32JobSectorIt sector */
    u32SectorEndAddr = (*(Fls_pConfigPtr->paSectorEndAddr))[Fls_u32JobSectorIt];

    /* Adjust the u32SectorEndAddr according to max number of bytes to transfer
        during one cycle of Fls_MainFunction */
    if( u32SectorEndAddr > u32MaxTransferEndAddr )
    {
        u32SectorEndAddr = u32MaxTransferEndAddr;
		Fls_LLDReturnType eRetVal = FLASH_E_OK;
    }
    else
    {
        /* No adjustment is necessary. There is more sectors
            to transfer */
    }

    /* Calculate number of bytes to read from the sector */
    return(( u32SectorEndAddr - Fls_u32JobAddrIt ) + 1U );
}

/**
* @brief       Read up to one physical flash sector
*
* @details      Call low level flash driver service to read given number of bytes
*               from given sector offset.
*
* @param[in]    SectorOffset Flash sector offset to read data from
* @param[in]    Length Number of bytes to read
*
* @return       Fls_LLDReturnType
* @retval       FLASH_E_OK read operation succeeded
* @retval       FLASH_E_FAILED read operation failed due to a hardware error
*
* @pre          The module must be initialized, the Fls_u32JobSectorIt internal job
*               variable must contain valid index of logical sector to read,
*               u32SectorOffset and u32Length must be in physical    sector boundary,
*               Fls_pJobDataDestPtr internal job variable must point to data read buffer
*
* @implements     Fls_DoJobRead_Activity
*
*/
static Fls_LLDReturnType Fls_DoJobRead (const Fls_AddressType u32SectorOffset, const Fls_AddressType u32Length)
{
    Fls_LLDReturnType eRetVal = FLASH_E_OK;

    eRetVal = Fls_IPW_SectorRead( u32SectorOffset, u32Length, Fls_pJobDataDestPtr );

    /* Do physical sector read */
    if( FLASH_E_FAILED == eRetVal )
    {

    }
    return( eRetVal );
}

/**
* @brief        Write up to one physical flash sector
*
* @details      Call low level flash driver service to write given number of bytes
*               at given sector offset
*
* @param[in]    u32SectorOffset Flash sector offset to write data from
* @param[in]    Length Number of bytes to read
*
* @return       Fls_LLDReturnType
* @retval       FLASH_E_OK write operation succeeded
* @retval       FLASH_E_FAILED write operation failed due to a hardware error
*
* @pre          The module must be initialized, the Fls_u32JobSectorIt internal job
*               variable must contain valid index of logical sector to write,
*               u32SectorOffset and u32Length must be in physical sector boundary and page
*               aligned, Fls_pJobDataSrcPtr internal job variable must point to data
                write buffer
*
* @implements     Fls_DoJobWrite_Activity
*
*/
static Fls_LLDReturnType Fls_DoJobWrite ( const Fls_AddressType u32SectorOffset, const Fls_AddressType u32Length)
{
    Fls_LLDReturnType eRetVal = FLASH_E_OK;
    boolean bAsynch = (boolean)0;

    /* Get the Fls_IPW_SectorWrite function operation mode */
    if( (*(Fls_pConfigPtr->paSectorFlags))[Fls_u32JobSectorIt] &
        (FLS_PAGE_WRITE_ASYNCH | FLS_SECTOR_IRQ_MODE)
      )
    {
        /* The sector write is performed in an asynchronous manner, it is ASYNC or IRQ mode. */
        bAsynch = (boolean)TRUE;
    }
    else
    {
        /* The sector write is SYNC, neither ASYNC or IRQ mode. */
        bAsynch = (boolean)FALSE;
    }

    eRetVal = Fls_IPW_SectorWrite( u32SectorOffset, u32Length, Fls_pJobDataSrcPtr, bAsynch );

    if( FLASH_E_FAILED == eRetVal )
    {
    }

    return( eRetVal );
}

/**
    @brief Read, write, or compare flash data
    @details Call Fls_DoJobRead, Fls_DoJobWrite, or Fls_DoJobCompare
        functions to read, write, or compare flash data. The functions
        ensures that only pre-configured maximum number of bytes is
        read, written, or compared during one cycle of
        the Fls_MainFunction call

    @return MemIf_JobResultType
    @retval MEMIF_JOB_OK read, write, or compare operation succeeded and there
        is no more data to read, write, or compare
    @retval MEMIF_JOB_PENDING read, write, or compare operation succeeded
        and there is still some data to read, write, or compare
    @retval MEMIF_JOB_FAILED read, write, or compare operation failed due
        to a hardware error
    @retval MEMIF_BLOCK_INCONSISTENT Compared data from a flash compare
        eJob are not equal

    @pre The module must be initialized, the Fls_u32JobSectorIt internal
        eJob variable must contain index of logical sector to read, write,
        or compare, the Fls_u32JobAddrIt must contain logical address to read,
        write or compare, and Fls_u32JobAddrEnd must contain last address to read,
        write or compare.

    @post Updates the Fls_u32JobSectorIt, Fls_pJobDataSrcPtr, Fls_pJobDataDestPtr,
        and Fls_u32JobAddrIt internal eJob variables

    @implements Fls_DoJobDataTransfer_Activity
*/
static MemIf_JobResultType Fls_DoJobDataTransfer (const Fls_JobType eJob, const Fls_LengthType u32MaxTransfer)
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_PENDING;

    /* No more data to write */
    if( Fls_u32JobAddrIt > Fls_u32JobAddrEnd )
    {
        eRetVal = MEMIF_JOB_OK;
    }
    else
    {
        Fls_AddressType u32MaxTransferAddrEnd = 0UL;
        Fls_LLDReturnType eTransferRetVal = FLASH_E_FAILED;

        /* Last address to transfer data to/from in this call of
            the Fls_MainFunction() functions */
        u32MaxTransferAddrEnd = Fls_CalcMaxTransferAddrEnd( u32MaxTransfer );

        /* Transfer sector by sector until the u32MaxTransferAddrEnd address is reached
            or error is detected */
        do
        {
            Fls_AddressType u32SectorTransferOffset = 0UL;
            Fls_AddressType u32SectorTransferLength = 0UL;

            /* Calculate offset and length of the sector data transfer */
            u32SectorTransferOffset = Fls_CalcSectorTransferOffset();
            u32SectorTransferLength = Fls_CalcSectorTransferLength( u32MaxTransferAddrEnd );

            /* Update the Fls_JobAddressIt iterator. (It is not modified in the below functions, but it has to
                be updated here because it is checked in the IRQ mode, which might happen right in the function call.) */
            Fls_u32JobAddrIt += u32SectorTransferLength;

            /* Call Low-level driver to transfer data to/from
                physical flash device */
            switch( eJob )
            {
                case FLS_JOB_WRITE:
                    eTransferRetVal =  Fls_DoJobWrite( u32SectorTransferOffset,
                                                      u32SectorTransferLength
                                                    );
                    /* Update the source data pointer for next write */
                    /*
                    * @violates @ref fls_c_REF_7 Array indexing shall be the
                    * only allowed form of pointer arithmetic
                    */
                    Fls_pJobDataSrcPtr += u32SectorTransferLength;
                    break;

                case FLS_JOB_READ:
                    eTransferRetVal =  Fls_DoJobRead( u32SectorTransferOffset,
                                                     u32SectorTransferLength
                                                   );

                    /* Update the destination data pointer for next read */
                    /*
                    * @violates @ref fls_c_REF_7 Array indexing shall be the
                    * only allowed form of pointer arithmetic
                    */
                    Fls_pJobDataDestPtr += u32SectorTransferLength;
                    break;

                default:
                    /* Do nothing - should not happen in Fully Trusted Environment;
                       "default" clause added to fulfill MISRA Rule 15.3 */
                    break;
            }

            /* Fls_u32JobSectorIt should be increased here only for SYNC mode and for read/compare job
               For a Write Job in ASYNC mode it should be set by Fls_IPW_LLDMainFunction just before
               completing the Job */
            /* Fls_u32JobSectorIt should be increased here for:
                    READ jobs        - SYNC mode.
                    COMPARE jobs     - SYNC mode.
                    BLANK CHECK jobs - SYNC mode.
                    WRITE jobs       - SYNC mode.
               Fls_u32JobSectorIt will not be increased here for:
                    ERASE jobs       - SYNC, ASYNC or IRQ   (Fls_u32JobSectorIt will be set at the end of Fls_DoJobErase for SYNC mode, at the end of Fls_IPW_LLDMainFunction for ASYNC mode and at the end of the interrupt routine for IRQ mode)
                    READ jobs        - IRQ mode.            (Fls_u32JobSectorIt will be set at the end of the interrupt routine)
                    COMPARE jobs     - IRQ mode.            (Fls_u32JobSectorIt will be set at the end of the interrupt routine)
                    BLANK CHECK jobs - IRQ mode.            (Fls_u32JobSectorIt will be set at the end of the interrupt routine)
                    WRITE jobs       - ASYNC or IRQ mode.   (Fls_u32JobSectorIt will be set at the end of Fls_IPW_LLDMainFunction, if batch is finished)
            */
            if( FLASH_E_OK == eTransferRetVal ) /* If the previous(SYNC job) has finished successfully */
            {
                if( (Fls_u32JobAddrIt > (*(Fls_pConfigPtr->paSectorEndAddr))[Fls_u32JobSectorIt]))
                {
                    /* Move on to the next sector */
                    Fls_u32JobSectorIt++;
                }

            }
        }
        while(( Fls_u32JobAddrIt <= u32MaxTransferAddrEnd ) &&
              ( FLASH_E_OK == eTransferRetVal )
             );


        if( FLASH_E_FAILED == eTransferRetVal )
        {
            eRetVal = MEMIF_JOB_FAILED;
        }
        else if( FLASH_E_BLOCK_INCONSISTENT == eTransferRetVal )
        {
            /* compare job only */
            eRetVal = MEMIF_BLOCK_INCONSISTENT;
        }
        else if(( Fls_u32JobAddrIt > Fls_u32JobAddrEnd ) &&
                ( FLASH_E_OK == eTransferRetVal )
               )
        {
            /* All desired job data has been successfully transferred */
            eRetVal = MEMIF_JOB_OK;
        }
        else
        {
            /* The write operation is either pending (FLASH_E_PENDING)
            in hardware in case of asynchronous operation or
            finished (FLASH_E_OK) in case of synchronous operation
            and there is more pages to write */
        }
    }

    return( eRetVal );
}


/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/

/**
* @brief        The function initializes Fls module.
* @details      The function sets the internal module variables according to given
*               configuration set.
*
* @param[in]    pConfigPtr        Pointer to flash driver configuration set.
*
* @api
*
* @pre          @p pConfigPtr must not be @p NULL_PTR and the module status must not
*               be @p MEMIF_BUSY.
*
* @implements     Fls_Init_Activity
*
*/
/*
* @violates @ref fls_c_REF_8 All declarations and definitions
* of objects or functions at file scope shall have internal linkage unless external linkage required
*/
void Fls_Init(void)
{
    if(MEMIF_JOB_PENDING == Fls_eJobResult)
    {

    }
    else
    {
        Fls_pConfigPtr = &Fls_Config;
		/* Set the max number of bytes to read/write
			during Fls_MainFunction call */
		if (MEMIF_MODE_SLOW == Fls_pConfigPtr->eDefaultMode)
		{
			Fls_u32MaxRead = Fls_pConfigPtr->u32MaxReadNormalMode;
			Fls_u32MaxWrite = Fls_pConfigPtr->u32MaxWriteNormalMode;
		}
		else
		{
			Fls_u32MaxRead = Fls_pConfigPtr->whfdiwhfirwi3hfo;
			Fls_u32MaxWrite = Fls_pConfigPtr->u32MaxWriteFastMode;
		}
		/* Initialize flash hardware
		   NOTE: The variable 'Fls_eLLDJobResult' will be updated in the below function*/
		Fls_IPW_Init();

		if(  MEMIF_JOB_FAILED == Fls_eLLDJobResult )
		{
			Fls_eJobResult = MEMIF_JOB_FAILED;
		}
		else
		{
			Fls_eJobResult = MEMIF_JOB_OK;
		}
    }
}

/**
* @brief            Erase one or more complete flash sectors.
* @details          Starts an erase job asynchronously. The actual job is performed
 *                  by the @p Fls_MainFunction.
*
* @param[in]        TargetAddress        Target address in flash memory.
* @param[in]        Length               Number of bytes to erase.
*
* @return           Std_ReturnType
* @retval           E_OK                    Erase command has been accepted.
* @retval           E_NOT_OK                Erase command has not been accepted.
*
* @api
*
* @pre              The module has to be initialized and not busy.
* @post             @p Fls_Erase changes module status and some internal variables
*                   (@p Fls_u32JobSectorIt, @p Fls_u32JobSectorEnd, @p Fls_Job,
*                   @p Fls_eJobResult).
*
* @implements       Fls_Erase_Activity
*
*/
/*
* @violates @ref fls_c_REF_8 All declarations and definitions
* of objects or functions at file scope shall have internal linkage unless external linkage required
*/
Std_ReturnType Fls_Erase(Fls_AddressType u32TargetAddress,Fls_LengthType u32Length)
{
    Std_ReturnType u8RetVal = (Std_ReturnType)E_OK;


    /* Perform calculations outside the critical section in order
        to limit time spend in the CS */
    u32TmpJobSectorIt = Fls_GetSectorIndexByAddr( u32TargetAddress );
    u32TmpJobSectorEnd = Fls_GetSectorIndexByAddr(( u32TargetAddress + u32Length ) - 1U );


    if( MEMIF_JOB_PENDING == Fls_eJobResult )
    {
        u8RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Configure the erase job */
        Fls_u32JobSectorIt = u32TmpJobSectorIt;
        Fls_u32JobSectorEnd = u32TmpJobSectorEnd;
        Fls_eJob = FLS_JOB_ERASE;
        Fls_u8JobStart = 1U;

        /* Execute the erase job */
        Fls_eJobResult = MEMIF_JOB_PENDING;
    }

    return( u8RetVal );
}

/**
* @brief            Write one or more complete flash pages to the flash device.
* @details          Starts a write job asynchronously. The actual job is performed by
*                   @p Fls_MainFunction.
*
* @param[in]        TargetAddress        Target address in flash memory.
* @param[in]        SourceAddressPtr     Pointer to source data buffer.
* @param[in]        Length               Number of bytes to write.
*
* @return           Std_ReturnType
* @retval           E_OK                 Write command has been accepted.
* @retval           E_NOT_OK             Write command has not been accepted.
*
* @api
*
* @pre              The module has to be initialized and not busy.
* @post             @p Fls_Write changes module status and some internal variables
 *                  (@p Fls_u32JobSectorIt, @p Fls_u32JobAddrIt, @p Fls_u32JobAddrEnd,
 *                  @p Fls_pJobDataSrcPtr, @p Fls_eJob, @p Fls_eJobResult).
*
* @implements       Fls_Write_Activity
*
*/
/*
* @violates @ref fls_c_REF_8 All declarations and definitions
* of objects or functions at file scope shall have internal linkage unless external linkage required
*/
Std_ReturnType Fls_Write (Fls_AddressType u32TargetAddress,	const uint8 * pSourceAddressPtr, Fls_LengthType u32Length)
{
    Std_ReturnType	u8RetVal = (Std_ReturnType)E_OK;
    Fls_SectorIndexType u32TmpJobSectorIt = 0UL;
    Fls_AddressType u32TmpJobAddrEnd = 0UL;

    /* Perform calculations outside the critical section in order
        to limit time spend in the CS */
    u32TmpJobSectorIt = Fls_GetSectorIndexByAddr(u32TargetAddress);
    u32TmpJobAddrEnd = ( u32TargetAddress + u32Length ) - 1U;

    if( MEMIF_JOB_PENDING == Fls_eJobResult )
    {
        u8RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Configure the write job */
        Fls_u32JobSectorIt = u32TmpJobSectorIt;
        Fls_u32JobSectorEnd = Fls_GetSectorIndexByAddr(u32TmpJobAddrEnd);
        Fls_u32JobAddrEnd = u32TmpJobAddrEnd;
        Fls_u32JobAddrIt = u32TargetAddress;
        Fls_pJobDataSrcPtr = pSourceAddressPtr;
        Fls_eJob = FLS_JOB_WRITE;
        Fls_u8JobStart = 1U;

        /* Execute the write job */
        Fls_eJobResult = MEMIF_JOB_PENDING;
    }
    return( u8RetVal );
}

MemIf_JobResultType Fls_GetJobResult( void )
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_OK;

    eRetVal = Fls_eJobResult;

    return( eRetVal );
}

/*
* @implements       Fls_Read_Activity
*
*/
/*
* @violates @ref fls_c_REF_8 All declarations and definitions
* of objects or functions at file scope shall have internal linkage unless external linkage required
*/
Std_ReturnType Fls_Read (Fls_AddressType u32SourceAddress, uint8 * pTargetAddressPtr, Fls_LengthType u32Length)
{
    Std_ReturnType u8RetVal = (Std_ReturnType)E_OK;
    Fls_SectorIndexType u32TmpJobSectorIt = 0UL;
    Fls_AddressType u32TmpJobAddrEnd = 0UL;

    /* Perform calculations outside the critical section in order
        to limit time spend in the CS */
    u32TmpJobSectorIt = Fls_GetSeefhi3rhfiByAddr( u32SourceAddress );
    u32TmpJobAddrEnd = ( u32SourceAddress + u32Length ) - 1U;

    if( MEMIF_JOB_PENDING == Fls_eJobResult )
    {
        u8RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Configure the read job */
        Fls_u32JobSectorIt = u32TmpJobSectorIt;
        Fls_u32JobSectorEnd = Fls_GetSectorIndexByAddr(u32TmpJobAddrEnd);
        Fls_u32JobAddrIt = u32SourceAddress;
        Fls_u32JobAddrEnd = u32TmpJobAddrEnd;
        Fls_pJobDataDestPtr = pTargetAddressPtr;
        Fls_eJob = FLS_JOB_READ;

        Fls_u8JobStart = 1U;

        /* Execute the read job */
        Fls_eJobResult = MEMIF_JOB_PENDING;

    }
    return( u8RetVal );
}

/**
* @brief            Performs actual flash read, write, erase and compare jobs.
* @details          Bytes number processed per cycle depends by job type (erase, write, read, compare)
*                   current FLS module's operating mode (normal, fast)
*                   and write, erase Mode of Execution (sync, async).
*
* @api
*
* @pre              The module has to be initialized.
*
*
* @implements       Fls_MainFunction_Activity
*
* @note             This function have to be called ciclically by the Basic Software Module;
*                   it will do nothing if there aren't pending job.
*/
/*
* @violates @ref fls_c_REF_8 All declarations and definitions
* of objects or functions at file scope shall have internal linkage unless external linkage required
*/
void Fls_MainFunction( void )
{
    if( MEMIF_JOB_PENDING == Fls_eJobResult )
    {
        MemIf_JobResultType eWorkResult = MEMIF_JOB_OK;

        if( 1U == Fls_u8JobStart )
        {
            Fls_u8JobStart = 0U;

            /* Clear result of hardware job */
            Fls_LLDClrJobResult();

            //Fls_IPW_AbortSuspended();

        }

        /* Check the status of pending jobs. */
        if( FLASH_JOB_NONE != Fls_eLLDJob ) /* If there is any Async job scheduled. */
        {
            /* Process ongoing erase or write asynchronous hardware job */
            Fls_IPW_LLDMainFunction();  /* Process the maximum defined length(configuration parameter), or until a sector boundary. */
        }
        else
        {
            /* Empty clause added to fulfill MISRA. */
        }

        /* Get status/result of ongoing erase or write Async or Irq mode hardware job */
        eWorkResult = Fls_LLDGetJobResult();


        if( MEMIF_JOB_OK == eWorkResult )
        {
            switch( Fls_eJob )
            {
                case FLS_JOB_ERASE:
                        eWorkResult = Fls_DoJobErase();
                    break;
                case FLS_JOB_WRITE:
                        eWorkResult = Fls_DoJobDataTransfer( Fls_eJob, Fls_u32MaxWrite );
                    break;
                case FLS_JOB_READ:
                /* For Read and Compare jobs the static function Fls_DoJobDataTransfer( )
                   will be used with the same input parameters */
                        eWorkResult = Fls_DoJobDataTransfer( Fls_eJob, Fls_u32MaxRead );
                    break;
                default:
                    /* Do nothing - should not happen in Fully Trusted Environment;
                        'default' clause added to fulfill MISRA Rule 15.3 */
                    break;
            }
        }
        else
        {
            /* eWorkResult != MEMIF_JOB_OK */
        }

        Fls_eJobResult = eWorkResult;

        if( MEMIF_JOB_OK == eWorkResult )
        {
            if( NULL_PTR != Fls_pConfigPtr->jobEndNotificationPtr )
            {
                /* Call FlsJobEndNotification function if configured */
                Fls_pConfigPtr->jobEndNotificationPtr();
            }
            /*else
            {
                    Callback notification configured as null pointer
            }*/
        }
        else if(( MEMIF_JOB_FAILED == eWorkResult ) ||
                ( MEMIF_BLOCK_INCONSISTENT == eWorkResult )
                )
        {

            if( NULL_PTR != Fls_pConfigPtr->jobErrorNotificationPtr )
            {
                /* Call FlsJobErrorNotification function if configured */
                Fls_pConfigPtr->jobErrorNotificationPtr();
            }
            else
            {
                /* Callback notification configured as null pointer */
            }
        }
        else
        {
            /* Compiler_Warning: else clause added to fulfil the MISRA rule 14.10 */
            /* Not done yet. There is more data to transfer in following
                Fls_MainFunction call(for Async jobs, or in interrupt context for IRQ type jobs). */
        }
    }
    else
    {
        /* Nothing to do since no job is pending */
    }
}

/**
* @brief          Returns the LLD JobResult
*/
static MemIf_JobResultType Fls_LLDGetJobResult( void )
{
    return( Fls_eLLDJobResult );
}

/**
* @brief          Clear status of erase or write hardware job result.
* @details        Set the internal status to  MEMIF_JOB_OK
*/
static void Fls_LLDClrJobResult( void )
{
    Fls_eLLDJobResult = MEMIF_JOB_OK;
    Fls_eLLDJob = FLASH_JOB_NONE;
    Fls_eLLDIrqJob = FLASH_IRQ_JOB_NONE;
}
