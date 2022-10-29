#include "Fls.h"
#include "Fls_Flash.h"
#include "Fls_Flash_Types.h"
#include "fc4xxx_driver_flash.h"

/* ################################################################################## */
/* ####################################### Macro #################################### */

#define FLASHDRV_ADDR            0x01817000U

#define CUSTOMER_NAME            "Flagchip"


/* ################################################################################## */
/* ################################### Type define ################################## */


#define STATUS_SUCCESS       0x000UL
#define STATUS_ERROR         0x001UL
#define STATUS_HVOP          0x002UL
#define STATUS_PEG_FAILED    0x003UL
#define STATUS_ECC_ERROR     0x004UL
#define STATUS_ECC_WARNING   0x005UL
#define STATUS_RWW_ERROR     0x006UL
#define STATUS_EDC_ERROR     0x007UL
#define STATUS_ENC_ERROR     0x008UL
#define STATUS_PEP_ERROR     0x009UL
#define STATUS_WDG_ERROR     0x00AUL
#define STATUS_RE_ERROR      0x00BUL


#define FLASH_BLOCK_SELECT0    0x000UL
#define FLASH_BLOCK_SELECT1    0x001UL

#define FLASH_PRGM_SIZE1     2UL
#define FLASH_PRGM_SIZE2     4UL
#define FLASH_PRGM_SIZE3     6UL
#define FLASH_PRGM_SIZE4     8UL
#define FLASH_PRGM_SIZE5     10UL
#define FLASH_PRGM_SIZE6     12UL
#define FLASH_PRGM_SIZE7     14UL
#define FLASH_PRGM_SIZE8     16UL
#define FLASH_PRGM_SIZE9     18UL
#define FLASH_PRGM_SIZE10    20UL
#define FLASH_PRGM_SIZE11    22UL
#define FLASH_PRGM_SIZE12    24UL
#define FLASH_PRGM_SIZE13    26UL
#define FLASH_PRGM_SIZE14    28UL
#define FLASH_PRGM_SIZE15    30UL
#define FLASH_PRGM_SIZE16    32UL

#define FLASH_WAIT_SELECT1    1UL
#define FLASH_WAIT_SELECT2    2UL
#define FLASH_WAIT_SELECT3    3UL
#define FLASH_WAIT_SELECT4    4UL
#define FLASH_WAIT_SELECT5    5UL
#define FLASH_WAIT_SELECT6    6UL
#define FLASH_WAIT_SELECT7    7UL
#define FLASH_WAIT_SELECT8    8UL

#define FLASH_MARGIN_LEVEL0    0UL
#define FLASH_MARGIN_LEVEL1    1UL

#define FLASH_WDG_TIMEOUT_SELECT0    0UL
#define FLASH_WDG_TIMEOUT_SELECT1    1UL
#define FLASH_WDG_TIMEOUT_SELECT2    2UL
#define FLASH_WDG_TIMEOUT_SELECT3    3UL

#define FLASH_WDG_ENABLE     0UL
#define FLASH_WDG_DISABLE    1UL

#define FLASH_WDG_INT_ENABLE     1UL
#define FLASH_WDG_INT_DISABLE    0UL

#define FLASH_ECC_CHECK    0x3UL
#define FLASH_SBC_CHECK    0x4UL
#define FLASH_RWW_CHECK    0x5UL
#define FLASH_EDC_CHECK    0x6UL
#define FLASH_PEG_CHECK    0x7UL
#define FLASH_PEP_CHECK    0x8UL
#define FLASH_RE_CHECK     0x9UL

#define FLASH_ECC_ERROR    0x3UL
#define FLASH_SBC_ERROR    0x4UL
#define FLASH_RWW_ERROR    0x5UL
#define FLASH_EDC_ERROR    0x6UL
#define FLASH_PEP_ERROR    0x7UL

#define WDG_TUNE_DISABLE    0x1UL
#define WDG_TUNE_ENABLE     0x0UL

#define FLASH_BUF_PREF_DISABLE    0x0UL
#define FLASH_BUF_PREF_ENABLE     0x1UL

#define FLASH_READ_WAIT_LOCK_DISABLE    0x0UL
#define FLASH_READ_WAIT_LOCK_ENABLE     0x1UL

#define FLASH_AUTO_HOLD_DISABLE       0x0UL
#define FLASH_AUTO_HOLD_ENABLE        0x1UL

#define FLASH_ARRAY_INTEGRITY_SEQ1    0x0UL
#define FLASH_ARRAY_INTEGRITY_SEQ2    0x1UL


typedef struct
{
    uint32_t lock_sel;
    uint32_t  wait_number;
} FLASH_DRV_READ_WAIT_CFG_T;




typedef struct
{
    uint32_t blk_sel;
    uint32_t dest;
} FLASH_DRV_ERASESECTOR_CFG_T;

typedef struct
{
    uint32_t dest;
    uint32_t size;
    uint32_t *pData;
    uint32_t  wdg_tune;
} FLASH_DRV_PRGM_CFG_T;

