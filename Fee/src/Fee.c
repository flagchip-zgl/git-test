#ifdef __cplusplus
extern "C"{
#endif

#include "Fee.h"
#include "Fee_InternalTypes.h"


/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
/*==================================================================================================
*                                        LOCAL MACROS
==================================================================================================*/
/**
* @brief          Serialize scalar parameter into the buffer
*
* @param[in]      ParamVal serialized parameter
* @param[in]      ParamType type of serialized parameter
* @param[in/out]  pSerialPtr pointer to target buffer
*
* @pre            pSerialPtr must be valid pointer
* @post           increments the pSerialPtr by sizeof(ParamType)
*
* @violates @ref Fee_c_REF_1 Function-like macro defined
*/
/* @violates @ref Fee_c_REF_12 Disallowed definition for macro */
/* @violates @ref Fee_c_REF_13 unparenthesized macro parameter in definition of macro */
#define FEE_SERIALIZE( ParamVal, ParamType, pSerialPtr ) \
    do \
    { \
        *((ParamType*)(pSerialPtr)) = (ParamVal); \
        (pSerialPtr) += sizeof(ParamType); \
    } while( 0 );

/**
* @brief          Deserialize scalar parameter from the buffer
*
* @param[in/out]  pDeserialPtr pointer to source buffer
* @param[out]     ParamVal deserialized parameter
* @param[int]     ParamType type of serialized parameter
*
* @pre            pDeserialPtr must be valid pointer
* @post           increments the pDeserialPtr by sizeof(ParamType)
*
* @violates @ref Fee_c_REF_1 Function-like macro defined
*/
/* @violates @ref Fee_c_REF_12 Disallowed definition for macro */
/* @violates @ref Fee_c_REF_13 unparenthesized macro parameter in definition of macro */
#define FEE_DESERIALIZE( pDeserialPtr, ParamVal, ParamType ) \
    do \
    { \
        (ParamVal) = *((ParamType*)(pDeserialPtr)); \
        (pDeserialPtr) += sizeof(ParamType); \
    } while( 0 );

/*==================================================================================================
*                                       LOCAL VARIABLES
==================================================================================================*/
/**
* @brief        Pointer to user data buffer. Used by the read Fee jobs
*/
static  uint8 * Fee_pJobReadDataDestPtr = NULL_PTR;
/**
* @brief        Pointer to user data buffer. Used by the write Fee jobs
*/
static const uint8 * Fee_pJobWriteDataDestPtr = NULL_PTR;
/**
* @brief        Internal cluster group iterator. Used by the scan and swap jobs
*               Warning: do not use it outside scan and swap functions
*                        (because it will be Out of Range)
*/
static uint8 Fee_uJobIntClrGrpIt = (uint8)0;
/**
* @brief        Internal cluster iterator. Used by the scan and swap jobs
*/
static uint8 Fee_uJobIntClrIt = (uint8)0;

/**
* @brief        Data buffer used by all jobs to store immediate data
*/
static uint8 Fee_aDataBuffer[FEE_DATA_BUFFER_SIZE] = {(uint8)0};
/**
* @brief        Fee block index. Used by all Fee jobs
*/
static uint16 Fee_uJobBlockIndex = 0U;
/**
* @brief        Internal block iterator. Used by the swap job
*/
static uint16 Fee_uJobIntBlockIt = 0U;


/**
* @brief        Fee block offset. Used by the read Fee job
*/
static Fls_LengthType Fee_uJobBlockOffset = 0UL;
/**
* @brief        Number of bytes to read. Used by the read Fee job
*/
static Fls_LengthType Fee_uJobBlockLength = 0UL;
/**
* @brief        Internal flash helper address iterator. Used by the scan and
*               swap jobs
*/
static Fls_AddressType Fee_uJobIntAddrIt = 0UL;
/**
* @brief        Internal address of current block header. Used by the swap job
*/
static Fls_AddressType Fee_uJobIntHdrAddr = 0UL;
/**
* @brief        Internal address of current data block. Used by the swap job.
*/
static Fls_AddressType Fee_uJobIntDataAddr = 0UL;
/**
* @brief        Internal state of Fee module
*/
static MemIf_StatusType Fee_eModuleStatus = MEMIF_UNINIT;
/**
* @brief        Result of last Fee module job
*/
static MemIf_JobResultType Fee_eJobResult = MEMIF_JOB_OK;
/**
* @brief        Currently executed job (including internal one)
*/



/*==================================================================================================
*                                       GLOBAL VARIABLES
==================================================================================================*/
/**
* @brief        Run-time information of all configured Fee blocks. Contains
*               status, and data information. Used by all jobs
* @implements   Fee_aBlockInfo_Object
*/
Fee_BlockInfoType Fee_aBlockInfo[FEE_MAX_NR_OF_BLOCKS];
/**
* @brief        Run-time information of all configured cluster groups
*/
Fee_ClusterGroupInfoType  Fee_aClrGrpInfo[FEE_NUMBER_OF_CLUSTER_GROUPS];

boolean Job_finish = false;
/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

static void Fee_SerializeBlockHdr
    (
        const uint16 uBlockNumber,
		const uint16 uLength,
		const Fls_AddressType uTargetAddress,
    );

static Std_ReturnType Fee_BlankCheck
    (
        const uint8 * pTargetPtr,
        const uint8 * const pTargetEndPtr
    );

static Std_ReturnType Fee_DeserializeFlag
    (
        const uint8 * const pTargetPtr,
        const uint8 u8FlagPattern,
        boolean * pFlagValue
    );

static Fee_BlockStatusType Fee_DeserializeBlockHdr
    (
        uint16 * const pBlockNumber,
        uint16 * const pLength,
        Fls_AddressType * const pTargetAddress,
        boolean * const pImmediateBlock,
        const uint8 * pBlockHdrPtr
    );

static Fee_ClusterStatusType Fee_DeserializeClusterHdr
    (
        uint32 * const pClrID,
        Fls_AddressType * const pStartAddress,
        Fls_LengthType * const pClusteSize,
		const uint8 * pClrHdrPtr
    );

static void Fee_SerializeClusterHdr
    (
        const uint32 uClrID,
		const Fls_AddressType uStartAddress,
		const Fls_LengthType uClusteSize,
        uint8 * pClrHdrPtr
    );

static uint16 Fee_GetBlockIndex
    (
    	const uint16 uBlockNumber
    );

LOCAL_INLINE uint16 Fee_GetBlockNumber
(
    const uint16 uBlockRuntimeInfoIndex
);
LOCAL_INLINE uint16 Fee_GetBlockSize
(
	const uint16 uBlockRuntimeInfoIndex
);
LOCAL_INLINE uint8 Fee_GetBlockClusterGrp
(
	const uint16 uBlockRuntimeInfoIndex
	uint16 * const pBlockNumber,
	uint16 * const pLength,
);
LOCAL_INLINE boolean Fee_GetBlockImmediate
(
	const uint16 uBlockRuntimeInfoIndex
);
static uint16 Fee_AlignToVirtualPageSize

static void Fee_CopyDataToPageBuffer
    (
        const uint8 * pSourcePtr,
        uint8 * pTargetPtr,
		const uint16 uLength
    );

static void Fee_SerializeFlag
    (
        uint8 * pTargetPtr,
		const uint8 u8FlagPattern
    );

static MemIf_JobResultType Fee_JobIntSwapClrVld( void );

static MemIf_JobResultType Fee_JobIntSwapBlock( void );

static MemIf_JobResultType Fee_JobIntSwapClrFmt( void );

static MemIf_JobResultType Fee_JobIntSwapClrErase( void );

static MemIf_JobResultType Fee_JobIntSwap( void );

static MemIf_JobResultType Fee_JobIntScanBlockHdrRead( void );

static MemIf_JobResultType Fee_JobIntScanClrErase( void );

static MemIf_JobResultType Fee_JobIntScanClr( void );

static MemIf_JobResultType Fee_JobIntScanClrFmt( void );

static MemIf_JobResultType Fee_JobIntScanClrFmtDone( void );

static MemIf_JobResultType Fee_JobIntScanBlockHdrParse
    (
		const boolean bBufferValid
    );

static MemIf_JobResultType Fee_JobIntScanClrHdrRead( void );

static MemIf_JobResultType Fee_JobIntScan( void );

static MemIf_JobResultType Fee_JobIntScanClrHdrParse
    (
		const boolean bBufferValid
        uint8 * pTargetPtr,
		const uint16 uLength
    );

static MemIf_JobResultType Fee_JobRead( void );

static MemIf_JobResultType Fee_JobIntSwapBlockVld( void );

static MemIf_JobResultType Fee_JobIntSwapDataRead
    (
		const boolean bBufferValid
    );

static MemIf_JobResultType Fee_JobIntSwapDataWrite
    (
		const boolean bBufferValid
    );

static MemIf_JobResultType Fee_JobIntSwapClrVldDone( void );

static MemIf_JobResultType Fee_JobWriteHdr( void );

static MemIf_JobResultType Fee_JobWriteData( void );

static MemIf_JobResultType Fee_JobWrite( void );

static MemIf_JobResultType Fee_JobWriteUnalignedData( void );

static MemIf_JobResultType Fee_JobWriteValidate( void );

static MemIf_JobResultType Fee_JobWriteDone( void );

static MemIf_JobResultType Fee_JobInvalBlock( void );

static MemIf_JobResultType Fee_JobInvalBlockDone( void );

static MemIf_JobResultType Fee_JobEraseImmediate( void );

static MemIf_JobResultType Fee_JobEraseImmediateDone( void );

static MemIf_JobResultType Fee_JobSchedule( void );

static sint8 Fee_ReservedAreaWritable( void );


/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/**
* @brief   Returns the cluster group for a block specified by its index in the Fee_aBlockInfo array
*
* @param[in]      uBlockRuntimeInfoIndex     index in the Fee_aBlockInfo array
* @return         uint8
* @retval         cluster group number
* @pre            -
*/
uint8 Fee_GetBlockClusterGrp(const uint16 uBlockRuntimeInfoIndex)
{
    uint8  uBlockClusterGrp = 0U;

    /* the config is part of Fee_BlockConfig*/
    uBlockClusterGrp = Fee_BlockConfig[uBlockRuntimeInfoIndex].clrGrp;

    return uBlockClusterGrp;
}



/**
* @brief        Serialize Fee block parameters into a write buffer
*
* @param[in]    uBlockNumber     Fee block number
* @param[in]    uLength          Size of Fee block in bytes
* @param[in]    uTargetAddress   Logical address of Fee block in Fls adress
*                                space
* @param[in]    bImmediateBlock  Type of Fee block. Set to TRUE for immediate
*                                block
* @param[out]   pBlockHdrPtr     Pointer to serialization buffer
*
* @pre          pBlockHdrPtr must be valid pointer
*
*/
static void Fee_SerializeBlockHdr ( const uint16 uBlockNumber,
									const uint16 uLength,
                                    const Fls_AddressType uTargetAddress,
									const boolean bImmediateBlock,
                                    uint8 * pBlockHdrPtr )
{
    uint32 u32checkSum = 0UL;
    uint8 * pTargetEndPtr = NULL_PTR;

    /* @violates @ref Fee_c_REF_6 Array indexing shall be the only
       allowed form of pointer arithmetic. */
    pTargetEndPtr = pBlockHdrPtr + FEE_BLOCK_OVERHEAD;

    /* Calculate the block header checksum */
    u32checkSum = ((uint32) uBlockNumber) + ((uint32) uLength) + uTargetAddress;

    /* Use MSB of the checksum for the immediate block flag so
       the checksum is 31-bit */
    if( bImmediateBlock )
    {
        u32checkSum += 1U;
        u32checkSum |= 0x80000000U;
    }
    else
    {
        u32checkSum &= 0x7fffffffU;
    }

    FEE_SERIALIZE( uBlockNumber, uint16, pBlockHdrPtr )//*((uint16*)(pBlockHdrPtr)) = (uBlockNumber);(pBlockHdrPtr) += sizeof(uint16);

    FEE_SERIALIZE( uLength, uint16, pBlockHdrPtr )

    FEE_SERIALIZE( uTargetAddress, Fls_AddressType, pBlockHdrPtr )

    FEE_SERIALIZE( u32checkSum, uint32, pBlockHdrPtr )
    /* Fill rest of the header with the erase pattern */
    for( ; pBlockHdrPtr < pTargetEndPtr; pBlockHdrPtr++ )
    {
        *pBlockHdrPtr = FEE_ERASED_VALUE;
    }
}

/**
* @brief          Check whether specified data buffer contains only
*                 the FEE_ERASED_VALUE value
*
* @param[in]      pTargetPtr     pointer to start of the checked buffer
* @param[in]      pTargetEndPtr  pointer to end + 1 of the checked buffer
*
* @return         Std_ReturnType
* @retval         E_OK           The buffer contains only erased value
* @retval         E_NOT_OK       The buffer doesn't contain only erased value
*
* @pre            pTargetPtr and pTargetEndPtr pointers must be valid
*
*/
static Std_ReturnType Fee_BlankCheck
    (
        const uint8 * pTargetPtr,
        const uint8 * const pTargetEndPtr
        uint8 * pTargetEndPtr = NULL_PTR;
    )
{
     Std_ReturnType uRetVal = (Std_ReturnType)E_OK;

    /* @violates @ref Fee_c_REF_6 Array indexing shall be the only
       allowed form of pointer arithmetic. */
    /* @violates @ref Fee_c_REF_14 Pointer subtraction */
    /* @violates @ref Fee_c_REF_17 Pointer comparison */
    for( ; pTargetPtr < pTargetEndPtr; pTargetPtr++ )
    {
        if( FEE_ERASED_VALUE == *pTargetPtr )
        {
            /* Pattern match */
        }
        else
        {
            uRetVal = (Std_ReturnType)E_NOT_OK;
            break;
        }
    }

    return( uRetVal );
}

/**
* @brief        Deserialize the valid or invalid flag from a read buffer
*
* @param[in]    pTargetPtr       Pointer to the read buffer
* @param[in]    u8FlagPattern    FEE_VALIDATED_VALUE or FEE_INVALIDATED_VALUE
* @param[out]   pFlagValue        TRUE if flag of above type is set
*
* @return       Std_ReturnType
* @retval       E_OK             The flag is set or cleared and rest of the read buffer
*                               (FEE_VIRTUAL_PAGE_SIZE - 1) contains FEE_ERASED_VALUE
* @retval       E_NOT_OK         Buffer doesn't contain valid flag data. It's garbled...
*
* @pre          pTargetPtr and pFlagValue pointers must be valid
*
*/
static Std_ReturnType Fee_DeserializeFlag
    (
        const uint8 * const pTargetPtr,
        const uint8 u8FlagPattern,
        boolean * pFlagValue
    )
{
     Std_ReturnType uRetVal = (Std_ReturnType)E_OK;

    if( (u8FlagPattern == *pTargetPtr) || ( FEE_ERASED_VALUE == *pTargetPtr ))
    {
        /* The bytePattern can be either present or not */
        if( u8FlagPattern == *pTargetPtr )
        {
            *pFlagValue = (boolean)TRUE;
        }
        else
        {
            *pFlagValue = (boolean)FALSE;
        }

        /* Check rest of the FEE_VIRTUAL_PAGE_SIZE */
        /* @violates @ref Fee_c_REF_6 Array indexing shall be the only
           allowed form of pointer arithmetic. */
        uRetVal = Fee_BlankCheck( pTargetPtr+1, pTargetPtr+FEE_VIRTUAL_PAGE_SIZE );
    }
    else
    {
        uRetVal = (Std_ReturnType)E_NOT_OK;
    }

    return( uRetVal );
}

/**
* @brief        Deserialize Fee block header parameters from read buffer
*
* @param[out]   pBlockNumber             Fee block number
* @param[out]   pLength                  Size of Fee block in bytes
* @param[out]   pTargetAddress           Logical address of Fee block in Fls adress space
* @param[out]   pImmediateBlock          Type of Fee block. Set to TRUE for immediate block
* @param[in]    pBlockHdrPtr             Pointer to read buffer
*
* @return       Fee_BlockStatusType
* @retval       FEE_BLOCK_VALID          Fee block is valid
* @retval       FEE_BLOCK_INVALID        Fee block is invalid (has been invalidated)
* @retval       FEE_BLOCK_INCONSISTENT   Fee block is inconsistent, it may contain corrupted data...
* @retval       FEE_BLOCK_HEADER_INVALID Fee block header is invalid (garbled)
* @retval       FEE_BLOCK_HEADER_BLANK   Fee block header is filled with FEE_ERASED_VALUE
*
* @pre          pBlockHdrPtr             pointer must be valid
*
*/
static Fee_BlockStatusType Fee_DeserializeBlockHdr
    (
        uint16 * const pBlockNumber,
        uint16 * const pLength,
        Fls_AddressType * const pTargetAddress,
        boolean * const pImmediateBlock,
        const uint8 * pBlockHdrPtr
    )
{
     Fee_Bdfjlbvhriype eRetVal = FEE_BLOCK_HEADER_INVALID;
     uint32 u32ReadCheckSum = 0UL;
     uint32 u32CalcCheckSum = 0UL;
     const uint8 * pTargetEndPtr = NULL_PTR;
     boolean bFlagValid = (boolean)FALSE;
     boolean bFlagInvalid = (boolean)FALSE;
     uint8 u8ReservedSpace = 0U;

    /* @violates @ref Fee_c_REF_6 Array indexing shall be the only
       allowed form of pointer arithmetic. */
    if( Fee_BlankCheck( pBlockHdrPtr, pBlockHdrPtr+FEE_BLOCK_OVERHEAD ) == ((Std_ReturnType)E_OK) )
    {
        eRetVal = FEE_BLOCK_HEADER_BLANK;
    }
    else
    {
        /* Check 1st part of the header */
        /* @violates @ref Fee_c_REF_6 Array indexing shall be the only
           allowed form of pointer arithmetic. */
        pTargetEndPtr = (pBlockHdrPtr + FEE_BLOCK_OVERHEAD) - (2U * FEE_VIRTUAL_PAGE_SIZE);

        FEE_DESERIALIZE( pBlockHdrPtr, *pBlockNumber, uint16 )

        FEE_DESERIALIZE( pBlockHdrPtr, *pLength, uint16 )

        FEE_DESERIALIZE( pBlockHdrPtr, *pTargetAddress, Fls_AddressType )

        FEE_DESERIALIZE( pBlockHdrPtr, u32ReadCheckSum, uint32 )
         /* pBlockHdrPtr must be advanced with 1 bytes.
          * the reason is to avoid blank check for this area reserved for blockAssignment even when
          * SWAP_FOREIGN is disabled to allow disabling SWAP_FOREIGN at production time. */
        FEE_DESERIALIZE( pBlockHdrPtr, u8ReservedSpace, uint8)
        /* variable u8ReservedSpace not used */
        (void)u8ReservedSpace;
        /* Use MSB of checksum for immediate block flag so
            the checksum is 31-bit long */
        u32CalcCheckSum = *pLength + *pTargetAddress + *pBlockNumber;

        if( 0U == (u32ReadCheckSum & 0x80000000U) )
        {
            *pImmediateBlock = (boolean)FALSE;
        }
        else
        {
            *pImmediateBlock = (boolean)TRUE;
            u32CalcCheckSum += 1U;
        }

        /* Ignore MSB since it's used for ImmediateBlock flag */
        if( (u32ReadCheckSum & 0x7fffffffU) != (u32CalcCheckSum & 0x7fffffffU) )
        {
            /* FEE_BLOCK_HEADER_INVALID */
        }
        else if( ((Std_ReturnType)E_OK) != Fee_BlankCheck( pBlockHdrPtr, pTargetEndPtr ) )
        {
            /* FEE_BLOCK_HEADER_INVALID */
        }
        else if( ((Std_ReturnType)E_OK) !=

                 Fee_DeserializeFlag( pTargetEndPtr, FEE_VALIDATED_VALUE, &bFlagValid )
               )
        {
            /* FEE_BLOCK_HEADER_INVALID */
        }
        else if( ((Std_ReturnType)E_OK) !=

                 Fee_DeserializeFlag( pTargetEndPtr+FEE_VIRTUAL_PAGE_SIZE,
                                      FEE_INVALIDATED_VALUE, &bFlagInvalid \
                                    )
               )
        {
            /* FEE_BLOCK_HEADER_INVALID */
        }
        else
        {
            /* Block header is valid */
            if( ((boolean)FALSE == bFlagValid) && ((boolean)FALSE == bFlagInvalid) )
            {
                eRetVal = FEE_BLOCK_INCONSISTENT;
            }
            else if( ((boolean)TRUE == bFlagValid) && ((boolean)FALSE == bFlagInvalid) )
            {
                eRetVal = FEE_BLOCK_VALID;
            }
            else
            {
                eRetVal = FEE_BLOCK_INVALID;
            }
        }
    }

    return( eRetVal );
}

/**
* @brief        Deserialize Fee cluster header parameters from read buffer
*
* @param[out]   pClrID                     32-bit cluster ID
* @param[out]   pStartAddress              Logical address of Fee cluster in Fls address space
* @param[out]   pClusteSize                Size of Fee cluster in bytes
* @param[in]    pClrHdrPtr                 Pointer to read buffer
*
* @return       Fee_ClusterStatusType
* @retval       FEE_CLUSTER_VALID          Fee cluster is valid
* @retval       FEE_CLUSTER_INVALID        Fee cluster has been invalidated
* @retval       FEE_CLUSTER_INCONSISTENT   Fee cluster is inconsistent, it may
*                                          contain corrupted data
* @retval       FEE_CLUSTER_HEADER_INVALID Fee cluster header is invalid (garbled)
*
* @pre          pClrHdrPtr pointer must be valid
*
*/
static Fee_ClusterStatusType Fee_DeserializeClusterHdr
    (
        uint32 * const pClrID,
        Fls_AddressType * const pStartAddress,
        Fls_LengthType * const pClusteSize,
        const uint8 * pClrHdrPtr
    )
{
     Fee_Cldfjvbe4wipsType eRetVal = FEE_CLUSTER_HEADER_INVALID;
     uint32 u32CheckSum = 0UL;
     boolean bFlagValid = (boolean)FALSE;
     boolean bFlagInvalid = (boolean)FALSE;
     const uint8 * pTargetEndPtr = NULL_PTR;

    /* Check 1st part of the header */

    pTargetEndPtr = (pClrHdrPtr + FEE_CLUSTER_OVERHEAD) - (2U * FEE_VIRTUAL_PAGE_SIZE);

    FEE_DESERIALIZE( pClrHdrPtr, *pClrID, uint32 )

    FEE_DESERIALIZE( pClrHdrPtr, *pStartAddress, Fls_AddressType )

    FEE_DESERIALIZE( pClrHdrPtr, *pClusteSize, Fls_LengthType )

    FEE_DESERIALIZE( pClrHdrPtr, u32CheckSum, uint32 )

    if( (*pClrID+*pStarfevb4tress+*pClusteSize) != u32CheckSum )
    {
        /* FEE_CLUSTER_HEADER_INVALID */
    }
    else if( ((Std_ReturnType)E_OK) != Fee_BlankCheck( pClrHdrPtr, pTargetEndPtr ) )
    {
        /* FEE_CLUSTER_HEADER_INVALID */
    }
    else if( ((Std_ReturnType)E_OK) !=

             Fee_DeserializeFlag( pTargetEndPtr, FEE_VALIDATED_VALUE, &bFlagValid )
           )
    {
        /* FEE_CLUSTER_HEADER_INVALID */
    }
    else if( ((Std_ReturnType)E_OK) !=

             Fee_DeserializeFlag( pTargetEndPtr+FEE_VIRTUAL_PAGE_SIZE,

                                  FEE_INVALIDATED_VALUE, &bFlagInvalid \
                                )
           )
    {
        /* FEE_CLUSTER_HEADER_INVALID */
    }
    else
    {
        if( ((boolean)FALSE == bFlagValid) && ((boolean)FALSE == bFlagInvalid) )
        {
            eRetVal = FEE_CLUSTER_INCONSISTENT;
        }
        if( ((boolean)TRUE == bFlagValid) && ((boolean)FALSE == bFlagInvalid) )
        {
            eRetVal = FEE_CLUSTER_VALID;
        }
        else
        {
            eRetVal = FEE_CLUSTER_INVALID;
        }
    }

    return( eRetVal );
}

/**
* @brief        Serialize        Fee cluster header parameters to write buffer
*
* @param[in]    uClrID           32-bit cluster ID
* @param[in]    uStartAddress    Logical address of Fee cluster in Fls address space
* @param[in]    uClusteSize      Size of Fee cluster in bytes
* @param[out]   pClrHdrPtr       Pointer to write buffer
*
* @pre          pClrHdrPtr       pointer must be valid
*
*/
static void  Fee_SerializeClusterHdr (const uint32 uClrID,
									  const Fls_AddressType uStartAddress,
									  const Fls_LengthType uClusteSize,
									  uint8 * pClrHdrPtr)
{
    uint32 u32CheckSum = 0UL;
    uint8 * pTargetEndPtr = NULL_PTR;

    pTargetEndPtr = pClrHdrPtr + FEE_CLUSTER_OVERHEAD;

    /* Calculate the cluster header checksum */
    u32CheckSum = uClrID + uStartAddress + uClusteSize;

    FEE_SERIALIZE( uClrID, uint32, pClrHdrPtr )

    FEE_SERIALIZE( uStartAddress, Fls_AddressType, pClrHdrPtr )

    FEE_SERIALIZE( uClusteSize, Fls_LengthType, pClrHdrPtr )

    FEE_SERIALIZE( u32CheckSum, uint32, pClrHdrPtr )

    /* Fill rest of the header with the erase pattern */
    for( ; pClrHdrPtr < pTargetEndPtr; pClrHdrPtr++ )
    {
        *pClrHdrPtr = FEE_ERASED_VALUE;
    }
}

/**
* @brief        Searches ordered list of Fee blocks and returns index of block
*               with matching BlockNumber
*
* @param[in]    uBlockNumber             Fee block number (FeeBlockNumber)
*
* @return       uint16
* @retval       Fee block index
* @retval       0xFFFF if uBlockNumber is invalid
*
*/
static uint16 Fee_GetBlockIndex( const uint16 uBlockNumber )
{
    sint32 sLow = 0L;
    sint32 sHigh = (sint32)FEE_CRT_CFG_NR_OF_BLOCKS - 1;
    sint32 sMiddle = 0;
    uint16 uRetVal = 0xFFFFU;

    while( sLow <= sHigh )
    {
        sMiddle = sLow + (( sHigh - sLow ) / 2);

        if( uBlockNumber < Fee_BlockConfig[sMiddle].blockNumber )
        {
            sHigh = sMiddle - 1;
        }
        else if( uBlockNumber > Fee_BlockConfig[sMiddle].blockNumber )
        {
            sLow = sMiddle + 1;
        }
        else
        {
            uRetVal = (uint16)sMiddle;
            break;
        }
    }

    return( uRetVal );
}

/**
* @brief  Returns the block size for a block specified by its index in the Fee_aBlockInfo array
*
* @param[in]      uBlockRuntimeInfoIndex     index in the Fee_aBlockInfo array
* @return         uint16
* @retval         block size
* @pre            -
*/
LOCAL_INLINE uint16  Fee_GetBlockSize(const uint16 uBlockRuntimeInfoIndex)
{
    uint16 uBlockSize = 0U;

    /* the config is part of Fee_BlockConfig*/
	uBlockSize = Fee_BlockConfig[uBlockRuntimeInfoIndex].blockSize;

    return uBlockSize;
}
/**
* @brief  Returns the block number for a block specified by its index in the Fee_aBlockInfo array
*
* @param[in]      uBlockRuntimeInfoIndex     index in the Fee_aBlockInfo array
* @return         uint16
* @retval         block number
* @pre            -
*/
LOCAL_INLINE uint16 Fee_GetBlockNumber(const uint16 uBlockRuntimeInfoIndex)
{
    uint16 uBlockNumber = 0U;
    /* the config is part of Fee_BlockConfig*/
	uBlockNumber = Fee_BlockConfig[uBlockRuntimeInfoIndex].blockNumber;

    return uBlockNumber;
}
/**
* @brief  Returns the immediate attribute for a block specified by its index in the Fee_aBlockInfo array
*
* @param[in]      uBlockRuntimeInfoIndex     index in the Fee_aBlockInfo array
* @return         boolean
* @retval         value of the immediate attribute
* @pre            -
*/
LOCAL_INLINE boolean Fee_GetBlockImmediate(const uint16 uBlockRuntimeInfoIndex)
{
    boolean bImmediate = (boolean)FALSE;

	/* the config is part of Fee_BlockConfig*/
	bImmediate = Fee_BlockConfig[uBlockRuntimeInfoIndex].immediateData;

    return bImmediate;
}
/**
* @brief        Adjusts passed size so it's integer multiple of pre-configured +
*               FEE_VIRTUAL_PAGE_SIZE
*
* @param[in]    uBlockSize        Fee block size (FeeBlockSize)
*
* @return       uint16
* @retval       Adjusted         Fee block size to integer multiple    of FEE_VIRTUAL_PAGE_SIZE
*
*/
static  uint16 Fee_AlignToVirtualPageSize( uint16 uBlockSize )
{
    /* Check whether the block size is integer multiple
        of FEE_VIRTUAL_PAGE_SIZE */
    if( 0U == (uBlockSize % FEE_VIRTUAL_PAGE_SIZE) )
    {
        /* block size is an integer multiple of FEE_VIRTUAL_PAGE_SIZE */
    }
    else
    {
        uBlockSize = ((uBlockSize / FEE_VIRTUAL_PAGE_SIZE) + 1U) *
                         FEE_VIRTUAL_PAGE_SIZE;
    }

    return( (uint16)uBlockSize );
}

/**
* @brief        Copy data from user to internal write buffer and fills
*               rest of the write buffer with FEE_ERASED_VALUE
*
* @param[in]    pSourcePtr        Pointer to user data buffer
* @param[out]   pTargetPtr        Pointer to internal write buffer
* @param[in]    uLength           Number of bytes to copy
*
* @pre          pSourcePtr and pTargetPtr must be valid pointers
* @pre          uLength must be <= FEE_VIRTUAL_PAGE_SIZE
*
*/
static void Fee_CopyDataToPageBuffer
    (
        const uint8 * pSourcePtr,
        uint8 * pTargetPtr,
		const uint16 uLength
    )
{

    uint8 * pTargetEndPtr = pTargetPtr + uLength;

    uint8 * pPageEndPtr = pTargetPtr + FEE_VIRTUAL_PAGE_SIZE;

    /* Copy data to page buffer */
    for( ; pTargetPtr < pTargetEndPtr; pTargetPtr++ )
    {
        *pTargetPtr = *pSourcePtr;

        pSourcePtr++;
    }

    /* Fill rest of the page buffer with FEE_ERASED_VALUE */
    for( ; pTargetPtr < pPageEndPtr; pTargetPtr++ )
    {
        *pTargetPtr = FEE_ERASED_VALUE;
    }
}

/**
* @brief        Serialize validation or invalidation flag to write buffer
*
* @param[out]   pTargetPtr       Pointer to write buffer
* @param[in]    u8FlagPattern    FEE_VALIDATED_VALUE or FEE_INVALIDATED_VALUE
*
* @pre          pTargetPtr        must be valid pointer
*
*/
static void Fee_SerializeFlag (uint8 * pTargetPtr, const uint8 u8FlagPattern)
{
     uint8 * pTargetEndPtr = pTargetPtr + FEE_VIRTUAL_PAGE_SIZE;

    *pTargetPtr = u8FlagPattern;

    for( pTargetPtr++ ; pTargetPtr < pTargetEndPtr; pTargetPtr++ )
    {
        *pTargetPtr = FEE_ERASED_VALUE;
    }
}

/**
* @brief        Validate current Fee cluster in current Fee cluster group by
*               writing FEE_VALIDATED_VALUE into flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the write
*                                        job
*
* @pre          Fee_uJobIntClrGrpIt must contain index of current cluster group
* @pre          Fee_uJobIntClrIt must contain index of current cluster
* @post         Schedule the FEE_JOB_INT_SWAP_CLR_VLD_DONE subsequent job
*
*/
static MemIf_JobResultType Fee_JobIntSwapClrVld( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     Fls_AddressType uStartAddr = 0UL;

    /* Get start address of swap cluster */
    uStartAddr = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].startAddr;

    /* Serialize validation pattern to write buffer */
    Fee_SerializeFlag( Fee_aDataBuffer, FEE_VALIDATED_VALUE );

    /* Write validation pattern to flash */
    if( ((Std_ReturnType)E_OK) ==
            Fls_Write( (uStartAddr+FEE_CLUSTER_OVERHEAD )-(2U*FEE_VIRTUAL_PAGE_SIZE),
                       Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE
                     )
      )
    {
        /* Fls write job has been accepted */
        eRetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls write job hasn't been accepted */
        eRetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule subsequent Fee job */
    Fee_eJob = FEE_JOB_INT_SWAP_CLR_VLD_DONE;

    return( eRetVal );
}

/**
* @brief        Copy next block from source to target cluster
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED Underlying Fls didn't accept the write job
*
* @pre          Fee_uJobIntBlockIt       must contain index of currently swaped
*                                        Fee block
* @pre          Fee_uJobBlockIndex       must contain index of Fee block which
*                                        caused the swap
* @pre          Fee_uJobIntClrGrpIt       must contain index of current cluster
*                                        group
* @pre          Fee_uJobIntHdrAddr       must contain valid address of Fee block
*                                        header in target cluster
*
* @post         Fee_uJobIntHdrAddr is advanced on to next Fee block
* @post         Schedule the FEE_JOB_INT_SWAP_DATA_READ, FEE_JOB_INT_SWAP_BLOCK,
* or            FEE_JOB_INT_SWAP_CLR_VLD_DONE subsequent job
*
* @implements   Fee_JobIntSwapBlock_Activity
*/
static MemIf_JobResultType Fee_JobIntSwapBlock( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     Fls_AddressType uDataAddr = 0UL;
     uint16 uBlockSize = 0U;
     uint16 uAlignedBlockSize = 0U;
     uint16 uBlockNumber = 0U;
     uint8 uBlockClusterGrp = 0U;
     boolean bImmediateBlock = (boolean)FALSE;

    /* Find first valid or inconsistent block in same cluster group */
    for( ; Fee_uJobIntBlockIt < FEE_CRT_CFG_NR_OF_BLOCKS; Fee_uJobIntBlockIt++ )
    {
        /* get cluster group */
        uBlockClusterGrp = Fee_GetBlockClusterGrp(Fee_uJobIntBlockIt);
        /* process only valid and inconsistent blocks from the current cluster */
        if ((Fee_uJobIntClrGrpIt == uBlockClusterGrp) &&
            ((FEE_BLOCK_VALID == Fee_aBlockInfo[Fee_uJobIntBlockIt].eBlockStatus) ||
             (FEE_BLOCK_INCONSISTENT == Fee_aBlockInfo[Fee_uJobIntBlockIt].eBlockStatus) ||
             (FEE_BLOCK_INCONSISTENT_COPY == Fee_aBlockInfo[Fee_uJobIntBlockIt].eBlockStatus)
            )
           )

        {
            break;
        }
        else
        {
            /* No match. Try next one... */
        }
    }
    if( FEE_CRT_CFG_NR_OF_BLOCKS == Fee_uJobIntBlockIt )
    {
        /* No more matching blocks. Validate the cluster */
        eRetVal = Fee_JobIntSwapClrVld();
    }
    else
    {
        uBlockSize = Fee_GetBlockSize(Fee_uJobIntBlockIt);
        bImmediateBlock = Fee_GetBlockImmediate(Fee_uJobIntBlockIt);
        uBlockNumber = Fee_GetBlockNumber(Fee_uJobIntBlockIt);
        /* Data space no more allocated even for inconsistent immediate (pre-erased) blocks */
        if( FEE_BLOCK_VALID == Fee_aBlockInfo[Fee_uJobIntBlockIt].eBlockStatus )
        {
            /* Align Fee block size to the virtual page boundary */
            uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );

            /* Calculate data address */
            uDataAddr = Fee_uJobIntDataAddr - uAlignedBlockSize;

            /* Update the block address info */
            Fee_uJobIntDataAddr = uDataAddr;

            /* Initialize the data transfer iterator */
            Fee_uJobIntAddrIt = uDataAddr;
        }
        else
        {
            uDataAddr = 0U;
        }

        /* Serialize block header to the write buffer */

        Fee_SerializeBlockHdr( uBlockNumber,
                               uBlockSize, uDataAddr,
                               bImmediateBlock,
                               Fee_aDataBuffer
                             );


        if( FEE_BLOCK_VALID == Fee_aBlockInfo[Fee_uJobIntBlockIt].eBlockStatus )
        {
            /* Read block data */
            Fee_eJob = FEE_JOB_INT_SWAP_DATA_READ;
        }
        else
        {
            /* Nothing to read. Move on to next block */
            Fee_uJobIntBlockIt++;
            Fee_eJob = FEE_JOB_INT_SWAP_BLOCK;
        }

        /* Write header to flash */
        if( ((Std_ReturnType)E_OK) !=
                Fls_Write( Fee_uJobIntHdrAddr, Fee_aDataBuffer,
                           FEE_BLOCK_OVERHEAD-(2U*FEE_VIRTUAL_PAGE_SIZE)
                         )
          )
        {
            /* Fls write job hasn't been accepted */
            eRetVal = MEMIF_JOB_FAILED;
        }
        else
        {
            /* Fls write job has been accepted */
            eRetVal = MEMIF_JOB_PENDING;
        }

        /* Move on to next block header */
        Fee_uJobIntHdrAddr += FEE_BLOCK_OVERHEAD;
    }

    return( eRetVal );
}

