#ifndef QSPI_H
#define QSPI_H

// Команды для Winbond W25Qxx

#define CMD_READ_STATUS_REG1      0x05
#define CMD_READ_STATUS_REG2      0x35
#define CMD_READ_STATUS_REG3      0x15
#define CMD_WRITE_ENABLE          0x06
#define CMD_FAST_READ             0x0B
#define CMD_WRITE_STATUS_REG2     0x31  // Для Winbond. У Macronix/Micron может быть общая команда 0x01
#define CMD_MANUFACTURER_ID       0x90
#define CMD_JEDEC_ID              0x9F
#define CMD_ENTER_QPI             0x38  // Команда перехода в режим QPI
#define CMD_EXIT_QPI              0xFF  // Команда выхода из режима QPI
#define CMD_SECTOR_ERASE_4K       0x20
#define CMD_BLOCK_ERASE_32K       0x52
#define CMD_BLOCK_ERASE_64K       0xD8
#define CMD_PAGE_PROGRAM          0x02
#define CMD_QUAD_PAGE_PROGRAM     0x32

#define WINBOND_CODE              0xEF  // Winbond Manufacturer Id


#define QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE 0x00000000U                     /*!<Indirect write mode*/
#define QSPI_FUNCTIONAL_MODE_INDIRECT_READ  ((uint32_t)QUADSPI_CCR_FMODE_0) /*!<Indirect read mode*/
#define QSPI_FUNCTIONAL_MODE_AUTO_POLLING   ((uint32_t)QUADSPI_CCR_FMODE_1) /*!<Automatic polling mode*/
#define QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED  ((uint32_t)QUADSPI_CCR_FMODE)   /*!<Memory-mapped mode*/

#define QSPI_ALTERNATE_BYTES_8_BITS    0x00000000U                      /*!<8-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_16_BITS   ((uint32_t)QUADSPI_CCR_ABSIZE_0) /*!<16-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_24_BITS   ((uint32_t)QUADSPI_CCR_ABSIZE_1) /*!<24-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_32_BITS   ((uint32_t)QUADSPI_CCR_ABSIZE)   /*!<32-bit alternate bytes*/

#define QSPI_INSTRUCTION_NONE          0x00000000U                     /*!<No instruction*/
#define QSPI_INSTRUCTION_1_LINE        ((uint32_t)QUADSPI_CCR_IMODE_0) /*!<Instruction on a single line*/
#define QSPI_INSTRUCTION_2_LINES       ((uint32_t)QUADSPI_CCR_IMODE_1) /*!<Instruction on two lines*/
#define QSPI_INSTRUCTION_4_LINES       ((uint32_t)QUADSPI_CCR_IMODE)   /*!<Instruction on four lines*/

#define QSPI_ADDRESS_NONE              0x00000000U                      /*!<No address*/
#define QSPI_ADDRESS_1_LINE            ((uint32_t)QUADSPI_CCR_ADMODE_0) /*!<Address on a single line*/
#define QSPI_ADDRESS_2_LINES           ((uint32_t)QUADSPI_CCR_ADMODE_1) /*!<Address on two lines*/
#define QSPI_ADDRESS_4_LINES           ((uint32_t)QUADSPI_CCR_ADMODE)   /*!<Address on four lines*/

#define QSPI_ALTERNATE_BYTES_NONE      0x00000000U                      /*!<No alternate bytes*/
#define QSPI_ALTERNATE_BYTES_1_LINE    ((uint32_t)QUADSPI_CCR_ABMODE_0) /*!<Alternate bytes on a single line*/
#define QSPI_ALTERNATE_BYTES_2_LINES   ((uint32_t)QUADSPI_CCR_ABMODE_1) /*!<Alternate bytes on two lines*/
#define QSPI_ALTERNATE_BYTES_4_LINES   ((uint32_t)QUADSPI_CCR_ABMODE)   /*!<Alternate bytes on four lines*/