typedef struct
{
    uint32_t dest;
    uint32_t sed0;
    uint32_t sed1;
    uint32_t sed2;
    uint32_t  wait_number;
    uint32_t  mrv;
    uint32_t als;
} FLASH_DRV_TEST_CHECK_CFG_T;


typedef struct
{
    uint32_t time;
    uint32_t wint_en;
    uint32_t wen;
} FLASH_DRV_WDG_CFG_T;

typedef struct
{
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t dest;
} FLASH_DRV_ECC_CHECK_CFG_T;

typedef struct
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
} FLASH_DRV_TEST_CHECK_DATA_CFG_T;

typedef struct
{
    uint32_t CBUF;
    uint32_t DBUF;
    uint32_t CPREF;
    uint32_t DPREF;
} FLASH_DRV_BUF_PREF_CFG_T;




typedef uint32_t (* FLASH_ERASE_BLOCK_FUNC)(uint32_t blk_sel);
typedef uint32_t (* FLASH_VOID_FUNC)(void);
typedef uint32_t (* FLASH_ERASE_SECTOR_FUNC)(FLASH_DRV_ERASESECTOR_CFG_T *flash_api_cfg);
typedef uint32_t (* FLASH_PROGRAM_FUNC)(FLASH_DRV_PRGM_CFG_T *flash_api_cfg);
typedef uint32_t (* FLASH_TEST_CHECK_FUNC)(FLASH_DRV_TEST_CHECK_CFG_T *flash_api_cfg);
typedef uint32_t (* FLASH_READ_STATUS_CHECK_FUNC)(uint32_t  flash_api_cfg);
typedef uint32_t (* FLASH_READ_STATUS_CLEAR_FUNC)(uint32_t  flash_api_cfg);
typedef uint32_t (* FLASH_TEST_STATUS_FUNC)(FLASH_DRV_TEST_CHECK_DATA_CFG_T *flash_api_cfg);
typedef uint32_t (* FLASH_WDOG_CFG_FUNC)(FLASH_DRV_WDG_CFG_T *flash_api_cfg);
typedef uint32_t (* FLASH_ECC_CHECK_FUNC)(FLASH_DRV_ECC_CHECK_CFG_T *flash_api_cfg);
typedef uint32_t (* FLASH_BUF_PREF_FUNC)(FLASH_DRV_BUF_PREF_CFG_T *flash_api_cfg);
typedef uint32_t (* FLASH_READ_WAIT_CFG)(FLASH_DRV_READ_WAIT_CFG_T *flash_api_cfg);
typedef uint32_t (* FLASH_AUTO_HOLD_FUNC)(uint32_t flash_api_cfg);




typedef struct
{
    FLASH_ERASE_BLOCK_FUNC                  FLASH_DRV_EraseBlock;
    FLASH_VOID_FUNC                         FLASH_DRV_EraseBlock_Clear;
    FLASH_ERASE_SECTOR_FUNC                 FLASH_DRV_EraseSector;
    FLASH_VOID_FUNC                         FLASH_DRV_EraseSector_Clear;
    FLASH_VOID_FUNC                         FLASH_DRV_EraseNvr;
    FLASH_VOID_FUNC                         FLASH_DRV_EraseNvr_Clear;
    FLASH_PROGRAM_FUNC                      FLASH_DRV_Program;
    FLASH_VOID_FUNC                         FLASH_DRV_Program_Clear;
    FLASH_PROGRAM_FUNC                      FLASH_DRV_Program_Nvr;
    FLASH_VOID_FUNC                         FLASH_DRV_Program_Nvr_Clear;
    FLASH_TEST_CHECK_FUNC                   FLASH_DRV_Array_Integrity_Check;
    FLASH_TEST_CHECK_FUNC                   FLASH_DRV_Margin_Read_Check;
    FLASH_READ_STATUS_CHECK_FUNC            FLASH_DRV_Read_Status_Check;
    FLASH_READ_STATUS_CLEAR_FUNC            FLASH_DRV_Read_Status_Check_Clear;
    FLASH_VOID_FUNC                         FLASH_DRV_HV_Status_Check;
    FLASH_TEST_STATUS_FUNC                  FLASH_DRV_Test_Status_Check;
    FLASH_WDOG_CFG_FUNC                     FLASH_DRV_Wdog_Configure;
    FLASH_VOID_FUNC                         FLASH_DRV_Wdog_Status;
    FLASH_VOID_FUNC                         FLASH_DRV_Wdog_Status_Clear;
    FLASH_ECC_CHECK_FUNC                    FLASH_DRV_EDC_Check;
    FLASH_ECC_CHECK_FUNC                    FLASH_DRV_ECC_Check;
    FLASH_VOID_FUNC                         FLASH_DRV_Test_Mode_Close;
    FLASH_BUF_PREF_FUNC                     FLASH_DRV_BUF_PREF_CFG;
    FLASH_READ_WAIT_CFG                     FLASH_DRV_Read_Wait_CFG;
    FLASH_AUTO_HOLD_FUNC                    FLASH_DRV_ENABLE_HOLD_CFG;
} FLASH_ROM_API_ENTRY_T;



/* ################################################################################## */
/* ################################### Const Variables ############################## */