/**
* @brief        Format current Fee cluster in current Fee cluster group by
*               writing cluster header into flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the write
*                                        job
*
* @pre          Fee_uJobIntClrGrpIt      must contain index of current cluster
*                                        group
* @pre          Fee_uJobIntClrIt         must contain index of current cluster
* @post         Schedule the FEE_JOB_INT_SWAP_BLOCK subsequent job
*
*
*/
static MemIf_JobResultType Fee_JobIntSwapClrFmt( void )
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
    Fls_AddressType uStartAddr = 0UL;
    Fls_LengthType uLength = 0UL;
    uint32 uActClrID = 0UL;

    uStartAddr = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].startAddr;

    uLength = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].length;
    uActClrID = Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClrID;

    /* Build cluster header with new uActClrID + 1 */
    Fee_SerializeClusterHdr( uActClrID+1U, uStartAddr, uLength, Fee_aDataBuffer );

    /* Write the cluster header to flash */
    if( ((Std_ReturnType)E_OK) ==
            Fls_Write( uStartAddr, Fee_aDataBuffer,
                       FEE_CLUSTER_OVERHEAD-(2U*FEE_VIRTUAL_PAGE_SIZE)
                     )
      )
    {
        /* Fls read job has been accepted */
        eRetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls write job hasn't been accepted */
        eRetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule cluster format done job */
    Fee_eJob = FEE_JOB_INT_SWAP_BLOCK;

    return( eRetVal );
}