#define QSPI_DATA_NONE                 0x00000000U                     /*!<No data*/
#define QSPI_DATA_1_LINE               ((uint32_t)QUADSPI_CCR_DMODE_0) /*!<Data on a single line*/
#define QSPI_DATA_2_LINES              ((uint32_t)QUADSPI_CCR_DMODE_1) /*!<Data on two lines*/
#define QSPI_DATA_4_LINES              ((uint32_t)QUADSPI_CCR_DMODE)   /*!<Data on four lines*/

#define QSPI_ADDRESS_8_BITS            0x00000000U                      /*!<8-bit address*/
#define QSPI_ADDRESS_16_BITS           ((uint32_t)QUADSPI_CCR_ADSIZE_0) /*!<16-bit address*/
#define QSPI_ADDRESS_24_BITS           ((uint32_t)QUADSPI_CCR_ADSIZE_1) /*!<24-bit address*/
#define QSPI_ADDRESS_32_BITS           ((uint32_t)QUADSPI_CCR_ADSIZE)   /*!<32-bit address*/

#define QSPI_SIOO_INST_EVERY_CMD       0x00000000U                  /*!<Send instruction on every transaction*/
#define QSPI_SIOO_INST_ONLY_FIRST_CMD  ((uint32_t)QUADSPI_CCR_SIOO) /*!<Send instruction only for the first command*/

#define QSPI_SAMPLE_SHIFTING_NONE      0x00000000U   // No clock cycle shift to sample data
#define QSPI_CS_HIGH_TIME_1_CYCLE      0x00000000U   // nCS stay high for at least 1 clock cycle between commands
#define QSPI_CLOCK_MODE_0              0x00000000U   // Clk stays low while nCS is released
#define QSPI_FLASH_ID_1                0x00000000U   // FLASH 1 selected
#define QSPI_FLASH_ID_2                ((uint32_t)QUADSPI_CR_FSEL) // FLASH 2 selected
#define QSPI_DUALFLASH_DISABLE         0x00000000U   // Dual-flash mode disabled

#define QSPI_CLOCK_PRESCALER           169
#define QSPI_FIFO_THRESHOLD            1
#define QSPI_FLASH_SIZE                23 // 23 bit addr = 128 MBits / 16MBytes
#define QSPI_SAMPLE_SHIFTING           QSPI_SAMPLE_SHIFTING_NONE
#define QSPI_CHIP_SELECT_HIGH_TIME     QSPI_CS_HIGH_TIME_1_CYCLE
#define QSPI_CLOCK_MODE                QSPI_CLOCK_MODE_0
#define QSPI_FLASH_ID                  QSPI_FLASH_ID_2
#define QSPI_DUAL_FLASH                QSPI_DUALFLASH_DISABLE
#define QSPI_AUTOMATIC_STOP_ENABLE     ((uint32_t)QUADSPI_CR_APMS) /*!<AutoPolling stops as soon as there is a match*/
#define QSPI_MATCH_MODE_AND            0x00000000U                /*!<AND match mode between unmasked bits*/
#define QSPI_MATCH_MODE_OR             ((uint32_t)QUADSPI_CR_PMM) /*!<OR match mode between unmasked bits*/


// JEDEC ID
typedef struct {
    __IO uint8_t manId;      // Manufacturer ID
    __IO uint8_t memTypeId;  // Memory Type
    __IO uint8_t capacityId; // Memory capacity
} JedecId;

// Manufacturer / Device ID
typedef struct {
    __IO uint8_t manId;   // Manufacturer ID
    __IO uint8_t devId;   // Device ID
} ManufacturerId;


void QSPI_Init(void);
void QSPI_Enable_QPI_Mode(void);
void QSPI_Read_JEDEC_ID(void);
void QSPI_Read_Manf_Device_ID(uint8_t linesNum);
uint32_t QSPI_GetStatus(uint8_t linesNum);
void QSPI_ReadBlock(uint32_t address, uint8_t *pBuffer, uint32_t size);
void QSPI_WritePage(uint32_t PageAddress, uint8_t* pData, uint32_t Size);
void QSPI_EnableMemoryMappedMode(void);

#endif