/** flash driver header for finding function in special address */

static  const FLASH_ROM_API_ENTRY_T  *s_pFlashDriver_FuncHeader = (const FLASH_ROM_API_ENTRY_T *)(FLASHDRV_ADDR);



/* ################################################################################## */
/* ################################ Local Variables ################################# */
uint8_t debugmacro_text[]="xodgppijh8)Flagchip]dsf;f01";
uint8_t debugmacro_data[]="dfasgsdgasdgf";

/* ################################################################################## */
/* ########################### Local Prototype Functions ############################ */
static uint32_t FLASHDRIVER_LL_EraseSector(FLASH_DRV_ERASESECTOR_CFG_T *flash_api_cfg);
static FLASH_StatusType FLASHDRIVER_WriteCheck(FLASH_DRIVER_ParamType *pFlashParam);
static FLASH_StatusType FLASHDRIVER_LockSector(uint32_t u32Address, uint8_t bLock);
static FLASH_StatusType FLASHDRIVER_GetFlashConfig(uint32_t u32Address, FLASH_DRV_ERASESECTOR_CFG_T *pFlash_api_cfg);
static uint32_t FLASHDRIVER_LL_EraseSector(FLASH_DRV_ERASESECTOR_CFG_T *flash_api_cfg);
static uint32_t FLASHDRIVER_LL_HVStatusCheck(void);
static uint32_t FLASHDRIVER_LL_EraseSectorClear(void);
static uint32_t FLASHDRIVER_LL_Program(FLASH_DRV_PRGM_CFG_T *flash_api_cfg);
static uint32_t FLASHDRIVER_LL_ProgramClear(void);


/* ################################################################################## */
/* ########################### Global Prototype Functions ########################### */





/* ################################################################################## */
/* ################################ Local Functions ################################# */



/**
 * \brief Flash Driver Function for Erasing Address Check
 *
 * \param pFlashParam flash driver erase parameter
 */
static FLASH_StatusType FLASHDRIVER_LL_EraseCheck(FLASH_DRIVER_ParamType *pFlashParam)
{
    uint32_t u32Addr, u32Length;
    FLASH_StatusType tRetVal;

    u32Addr = pFlashParam->u32Address;
    u32Length = pFlashParam->u32Length;

    tRetVal = FLASH_ERROR_OK;

    pFlashParam->u32ErrorAddress = 0x0U;

    /* check address align */
    if (u32Addr & (FLASH_ERASE_SECTOR_SIZE - 1U))
    {
        tRetVal = FLASH_ERROR_INVALID_ADDR;
        pFlashParam->u32ErrorAddress = u32Addr;

    }
    else
    {
        /* check length align */
        if (u32Length & (FLASH_ERASE_SECTOR_SIZE - 1U))
        {
            tRetVal = FLASH_ERROR_INVALID_SIZE;
            pFlashParam->u32ErrorAddress = u32Addr;
        }

    }

    return tRetVal;
}


/**
 * \brief Flash Driver Function for Writing address Check
 *
 * \param pFlashParam flash driver write parameter
 */
static FLASH_StatusType FLASHDRIVER_WriteCheck(FLASH_DRIVER_ParamType *pFlashParam)
{
    uint32_t u32Addr, u32Length;
    FLASH_StatusType tRetVal;

    u32Addr = pFlashParam->u32Address;
    u32Length = pFlashParam->u32Length;

    tRetVal = FLASH_ERROR_OK;
    pFlashParam->u32ErrorAddress = 0x0U;


    /* check address align */
    if (u32Addr & (FLASH_PROGRAM_PAGE_MIN_SIZE - 1U))
    {
        tRetVal = FLASH_ERROR_INVALID_ADDR;
        pFlashParam->u32ErrorAddress = pFlashParam->u32Address;
    }

    else
    {
        /* check length align */
        if (u32Length & (FLASH_PROGRAM_PAGE_MIN_SIZE - 1U))
        {
            tRetVal = FLASH_ERROR_INVALID_SIZE;
            pFlashParam->u32ErrorAddress = pFlashParam->u32Address;
        }
    }

    return tRetVal;
}



/**
 * \brief Flash Driver Function for lock/unlock sector
 *
 * \param u32Address sector address
 * \param bLock 0U-unlock, 1U-lock
 */
static FLASH_StatusType FLASHDRIVER_LockSector(uint32_t u32Address, uint8_t bLock)
{
    FLASH_StatusType tRetVal;
    tRetVal = FLASH_ERROR_OK;

    if (u32Address <= FLASH_BANK0_END)
    {
        if (bLock)
        {
            FMC->FB_FPELCK[0U] = 0xFFFFFFFFU;
        }
        else
        {
            FMC->FB_FPELCK[0U] = 0xFFFFFFFFU ^ (1UL << (u32Address >> 13));
        }

    }
    else if (u32Address < FLASH_BANK1_END)
    {
        if (bLock)
        {
            FMC->FB_FPELCK[1U]  = 0xFFFFFFFFU;
        }
        else
        {
            FMC->FB_FPELCK[1U]  = 0xFFFFFFFFU ^ (1UL << ((u32Address - 0x40000) >> 13));
        }
    }
    else
    {
        tRetVal = FLASH_ERROR_INVALID_ADDR;
    }

    return tRetVal;
}