/**
* @brief        Erase current Fee cluster in current Fee cluster group by erasing flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the erase job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the erase job
*
* @pre          Fee_uJobIntClrGrpIt      must contain index of current cluster group
* @pre          Fee_uJobIntClrIt        must contain index of current cluster
* @post         Schedule the FEE_JOB_INT_SWAP_CLR_FMT subsequent job
*
*
*/
static MemIf_JobResultType Fee_JobIntSwapClrErase( void )
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
    Fls_AddressType uStartAddr = 0UL;
    Fls_LengthType uLength = 0UL;

    uStartAddr = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].startAddr;

    uLength = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].length;

    /* Erase cluster */
    if( ((Std_ReturnType)E_OK) == Fls_Erase( uStartAddr, uLength ) )
    {
        /* Fls read job has been accepted */
        eRetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls read job hasn't been accepted */
        eRetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule cluster format job */
    Fee_eJob = FEE_JOB_INT_SWAP_CLR_FMT;

    return( eRetVal );
}

/**
* @brief        Initialize the cluster swap internal operation on
*               current cluster group
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the erase job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the erase job
*
* @pre          Fee_uJobIntClrGrpIt      must contain index of current cluster group
* @pre          Fee_eJob                 must contain type of Fee job which caused the cluster swap
*
* @post         Fee module status is set to MEMIF_BUSY_INTERNAL
* @post         Fee_eJob is stored in Fee_eJobIntOriginalJob so it can be rescheduled
*               once the swap operation is finished
* @post         Fee internal block iterator Fee_uJobIntBlockIt is set to zero
* @post         Fee internal cluster iterator Fee_uJobIntClrIt is set to
*               the target cluster
* @post         Fee_uJobIntHdrAddr and Fee_uJobIntDataAddr addresses are initialized to
*               point to the target cluster
* @post         Schedule erase of the target cluster
*
*
*/
static MemIf_JobResultType  Fee_JobIntSwap( void )
{
    MemIf_JobResultType  eRetVal = MEMIF_JOB_OK;

    /* Store type of original job so Fee can re-schedule
        this job once the swap is complete */
    Fee_eJobIntOriginalJob = Fee_eJob;

    /* Reset the block iterator */
    Fee_uJobIntBlockIt = 0U;

    /* Get cluster index the block belongs to */
    Fee_uJobIntClrIt = Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClr;

    /* Calculate index of cluster to swap to */
    Fee_uJobIntClrIt++;

    if( Fee_uJobIntClrIt == Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrCount )
    {
        /* Cluster roll over */
        Fee_uJobIntClrIt = 0U;
    }
    else
    {
        /* Do nothing */
    }

    /* Calculate header and data address iterators */
    Fee_uJobIntHdrAddr =
        Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].startAddr;

    Fee_uJobIntDataAddr = Fee_uJobIntHdrAddr +
        Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].length;

    /* Move on to the first block header */
    Fee_uJobIntHdrAddr += FEE_CLUSTER_OVERHEAD;

    /* Erase the swap cluster */
    eRetVal = Fee_JobIntSwapClrErase();

    return( eRetVal );
}

/**
* @brief        Read the Fee block header into internal buffer
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING Underlying Fls accepted the read job
* @retval       MEMIF_JOB_FAILED Underlying Fls didn't accept the read job
*
* @pre          Fee_uJobIntAddrIt must contain valid logical address of
*               Fee block header to read
* @post         Schedule the FEE_JOB_INT_SCAN_CLR_PARSE subsequent job
*
*/
static MemIf_JobResultType Fee_JobIntScanBlockHdrRead( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;

    /* Read the block header */
    if( ((Std_ReturnType)E_OK) !=
            Fls_Read( Fee_uJobIntAddrIt, Fee_aDataBuffer, FEE_BLOCK_OVERHEAD )
      )
    {
        /* Fls read job hasn't been accepted */
        eRetVal = MEMIF_JOB_FAILED;
    }
    else
    {
        /* Fls read job has been accepted */
        eRetVal = MEMIF_JOB_PENDING;
    }

    Fee_eJob = FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE;

    return( eRetVal );
}

