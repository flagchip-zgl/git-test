
#ifndef FEE_INTERNALTYPES_H
#define FEE_INTERNALTYPES_H

#ifdef __cplusplus
extern "C"{
#endif


/*==================================================================================================
*                                             ENUMS
==================================================================================================*/

/**
* @brief        Status of Fee block header
* @api
*/
typedef enum
{
    FEE_BLOCK_VALID = 0,         /**< @brief Fee block is valid */
    FEE_BLOCK_INVALID,           /**< @brief Fee block is invalid
                                      (has been invalidated) */
    FEE_BLOCK_INCONSISTENT,      /**< @brief Fee block is inconsistent
                                      (contains bogus data) */
    FEE_BLOCK_HEADER_INVALID,    /**< @brief Fee block header is garbled */
    FEE_BLOCK_INVALIDATED,       /**< @brief Fee block header is invalidated by
                                      Fee_InvalidateBlock(BlockNumber)(not used when
                                      FEE_BLOCK_ALWAYS_AVAILABLE == STD_OFF) */
    FEE_BLOCK_HEADER_BLANK,      /**< @brief Fee block header is blank,
                                      it is used to mark the end of Fee block header list
                                      when parsing the memory at initialization*/
    FEE_BLOCK_INCONSISTENT_COPY,  /**< @brief FEE data read error during swap
                                     (ie data area was allocated) */
    FEE_BLOCK_NEVER_WRITTEN     /**< @brief FEE block was never written in data flash */

} Fee_BlockStatusType;

/**
* @brief        Status of Fee cluster header
* @api
*/
typedef enum
{
    FEE_CLUSTER_VALID = 0,       /**< @brief Fee cluster is valid */
    FEE_CLUSTER_INVALID,         /**< @brief Fee cluster is invalid */
    FEE_CLUSTER_INCONSISTENT,    /**< @brief Fee cluster is inconsistent
                                      (contains bogus data) */
    FEE_CLUSTER_HEADER_INVALID   /**< @brief Fee cluster header is garbled */
} Fee_ClusterStatusType;

/**
* @brief        Type of job currently executed by Fee_MainFunction
* @api
*/
typedef enum
{
    /* Fee_Read() related jobs */

    FEE_JOB_READ = 0,                /**< @brief Read Fee block */

    /* Fee_Write() related jobs */

    FEE_JOB_WRITE,                   /**< @brief Write Fee block to flash */
    FEE_JOB_WRITE_DATA,              /**< @brief Write Fee block data to flash
                                          */
    FEE_JOB_WRITE_UNALIGNED_DATA,    /**< @brief Write unaligned rest of Fee
                                          block data to flash */
    FEE_JOB_WRITE_VALIDATE,          /**< @brief Validate Fee block by writing
                                          validation flag to flash */
    FEE_JOB_WRITE_DONE,              /**< @brief Finalize validation of Fee
                                          block */
    /* Fee_InvalidateBlock() related jobs */

    FEE_JOB_INVAL_BLOCK,             /**< @brief Invalidate Fee block by writing
                                          the invalidation flag to flash */
    FEE_JOB_INVAL_BLOCK_DONE,        /**< @brief Finalize invalidation of Fee
                                          block */

    /* Fee_EraseImmediateBlock() related jobs */

    FEE_JOB_ERASE_IMMEDIATE,         /**< @brief Erase (pre-allocate) immediate
                                          Fee block */
    FEE_JOB_ERASE_IMMEDIATE_DONE,    /**< @brief Finalize erase (pre-allocation)
                                          of Fee block */
    /* Fee_Init() realted jobs */
    FEE_JOB_INT_SCAN,                /**< @brief Initialize the cluster scan job
                                          */
    FEE_JOB_INT_SCAN_CLR_HDR_PARSE,  /**< @brief Parse Fee cluster header */
    FEE_JOB_INT_SCAN_CLR,            /**< @brief Scan active cluster of current
                                          cluster group */
    FEE_JOB_INT_SCAN_CLR_FMT,        /**< @brief Format first Fee cluster */
    FEE_JOB_INT_SCAN_CLR_FMT_DONE,   /**< @brief Finalize format of first Fee
                                          cluster */
    FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE,/**< @brief Parse Fee block header */

    /* Internal swap jobs */

    FEE_JOB_INT_SWAP_BLOCK,          /**< @brief Copy next block from source to
                                          target cluster */
    FEE_JOB_INT_SWAP_CLR_FMT,        /**< @brief Format current Fee cluster in
                                          current Fee cluster group */
    FEE_JOB_INT_SWAP_DATA_READ,      /**< @brief Read data from source cluster
                                          to internal Fee buffer */
    FEE_JOB_INT_SWAP_DATA_WRITE,     /**< @brief Write data from internal
                                          Fee buffer to target cluster */
    FEE_JOB_INT_SWAP_CLR_VLD_DONE,   /**< @brief Finalize cluster validation */
    /* Fee system jobs */
    FEE_JOB_DONE                     /**< @brief No more subsequent jobs to
                                          schedule */
} Fee_JobType;

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/**
* @brief        Fee cluster group run-time status
* @api
*/
typedef struct
{
    Fls_AddressType uDataAddrIt;  /**< @brief Address of current Fee data block in flash */
    Fls_AddressType uHdrAddrIt;   /**< @brief Address of current Fee block header in flash */
    uint32 uActClrID;             /**< @brief ID of active cluster */
    uint8 uActClr;                /**< @brief Index of active cluster */
} Fee_ClusterGroupInfoType;

/**
* @brief        Fee block run-time status
* @api
*/
typedef struct
{
    Fls_AddressType uDataAddr;      /**< @brief Address of Fee block data in flash */
    Fls_AddressType uInvalidAddr;   /**< @brief Address of Fee block invalidation field in flash */
    Fee_BlockStatusType eBlockStatus;   /**< @brief Current status of Fee block */
} Fee_BlockInfoType;

#ifdef __cplusplus
}
#endif

#endif /* FEE_INTERNALTYPES_H */

/** @}*/