/**
 * \brief Get Flash Configuration
 *
 * \param u32Address the flash address
 * \param pFlash_api_cfg out flash parameter
 */
static FLASH_StatusType FLASHDRIVER_GetFlashConfig(uint32_t u32Address, FLASH_DRV_ERASESECTOR_CFG_T *pFlash_api_cfg)
{
    FLASH_StatusType tRetVal;
    tRetVal = FLASH_ERROR_OK;

    if (u32Address <= FLASH_BANK0_END)
    {
        pFlash_api_cfg->blk_sel = 0UL;
        pFlash_api_cfg->dest = u32Address;

    }
    else if (u32Address < FLASH_BANK1_END)
    {
        pFlash_api_cfg->blk_sel = 1UL;
        pFlash_api_cfg->dest = u32Address;
    }
    else
    {
        tRetVal = FLASH_ERROR_INVALID_ADDR;
    }

    return tRetVal;
}

/**
 * \brief Erase Sector Asm Entry
 *
 * \param flash_api_cfg Erase Parameters
 * \return STATUS_SUCCESS is OK, others are not ok
 */
static uint32_t FLASHDRIVER_LL_EraseSector(FLASH_DRV_ERASESECTOR_CFG_T *flash_api_cfg)
{
    uint32_t u32FuncAddr;
    uint32_t u32RetVal;

    u32FuncAddr = (uint32_t)(s_pFlashDriver_FuncHeader->FLASH_DRV_EraseSector);

    asm volatile(
                "MOV     R1, %1\n\t"
                "MOV     R0, %2\n\t"
                "BLX     R1\n\t"
                "MOV     %0, R0\n\t"
                : "=r"(u32RetVal)
                : "r"(u32FuncAddr), "r"(flash_api_cfg)
                : "memory", "r0", "r1", "r2", "r3", "r4", "r14"
    );

    return u32RetVal;
}

/**
 * \brief Check HV status
 *
 * \return STATUS_SUCCESS is OK, others are not ok
 */
static uint32_t FLASHDRIVER_LL_HVStatusCheck(void)
{
    uint32_t u32FuncAddr;
    uint32_t u32RetVal;

    u32FuncAddr = (uint32_t)(s_pFlashDriver_FuncHeader->FLASH_DRV_HV_Status_Check);

    asm volatile(
                "MOV     R0, %1\n\t"
                "BLX     R0\n\t"
                "MOV     %0, R0\n\t"
                : "=r"(u32RetVal)
                : "r"(u32FuncAddr)
                : "memory", "r0", "r3", "r14"
    );

    return u32RetVal;
}

/**
 * \brief Call After FLASHDRIVER_LL_EraseSector to clear status
 *
 * \return STATUS_SUCCESS is OK, others are not ok
 */
static uint32_t FLASHDRIVER_LL_EraseSectorClear(void)
{
    uint32_t u32FuncAddr;
    uint32_t u32RetVal;

    u32FuncAddr = (uint32_t)(s_pFlashDriver_FuncHeader->FLASH_DRV_EraseSector_Clear);

    asm volatile(
                "MOV     R0, %1\n\t"
                "BLX     R0\n\t"
                "MOV     %0, R0\n\t"
                : "=r"(u32RetVal)
                : "r"(u32FuncAddr)
                : "memory", "r0", "r1", "r2", "r3", "r14"
    );

    return u32RetVal;
}

/**
 * \brief Program Asm Entry
 *
 * \param flash_api_cfg
 * \return STATUS_SUCCESS is OK, others are not ok
 */
static uint32_t FLASHDRIVER_LL_Program(FLASH_DRV_PRGM_CFG_T *flash_api_cfg)
{
    uint32_t u32FuncAddr;
    uint32_t u32RetVal;

    u32FuncAddr = (uint32_t)(s_pFlashDriver_FuncHeader->FLASH_DRV_Program);

    asm volatile(
                "MOV     R1, %1\n\t"
                "MOV     R0, %2\n\t"
                "BLX     R1\n\t"
                "MOV     %0, R0\n\t"
                : "=r"(u32RetVal)
                : "r"(u32FuncAddr), "r"(flash_api_cfg)
                : "memory", "r0", "r1", "r2", "r3", "r14"
    );

    return u32RetVal;
}

/**
 * \brief Call After FLASHDRIVER_LL_Program
 *
 * \return STATUS_SUCCESS is OK, others are not ok
 */
static uint32_t FLASHDRIVER_LL_ProgramClear(void)
{
    uint32_t u32FuncAddr;
    uint32_t u32RetVal;

    u32FuncAddr = (uint32_t)(s_pFlashDriver_FuncHeader->FLASH_DRV_Program_Clear);

    asm volatile(
                "MOV     R0, %1\n\t"
                "BLX     R0\n\t"
                "MOV     %0, R0\n\t"
                : "=r"(u32RetVal)
                : "r"(u32FuncAddr)
                : "memory", "r0", "r1", "r2", "r3", "r14"
    );

    return u32RetVal;
}