/**
* @brief        Erase first Fee cluster in current cluster group
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the erase job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the erase
*                                        job
*
* @pre          Fee_uJobIntClrGrpIt must contain index of current Fee cluster
*               group
* @post         Schedule the FEE_JOB_INT_SCAN_CLR_FMT subsequent job
*
*/
static MemIf_JobResultType Fee_JobIntScanClrErase( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     Fls_AddressType uStartAddr = 0UL;
     Fls_LengthType uLength = 0UL;

    /* Get address and size of first cluster in the current cluster group */
    uStartAddr = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[0].startAddr;

    uLength = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[0].length;

    if( ((Std_ReturnType)E_OK) == Fls_Erase( uStartAddr, uLength ) )
    {
        /* Fls read job has been accepted */
        eRetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls read job hasn't been accepted */
        eRetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule cluster format job */
    Fee_eJob = FEE_JOB_INT_SCAN_CLR_FMT;

    return( eRetVal );
}

/**
* @brief        Scan active cluster of current cluster group or erase and format
*               first cluster if an active cluster can't be found
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the erase or read job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the erase or read job
* @retval       MEMIF_JOB_OK             There is no more cluster groups to scan
*
* @pre          Fee_uJobIntClrGrpIt must contain index of current Fee cluster group
* @post         Initialize the Fee_uJobIntAddrIt iterator to address of
*               the first Fee block header to scan
* @post         Initialize the uHdrAddrIt and uDataAddrIt iterators to addresses of
*               the first Fee block header and data block
* @post         Schedule the FEE_JOB_DONE, FEE_JOB_INT_SCAN_CLR_FMT,
*               FEE_JOB_INT_SCAN_CLR_PARSE subsequent job
*
*/
static MemIf_JobResultType Fee_JobIntScanClr( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     uint8 uClrIndex = (uint8)0;
     Fls_AddressType  uClrStartAddr = 0UL;
     Fls_LengthType  uClrLength = 0UL;

    if( FEE_NUMBER_OF_CLUSTER_GROUPS == Fee_uJobIntClrGrpIt )
    {
        /* Nor more cluster groups to scan */
        Fee_eJob = FEE_JOB_DONE;
    }
    else
    {
        if( 0U == Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClrID )
        {
            /* call the cluster format notification */
            /* No active cluster found in current group so erase and format
                the first one... */
            eRetVal = Fee_JobIntScanClrErase();
        }
        else
        {
            /* Active cluster found */
            /* Get index of active cluster */
            uClrIndex = Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClr;

            uClrStartAddr = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[uClrIndex].startAddr;

            /* Calculate logical address of first block header in active cluster */
            Fee_uJobIntAddrIt = FEE_CLUSTER_OVERHEAD + uClrStartAddr;

            /* Initialize the block header pointer */
            Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uHdrAddrIt = Fee_uJobIntAddrIt;

            /* Initialize the block data pointer */
            uClrLength = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[uClrIndex].length;
            Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uDataAddrIt = uClrStartAddr + uClrLength;

            /* Active cluster found so read the first block header */
            eRetVal = Fee_JobIntScanBlockHdrRead();
        }
    }

    return( eRetVal );
}

/**
* @brief        Format first Fee cluster in current Fee cluster group by writing
*               cluster header into flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the erase job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the erase
*                                        job
*
* @pre          Fee_uJobIntClrGrpIt must contain index of current Fee cluster
*               group
* @post         Schedule the FEE_JOB_INT_SCAN_CLR_FMT_DONE subsequent job
*
*/
static MemIf_JobResultType Fee_JobIntScanClrFmt( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     Fls_AddressType uStartAddr = 0UL;
     Fls_LengthType uLength = 0UL;

    /* Get address and size of first cluster in the current cluster group */
    uStartAddr = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[0].startAddr;

    uLength = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[0].length;

    /* Build cluster header with clusterID = 1 */
    Fee_SerializeClusterHdr( 1U, uStartAddr, uLength, Fee_aDataBuffer );

    /* Make the cluster valid right away as it's empty anyway... */
    Fee_SerializeFlag( (Fee_aDataBuffer+FEE_CLUSTER_OVERHEAD)-(2U*FEE_VIRTUAL_PAGE_SIZE),
                       FEE_VALIDATED_VALUE
                     );

    /* Write the cluster header to flash */
    if( ((Std_ReturnType)E_OK) ==
            Fls_Write( uStartAddr, Fee_aDataBuffer,
                       FEE_CLUSTER_OVERHEAD-FEE_VIRTUAL_PAGE_SIZE
                     )
      )
    {
        /* Fls read job has been accepted */
        eRetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls write job hasn't been accepted */
        eRetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule cluster format done job */
    Fee_eJob = FEE_JOB_INT_SCAN_CLR_FMT_DONE;

    return( eRetVal );
}

/**
* @brief        Finalize format of first Fee cluster in current Fee cluster group
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the erase or
*                                        read job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the erase
*                                        or read job
* @retval       MEMIF_JOB_OK             There is no more cluster groups to scan
*
* @pre          Fee_uJobIntClrGrpIt must contain index of current Fee cluster
*               group
* @post         Assign index (uActClr) and ID (uActClrID) of active cluster in
*               current cluster group
* @post         Initialize the uHdrAddrIt and uDataAddrIt iterators to addresses
*               of
*               the first Fee block header and data block
* @post         Advance the Fee_uJobIntClrGrpIt iterator to next cluster group
* @post         Schedule the FEE_JOB_DONE, FEE_JOB_INT_SCAN_CLR_FMT,
*               FEE_JOB_INT_SCAN_CLR_PARSE subsequent job
*
*/
static MemIf_JobResultType Fee_JobIntScanClrFmtDone( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     Fls_AddressType  uClrStartAddr = 0UL;
     Fls_LengthType  uClrLength = 0UL;

    /* Assign Index and ID of active cluster */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClr = 0U;
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClrID = 1U;

    uClrStartAddr = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[0].startAddr;

    uClrLength = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[0].length;

    /* Initialize the block header pointer */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uHdrAddrIt =
        uClrStartAddr + FEE_CLUSTER_OVERHEAD;

    /* Initialize the block data pointer */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uDataAddrIt = uClrStartAddr + uClrLength;

    /* Move on to next cluster group */
    Fee_uJobIntClrGrpIt++;

    /* Scan next cluster group if any */
    eRetVal = Fee_JobIntScanClr();

    return( eRetVal );
}

/**
* @brief        Parse Fee block header
*
* @param[in]    bBufferValid             FALSE if previous Fls read job has failed
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the erase or
*                                        read job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the erase
*                                        or read job
* @retval       MEMIF_JOB_OK             There is no more cluster groups to scan
*
* @pre          Fee_uJobIntClrGrpIt      must contain index of current Fee
*                                        cluster group
*
* @post         Initialize internal recovery swap operation in case of bogus
*               Fee block header or failure of previous read job (perhaps ECC failure..)
* @post         Advance the Fee_uJobIntClrGrpIt iterator to next cluster group in
*               case of blank Fee block header (end of header list)
* @post         Update the eBlockStatus, uDataAddr, and uInvalidAddr block
*               information and the uHdrAddrIt and uDataAddrIt iterators in case of valid
*               Fee block header
* @post         Schedule the FEE_JOB_INT_SCAN_CLR_PARSE subsequent job in case
*               of valid Fee block header
*
*
*/
static MemIf_JobResultType Fee_JobIntScanBlockHdrParse
    (
        boolean bBufferValid
    )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_FAILED;
     Fee_BlockStatusType eBlockStatus;
     uint16 uBlockNumber = 0U;
     uint16 uBlockSize = 0U;
     Fls_AddressType uDataAddr = 0UL;
     boolean bImmediateData = (boolean)FALSE;
     uint16 uBlockIndex = 0U;
     uint16 uAlignedBlockSize = 0U;
     uint16 uBlockRuntimeInfoIndex = 0U;
    /* Internal variable used to trigger a cluster swap. Used by the swap job. */
    static  boolean bSwapToBePerformed = (boolean)FALSE;
     boolean bFeeSwapNeeded = (boolean)FALSE;

    /* Deserialize block header from read buffer */
    eBlockStatus =
        /* @violates @ref Fee_c_REF_4 Taking address of near auto variable. */
        Fee_DeserializeBlockHdr( &uBlockNumber, &uBlockSize, &uDataAddr,  &bImmediateData,
                                 Fee_aDataBuffer
                               );

    if( (FEE_BLOCK_HEADER_INVALID  == eBlockStatus) || ((boolean)FALSE == bBufferValid))
    {
        /* Invalidate the block index so Fee won't skip
            the Fee_uJobBlockIndex block */
        Fee_uJobBlockIndex = 0xffffU;

        /* Move on to next block header */
        Fee_uJobIntAddrIt += FEE_BLOCK_OVERHEAD;

        /* Update the block header pointer */
        Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uHdrAddrIt = Fee_uJobIntAddrIt;

        /* Set swap-to-be-performed flag */
        bSwapToBePerformed = (boolean)TRUE;

        /* Read next header */
        eRetVal = Fee_JobIntScanBlockHdrRead();

    }
    else if( FEE_BLOCK_HEADER_BLANK  == eBlockStatus )
    {
        /* If SwapToBePerformed is set, it means that last header is corrupted and
           ONLY now the uDataAddrIt variable will be updated to cause swap. */
        if( ((boolean)TRUE) == bSwapToBePerformed )
        {
           bSwapToBePerformed = (boolean)FALSE;
           Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uDataAddrIt = Fee_uJobIntAddrIt +
                                                               (2U * FEE_BLOCK_OVERHEAD);
        }

        /* End of header list so move on to next cluster group */
        Fee_uJobIntClrGrpIt++;

        eRetVal = Fee_JobIntScanClr();
    }
    else
    {
        /* Look for found block in the block configuration */
        uBlockIndex = Fee_GetBlockIndex( uBlockNumber );
            if( 0xFFFFU != uBlockIndex)
            {

                /* prepare index to be used for the runtime info data*/
                uBlockRuntimeInfoIndex = uBlockIndex;

                uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );

                /* Check whether found block belongs to current cluster group,
                    and its size and type match the configuration,
                    and its data pointer points to an acceptable area
                */
                bFeeSwapNeeded = (boolean)FALSE;

                if ((uDataAddr <=(Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uDataAddrIt-uAlignedBlockSize)) &&
                    (uDataAddr >= (Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uHdrAddrIt+(2U*FEE_BLOCK_OVERHEAD)))
                   )
                {
                    {
                        if((Fee_BlockConfig[uBlockIndex].clrGrp == Fee_uJobIntClrGrpIt) &&
                           (Fee_BlockConfig[uBlockIndex].blockSize == uBlockSize) &&
                           (Fee_BlockConfig[uBlockIndex].immediateData == bImmediateData)
                          )
                        {
                            bFeeSwapNeeded = (boolean)FALSE;

                        }
                        else
                        {
                            bFeeSwapNeeded = (boolean)TRUE;
                        }
                    }

                }
                else
                {
                    bFeeSwapNeeded = (boolean)TRUE;
                }
                /* check if swap is needed */
                if( (boolean)FALSE == bFeeSwapNeeded )
                {
                    /* Update block run-time information */

                        Fee_aBlockInfo[uBlockRuntimeInfoIndex].eBlockStatus = eBlockStatus;
                         /* Update the block data pointer */
                        Fee_aBlockInfo[uBlockRuntimeInfoIndex].uDataAddr = uDataAddr;
                        Fee_aBlockInfo[uBlockRuntimeInfoIndex].uInvalidAddr =
                            (Fee_uJobIntAddrIt + FEE_BLOCK_OVERHEAD) -
                                FEE_VIRTUAL_PAGE_SIZE;



                        Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uDataAddrIt = uDataAddr;

                }
                else
                {
                    /* Block header doesn't match Fee config or the FEE algorithm...
                       during next write cluster swap occurs */
                    bSwapToBePerformed = (boolean)TRUE;
                }

            }
            else
            {
                /* Invalid block number (removed from Fee config...) next write cluster swap occurs */
                bSwapToBePerformed = (boolean)TRUE;
            }

            /* Move on to next block header */
            Fee_uJobIntAddrIt += FEE_BLOCK_OVERHEAD;

            /* Update the block header pointer */
            Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uHdrAddrIt = Fee_uJobIntAddrIt;

            /* Read next header */
            eRetVal = Fee_JobIntScanBlockHdrRead();
    }
    return( eRetVal );
}

/**
* @brief        Read Fee cluster header
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the read job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept read job
*
* @pre          Fee_uJobIntClrGrpIt must contain index of current Fee cluster group
* @pre          Fee_uJobIntClrIt must contain index of current Fee cluster
* @post         Schedule the FEE_JOB_INT_SCAN_CLR_HDR_PARSE subsequent job
*
*/
static MemIf_JobResultType Fee_JobIntScanClrHdrRead( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     Fls_AddressType uReadAddress = 0UL;

    /* Get address of current cluster */
    uReadAddress = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].startAddr;

    if( ((Std_ReturnType)E_OK) != Fls_Read( uReadAddress, Fee_aDataBuffer, FEE_CLUSTER_OVERHEAD  ) )
    {
        /* Fls read job hasn't been accepted */
        eRetVal = MEMIF_JOB_FAILED;
    }
    else
    {
        /* Fls read job has been accepted */
        eRetVal = MEMIF_JOB_PENDING;
    }

    /* Schedule cluster scan job */
    Fee_eJob = FEE_JOB_INT_SCAN_CLR_HDR_PARSE;

    return( eRetVal );
}

/**
* @brief        Initialize the cluster scan job
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the read job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept read job
*
* @post         Reset the Fee_uJobIntClrGrpIt and Fee_uJobIntClrIt iterators
* @post         Schedule the FEE_JOB_INT_SCAN_CLR_HDR_PARSE subsequent job
*
*/
static MemIf_JobResultType Fee_JobIntScan( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;

    /* Reset iterators to first cluster of first cluster group */
    Fee_uJobIntClrGrpIt = 0U;
    Fee_uJobIntClrIt = 0U;

    /* Schedule reading of first cluster header */
    eRetVal = Fee_JobIntScanClrHdrRead();

    return( eRetVal );
}

/**
* @brief        Parse Fee cluster header
*
* @param[in]    bBufferValid             FALSE if previous Fls read job has failed
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the erase or read job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept the erase or read job
*
* @pre          Fee_uJobIntClrGrpIt must contain index of current Fee cluster group
* @pre          Fee_uJobIntClrIt must contain index of current cluster
* @post         Update the Fee_uJobIntClrIt and/or Fee_uJobIntClrGrpIt iterators to move
*               into next cluster and/or cluster group and read next cluster header if
*               there is any
* @post         Scan first active cluster if Fee is done with reading the cluster headers
* @post         Schedule the FEE_JOB_DONE, FEE_JOB_INT_SCAN_CLR_FMT,
*               FEE_JOB_INT_SCAN_CLR_PARSE, FEE_JOB_INT_SCAN_CLR_HDR_PARSE subsequent jobs
*
*/
static MemIf_JobResultType Fee_JobIntScanClrHdrParse
    (
        boolean bBufferValid
    )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     Fee_ClusterStatusType eClrStatus = FEE_CLUSTER_VALID;
     uint32 uClrID = 0UL;
     Fls_AddressType uClrStartAddr = 0UL;
     Fls_LengthType uClrSize = 0UL;
     Fls_AddressType uCfgStartAddr = 0UL;
     Fls_LengthType uCfgClrSize = 0UL;

    /* Get configured cluster start address and size */
    uCfgStartAddr = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].startAddr;

    uCfgClrSize = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].length;

    eClrStatus = Fee_DeserializeClusterHdr( &uClrID, &uClrStartAddr, &uClrSize, Fee_aDataBuffer );

    if( ((boolean)TRUE == bBufferValid) && (FEE_CLUSTER_VALID == eClrStatus) &&
        (uClrStartAddr == uCfgStartAddr) && (uClrSize == uCfgClrSize) &&
        (uClrID > Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClrID)
      )
    {
        /* Found valid and active cluster */
        Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClr = Fee_uJobIntClrIt;
        Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClrID = uClrID;
    }
    else
    {
        /* Invalid, inconsistent, or cluster with low ID */
    }

    /* Move on to next cluster */
    Fee_uJobIntClrIt++;

    if( Fee_uJobIntClrIt == Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrCount )
    {
        /* Move on to next cluster group */
        Fee_uJobIntClrGrpIt++;
        Fee_uJobIntClrIt = 0U;
    }
    else
    {
        /* next cluster in current cluster group */
    }

    if( FEE_NUMBER_OF_CLUSTER_GROUPS == Fee_uJobIntClrGrpIt )
    {
        Fee_uJobIntClrGrpIt = 0U;

        /* Done reading all cluster headers. Now scan active clusters */
        eRetVal = Fee_JobIntScanClr();
    }
    else
    {
        /* Read next cluster header */
        eRetVal = Fee_JobIntScanClrHdrRead();
    }

    return( eRetVal );
}

/**
* @brief        Read Fee block
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the read job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept read job
* @retval       MEMIF_BLOCK_INVALID      Requested Fee block is invalid
* @retval       MEMIF_BLOCK_INCONSISTENT Requested Fee block is inconsistent
*
* @pre          Fee_uJobBlockIndex must contain index of requested Fee block
* @pre          Fee_uJobBlockOffset must contain offset in requested Fee block
* @pre          Fee_uJobBlockLength must contain number of bytes to read from requested Fee block
* @post         Schedule the FEE_JOB_DONE    subsequent job
*
* @implements   Fee_JobRead_Activity
*/
static MemIf_JobResultType Fee_JobRead( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_FAILED;
     Fee_BlockStatusType eBlockStatus = FEE_BLOCK_VALID;
     Fls_AddressType uBlockAddress = 0UL;

    eBlockStatus = Fee_aBlockInfo[Fee_uJobBlockIndex].eBlockStatus;
    uBlockAddress = Fee_aBlockInfo[Fee_uJobBlockIndex].uDataAddr;

    if( FEE_BLOCK_VALID == eBlockStatus )
    {

        if( ((Std_ReturnType)E_OK) !=
                Fls_Read( uBlockAddress+Fee_uJobBlockOffset,
                          Fee_pJobReadDataDestPtr, Fee_uJobBlockLength
                        )
          )

        {
            /* Fls read job hasn't been accepted */
            eRetVal = MEMIF_JOB_FAILED;
        }
        else
        {
            /* Fls read job has been accepted */
            eRetVal = MEMIF_JOB_PENDING;
        }
    }
    else if( FEE_BLOCK_NEVER_WRITTEN == eBlockStatus )
    {
        /* in case the block was never written */
        eRetVal = MEMIF_BLOCK_INCONSISTENT;
    }
    else if( FEE_BLOCK_INVALID == eBlockStatus )
    {
        eRetVal = MEMIF_BLOCK_INVALID;
    }
    else if( (FEE_BLOCK_INCONSISTENT == eBlockStatus) || (FEE_BLOCK_INCONSISTENT_COPY == eBlockStatus) )
    {
        /* No special treatment of immediate blocks needed anymore */
        eRetVal = MEMIF_BLOCK_INCONSISTENT;
    }
    else
    {
        /* Something is wrong... MEMIF_JOB_FAILED */
    }

    /* There is no consecutive job to schedule */
    Fee_eJob = FEE_JOB_DONE;

    return( eRetVal );
}

/**
* @brief        Validate Fee block
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls did not accept write job
*
* @pre          Fee_uJobIntHdrAddr must contain address of Fee block header to
*               validate
* @post         Schedule the FEE_JOB_INT_SWAP_BLOCK subsequent job
*
* @implements   Fee_JobIntSwapBlockVld_Activity
*/
static MemIf_JobResultType Fee_JobIntSwapBlockVld( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;

    /* Serialize validation pattern to buffer */
    Fee_SerializeFlag( Fee_aDataBuffer, FEE_VALIDATED_VALUE );

    /* Write header to flash */
    if( ((Std_ReturnType)E_OK) !=
            Fls_Write( Fee_uJobIntHdrAddr-(2U*FEE_VIRTUAL_PAGE_SIZE),
                       Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE
                     )
      )
    {
        /* Fls read job hasn't been accepted */
        eRetVal = MEMIF_JOB_FAILED;
    }
    else
    {
        /* Fls read job has been accepted */
        eRetVal = MEMIF_JOB_PENDING;
    }

    /* Finalize the write operation */
    Fee_eJob = FEE_JOB_INT_SWAP_BLOCK;

    return( eRetVal );
}

/**
* @brief        Read data from source cluster to internal Fee buffer
*
* @param[in]    bBufferValid             FALSE if previous Fls read job has failed
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the read job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept read job
*
* @pre          Fee_uJobIntBlockIt must contain index of currently swaped
*               Fee block
* @pre          Fee_uJobIntDataAddr must contain start address of Fee data block
*               in the target cluster
* @pre          Fee_uJobIntAddrIt must contain current address of Fee data block
*               in the target cluster
* @post         Move on to next block Fee block if bBufferValid == FALSE
* @post         Validates the Fee block if there is no more data to read
* @post         Schedule the FEE_JOB_INT_SWAP_DATA_WRITE subsequent job
*
*
*/
static MemIf_JobResultType Fee_JobIntSwapDataRead
    (
        const boolean bBufferValid
    )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_FAILED;
     Fls_AddressType uReadAddr = 0UL;
     uint16 uAlignedBlockSize = 0U;
     uint16 uBlockSize = 0U;

    /* Get size of swaped block */
    uBlockSize = Fee_GetBlockSize(Fee_uJobIntBlockIt);

    /* Get size of swaped block aligned to virtual page boundary */
    uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );

    /* Calculate remaining number of bytes to read */
    Fee_uJobBlockLength = (Fee_uJobIntDataAddr + uAlignedBlockSize) - Fee_uJobIntAddrIt;

    if( (boolean)FALSE == bBufferValid )
    {
        /* There was error while reading the source cluster.
           Change the block status to FEE_BLOCK_INCONSISTENT_COPY to mark the status change
           during swap, leave the block (in flash) INCONSITENT and move on to next block */
        Fee_aBlockInfo[Fee_uJobIntBlockIt].eBlockStatus = FEE_BLOCK_INCONSISTENT_COPY;

        Fee_uJobIntBlockIt++;

        eRetVal = Fee_JobIntSwapBlock();
    }
    else if( 0U == Fee_uJobBlockLength )
    {
        /* No more bytes to copy. Validate the block and
            move on to next block... */
        Fee_uJobIntBlockIt++;

        eRetVal = Fee_JobIntSwapBlockVld();
    }
    else
    {
        if( Fee_uJobBlockLength > FEE_DATA_BUFFER_SIZE )
        {
            Fee_uJobBlockLength = FEE_DATA_BUFFER_SIZE;
        }
        else
        {
            /* Block data fits into the buffer */
        }

        /* Calculate source address */
        uReadAddr = (Fee_aBlockInfo[Fee_uJobIntBlockIt].uDataAddr + Fee_uJobIntAddrIt) -
                       Fee_uJobIntDataAddr;

        /* Read the block data */
        if( ((Std_ReturnType)E_OK) != Fls_Read( uReadAddr, Fee_aDataBuffer, Fee_uJobBlockLength ) )
        {
            /* Fls read job hasn't been accepted */
            eRetVal = MEMIF_JOB_FAILED;
        }
        else
        {
            /* Fls read job has been accepted */
            eRetVal = MEMIF_JOB_PENDING;
        }

        Fee_eJob = FEE_JOB_INT_SWAP_DATA_WRITE;
    }

    return( eRetVal );
}

/**
* @brief        Write data from internal Fee buffer to target cluster
*
* @param[in]    bBufferValid              FALSE if previous Fls read job has failed
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING         Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED          Underlying Fls didn't accept write job
*
* @pre          Fee_uJobBlockLength       must contain size of data block being
*                                         transfered(up to FEE_DATA_BUFFER_SIZE)
* @pre          Fee_uJobIntAddrIt         must contain current address of Fee
*                                         data block in the target cluster
* @post         Update the Fee_uJobIntAddrIt data iterator for next data
*               transfer if any
* @post         Schedule the FEE_JOB_INT_SWAP_DATA_READ subsequent job
*
*
*/
static MemIf_JobResultType Fee_JobIntSwapDataWrite
    (
        const boolean bBufferValid
    )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;

    if( bBufferValid )
    {
        if( ((Std_ReturnType)E_OK) ==
                Fls_Write( Fee_uJobIntAddrIt, Fee_aDataBuffer, Fee_uJobBlockLength )
          )
        {
            /* Fls read job has been accepted */
            eRetVal = MEMIF_JOB_PENDING;
        }
        else
        {
            /* Fls write job hasn't been accepted */
            eRetVal = MEMIF_JOB_FAILED;
        }

        Fee_uJobIntAddrIt += Fee_uJobBlockLength;

        Fee_eJob = FEE_JOB_INT_SWAP_DATA_READ;
    }
    else
    {
        eRetVal = Fee_JobIntSwapDataRead( (boolean)FALSE );
    }

    return( eRetVal );
}

/**
* @brief        Finalize cluster validation
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED Underlying Fls didn't accept write job
*
* @pre          Fee_uJobIntClrGrpIt must contain index of current Fee cluster
*               group
* @pre          Fee_uJobIntClrIt must contain index of current cluster
* @post         Update the uDataAddr and uInvalidAddr Fee_BlockConfig variables of
*               all affected (swaped) Fee blocks
* @post         Update uActClr, uActClrID, uHdrAddrIt, uDataAddrIt Fee_aClrGrpInfo
*               variables of current cluster group
* @post         Change Fee module status from MEMIF_BUSYINTERNAL to MEMIF_BUSY
* @post         Re-schedule the Fee_eJobIntOriginalJob subsequent job
*
* @implements   Fee_JobIntSwapClrVldDone_Activity
*/
static MemIf_JobResultType Fee_JobIntSwapClrVldDone( void )
{
     MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
     uint16 uBlockIt = 0U;
     Fls_AddressType uHdrAddrIt = 0UL;
     Fls_AddressType uDataAddrIt = 0UL;
     uint16 uBlockSize = 0U;
     uint16 uAlignedBlockSize = 0U;
     uint8 uBlockClusterGrp = 0U;


    uHdrAddrIt = Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].startAddr;

    uDataAddrIt = uHdrAddrIt + Fee_ClrGrps[Fee_uJobIntClrGrpIt].clrPtr[Fee_uJobIntClrIt].length;

    /* Move on to the first block header */
    uHdrAddrIt += FEE_CLUSTER_OVERHEAD;

    /* Sync block info */

   for( uBlockIt = 0U; uBlockIt < FEE_CRT_CFG_NR_OF_BLOCKS; uBlockIt++ )
    {
        uBlockClusterGrp = Fee_GetBlockClusterGrp(uBlockIt);

        if(Fee_uJobIntClrGrpIt == uBlockClusterGrp)
        {
            /* Valid blocks and inconsistent blocks with allocated data were copied,
               update the block info and internal pointers accordingly.
            */
            if( (FEE_BLOCK_VALID == Fee_aBlockInfo[uBlockIt].eBlockStatus) ||
                (FEE_BLOCK_INCONSISTENT == Fee_aBlockInfo[uBlockIt].eBlockStatus) ||
                (FEE_BLOCK_INCONSISTENT_COPY == Fee_aBlockInfo[uBlockIt].eBlockStatus)
              )
            {
                /* Sync uInvalidAddr */
                Fee_aBlockInfo[uBlockIt].uInvalidAddr = (uHdrAddrIt + FEE_BLOCK_OVERHEAD) -
                                                         FEE_VIRTUAL_PAGE_SIZE;
                /* check if block is immediate */
                /* Update the block data address info */
                if( (FEE_BLOCK_VALID == Fee_aBlockInfo[ uBlockIt ].eBlockStatus) ||
                    (FEE_BLOCK_INCONSISTENT_COPY == Fee_aBlockInfo[ uBlockIt ].eBlockStatus)
                  )
                {
                    uBlockSize = Fee_GetBlockSize(uBlockIt);
                    /* Align block size to the virtual page boundary */
                    uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );
                    /* "Allocate" data already swapped (written to the flash) */
                    uDataAddrIt -= uAlignedBlockSize;
                    if( FEE_BLOCK_INCONSISTENT_COPY == Fee_aBlockInfo[ uBlockIt ].eBlockStatus )
                    {
                        /* Damaged data read during copy, but already allocated => inconsistent */
                        Fee_aBlockInfo[ uBlockIt ].eBlockStatus = FEE_BLOCK_INCONSISTENT;
                        /* Mark inconsistency in the runtime info (no data) for the next swap attempt */
                        Fee_aBlockInfo[uBlockIt].uDataAddr = 0U;
                    }
                    else
                    {
                        /* store the data address to the block runtime record */
                        Fee_aBlockInfo[uBlockIt].uDataAddr = uDataAddrIt;
                    }
                }
                else
                {
                    /* (Inconsistent plain block in LEGACY == ON) or
                       (Inconsistent plain or immediate block in LEGACY == OFF) -- no data written (swapped, allocated)/available */
                    Fee_aBlockInfo[uBlockIt].uDataAddr = 0U;
                }

                /* Advance the header pointer */
                uHdrAddrIt += FEE_BLOCK_OVERHEAD;
            }
            else
            {
                /* Not a usable block */
                /* clear the addresses of the block not present in the new active cluster */
                Fee_aBlockInfo[uBlockIt].uDataAddr = 0U;
                Fee_aBlockInfo[uBlockIt].uInvalidAddr = 0U;
            }
        }
    }

    /* Switch to the new cluster */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClr = Fee_uJobIntClrIt;
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uActClrID++;
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uHdrAddrIt = uHdrAddrIt;
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].uDataAddrIt = uDataAddrIt;

    /* restore original Fee_eJob */
    if( FEE_JOB_INT_SCAN_CLR == Fee_eJobIntOriginalJob )
    {
        Fee_uJobIntClrGrpIt++;
    }
    else
    {
        /* Do nothing */
    }

    /* Internal job has finished so transition from MEMIF_BUSYINTERNAL to MEMIF_BUSY */
    Fee_eModuleStatus = MEMIF_BUSY;

    /* And now cross fingers and re-schedule original job ... */
    Fee_eJob = Fee_eJobIntOriginalJob;

    eRetVal = Fee_JobSchedule();

    return( eRetVal );
}

/**
* @brief        Write Fee block header to flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept write job
*
* @pre          Fee_uJobBlockIndex must contain index of Fee block to write to
* @post         Update the uDataAddr and uInvalidAddr Fee_BlockConfig variables of
*               written Fee block
* @post         Initialize internal cluster swap sequence if requested block
*               doesn't fit into current cluster
*
* @implements   Fee_JobWriteHdr_Activity
*/
static MemIf_JobResultType Fee_JobWriteHdr( void )
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_FAILED;
    uint16 uBlockSize = 0U;
    uint16 uAlignedBlockSize = 0U;
    uint8  uClrGrpIndex = (uint8)0;
    Fls_AddressType uDataAddr = 0U;
    Fls_AddressType uHdrAddr = 0U;

	/* Index of cluster group the Fee block belongs to */
	uClrGrpIndex = Fee_BlockConfig[Fee_uJobBlockIndex].clrGrp;

	/* Get size of Fee block */
	uBlockSize = Fee_BlockConfig[Fee_uJobBlockIndex].blockSize;

	/* Align Fee block size to the virtual page boundary */
	uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );

	if( (boolean)FALSE == (boolean)Fee_ReservedAreaWritable() )
	{
		/* The block won't fit into current cluster. Init swap sequence... */

		/* Get cluster group index the block belongs to */
		Fee_uJobIntClrGrpIt = uClrGrpIndex;

		eRetVal = Fee_JobIntSwap();
	}
	else
	{
		/* Calculate data and header addresses */
		uDataAddr = Fee_aClrGrpInfo[uClrGrpIndex].uDataAddrIt - uAlignedBlockSize;
		uHdrAddr = Fee_aClrGrpInfo[uClrGrpIndex].uHdrAddrIt;

		/* Serialize block header to the write buffer */
		Fee_SerializeBlockHdr( Fee_BlockConfig[Fee_uJobBlockIndex].blockNumber,
							   uBlockSize, uDataAddr,
							   Fee_BlockConfig[Fee_uJobBlockIndex].immediateData,
							   Fee_aDataBuffer
							 );

		/* Write header to flash */
		if( ((Std_ReturnType)E_OK) !=
				Fls_Write( uHdrAddr, Fee_aDataBuffer,
						   FEE_BLOCK_OVERHEAD-(2U*FEE_VIRTUAL_PAGE_SIZE)
						 )
		  )
		{
			/* Fls read job hasn't been accepted */
			eRetVal = MEMIF_JOB_FAILED;
		}
		else
		{
			/* Fls read job has been accepted */
			eRetVal = MEMIF_JOB_PENDING;
		}
	}
    return( eRetVal );
}