/**
 * \brief Flash Driver Function for Erasing
 *
 * \param pFlashParam flash driver erase parameter
 * \return ErrorType
 */
FLASH_StatusType FLASHDRIVER_Erase(FLASH_DRIVER_ParamType *pFlashParam)
{
    uint32_t u32Addr, u32Length;
    uint32_t u32TryCount;
    FLASH_DRV_ERASESECTOR_CFG_T tFlash_api_cfg;
    FLASH_StatusType tRetVal;
    uint32_t u32Temp;

    tRetVal = FLASH_ERROR_OK;



    /* FLASH_DRV_WDG_CFG_T tFlash_wdg_cfg; */

    u32Addr = pFlashParam->u32Address;
    u32Length = pFlashParam->u32Length;

    tRetVal = FLASH_ERROR_OK;
    pFlashParam->u32ErrorAddress = 0x0U;

    tRetVal = FLASHDRIVER_LL_EraseCheck(pFlashParam);

    if (tRetVal == FLASH_ERROR_OK)
    {

        /* flash watchdog config */
        /* tFlash_wdg_cfg.time = FLASH_WDG_TIMEOUT_SELECT3; */
        /* tFlash_wdg_cfg.wen = FLASH_WDG_ENABLE; */
        /* tFlash_wdg_cfg.wint_en = FLASH_WDG_INT_DISABLE; */
        /* s_pFlashDriver_FuncHeader->FLASH_DRV_Wdog_Configure(&tFlash_wdg_cfg); */


        pFlashParam->u32Length = FLASH_ERASE_SECTOR_SIZE;

        /* loop erase */
        for (pFlashParam->u32Address = u32Addr; pFlashParam->u32Address < u32Addr + u32Length;
             pFlashParam->u32Address += FLASH_ERASE_SECTOR_SIZE)
        {

            FLASHDRIVER_LockSector(pFlashParam->u32Address, 0U);
            FLASHDRIVER_GetFlashConfig(pFlashParam->u32Address, &tFlash_api_cfg);
            /* start erase */
            u32Temp = FLASHDRIVER_LL_EraseSector(&tFlash_api_cfg);

            tRetVal = (u32Temp == STATUS_SUCCESS) ? FLASH_ERROR_OK : FLASH_ERROR_FAILED;


            /* check erase operation valid */
            if (tRetVal != FLASH_ERROR_OK)
            {
                /* erase operation failed, exit */
                pFlashParam->u32ErrorAddress = pFlashParam->u32Address;
                break;
            }
            else
            {
                FLASHDRIVER_LL_Delay();
                /* check erasing still in progress */
                tRetVal = FLASH_ERROR_FAILED;
                u32TryCount = 0;
                while ((tRetVal != FLASH_ERROR_OK) && (u32TryCount++ < 100000))
                {
                    u32Temp = FLASHDRIVER_LL_HVStatusCheck();
                    /* check if finished */
                    if (u32Temp == STATUS_HVOP)
                    {
                        tRetVal = FLASH_ERROR_FAILED;
                    }
                    else
                    {
                        tRetVal = FLASH_ERROR_OK;
                    }
                }

                if (tRetVal != FLASH_ERROR_OK)
                {
                    /* erasing timeout, exit */
                    pFlashParam->u32ErrorAddress = pFlashParam->u32Address;
                    break;
                }
                else
                {
                    FLASHDRIVER_LL_Delay();
                    /* check erasing result */
                    u32Temp = FLASHDRIVER_LL_EraseSectorClear();
                    tRetVal = (u32Temp == STATUS_SUCCESS) ? FLASH_ERROR_OK : FLASH_ERROR_FAILED;

                    if (tRetVal != FLASH_ERROR_OK)
                    {
                        /* erasing failed, exit */
                        pFlashParam->u32ErrorAddress = pFlashParam->u32Address;
                        break;
                    }
                    else
                    {
                        /* trigger watchdog function */
                        if (pFlashParam->wdTriggerFct != ((void *)0))
                        {
                            pFlashParam->wdTriggerFct();
                        }

                    }
                }
            }
        }
    }


    return tRetVal;

}

/**
 * \brief Flash Driver Function for Writing
 *
 * \param pFlashParam flash driver write parameter
 * \return ErrorType
 */
FLASH_StatusType FLASHDRIVER_Write(FLASH_DRIVER_ParamType *pFlashParam)
{
    uint32_t u32Addr, u32Length, u32DataAddr, u32AlignLen, u32TempLen;
    /* uint8_t *pTempBuf; */
    uint32_t u32AlignOffset, u32Index, u32Count;
    uint32_t u32TryCount;
    /* FLASH_DRV_WDG_CFG_T tFlash_wdg_cfg; */
    FLASH_DRV_PRGM_CFG_T tFlash_api_cfg;

    FLASH_StatusType tRetVal;
    uint32_t u32Temp;


    tRetVal = FLASH_ERROR_OK;



    u32Addr = pFlashParam->u32Address;
    u32Length = pFlashParam->u32Length;

    pFlashParam->u32ErrorAddress = 0x0U;


    tRetVal = FLASHDRIVER_WriteCheck(pFlashParam);

    if (tRetVal == FLASH_ERROR_OK)
    {
        /* flash watchdog config */
        /* tFlash_wdg_cfg.time = 0x03UL; */
        /* tFlash_wdg_cfg.wen = 0x01UL; */
        /* tFlash_wdg_cfg.wint_en = FLASH_WDG_INT_DISABLE; */
        /* s_pFlashDriver_FuncHeader->FLASH_DRV_Wdog_Configure(&tFlash_wdg_cfg); */

        /* align address, write must align to FLASH_PROGRAM_PAGE_MAX_SIZE */
        u32AlignOffset = u32Addr & (FLASH_PROGRAM_PAGE_MAX_SIZE - 1U);
        u32AlignLen = u32Length + u32AlignOffset;

        u32Count = u32AlignLen & (FLASH_PROGRAM_PAGE_MAX_SIZE - 1U);

        u32Count = u32Count > 0U ? 1U : 0U;

        u32Count += u32AlignLen / FLASH_PROGRAM_PAGE_MAX_SIZE;

        u32DataAddr = (uint32_t)pFlashParam->pData;
        u32TempLen = u32AlignLen;

        for (u32Index = 0U; u32Index < u32Count; u32Index++)
        {
            /* real write length in this cycle */
            u32TempLen = u32Length + u32AlignOffset;
            u32TempLen = u32TempLen >= FLASH_PROGRAM_PAGE_MAX_SIZE ? FLASH_PROGRAM_PAGE_MAX_SIZE : u32TempLen;
            u32TempLen -= u32AlignOffset;

            pFlashParam->u32Address = u32Addr;
            pFlashParam->u32Length = u32TempLen;

            /* unlock sector */
            FLASHDRIVER_LockSector(pFlashParam->u32Address, 0U);
            tFlash_api_cfg.dest = pFlashParam->u32Address;
            tFlash_api_cfg.size = pFlashParam->u32Length / 4; /* one data is 4 bytes */
            tFlash_api_cfg.pData = (uint32_t *)u32DataAddr;
            tFlash_api_cfg.wdg_tune = WDG_TUNE_DISABLE;



            /* next address and length */
            u32Addr += u32TempLen;
            u32DataAddr += u32TempLen;
            u32Length -= u32TempLen;

            u32AlignOffset = 0U;

            /* start write */
            u32Temp = FLASHDRIVER_LL_Program(&tFlash_api_cfg);
            tRetVal = u32Temp == STATUS_SUCCESS ? FLASH_ERROR_OK : FLASH_ERROR_FAILED;



            /* check erase operation valid */
            if (tRetVal != FLASH_ERROR_OK)
            {
                /* write operation failed, exit */
                pFlashParam->u32ErrorAddress = pFlashParam->u32Address;
                break;
            }
            else
            {
                FLASHDRIVER_LL_Delay();

                /* check write still in progress */
                tRetVal = FLASH_ERROR_FAILED;
                u32TryCount = 0;
                while ((tRetVal != FLASH_ERROR_OK) && (u32TryCount++ < 10000))
                {
                    u32Temp = FLASHDRIVER_LL_HVStatusCheck();
                    /* check if finished */
                    if (u32Temp == STATUS_HVOP)
                    {
                        tRetVal = FLASH_ERROR_FAILED;
                    }
                    else
                    {
                        tRetVal = FLASH_ERROR_OK;
                    }
                }


                if (tRetVal != FLASH_ERROR_OK)
                {
                    /* erasing timeout, exit */
                    pFlashParam->u32ErrorAddress = pFlashParam->u32Address;
                    break;
                }
                else
                {
                    FLASHDRIVER_LL_Delay();

                    /* check write result */
                    u32Temp = FLASHDRIVER_LL_ProgramClear();
                    tRetVal = (u32Temp == STATUS_SUCCESS) ? FLASH_ERROR_OK : FLASH_ERROR_FAILED;

                    if (tRetVal != FLASH_ERROR_OK)
                    {
                        /* write failed, exit */
                        pFlashParam->u32ErrorAddress = pFlashParam->u32Address;
                        break;
                    }
                    else
                    {
                        /* trigger watchdog function */
                        if (pFlashParam->wdTriggerFct != ((void *)0))
                        {
                            pFlashParam->wdTriggerFct();
                        }

                    }
                }
            }


        }
    }

    return tRetVal;

}

static FLASH_DRIVER_ParamType s_tFlashParam;
static uint8_t s_aWriteBuffer[0x800];

uint16_t Only_Erase(uint32_t u32Addr, uint32_t u32EraseLen)
{
    FLASH_StatusType tRetVal;

    s_tFlashParam.u32Address = u32Addr;
    s_tFlashParam.u32Length  = u32EraseLen;

    tRetVal = FLASHDRIVER_Erase(&s_tFlashParam);


    return tRetVal;

}