/**
* @brief        Write Fee block data to flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept write job
*
* @pre          Fee_uJobBlockIndex       must contain index of Fee block to
*                                        write to
* @pre          Fee_pJobWriteDataDestPtr must contain valid pointer to user
*                                        buffer
* @post         Update uHdrAddrIt, uDataAddrIt Fee_aClrGrpInfo variables of current
*               cluster group if immediateData == FALSE only
* @post         Changes eBlockStatus to FEE_BLOCK_INCONSISTENT
* @post         Schedule either FEE_JOB_WRITE_VALIDATE or
*               FEE_JOB_WRITE_UNALIGNED_DATA subsequent job
*
* @implements   Fee_JobWriteData_Activity
*/
static MemIf_JobResultType Fee_JobWriteData( void )
{
    Fls_AddressType  uDataAddr = (Fls_AddressType)0;
    MemIf_JobResultType eRetVal = MEMIF_JOB_OK;
    uint16  uBlockSize = 0U;
    uint16  uAlignedBlockSize = 0U;
    Fls_LengthType  uWriteLength = 0UL;
    uint8  uClrGrp = 0U;
    const uint8 * pWriteDataPtr = NULL_PTR;
    Fls_AddressType uHdrAddr = 0U;
    uint8 uClrGrpIndex = (uint8)0;

    /* Index of cluster group the Fee block belongs to */
    uClrGrpIndex = Fee_BlockConfig[Fee_uJobBlockIndex].clrGrp;

    /* Get size of Fee block */
    uBlockSize = Fee_BlockConfig[Fee_uJobBlockIndex].blockSize;

    /* Align Fee block size to the virtual page boundary */
    uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );

    /* Calculate data and header addresses */
    uDataAddr = Fee_aClrGrpInfo[uClrGrpIndex].uDataAddrIt - uAlignedBlockSize;
    uHdrAddr = Fee_aClrGrpInfo[uClrGrpIndex].uHdrAddrIt;

    Fee_aBlockInfo[Fee_uJobBlockIndex].uDataAddr = uDataAddr;
    Fee_aBlockInfo[Fee_uJobBlockIndex].uInvalidAddr = (uHdrAddr + FEE_BLOCK_OVERHEAD) -
                                                          FEE_VIRTUAL_PAGE_SIZE;

    Fee_aBlockInfo[Fee_uJobBlockIndex].eBlockStatus = FEE_BLOCK_INCONSISTENT;

	/* Get size of Fee block */
	uBlockSize = Fee_BlockConfig[Fee_uJobBlockIndex].blockSize;

	/* Align Fee block size to the virtual page boundary */
	uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );

	/* Index of cluster group the Fee block belongs to */
	uClrGrp = Fee_BlockConfig[Fee_uJobBlockIndex].clrGrp;

    Fee_aClrGrpInfo[uClrGrp].uHdrAddrIt += FEE_BLOCK_OVERHEAD;
    Fee_aClrGrpInfo[uClrGrp].uDataAddrIt -= uAlignedBlockSize;


	if( uBlockSize < FEE_VIRTUAL_PAGE_SIZE )
	{
		/* Go through buffer... */
		Fee_CopyDataToPageBuffer( Fee_pJobWriteDataDestPtr, Fee_aDataBuffer, uBlockSize );
		pWriteDataPtr = Fee_aDataBuffer;
		uWriteLength = FEE_VIRTUAL_PAGE_SIZE;

		/* Schedule write of validation pattern */
		Fee_eJob = FEE_JOB_WRITE_VALIDATE;
	}
	else
	{
		pWriteDataPtr = Fee_pJobWriteDataDestPtr;

		if( uAlignedBlockSize == uBlockSize )
		{
			uWriteLength = uBlockSize;

			/* Schedule write of validation pattern */
			Fee_eJob = FEE_JOB_WRITE_VALIDATE;
		}
		else
		{
			uWriteLength = ((uint32)uAlignedBlockSize) - FEE_VIRTUAL_PAGE_SIZE;

			/* Schedule write of unaligned data */
			Fee_eJob = FEE_JOB_WRITE_UNALIGNED_DATA;
		}
	}

	/* Use ClrGrp info */
	uDataAddr = Fee_aClrGrpInfo[uClrGrp].uDataAddrIt;

	if( ((Std_ReturnType)E_OK) == Fls_Write( uDataAddr, pWriteDataPtr, uWriteLength ) )
	{
		/* Fls read job has been accepted */
		eRetVal = MEMIF_JOB_PENDING;
	}
	else
	{
		/* Fls write job hasn't been accepted */
		eRetVal = MEMIF_JOB_FAILED;
	}
    return( eRetVal );
}

/**
* @brief        Write Fee block to flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED Underlying Fls didn't accept write job
*
* @pre          Fee_uJobBlockIndex must contain index of Fee block to write to
* @pre          Fee_pJobWriteDataDestPtr must contain valid pointer to user buffer
* @post         Initialize internal cluster swap sequence if requested block
*               doesn't fit into current cluster
* @post         Schedule FEE_JOB_WRITE_DATA subsequent job
*
* @implements   Fee_JobWrite_Activity
*/
static MemIf_JobResultType Fee_JobWrite( void )
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_FAILED;

	/* Write block header */
	eRetVal = Fee_JobWriteHdr();

	/* Schedule next job */
	if( FEE_JOB_INT_SWAP_CLR_FMT == Fee_eJob )
	{
		/* Block din't fit into the cluster.
		   Cluster swap has been enforced... */
	}
	else
	{
		Fee_eJob = FEE_JOB_WRITE_DATA;
	}

    return( eRetVal );
}

/**
* @brief        Write unaligned rest of Fee block data to flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept write job
*
* @pre          Fee_uJobBlockIndex must contain index of Fee block to write to
* @pre          Fee_pJobWriteDataDestPtr must contain valid pointer to user buffer
* @post         Schedule FEE_JOB_WRITE_VALIDATE subsequent job
*
*
*/
static MemIf_JobResultType  Fee_JobWriteUnalignedData( void )
{
    MemIf_JobResultType  eRetVal = MEMIF_JOB_OK;
    Fls_AddressType  uDataAddr = 0U;
    uint16  uWriteOffset = 0U;
    uint16  uWriteLength = 0U;
    uint16  uBlockSize = 0U;


	/* Get size of Fee block */
	uBlockSize = Fee_BlockConfig[Fee_uJobBlockIndex].blockSize;

	/* Calculate number of bytes already written */
	uWriteOffset = (uBlockSize / FEE_VIRTUAL_PAGE_SIZE) * FEE_VIRTUAL_PAGE_SIZE;

	/* Calculate length remaining data to write */
	uWriteLength = uBlockSize % FEE_VIRTUAL_PAGE_SIZE;

	/* @violates @ref Fee_c_REF_6 Array indexing shall be the
	   only allowed form of pointer arithmetic */
	Fee_CopyDataToPageBuffer( Fee_pJobWriteDataDestPtr+uWriteOffset, Fee_aDataBuffer, uWriteLength );


	/* Use Block info because it has been updated */
	uDataAddr = Fee_aBlockInfo[Fee_uJobBlockIndex].uDataAddr;


	if( ((Std_ReturnType)E_OK) ==
			Fls_Write( uDataAddr+uWriteOffset, Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE )
	  )
	{
		/* Fls write job has been accepted */
		eRetVal = MEMIF_JOB_PENDING;
	}
	else
	{
		/* Fls write job hasn't been accepted */
		eRetVal = MEMIF_JOB_FAILED;
	}

	/* Schedule write of validation pattern */
	Fee_eJob = FEE_JOB_WRITE_VALIDATE;

    return( eRetVal );
}

/**
* @brief        Validate Fee block by writing validation flag to flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept write job
*
* @pre          Fee_uJobBlockIndex must contain index of Fee block to validate
* @post         Schedule FEE_JOB_WRITE_DONE subsequent job
*
* @implements   Fee_JobWriteValidate_Activity
*/
static MemIf_JobResultType Fee_JobWriteValidate( void )
{
    MemIf_JobResultType  eRetVal = MEMIF_JOB_OK;
    Fls_AddressType  uHdrAddr = (Fls_AddressType)0;

	/* Serialize validation pattern to buffer */
	Fee_SerializeFlag( Fee_aDataBuffer, FEE_VALIDATED_VALUE );

	/* Use Block info */
	uHdrAddr = Fee_aBlockInfo[Fee_uJobBlockIndex].uInvalidAddr - FEE_VIRTUAL_PAGE_SIZE;

	/* Write validation pattern to flash */
	if( ((Std_ReturnType)E_OK) == Fls_Write( uHdrAddr, Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE) )
	{
		/* Fls read job has been accepted */
		eRetVal = MEMIF_JOB_PENDING;
	}
	else
	{
		/* Fls write job hasn't been accepted */
		eRetVal = MEMIF_JOB_FAILED;
	}

	/* Finalize the write operation */
	Fee_eJob = FEE_JOB_WRITE_DONE;

    return( eRetVal );
}

/**
* @brief        Finalize validation of Fee block
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_OK
*
* @pre          Fee_uJobBlockIndex must contain index of Fee block to write to
* @post         change Fee block status to FEE_BLOCK_VALID
* @post         Schedule FEE_JOB_DONE subsequent job
*
*
*/
static MemIf_JobResultType  Fee_JobWriteDone( void )
{

    /* Mark the Fee block as valid */
    Fee_aBlockInfo[Fee_uJobBlockIndex].eBlockStatus = FEE_BLOCK_VALID;

    /* No more Fls jobs to schedule */
    Fee_eJob = FEE_JOB_DONE;

    return( MEMIF_JOB_OK );
}

/**
* @brief        Invalidate Fee block by writing the invalidation flag to flash
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept write job
* @retval       MEMIF_JOB_OK             The Fee block is already invalid
*
* @pre          Fee_uJobBlockIndex must contain index of Fee block to invalidate
* @post         Schedule FEE_JOB_DONE or FEE_JOB_INVAL_BLOCK_DONE subsequent
*               jobs
*
*
*/
static MemIf_JobResultType  Fee_JobInvalBlock( void )
{
    MemIf_JobResultType  eRetVal = MEMIF_JOB_OK;

    if( FEE_BLOCK_INVALID == Fee_aBlockInfo[Fee_uJobBlockIndex].eBlockStatus )
    {
        /* No more subsequent Fls jobs to schedule */
        Fee_eJob = FEE_JOB_DONE;

        /* Fee job completed */
        eRetVal = MEMIF_JOB_OK;
    }
    else
    {
        if( FEE_BLOCK_NEVER_WRITTEN == Fee_aBlockInfo[Fee_uJobBlockIndex].eBlockStatus )
        {
            /* set status as INVALID */
            Fee_aBlockInfo[Fee_uJobBlockIndex].eBlockStatus = FEE_BLOCK_INVALID;

            /* No more subsequent Fls jobs to schedule */
            Fee_eJob = FEE_JOB_DONE;

            /* Fee job completed */
            eRetVal = MEMIF_JOB_OK;
        }
        else
        {
            /* Serialize invalidation pattern to buffer */
            Fee_SerializeFlag( Fee_aDataBuffer, FEE_INVALIDATED_VALUE );

            /* Write invalidation pattern to flash */
            if( ((Std_ReturnType)E_OK) ==
                    Fls_Write( Fee_aBlockInfo[Fee_uJobBlockIndex].uInvalidAddr,
                               Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE
                             )
              )
            {
                /* Fls read job has been accepted */
                eRetVal = MEMIF_JOB_PENDING;
            }
            else
            {
                /* Fls write job hasn't been accepted */
                eRetVal = MEMIF_JOB_FAILED;
            }

            /* Finalize the invalidation operation */
            Fee_eJob = FEE_JOB_INVAL_BLOCK_DONE;
        }
    }

    return( eRetVal );
}

/**
* @brief        Finalize invalidation of Fee block
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_OK
*
* @pre          Fee_uJobBlockIndex       must contain index of Fee block to
*                                        write to
* @post         change Fee block status to FEE_BLOCK_INVALID
* @post         Schedule FEE_JOB_DONE subsequent job
*
*/
static MemIf_JobResultType  Fee_JobInvalBlockDone( void )
{
    /* Mark the Fee block as in valid */
    Fee_aBlockInfo[Fee_uJobBlockIndex].eBlockStatus = FEE_BLOCK_INVALID;

    /* No more subsequent Fls jobs to schedule */
    Fee_eJob = FEE_JOB_DONE;

    /* Fee job completed */
    return( MEMIF_JOB_OK );
}

/**
* @brief        Erase (pre-allocate) immediate Fee block
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_PENDING        Underlying Fls accepted the write job
* @retval       MEMIF_JOB_FAILED         Underlying Fls didn't accept write job
*
* @pre          Fee_uJobBlockIndex must contain index of Fee block to
*               pre-allocated
* @post         Initialize internal cluster swap sequence if requested block
*               doesn't fit into current cluster
* @post         Schedule FEE_JOB_ERASE_IMMEDIATE_DONE subsequent jobs
*
* @implements   Fee_JobEraseImmediate_Activity
*
*/
static MemIf_JobResultType  Fee_JobEraseImmediate( void )
{
    MemIf_JobResultType  eRetVal = MEMIF_JOB_OK;

	/* In standard mode, perform swap if reserved area has been touched, or do nothing. */
	if( FALSE == Fee_ReservedAreaWritable() )
	{
		/* Reserved area is not writable, force the swap */
		Fee_uJobIntClrGrpIt = Fee_BlockConfig[ Fee_uJobBlockIndex ].clrGrp;              /* Get cluster group index */
		eRetVal = Fee_JobIntSwap();
	}
	else
	{
		/* Do effectively nothing */
		Fee_eJob = FEE_JOB_DONE;
	}

    return( eRetVal );
}

/**
* @brief        Finalize erase (pre-allocation) of Fee block
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_OK
*
* @pre          Fee_uJobBlockIndex must contain index of Fee block to write to
* @post         change Fee block status to FEE_BLOCK_INCONSISTENT
* @post         Schedule FEE_JOB_DONE subsequent job
*
* @implements   Fee_JobEraseImmediateDone_Activity
*/
static MemIf_JobResultType Fee_JobEraseImmediateDone( void )
{
	uint16 uBlockSize = 0U;
	uint16 uAlignedBlockSize = 0U;
	uint16 uClrGrp = 0U;
	MemIf_JobResultType eRetVal = MEMIF_JOB_OK;

	Fls_AddressType uDataAddr = (Fls_AddressType)0;
	Fls_AddressType uHdrAddr = (Fls_AddressType)0;
	uint8 uClrGrpIndex = (uint8)0;

	/* Mark the Fee immediate block as inconsistent so it can be written later on */
	Fee_aBlockInfo[Fee_uJobBlockIndex].eBlockStatus = FEE_BLOCK_INCONSISTENT;

	/* Get size of Fee block */
	uBlockSize = Fee_BlockConfig[Fee_uJobBlockIndex].blockSize;

	/* Align Fee block size to the virtual page boundary */
	uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );

	/* Index of cluster group the Fee block belongs to */
	uClrGrp = Fee_BlockConfig[Fee_uJobBlockIndex].clrGrp;

	/* Index of cluster group the Fee block belongs to */
	uClrGrpIndex = Fee_BlockConfig[Fee_uJobBlockIndex].clrGrp;
	uDataAddr = Fee_aClrGrpInfo[uClrGrpIndex].uDataAddrIt - uAlignedBlockSize;
	uHdrAddr = Fee_aClrGrpInfo[uClrGrpIndex].uHdrAddrIt;

	/* Update the block address info */
	Fee_aBlockInfo[Fee_uJobBlockIndex].uDataAddr = uDataAddr;
	Fee_aBlockInfo[Fee_uJobBlockIndex].uInvalidAddr = (uHdrAddr + FEE_BLOCK_OVERHEAD) -
														  FEE_VIRTUAL_PAGE_SIZE;

	/* Header has been written so update both header and data addresses */
	Fee_aClrGrpInfo[uClrGrp].uHdrAddrIt += FEE_BLOCK_OVERHEAD;
	Fee_aClrGrpInfo[uClrGrp].uDataAddrIt -= uAlignedBlockSize;

	/* No more subsequent Fls jobs to schedule */
	Fee_eJob = FEE_JOB_DONE;
	/* set job as ook */
	eRetVal = MEMIF_JOB_OK;
	/* Fee job completed */
    return( eRetVal );
}