uint16_t Only_Write(uint32_t u32Addr, uint32_t u32DataLen)
{
    uint32_t u32Index;
    FLASH_StatusType tRetVal;

    s_tFlashParam.u32Address = (uint32_t)u32Addr;
    s_tFlashParam.u32Length = (uint32_t)u32DataLen;
    s_tFlashParam.pData = (uint8_t*)s_aWriteBuffer;

    for(u32Index=0U;u32Index<sizeof(s_aWriteBuffer)/sizeof(s_aWriteBuffer[0]);u32Index++)
    {
        s_aWriteBuffer[u32Index]=(uint8_t)(u32Index+1U);
    }

    tRetVal = FLASHDRIVER_Write( &s_tFlashParam);

    return tRetVal;

}

uint16_t Only_Read(uint32_t u32Addr, uint32_t u32DataLen)
{
    uint32_t u32Index;
    uint8_t *pTempData;

    pTempData = (uint8_t*)u32Addr;

    if(s_aWriteBuffer!=NULL)
    {
        for(u32Index=0U;u32Index<u32DataLen;u32Index++)
        {
            s_aWriteBuffer[u32Index] = pTempData[u32Index];
        }
    }

    return 0U;

}

static volatile uint32_t * Fls_Flash_pTargetAddressPtr = NULL_PTR;
static const uint32_t * Fls_Flash_pSourceAddressPtr = NULL_PTR;
static const volatile uint8 * volatile Fls_Flash_pReadAddressPtr = NULL_PTR;

static uint32 Fls_Flash_u32DoubleWordToCopy = 0UL;
static uint32 Fls_Flash_u32ProgramCycle = 0UL;

static void  Fls_Flash_PageWrite( const uint32_t Page_size )
{
	FLASH_DRV_PRGM_CFG_T tFlash_api_cfg;
	FLASHDRIVER_LockSector((uint32_t)Fls_Flash_pTargetAddressPtr, 0U);
	tFlash_api_cfg.dest = (uint32_t)Fls_Flash_pTargetAddressPtr;
	tFlash_api_cfg.size = Page_size / 4; /* one data is 4 bytes  */
	tFlash_api_cfg.pData = (uint32_t *)Fls_Flash_pSourceAddressPtr;
	tFlash_api_cfg.wdg_tune = WDG_TUNE_DISABLE;
	FLASHDRIVER_LL_Program(&tFlash_api_cfg);
	Fls_Flash_pTargetAddressPtr = Fls_Flash_pTargetAddressPtr + Page_size/4;
	Fls_Flash_pSourceAddressPtr = Fls_Flash_pSourceAddressPtr + Page_size/4;
}

void  Fls_Flash_MainFunctionErase( void )
{
	uint32_t u32Temp;
    /* if erase finished */
    u32Temp = FLASHDRIVER_LL_HVStatusCheck();
    /* check if finished */
    if (u32Temp == STATUS_HVOP)
    {

    }
    else
    {
    	u32Temp = FLASHDRIVER_LL_EraseSectorClear();
    	if (u32Temp == STATUS_SUCCESS)
    	{
            /* erase operation succeed */
            Fls_u32JobSectorIt++;   /* The sector was successfully erased, increment the sector iterator.*/

            Fls_eLLDJob = FLASH_JOB_NONE;
            Fls_eLLDJobResult = MEMIF_JOB_OK;
    	}
    	else
    	{
            Fls_eLLDJob = FLASH_JOB_NONE;
            Fls_eLLDJobResult = MEMIF_JOB_FAILED;
    	}
    }
}

void Fls_Flash_MainFunctionWrite( void )
{
    uint32_t u32DataSectorPageSize = 0UL;
    uint32_t u32Temp;
    /* if write finished */
    u32Temp = FLASHDRIVER_LL_HVStatusCheck();
    /* check if finished */
    if (u32Temp == STATUS_HVOP)
    {

    }
    else
    {
    	u32Temp = FLASHDRIVER_LL_ProgramClear();
		if(u32Temp == STATUS_SUCCESS)
		{
            /* more page program operation to be performed? */
            if(Fls_Flash_u32ProgramCycle<Fls_Flash_u32DoubleWordToCopy)
            {
                /* execute page programming */

                /*
                * @violates @ref fls_flash_c_REF_10 Array indexing shall be the
                * only allowed form of pointer arithmetic
                */
                Fls_Flash_PageWrite((*(Fls_pConfigPtr->paSectorPageSize))[Fls_u32JobSectorIt]);
                Fls_Flash_u32ProgramCycle++;
                u32DataSectorPageSize = (*(Fls_pConfigPtr->paSectorPageSize))[Fls_u32JobSectorIt];
                Fls_u32LLDRemainingDataToWrite -= u32DataSectorPageSize;

                Fls_eLLDJob = FLASH_JOB_WRITE;
                Fls_eLLDJobResult = MEMIF_JOB_PENDING;
            }
            else
            {
                /* all page program operations succeed */

                /* For a Write Job in ASYNC mode check if Fls_u32JobSectorIt should be increased */
                if( (Fls_u32JobAddrIt > ((*(Fls_pConfigPtr->paSectorEndAddr))[Fls_u32JobSectorIt]) ))
                {
                    /* Move on to the next sector */
                    Fls_u32JobSectorIt++;
                }

                /* program operation succeed */
                Fls_eLLDJob = FLASH_JOB_NONE;
                Fls_eLLDJobResult = MEMIF_JOB_OK;
            }
		}
        else
        {
            /* program operation was not successful */

            /* program operation failed */
            Fls_eLLDJob = FLASH_JOB_NONE;
            Fls_eLLDJobResult = MEMIF_JOB_FAILED;
            /* return */
        }
    }
}