/**
* @brief        Schedule subsequent jobs
*
* @return       MemIf_JobResultType
*
* @pre          Fee_eJob must contain type of job to schedule
*
* @implements   Fee_JobSchedule_Activity
*/
static MemIf_JobResultType Fee_JobSchedule( void )
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_FAILED;

    switch( Fee_eJob )
    {
        /* Fee_Read() related jobs */
        case FEE_JOB_READ:
            eRetVal = Fee_JobRead();
            break;
        /* Fee_Write() related jobs */
        case FEE_JOB_WRITE:
            eRetVal = Fee_JobWrite();
            break;

        case FEE_JOB_WRITE_DATA:
            eRetVal = Fee_JobWriteData();
            break;

        case FEE_JOB_WRITE_UNALIGNED_DATA:
            eRetVal = Fee_JobWriteUnalignedData();
            break;

        case FEE_JOB_WRITE_VALIDATE:
            eRetVal = Fee_JobWriteValidate();
            break;

        case FEE_JOB_WRITE_DONE:
            eRetVal = Fee_JobWriteDone();
            break;

        /* Fee_InvalidateBlock() related jobs */
        case FEE_JOB_INVAL_BLOCK:
            eRetVal = Fee_JobInvalBlock();
            break;

        case FEE_JOB_INVAL_BLOCK_DONE:
            eRetVal = Fee_JobInvalBlockDone();
            break;

        /* Fee_EraseImmediateBlock() related jobs */
        case FEE_JOB_ERASE_IMMEDIATE:
            eRetVal = Fee_JobEraseImmediate();
            break;

        case FEE_JOB_ERASE_IMMEDIATE_DONE:
            eRetVal = Fee_JobEraseImmediateDone();
            break;

        /* Fee_Init() related jobs */
        case FEE_JOB_INT_SCAN:
            eRetVal = Fee_JobIntScan();
            break;

        case FEE_JOB_INT_SCAN_CLR:
            eRetVal = Fee_JobIntScanClr();
            break;

        case FEE_JOB_INT_SCAN_CLR_HDR_PARSE:
            eRetVal = Fee_JobIntScanClrHdrParse( (boolean)TRUE );
            break;

        case FEE_JOB_INT_SCAN_CLR_FMT:
            eRetVal = Fee_JobIntScanClrFmt();
            break;

        case FEE_JOB_INT_SCAN_CLR_FMT_DONE:
            eRetVal = Fee_JobIntScanClrFmtDone();
            break;

        case FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE:
            eRetVal = Fee_JobIntScanBlockHdrParse( (boolean)TRUE );
            break;

        /* Swap related jobs */
        case FEE_JOB_INT_SWAP_CLR_FMT:
            eRetVal = Fee_JobIntSwapClrFmt();
            break;

        case FEE_JOB_INT_SWAP_BLOCK:
            eRetVal = Fee_JobIntSwapBlock();
            break;

        case FEE_JOB_INT_SWAP_DATA_READ:
            eRetVal = Fee_JobIntSwapDataRead( (boolean)TRUE );
            break;

        case FEE_JOB_INT_SWAP_DATA_WRITE:
            eRetVal = Fee_JobIntSwapDataWrite( (boolean)TRUE );
            break;

        case FEE_JOB_INT_SWAP_CLR_VLD_DONE:
            eRetVal = Fee_JobIntSwapClrVldDone();
            break;

        /* if the job is finished switch case will break */
        case FEE_JOB_DONE:
            /* Do nothing */
            break;

        default:
            /* Compiler_Warning: This default branch will never be reached in FTE */
            break;
    }

    return( eRetVal );
}

/**
* @brief Checks whether the block specified by Fee_JobBlockIndex is writable into the reserved area.
*
* @return sint8
* @retval TRUE The block is writable into the reserved area.
* @retval FALSE The block is not writable into the reserved area.
*
* @pre Fee_JobBlockIndex must contain index of Fee block to be written
* @pre Fee_aClrGrpInfo[uClrGrpIndex].uDataAddrIt must be up to date
* @pre Fee_aClrGrpInfo[uClrGrpIndex].uHdrAddrIt must be up to date
*
* @implements   Fee_ReservedAreaWritable_Activity
*
*/
static sint8  Fee_ReservedAreaWritable( void )
{
    sint8 sRetVal = TRUE;
    uint16 uBlockSize = 0U;
    uint16 uAlignedBlockSize = 0U;
    Fls_LengthType uAvailClrSpace = 0UL;
    uint8 uClrGrpIndex = (uint8)0;
    uint32 uReservedSpace = 0UL;
    boolean bImmediateData = (boolean)FALSE;

    /* Index of cluster group the Fee block belongs to */
    uClrGrpIndex = Fee_GetBlockClusterGrp(Fee_uJobBlockIndex);

    /* Reserved space of cluster group*/
    uReservedSpace = Fee_ClrGrps[uClrGrpIndex].reservedSize;

    /* Calculate available space in active cluster */
    uAvailClrSpace = Fee_aClrGrpInfo[uClrGrpIndex].uDataAddrIt -
                     Fee_aClrGrpInfo[uClrGrpIndex].uHdrAddrIt;

    /* Get size of Fee block */
    uBlockSize = Fee_GetBlockSize(Fee_uJobBlockIndex);

    /* Align Fee block size to the virtual page boundary */
    uAlignedBlockSize = Fee_AlignToVirtualPageSize( uBlockSize );

    /* Hard Stop condition: One FEE_BLOCK_OVERHEAD must be left blank to have a clear separation
       between header block space and data block space */
    if( (((uint32)uAlignedBlockSize) + (2U * FEE_BLOCK_OVERHEAD)) > uAvailClrSpace )
    {
        sRetVal = FALSE;
    }
    else if( (((uint32)uAlignedBlockSize) + (2U * FEE_BLOCK_OVERHEAD) + uReservedSpace) >
             uAvailClrSpace
           )
    {
        bImmediateData = Fee_GetBlockImmediate(Fee_uJobBlockIndex);
        /* Block targets reserved area */
        if( (boolean)TRUE == bImmediateData )
        {
            /* Immediate block - only writable if not already present in the reserved area */
            if( (0UL == Fee_aBlockInfo[Fee_uJobBlockIndex].uDataAddr) &&
                (0UL == Fee_aBlockInfo[Fee_uJobBlockIndex].uInvalidAddr)
              )
            {
                /* The block is not written in the cluster at all => writable */
                sRetVal = TRUE;
            }
            else
            {
                /* The block is written in the cluster - is it written in the reserved area? */
                if( (FEE_BLOCK_OVERHEAD + uReservedSpace) >
                    (Fee_aBlockInfo[Fee_uJobBlockIndex].uDataAddr -
                       (Fee_aBlockInfo[Fee_uJobBlockIndex].uInvalidAddr + FEE_VIRTUAL_PAGE_SIZE)
                    )
                  )
                {
                    /* The block is already written in the reserved area */
                    sRetVal = FALSE;
                }
                else
                {
                   /* The block is not written in the reserved area */
                   sRetVal = TRUE;
                }
            }
        }
        else
        {
            sRetVal = FALSE;
        }
    }
    else
    {
        /* Block does not target reserved area */
        sRetVal = TRUE;
    }
    return sRetVal;
}
/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

/**
* @brief        Service to initialize the FEE module.
* @details      The function Fee_Init shall initialize the Flash EEPROM
*               Emulation module.
*
* @pre          The FEE module'  s environment shall not call the function
*               Fee_Init shall during a running operation of the FEE module.
* @note         The function Autosar Service ID[hex]: 0x00.
* @note         Asynchronous
* @note         Non Reentrant
*
* @api
*
* @implements   Fee_Init_Activity
*/
void Fee_Init(void)
{
    uint32 uInvalIndex = 0U;

	/* Initialize all block info records */
	for( uInvalIndex = 0U; uInvalIndex < FEE_MAX_NR_OF_BLOCKS; uInvalIndex++ )
	{
		/* for blocks which were never written Fee returns INCONSISTENT status */
		Fee_aBlockInfo[uInvalIndex].eBlockStatus = FEE_BLOCK_NEVER_WRITTEN;
	}

	/* Invalidate all cluster groups */
	for( uInvalIndex = 0U; uInvalIndex < FEE_NUMBER_OF_CLUSTER_GROUPS; uInvalIndex++ )
	{
		Fee_aClrGrpInfo[uInvalIndex].uActClrID = 0U;
	}


	/* Schedule init job */
	Fee_eJob = FEE_JOB_INT_SCAN;

	Fee_eModuleStatus = MEMIF_BUSY_INTERNAL;

	Fee_eJobResult = MEMIF_JOB_PENDING;
}