void Fls_Flash_Init( void )
{

}

Fls_LLDReturnType Fls_Flash_SectorErase(const Fls_PhysicalSectorType ePhySector, const boolean bAsynch)
{
	FLASH_DRV_ERASESECTOR_CFG_T tFlash_api_cfg;
	uint32_t u32Temp;
	FLASH_StatusType tRetVal;
	UNUSED(bAsynch);
	Fls_Flash_pTargetAddressPtr = (*((*(Fls_pConfigPtr->pSectorList))[ePhySector])).pSectorStartAddressPtr;
    FLASHDRIVER_LockSector((uint32_t)Fls_Flash_pTargetAddressPtr, 0U);
    FLASHDRIVER_GetFlashConfig((uint32_t)Fls_Flash_pTargetAddressPtr, &tFlash_api_cfg);
    /* start erase */
    u32Temp = FLASHDRIVER_LL_EraseSector(&tFlash_api_cfg);

    tRetVal = (u32Temp == STATUS_SUCCESS) ? FLASH_ERROR_OK : FLASH_ERROR_FAILED;


    /* check erase operation valid */
    if (tRetVal != FLASH_ERROR_OK)
    {
        return FLASH_E_FAILED;
    }
    else
    {
        /* schedule async job check for Fls_Flash_MainFunction */
        Fls_eLLDJob = FLASH_JOB_ERASE;
        Fls_eLLDJobResult = MEMIF_JOB_PENDING;

        /*
        * @violates @ref fls_flash_c_REF_6 Return statement before end of function.
        */
        return FLASH_E_PENDING;
    }
}



Fls_LLDReturnType Fls_Flash_SectorWrite(const Fls_PhysicalSectorType ePhySector, const Fls_AddressType u32SectorOffset,
		                                const Fls_LengthType  u32DataLength, const uint8_t * const pDataPtr, const boolean bAsynch)
{
	uint32_t u32DataSectorPageSize = 0UL;
	UNUSED(bAsynch);
    /*
    * @violates @ref fls_flash_c_REF_2 A cast should not be performed
    * between a pointer to object type and a different pointer to object type
    */
    Fls_Flash_pSourceAddressPtr = (const uint32_t*)(pDataPtr);
    /*
    * @violates @ref fls_flash_c_REF_10 Array indexing shall be the
    * only allowed form of pointer arithmetic
    */
    Fls_Flash_pTargetAddressPtr = (*((*(Fls_pConfigPtr->pSectorList))[ePhySector])).pSectorStartAddressPtr + u32SectorOffset/4;

    Fls_u32LLDRemainingDataToWrite = u32DataLength;

    /* Number of double words (i.e.64bits=8bytes) to be copied  */
    /*
    * @violates @ref fls_flash_c_REF_10 Array indexing shall be the
    * only allowed form of pointer arithmetic
    */
    Fls_Flash_u32DoubleWordToCopy = u32DataLength/(*(Fls_pConfigPtr->paSectorPageSize))[Fls_u32JobSectorIt];
    Fls_Flash_u32ProgramCycle = 0U;

	/*It is time to do word or double word programming */
	Fls_Flash_PageWrite((*(Fls_pConfigPtr->paSectorPageSize))[Fls_u32JobSectorIt]);
	Fls_Flash_u32ProgramCycle++;
	u32DataSectorPageSize = (*(Fls_pConfigPtr->paSectorPageSize))[Fls_u32JobSectorIt];
	Fls_u32LLDRemainingDataToWrite -= u32DataSectorPageSize;

	Fls_eLLDJob = FLASH_JOB_WRITE;
	Fls_eLLDJobResult = MEMIF_JOB_PENDING;

	/*
	* @violates @ref fls_flash_c_REF_6 Return statement before end of function.
	*/
	return FLASH_E_PENDING;
}

Fls_LLDReturnType  Fls_Flash_SectorRead (const Fls_PhysicalSectorType ePhySector, const Fls_AddressType u32SectorOffset, Fls_LengthType u32DataLength,
										uint8 * pDataPtr)
{
    Fls_LLDReturnType eRetVal = FLASH_E_OK;

    /* Physical flash address to read */
    Fls_Flash_pReadAddressPtr = (volatile const uint8 *)((*((*(Fls_pConfigPtr->pSectorList))[ePhySector])).pSectorStartAddressPtr) + \
                              u32SectorOffset;

    for(uint32_t u32Index=0U; u32Index<u32DataLength; u32Index++)
    {
    	pDataPtr[u32Index] = Fls_Flash_pReadAddressPtr[u32Index];
    }

    eRetVal = FLASH_E_OK;

    return eRetVal;
}