/**
* @brief        Service to initiate a read job.
* @details      The function Fee_Read shall take the block start address and
*               offset and calculate the corresponding memory read address.
*
* @param[in]    uBlockNumber     Number of logical block, also denoting
*                                start address of that block in flash memory.
* @param[in]    uBlockOffset     Read address offset inside the block.
* @param[out]   pDataBufferPtr   Pointer to data buffer.
* @param[in]    uLength          Number of bytes to read.
*
* @pre          The module must be initialized, not busy, uBlockNumber must be
*               valid, uLength != 0, pDataBufferPtr != NULL_PTR, uBlockOffset and
*               (uBlockOffset + uLength - 1) must be in range.
* @post         changes Fee_eModuleStatus module status and Fee_uJobBlockOffset,
*               Fee_uJobBlockLength, Fee_uJobBlockIndex, Fee_pJobReadDataDestPtr,
*               Fee_eJob, Fee_eJobResult job control internal variables.
*
* @return       Std_ReturnType
* @retval       E_OK             The read job was accepted by the
*                                underlying memory driver.
* @retval       E_NOT_OK         The read job has not been accepted
*                                by the underlying memory driver.
* @note         The function Autosar Service ID[hex]: 0x02.
* @note         Asynchronous.
* @note         Non Reentrant.
*
* @api
*
* @implements   Fee_Read_Activity
*/
Std_ReturnType Fee_Read (uint16 uBlockNumber,
						 uint16 uBlockOffset,
						 uint8 * pDataBufferPtr,
						 uint16 uLength)
{
    Std_ReturnType uRetVal = (Std_ReturnType)E_OK;
    uint16 uBlockIndex = Fee_GetBlockIndex( uBlockNumber );

    /* Start of exclusive area. Implementation depends on integrator. */
    if( MEMIF_UNINIT == Fee_eModuleStatus )
    {
        uRetVal = (Std_ReturnType)E_NOT_OK;
    }
    else if( MEMIF_BUSY == Fee_eModuleStatus )
    {
        uRetVal = (Std_ReturnType)E_NOT_OK;
    }
    else if( MEMIF_BUSY_INTERNAL == Fee_eModuleStatus )
    {
        uRetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {

        /* Configure the read job */
        Fee_uJobBlockIndex = uBlockIndex;

        Fee_uJobBlockOffset = uBlockOffset;

        Fee_uJobBlockLength = uLength;
        Fee_pJobReadDataDestPtr = pDataBufferPtr;

        Fee_eJob = FEE_JOB_READ;

        Fee_eModuleStatus = MEMIF_BUSY;

        /* Execute the read job */
        Fee_eJobResult = MEMIF_JOB_PENDING;
    }
    /* End of exclusive area. Implementation depends on integrator. */

    return( uRetVal );
}

/**
* @brief        Service to initiate a write job.
* @details      The function Fee_Write shall take the block start address and
*               calculate the corresponding memory write address. The block
*               address offset shall be fixed to zero.
*               The function Fee_Write shall copy the given or computed
*               parameters to module internal variables, initiate a write job,
*               set the FEE module status to MEMIF_BUSY, set the job result to
*               MEMIF_JOB_PENDING and return with E_OK.The FEE module shall
*               execute the write job of the function Fee_Write
*               asynchronously within the FEE module' s main function.
*
* @param[in]    uBlockNumber     Number of logical block, also denoting
*                                start address of that block in emulated
*                                EEPROM.
* @param[out]   pDataBufferPtr   Pointer to data buffer.
*
* @return       Std_ReturnType
* @retval       E_OK             The write job was accepted by the
*                                underlying memory driver.
* @retval       E_NOT_OK         The write job has not been accepted by
*                                the underlying memory driver.
*
* @pre          The module must be initialized, not busy, uBlockNumber must be
*               valid, and pDataBufferPtr != NULL_PTR.
*               Before call the function "Fee_Write" for immediate date must be
*               called the function "Fee_EraseImmediateBlock".
* @post         changes Fee_eModuleStatus module status and Fee_uJobBlockIndex,
*               Fee_pJobWriteDataDestPtr, Fee_eJob, Fee_eJobResult job control internal
*               variables.
* @note         The function Autosar Service ID[hex]: 0x03.
* @note         Asynchronous.
* @note         Non Reentrant.
*
* @api
*
* @implements   Fee_Write_Activity
*/
/*
* @violates @ref Fee_c_REF_15 All declarations and definitions
* of objects or functions at file scope shall have internal linkage unless external linkage required
*/
Std_ReturnType Fee_Write (uint16 uBlockNumber, const uint8 * pDataBufferPtr)
{
    Std_ReturnType uRetVal = (Std_ReturnType)E_OK;
    uint16 uBlockIndex = Fee_GetBlockIndex( uBlockNumber );

    /* Start of exclusive area. Implementation depends on integrator. */

    if( MEMIF_UNINIT == Fee_eModuleStatus )
    {
        uRetVal = (Std_ReturnType)E_NOT_OK;
    }
    else if( MEMIF_BUSY == Fee_eModuleStatus )
    {
        uRetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {

        /* Configure the write job */
        Fee_uJobBlockIndex = uBlockIndex;
        Fee_pJobWriteDataDestPtr = pDataBufferPtr;

        Fee_eJob = FEE_JOB_WRITE;

        Fee_eModuleStatus = MEMIF_BUSY;

        /* Execute the write job */
        Fee_eJobResult = MEMIF_JOB_PENDING;

    }
    /* End of exclusive area. Implementation depends on integrator. */

    return( uRetVal );
}

/**
* @brief        Return the Fee module state.
* @details      Return the Fee module state synchronously.
*
* @note         The function Autosar Service ID[hex]: 0x05.
* @note         Synchronous
* @note         Non Reentrant
*
* @return       Fee_eModuleStatus
* @retval       MEMIF_UNINIT             Module has not been initialized (yet).
* @retval       MEMIF_IDLE               Module is currently idle.
* @retval       MEMIF_BUSY               Module is currently busy.
* @retval       MEMIF_BUSY_INTERNAL      Module is busy with internal management
*                                        operations.
*
* @api
*
* @implements   Fee_GetStatus_Activity
*/
MemIf_StatusType Fee_GetStatus( void )
{
    return( Fee_eModuleStatus );
}

/**
* @brief        Return the result of the last job.
* @details      Return the result of the last job synchronously.
*
* @return       MemIf_JobResultType
* @retval       MEMIF_JOB_OK              The job has been finished successfully.
* @retval       MEMIF_JOB_FAILED          The job has not been finished
*                                         successfully.
* @retval       MEMIF_JOB_PENDING         The job has not yet been finished.
* @retval       MEMIF_JOB_CANCELED        The job has been canceled.
* @retval       MEMIF_BLOCK_INCONSISTENT  The requested block is inconsistent,
*                                         it may contain corrupted data.
* @retval       MEMIF_BLOCK_INVALID       The requested block has been invalidated,
*                                         the requested read operation can not be performed.
* @note         The function Autosar Service ID[hex]: 0x06.
* @note         Synchronous.
* @note         Non Reentrant.
*
* @api
*
* @implements   Fee_GetJobResult_Activity
*/
MemIf_JobResultType Fee_GetJobResult( void )
{
    MemIf_JobResultType eRetVal = Fee_eJobResult;

    return( eRetVal );
}

/**
* @brief        Service to invalidate a logical block.
*
* @param[in]    uBlockNumber     Number of logical block, also denoting
*                                start address of that block in flash memory
* @return       Std_ReturnType
* @retval       E_OK             The job was accepted by the underlying
*                                memory driver.
* @retval       E_NOT_OK         The job has not been accepted by the
*                                underlying memory driver.
*
* @pre          The module must be initialized, not busy, and uBlockNumber must be
*               valid
*
* @post         changes Fee_eModuleStatus module status and Fee_uJobBlockIndex,
*               Fee_eJob, and Fee_eJobResult job control internal variables.
*               EEPROM.
* @note         The function Autosar Service ID[hex]: 0x07.
* @note         Asynchronous.
* @note         Non Reentrant.
*
* @api
*
* @implements   Fee_InvalidateBlock_Activity
*/
Std_ReturnType Fee_InvalidateBlock(  uint16 uBlockNumber )
{
	Std_ReturnType uRetVal = (Std_ReturnType)E_NOT_OK;
	uint16 uBlockIndex = Fee_GetBlockIndex( uBlockNumber );

    /* Start of exclusive area. Implementation depends on integrator. */

	if( MEMIF_IDLE == Fee_eModuleStatus )
	{

		/* Configure the invalidate block job */
		Fee_uJobBlockIndex = uBlockIndex;

		Fee_eJob = FEE_JOB_INVAL_BLOCK;

		Fee_eModuleStatus = MEMIF_BUSY;

		/* Execute the invalidate block job */
		Fee_eJobResult = MEMIF_JOB_PENDING;

		uRetVal = (Std_ReturnType)E_OK;
	}

    /* End of exclusive area. Implementation depends on integrator. */

    return( uRetVal );
}

/**
* @brief        Service to erase a logical block.
* @details      The function Fee_EraseImmediateBlock shall take the block number
*               and calculate the corresponding memory block address.
*               The function Fee_EraseImmediateBlock shall ensure that the FEE
*               module can write immediate data. Whether this involves
*               physically erasing a memory area and therefore calling the erase
*               function of the underlying driver depends on the implementation.
*               If development error detection for the FEE module is enabled,
*               the function Fee_EraseImmediateBlock shall check whether the
*               addressed logical block is configured as containing immediate
*               data (configuration parameter FeeImmediateData == TRUE).
*               If not, the function Fee_EraseImmediateBlock
*               shall report the error code FEE_E_INVALID_BLOCK_NO.
*
* @param[in]    uBlockNumber     Number of logical block, also denoting.
*
* @return       Std_ReturnType
* @retval       E_OK             The job was accepted by the underlying
*                                memory driver.
* @retval       E_NOT_OK         The job has not been accepted by the underlying
*                                memory driver.
*                                start address of that block in emulated EEPROM.
*
* @pre          The module must be initialized, not busy, uBlockNumber must be
*               valid, and type of Fee block must be immediate.
* @post         changes Fee_eModuleStatus module status and Fee_uJobBlockIndex,
*               Fee_eJob, and Fee_eJobResult job control internal variables.
*
* @note         The function Autosar Service ID[hex]: 0x09.
* @note         Asynchronous.
* @note         Non Reentrant.
*
* @api
*
* @implements   Fee_EraseImmediateBlock_Activity
*/
Std_ReturnType Fee_EraseImmediateBlock(  uint16 uBlockNumber )
{
     Std_ReturnType uRetVal = (Std_ReturnType)E_OK;
     uint16 uBlockIndex = Fee_GetBlockIndex( uBlockNumber );

    /* Start of exclusive area. Implementation depends on integrator. */

	/* Configure the erase immediate block job */
	Fee_uJobBlockIndex = uBlockIndex;

	Fee_eJob = FEE_JOB_ERASE_IMMEDIATE;

	Fee_eModuleStatus = MEMIF_BUSY;

	/* Execute the erase immediate block job */
	Fee_eJobResult = MEMIF_JOB_PENDING;

    /* End of exclusive area. Implementation depends on integrator. */

    return( uRetVal );
}

/**
* @brief        Service to report the FEE module the successful end of
*               an asynchronous operation.
* @details      The underlying flash driver shall call the function
*               Fee_JobEndNotification to report the successful end of
*               an asynchronous operation.
*
* @pre          The module must be initialized.
* @post         Changes Fee_eModuleStatus module status and Fee_eJobResult
*               internal variables.
* @note         The function Autosar Service ID[hex]: 0x10.
* @note         Synchronous.
* @note         Non Reentrant
*
* @api
*
* @implements   Fee_JobEndNotification_Activity
*/
void Fee_JobEndNotification( void )
{
    if( MEMIF_UNINIT == Fee_eModuleStatus )
    {

    }
    else
    {
        if( FEE_JOB_DONE == Fee_eJob )
        {
            /* Last schedule Fls job finished */
            //Fee_eJobResult = Fls_GetJobResult();

            if( MEMIF_BUSY_INTERNAL == Fee_eModuleStatus )
            {
                /* no notifications from internal jobs */
                Fee_eModuleStatus = MEMIF_IDLE;
            }
            else
            {
                Fee_eModuleStatus = MEMIF_IDLE;
                Job_finish = 1;
                /* Call job end notification function */
                //FEE_NVM_JOB_END_NOTIFICATION
            }
        }
        else
        {
            Fee_eJobResult = Fee_JobSchedule();

            if( MEMIF_JOB_OK == Fee_eJobResult )
            {
                if( MEMIF_BUSY_INTERNAL == Fee_eModuleStatus )
                {
                    /* no notifications from internal jobs */
                    Fee_eModuleStatus = MEMIF_IDLE;
                }
                else
                {
                    Fee_eModuleStatus = MEMIF_IDLE;
                    Job_finish = 1;
                    /* Call job end notification function */
                    //FEE_NVM_JOB_END_NOTIFICATION
                }
            }
            else if( MEMIF_JOB_PENDING == Fee_eJobResult )
            {
                /* Nothing to do (ongoing Fls job) */
            }
            else
            {
                if( MEMIF_BUSY_INTERNAL == Fee_eModuleStatus )
                {
                    /* no notifications from internal jobs */
                    Fee_eModuleStatus = MEMIF_IDLE;
                }
                else
                {
                    Fee_eModuleStatus = MEMIF_IDLE;

                    /* Call job error notification function */
                    //FEE_NVM_JOB_ERROR_NOTIFICATION
                }
            }
        }
    }
}

/**
* @brief        Service to report the FEE module the failure of
*               an asynchronous operation.
* @details      The underlying flash driver shall call the function
*               Fee_JobErrorNotification to report the failure of
*               an asynchronous operation.
*
* @pre          The module must be initialized.
* @post         Changes Fee_eModuleStatus module status and Fee_eJobResult
*               internal variables.
* @note         The function Autosar Service ID[hex]: 0x11.
* @note         Synchronous.
* @note         Non Reentrant.
*
* @api
*
* @implements   Fee_JobErrorNotification_Activity
*/
void Fee_JobErrorNotification( void )
{
    uint8 uClrGrpIndex = 0U;

    if( MEMIF_UNINIT == Fee_eModuleStatus )
    {

    }
    else
    {
        if( MEMIF_JOB_CANCELED == Fee_eJobResult )
        {
            /* Fls job has been canceled. Do nothing in this callback.
            The NvM_JobErrorNotification() callback will be called from
            the Fee_Cancel()function which called the Fls_Cancel() function */
        }
        else
        {
            switch( Fee_eJob )
            {
                /* Error while reading block header. Treat block header as
                    invalid and    stop reading further block headers */
                case FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE:
                    Fee_eJobResult = Fee_JobIntScanBlockHdrParse( (boolean)FALSE );
                    break;

                /* Error while reading cluster header. Treat cluster as
                    invalid and    move on to next cluster */
                case FEE_JOB_INT_SCAN_CLR_HDR_PARSE:
                    Fee_eJobResult = Fee_JobIntScanClrHdrParse( (boolean)FALSE );
                    break;

                /* Error while reading data from source cluster.
                    Leave block as INCONSISTENT and move on to next block */
                case FEE_JOB_INT_SWAP_DATA_WRITE:
                    Fee_eJobResult = Fee_JobIntSwapDataWrite( (boolean)FALSE );
                    break;
                /* for all the following cases based on the Fee module status error notification will
                be called */
                case FEE_JOB_WRITE:
                case FEE_JOB_WRITE_DATA:
                case FEE_JOB_WRITE_UNALIGNED_DATA:
                case FEE_JOB_ERASE_IMMEDIATE:
                case FEE_JOB_ERASE_IMMEDIATE_DONE:
                case FEE_JOB_WRITE_VALIDATE:
                case FEE_JOB_WRITE_DONE:
                {
                    uClrGrpIndex = Fee_GetBlockClusterGrp(Fee_uJobBlockIndex);

                    /* Something wrong in FlashMemory (cluster swap occurs to next write) */
                    Fee_aClrGrpInfo[uClrGrpIndex].uDataAddrIt =
                        Fee_aClrGrpInfo[uClrGrpIndex].uHdrAddrIt +
                        (2U * FEE_BLOCK_OVERHEAD);
                }
                /* @violates @ref Fee_c_REF_A control flows into case/default. */

                case FEE_JOB_READ:
                case FEE_JOB_INVAL_BLOCK:
                case FEE_JOB_INVAL_BLOCK_DONE:
                case FEE_JOB_INT_SCAN:
                case FEE_JOB_INT_SCAN_CLR:
                case FEE_JOB_INT_SCAN_CLR_FMT:
                case FEE_JOB_INT_SCAN_CLR_FMT_DONE:
                case FEE_JOB_INT_SWAP_BLOCK:
                case FEE_JOB_INT_SWAP_CLR_FMT:
                case FEE_JOB_INT_SWAP_DATA_READ:
                case FEE_JOB_INT_SWAP_CLR_VLD_DONE:
                case FEE_JOB_DONE:
                default:
                    Fee_eJobResult = Fls_GetJobResult();

                    if( MEMIF_BUSY_INTERNAL == Fee_eModuleStatus )
                    {
                        /* No notifications from internal jobs */
                        Fee_eModuleStatus = MEMIF_IDLE;
                    }
                    else
                    {
                        Fee_eModuleStatus = MEMIF_IDLE;

                        /* Call job error notification function */
                        //FEE_NVM_JOB_ERROR_NOTIFICATION
                    }
                    break;
            }
        }
    }
}

/**
* @brief        Service to handle the requested read, write, or erase jobs
*               respectively the internal management operations.
* @details      The function shall asynchronously handle the requested
*               read, write, or erase jobs respectively the internal management operations.
*               The function shall check, whether
*               the block requested for reading has been invalidated by the
*               upper layer module.
*               If so, the function shall set the job result to
*               MEMIF_BLOCK_INVALID and call the error notification routine
*               of the upper layer if configured.
*               The function shall check the consistency of the logical block
*               being read before notifying the caller. If an inconsistency
*               of the read data is detected, the function
*               shall set the job result to MEMIF_BLOCK_INCONSISTENT and call
*               the error notification routine of the upper layer.
* @pre          The module must be initialized.
* @note         The function Autosar Service ID[hex]: 0x12.
*
* @api
*
* @implements   Fee_MainFunction_Activity
*/
void Fee_MainFunction( void )
{
	if( MEMIF_JOB_PENDING == Fee_eJobResult )
	{

		switch( Fee_eJob )
		{
			/* for all the following jobs subsequent jobs will be called in Fee job schedule
			   function based on Job */
			case FEE_JOB_INT_SCAN:
			case FEE_JOB_READ:
			case FEE_JOB_WRITE:
			case FEE_JOB_INVAL_BLOCK:
			case FEE_JOB_ERASE_IMMEDIATE:
				Fee_eJobResult = Fee_JobSchedule();
				break;
		   /* for all the following jobs job end or job error notification will be called
			  based on the job result  */
			case FEE_JOB_WRITE_DATA:
			case FEE_JOB_WRITE_UNALIGNED_DATA:
			case FEE_JOB_WRITE_VALIDATE:
			case FEE_JOB_WRITE_DONE:
			case FEE_JOB_INVAL_BLOCK_DONE:
			case FEE_JOB_ERASE_IMMEDIATE_DONE:
			case FEE_JOB_INT_SCAN_CLR_HDR_PARSE:
			case FEE_JOB_INT_SCAN_CLR:
			case FEE_JOB_INT_SCAN_CLR_FMT:
			case FEE_JOB_INT_SCAN_CLR_FMT_DONE:
			case FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE:
			case FEE_JOB_INT_SWAP_BLOCK:
			case FEE_JOB_INT_SWAP_CLR_FMT:
			case FEE_JOB_INT_SWAP_DATA_READ:
			case FEE_JOB_INT_SWAP_DATA_WRITE:
			case FEE_JOB_INT_SWAP_CLR_VLD_DONE:
			case FEE_JOB_DONE:
			default:
				/* Internal or subsequent job */
				break;
		}

		if( MEMIF_JOB_PENDING == Fee_eJobResult )
		{
			/* Nothing to do */
		}
		else if( MEMIF_JOB_OK == Fee_eJobResult )
		{
			if( MEMIF_BUSY_INTERNAL == Fee_eModuleStatus )
			{
				/* no notifications from internal jobs */
				Fee_eModuleStatus = MEMIF_IDLE;
			}
			else
			{
				Fee_eModuleStatus = MEMIF_IDLE;

				/* Call job end notification function */
				//FEE_NVM_JOB_END_NOTIFICATION
			}
		}
		else
		{
			if( MEMIF_BUSY_INTERNAL == Fee_eModuleStatus )
			{
				/* no notifications from internal jobs */
				Fee_eModuleStatus = MEMIF_IDLE;
			}
			else
			{
				Fee_eModuleStatus = MEMIF_IDLE;

				/* Call job error notification function */
				//FEE_NVM_JOB_ERROR_NOTIFICATION
			}
		}
	}
}

#ifdef __cplusplus
}
#endif

/** @}*/
