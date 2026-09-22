#include "main.h"
#include "qspi.h"


/**
  * @brief  QSPI Command structure definition
  */
typedef struct {
  uint32_t Instruction;        /* Specifies the Instruction to be sent
                                  This parameter can be a value (8-bit) between 0x00 and 0xFF */
  uint32_t Address;            /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                                  This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
  uint32_t AlternateBytes;     /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                                  This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
  uint32_t AddressSize;        /* Specifies the Address Size
                                  This parameter can be a value of @ref QSPI_AddressSize */
  uint32_t AlternateBytesSize; /* Specifies the Alternate Bytes Size
                                  This parameter can be a value of @ref QSPI_AlternateBytesSize */
  uint32_t DummyCycles;        /* Specifies the Number of Dummy Cycles.
                                  This parameter can be a number between 0 and 31 */
  uint32_t InstructionMode;    /* Specifies the Instruction Mode
                                  This parameter can be a value of @ref QSPI_InstructionMode */
  uint32_t AddressMode;        /* Specifies the Address Mode
                                  This parameter can be a value of @ref QSPI_AddressMode */
  uint32_t AlternateByteMode;  /* Specifies the Alternate Bytes Mode
                                  This parameter can be a value of @ref QSPI_AlternateBytesMode */
  uint32_t DataMode;           /* Specifies the Data Mode (used for dummy cycles and data phases)
                                  This parameter can be a value of @ref QSPI_DataMode */
  uint32_t NbData;             /* Specifies the number of data to transfer. (This is the number of bytes)
                                  This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
                                  until end of memory)*/
  uint32_t DdrMode;            /* Specifies the double data rate mode for address, alternate byte and data phase
                                  This parameter can be a value of @ref QSPI_DdrMode */
  uint32_t DdrHoldHalfCycle;   /* Specifies if the DDR hold is enabled. When enabled it delays the data
                                  output by one quarter of QUADSPI output clock in DDR mode.
                                  This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
  uint32_t SIOOMode;           /* Specifies the send instruction only once mode
                                  This parameter can be a value of @ref QSPI_SIOOMode */
} QSPI_CommandT;

/**
  * @brief  QSPI Auto Polling mode configuration structure definition
  */
typedef struct
{
  uint32_t Match;              /* Specifies the value to be compared with the masked status register to get a match.
                                  This parameter can be any value between 0 and 0xFFFFFFFF */
  uint32_t Mask;               /* Specifies the mask to be applied to the status bytes received.
                                  This parameter can be any value between 0 and 0xFFFFFFFF */
  uint32_t Interval;           /* Specifies the number of clock cycles between two read during automatic polling phases.
                                  This parameter can be any value between 0 and 0xFFFF */
  uint32_t StatusBytesSize;    /* Specifies the size of the status bytes received.
                                  This parameter can be any value between 1 and 4 */
  uint32_t MatchMode;          /* Specifies the method used for determining a match.
                                  This parameter can be a value of @ref QSPI_MatchMode */
  uint32_t AutomaticStop;      /* Specifies if automatic polling is stopped after a match.
                                  This parameter can be a value of @ref QSPI_AutomaticStop */
} QSPI_AutoPollingT;

/**
  * @brief  QSPI Memory Mapped mode configuration structure definition
  */
typedef struct
{
  uint32_t TimeOutPeriod;      /* Specifies the number of clock to wait when the FIFO is full before to release the chip select.
                                  This parameter can be any value between 0 and 0xFFFF */
  uint32_t TimeOutActivation;  /* Specifies if the timeout counter is enabled to release the chip select.
                                  This parameter can be a value of @ref QSPI_TimeOutActivation */
} QSPI_MemoryMappedT;


static JedecId jedecId = {0};      // W25Q128: manId = EFh memTypeId = 60h capactyId = 18h
static ManufacturerId manId = {0}; // W25Q128: manId = EFh devId = 17h
static uint32_t statRegs = 0;


__STATIC_INLINE void QSPI_WaitFlagSet(const uint32_t Flag) {
    while (READ_BIT(QUADSPI->SR, Flag)==RESET){}
}

__STATIC_INLINE void QSPI_WaitFlagReset(const uint32_t Flag) {
    while (READ_BIT(QUADSPI->SR, Flag)!=RESET){}
}


void QSPI_Init(void) {
    LL_RCC_SetQUADSPIClockSource(LL_RCC_QUADSPI_CLKSOURCE_SYSCLK);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_QSPI);

    // Configure QSPI FIFO Threshold
    MODIFY_REG(QUADSPI->CR, QUADSPI_CR_FTHRES, (QSPI_FIFO_THRESHOLD - 1U) << QUADSPI_CR_FTHRES_Pos);

    QSPI_WaitFlagReset(QUADSPI_SR_BUSY);

    MODIFY_REG(QUADSPI->CR, (QUADSPI_CR_PRESCALER | QUADSPI_CR_SSHIFT | QUADSPI_CR_FSEL | QUADSPI_CR_DFM),
               ((QSPI_CLOCK_PRESCALER << QUADSPI_CR_PRESCALER_Pos) |
                   QSPI_SAMPLE_SHIFTING | QSPI_FLASH_ID | QSPI_DUAL_FLASH));

    /* Configure QSPI Flash Size, CS High Time and Clock Mode */
    MODIFY_REG(QUADSPI->DCR, (QUADSPI_DCR_FSIZE | QUADSPI_DCR_CSHT | QUADSPI_DCR_CKMODE),
               ((QSPI_FLASH_SIZE << QUADSPI_DCR_FSIZE_Pos) | QSPI_CHIP_SELECT_HIGH_TIME | QSPI_CLOCK_MODE));

    SET_BIT(QUADSPI->CR, QUADSPI_CR_EN);
}


/**
  * @brief  Configure the communication registers.
  * @param  cmd structure that contains the command configuration information
  * @param  FunctionalMode functional mode to configured
  *           This parameter can be one of the following values:
  *            @arg QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE: Indirect write mode
  *            @arg QSPI_FUNCTIONAL_MODE_INDIRECT_READ: Indirect read mode
  *            @arg QSPI_FUNCTIONAL_MODE_AUTO_POLLING: Automatic polling mode
  *            @arg QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED: Memory-mapped mode
  */
static void QSPI_Config(QSPI_CommandT *cmd, uint32_t FunctionalMode) {

  if (cmd->DataMode != QSPI_DATA_NONE && FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
    /* Configure QSPI: DLR register with the number of data to read or write */
    WRITE_REG(QUADSPI->DLR, (cmd->NbData - 1U));
  }

  if (cmd->InstructionMode != QSPI_INSTRUCTION_NONE) {
    if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE) {
      /* Configure QSPI: ABR register with alternate bytes value */
      WRITE_REG(QUADSPI->ABR, cmd->AlternateBytes);

      if (cmd->AddressMode != QSPI_ADDRESS_NONE) {
        /*---- Command with instruction, address and alternate bytes ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateBytesSize | cmd->AlternateByteMode |
                    cmd->AddressSize | cmd->AddressMode | cmd->InstructionMode |
                    cmd->Instruction | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
            /* Configure QSPI: AR register with address value */
            WRITE_REG(QUADSPI->AR, cmd->Address);
        }
      } else {
        /*---- Command with instruction and alternate bytes ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateBytesSize | cmd->AlternateByteMode |
                    cmd->AddressMode | cmd->InstructionMode |
                    cmd->Instruction | FunctionalMode));

        /* Clear AR register */
        CLEAR_REG(QUADSPI->AR);
      }
    } else {
      if (cmd->AddressMode != QSPI_ADDRESS_NONE) {
        /*---- Command with instruction and address ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateByteMode | cmd->AddressSize | cmd->AddressMode |
                    cmd->InstructionMode | cmd->Instruction | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
          /* Configure QSPI: AR register with address value */
          WRITE_REG(QUADSPI->AR, cmd->Address);
        }
      } else {
        /*---- Command with only instruction ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateByteMode | cmd->AddressMode |
                    cmd->InstructionMode | cmd->Instruction | FunctionalMode));

        /* Clear AR register */
        CLEAR_REG(QUADSPI->AR);
      }
    }
  } else {
    if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE) {
      /* Configure QSPI: ABR register with alternate bytes value */
      WRITE_REG(QUADSPI->ABR, cmd->AlternateBytes);

      if (cmd->AddressMode != QSPI_ADDRESS_NONE) {
        /*---- Command with address and alternate bytes ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateBytesSize | cmd->AlternateByteMode |
                    cmd->AddressSize | cmd->AddressMode |
                    cmd->InstructionMode | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
          /* Configure QSPI: AR register with address value */
          WRITE_REG(QUADSPI->AR, cmd->Address);
        }
      } else {
        /*---- Command with only alternate bytes ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateBytesSize | cmd->AlternateByteMode |
                    cmd->AddressMode | cmd->InstructionMode | FunctionalMode));

        /* Clear AR register */
        CLEAR_REG(QUADSPI->AR);
      }
    } else {
      if (cmd->AddressMode != QSPI_ADDRESS_NONE) {
        /*---- Command with only address ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateByteMode | cmd->AddressSize |
                    cmd->AddressMode | cmd->InstructionMode | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
          /* Configure QSPI: AR register with address value */
          WRITE_REG(QUADSPI->AR, cmd->Address);
        }
      } else {
        /*---- Command with only data phase ----*/
        if (cmd->DataMode != QSPI_DATA_NONE) {
          /* Configure QSPI: CCR register with all communications parameters */
          WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                      cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                      cmd->AlternateByteMode | cmd->AddressMode |
                      cmd->InstructionMode | FunctionalMode));

          /* Clear AR register */
          CLEAR_REG(QUADSPI->AR);
        }
      }
    }
  }
}


/**
  * @brief Transmit an amount of data in blocking mode.
  * @param pData pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  */
void QSPI_Transmit(uint8_t *pData) {
  __IO uint32_t *data_reg = &QUADSPI->DR;

  if (pData != 0) {
    // Configure counters and size of the handle
    uint32_t TxXferCount = READ_REG(QUADSPI->DLR) + 1U;
    // uint32_t TxXferSize = READ_REG(QUADSPI->DLR) + 1U;
    uint8_t *pTxBuffPtr = pData;

    // Configure QSPI: CCR register with functional as indirect write
    MODIFY_REG(QUADSPI->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

    while (TxXferCount > 0U) {
      // Wait until FT flag is set to send data
      QSPI_WaitFlagSet(QUADSPI_SR_FTF);

      *((__IO uint8_t *) data_reg) = *pTxBuffPtr;
      pTxBuffPtr++;
      TxXferCount--;
    }

    // Wait until TC flag is set to go back in idle state
    QSPI_WaitFlagSet(QUADSPI_SR_TCF);
    // Clear Transfer Complete bit
    // __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
    WRITE_REG(QUADSPI->FCR, QUADSPI_SR_TCF);
  }
}


/**
  * @brief Receive an amount of data in blocking mode.
  * @param pData pointer to data buffer
  * @note   This function is used only in Indirect Read Mode
  */
void QSPI_Receive(uint8_t *pData) {
  uint32_t addr_reg = READ_REG(QUADSPI->AR);
  __IO uint32_t *data_reg = &QUADSPI->DR;

  if (pData != 0) {
    // Configure counters and size of the handle
    uint32_t RxXferCount = READ_REG(QUADSPI->DLR) + 1U;
    uint8_t *pRxBuffPtr = pData;

    // Configure QSPI: CCR register with functional as indirect read
    MODIFY_REG(QUADSPI->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_READ);

    // Start the transfer by re-writing the address in AR register
    WRITE_REG(QUADSPI->AR, addr_reg);

    while (RxXferCount > 0U) {
      // Wait until FT or TC flag is set to read received data
      QSPI_WaitFlagSet(QUADSPI_SR_FTF | QUADSPI_SR_TCF);

      *pRxBuffPtr = *((__IO uint8_t *) data_reg);
      pRxBuffPtr++;
      RxXferCount--;
    }

    // Wait until TC flag is set to go back in idle state
    QSPI_WaitFlagSet(QUADSPI_SR_TCF);
    // Clear Transfer Complete bit
    // __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
    WRITE_REG(QUADSPI->FCR, QUADSPI_SR_TCF);
  }
}


/**
  * @brief  Configure the QSPI Automatic Polling Mode in blocking mode.
  * @param  cmd structure that contains the command configuration information.
  * @param  cfg structure that contains the polling configuration information.
  * @note   This function is used only in Automatic Polling Mode
  */
void QSPI_AutoPolling(QSPI_CommandT *cmd, QSPI_AutoPollingT const *cfg) {

    QSPI_WaitFlagReset(QUADSPI_SR_BUSY);

    WRITE_REG(QUADSPI->PSMAR, cfg->Match);
    WRITE_REG(QUADSPI->PSMKR, cfg->Mask);
    WRITE_REG(QUADSPI->PIR, cfg->Interval);

    /* Configure QSPI: CR register with Match mode and Automatic stop enabled
    (otherwise there will be an infinite loop in blocking mode) */
    MODIFY_REG(QUADSPI->CR, (QUADSPI_CR_PMM | QUADSPI_CR_APMS), (cfg->MatchMode | QSPI_AUTOMATIC_STOP_ENABLE));

    cmd->NbData = cfg->StatusBytesSize;
    QSPI_Config(cmd, QSPI_FUNCTIONAL_MODE_AUTO_POLLING);

    QSPI_WaitFlagSet(QUADSPI_SR_SMF);

    // __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_SM);
    WRITE_REG(QUADSPI->FCR, QUADSPI_SR_SMF);
}


/**
  * @brief Set the command configuration.
  * @param cmd : structure that contains the command configuration information
  * @note   This function is used only in Indirect Read or Write Modes
  */
void QSPI_Command(QSPI_CommandT *cmd) {

    /* Wait till BUSY flag reset */
    QSPI_WaitFlagReset(QUADSPI_SR_BUSY);// RESET

    /* Call the configuration function */
    QSPI_Config(cmd, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

    if (cmd->DataMode == QSPI_DATA_NONE) {
        /* When there is no data phase, the transfer start as soon as the configuration is done
        so wait until TC flag is set to go back in idle state */
        QSPI_WaitFlagSet(QUADSPI_SR_TCF); // SET

        // __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
        WRITE_REG(QUADSPI->FCR, QUADSPI_SR_TCF);
    }
}

/**
  * @brief  Configure the Memory Mapped mode.
  * @param  cmd structure that contains the command configuration information.
  * @param  cfg structure that contains the memory mapped configuration information.
  * @note   This function is used only in Memory mapped Mode
  */
void QSPI_MemoryMapped(QSPI_CommandT *cmd, QSPI_MemoryMappedT const *cfg) {

    QSPI_WaitFlagReset(QUADSPI_SR_BUSY);

    /* Configure QSPI: CR register with timeout counter enable */
    MODIFY_REG(QUADSPI->CR, QUADSPI_CR_TCEN, cfg->TimeOutActivation);

    if (cfg->TimeOutActivation == QUADSPI_CR_TCEN) {

      /* Configure QSPI: LPTR register with the low-power timeout value */
      WRITE_REG(QUADSPI->LPTR, cfg->TimeOutPeriod);

      WRITE_REG(QUADSPI->FCR, QUADSPI_SR_TOF);
      SET_BIT(QUADSPI->CR, QUADSPI_CR_TOIE);
    }

    QSPI_Config(cmd, QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED);
}


// Функция для отправки простых команд без данных (Write Enable, Enter QPI)
static void QSPI_WriteCmd(const uint8_t cmd) {
    QSPI_CommandT s_command = {0};
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = cmd;
    // s_command.AddressMode       = QSPI_ADDRESS_NONE;
    // s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    // s_command.DataMode          = QSPI_DATA_NONE;
    // s_command.DummyCycles       = 0;
    // s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    // s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    QSPI_Command(&s_command);
}

// Функция для отправки 4Line простых команд без данных (Write Enable, Enter QPI)
static void QSPI_WriteCmd4L(const uint8_t cmd) {
    QSPI_CommandT s_command = {0};
    s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction = cmd;
    // s_command.AddressMode       = QSPI_ADDRESS_NONE;
    // s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    // s_command.DataMode          = QSPI_DATA_NONE;
    // s_command.DummyCycles       = 0;
    // s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    // s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    QSPI_Command(&s_command);
}


static uint8_t stat1 = 0;
static uint8_t stat2 = 0;
static uint8_t stat3 = 0;
static uint8_t stat4 = 0;

// функция инициализации QPI
void QSPI_Enable_QPI_Mode(void) {
    QSPI_CommandT s_command = {0};
    uint8_t reg2 = 0;

    // QSPI_WriteCmd4L(CMD_EXIT_QPI);
    // request Manufacturer ID using 4 lines
    QSPI_Read_Manf_Device_ID(1);
    if (manId.manId == WINBOND_CODE) {
        return; // QPI is already active
    }

    // Читаем Status Register 2 по одной линии, чтобы узнать состояние бита QE
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = CMD_READ_STATUS_REG2;
    // s_command.AddressMode       = QSPI_ADDRESS_NONE;
    // s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_1_LINE;
    s_command.NbData = 1;
    // s_command.DummyCycles       = 0;

    QSPI_Command(&s_command);
    QSPI_Receive(&reg2);
    stat1 = reg2;

    // Если бит QE (обычно Bit 1 в SR2 для Winbond) не установлен — устанавливаем его
    if ((reg2 & 0x02) == 0) {
        stat2 = 0x51;
        // Перед записью в регистр статуса ОБЯЗАТЕЛЬНО шлем Write Enable
        QSPI_WriteCmd(CMD_WRITE_ENABLE);

        reg2 |= 0x02; // Устанавливаем бит QE

        // команда записи в Status Register 2
        s_command.Instruction = CMD_WRITE_STATUS_REG2;
        QSPI_Command(&s_command);
        QSPI_Transmit(&reg2);

        // читаем Status Register 2
        s_command.Instruction = CMD_READ_STATUS_REG2;
        QSPI_Command(&s_command);
        QSPI_Receive(&stat3);

        // Ждем завершения записи (опционально, можно опросить SR1 бит BUSY)
        LL_mDelay(1);

        // читаем Status Register 2
        s_command.Instruction = CMD_READ_STATUS_REG2;
        QSPI_Command(&s_command);
        QSPI_Receive(&stat4);
    }

    // Отправляем команду перевода Flash в режим QPI (команда идет еще по 1 линии)
    QSPI_WriteCmd(CMD_ENTER_QPI);
}


void QSPI_Read_JEDEC_ID(void) {
  QSPI_CommandT sCommand = {0};

  // Настройка структуры команды
  sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;     // Инструкция по 4 линиям
  sCommand.Instruction = CMD_JEDEC_ID;                     // Команда Read JEDEC ID
  // sCommand.AddressMode       = QSPI_ADDRESS_NONE;       // Адрес не передается
  // sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  // sCommand.DummyCycles       = 0;                       // Для 0x9F dummy-циклы не нужны
  sCommand.DataMode = QSPI_DATA_4_LINES;                   // Данные принимаем по 4 линиям
  sCommand.NbData = 3;                                     // Ожидаем 3 байта
  // sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  // Отправка команды в QSPI периферию
  QSPI_Command(&sCommand);

  // Получение данных от флеш-памяти
  QSPI_Receive((uint8_t *) &jedecId);
}

/**
 * @param linesNum 0 - 1 spi line, 1 - 4 spi line
 */
void QSPI_Read_Manf_Device_ID(const uint8_t linesNum) {
  QSPI_CommandT sCommand = {0};

  // Настройка структуры команды
  sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
  sCommand.Instruction = CMD_MANUFACTURER_ID;              // Команда Read Manufacturer / Device ID
  sCommand.AddressMode = QSPI_ADDRESS_4_LINES;             // Здесь адрес НУЖЕН
  sCommand.AddressSize = QSPI_ADDRESS_24_BITS;             // 24-битный адрес
  // sCommand.Address           = 0x000000;                // Адрес должен быть равен 0
  // sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  // sCommand.DummyCycles       = 0;                       // 0 циклов ожидания
  sCommand.DataMode = QSPI_DATA_4_LINES;
  sCommand.NbData = 2;                                     // Читаем 2 байта (Mfg ID + Dev ID)
  // sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  if (linesNum == 0) {
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.DataMode = QSPI_DATA_1_LINE;
  }

  // Отправка команды
  QSPI_Command(&sCommand);

  // Чтение ID
  QSPI_Receive((uint8_t *) &manId);
}


/**
 * @param linesNum 0 - 1 line, 1 - 4 lines
 * @return Status registers Reg1, Reg2, Reg3
 * Reg1
 * S7  S6  S5 S4  S3  S2  S1  S0
 * SRP SEC TB BP2 BP1 BP0 WEL BUSY
 * Reg2
 * S15 S14 S13 S12 S11 S10 S9 S8
 * SUS CMP LB3 LB2 LB1 (R) QE SRL
 * Reg3
 * S23  S22  S21  S20 S19 S18 S17 S16
 * HOLD DRV1 DRV0 (R) (R) WPS  R   R
 * /RST
 */
uint32_t QSPI_GetStatus(uint8_t linesNum) {
  QSPI_CommandT s_command = {0};
  uint8_t reg = 0;

  // 25Q64JVIQ 00h 02h 60h
  // Initialize the read status register command (Opcode 0x05)
  s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction = CMD_READ_STATUS_REG1; // Read Status Register
  // s_command.AddressMode       = QSPI_ADDRESS_NONE;
  // s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_4_LINES;
  // s_command.DummyCycles       = 0;
  s_command.NbData = 1;
  // s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  // s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  if (linesNum == 0) {
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.DataMode = QSPI_DATA_1_LINE;
  }

  // Send the command
  QSPI_Command(&s_command);

  // Receive the status register data
  QSPI_Receive(&reg);
  statRegs = reg;

  s_command.Instruction = CMD_READ_STATUS_REG2; // Read Status Register
  // Send the command
  QSPI_Command(&s_command);

  // Receive the status register data
  QSPI_Receive(&reg);
  statRegs |= reg << 8;

  s_command.Instruction = CMD_READ_STATUS_REG3; // Read Status Register
  // Send the command
  QSPI_Command(&s_command);

  // Receive the status register data
  QSPI_Receive(&reg);

  statRegs |= reg << 16;

  return statRegs;
}

/**
  * @brief  Чтение блока данных из QSPI Flash Winbond (Команда 0x0B - Fast Read, QPI mode)
  * @param  address: Внутренний адрес в памяти Flash (например, 0x000000)
  * @param  pBuffer: Указатель на буфер в RAM для сохранения данных
  * @param  size: Количество байт для чтения
  * @retval HAL_StatusTypeDef
  */
void QSPI_ReadBlock(uint32_t address, uint8_t *pBuffer, uint32_t size) {

    QSPI_CommandT sCommand = {0};

    // Настройка параметров команды чтения
    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;   // Команда отправляется по 4 линиям, QPI mode
    sCommand.Instruction       = CMD_FAST_READ;              // Команда Fast Read, QPI mode
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;       // Адрес отправляется по 4 линиям, QPI mode
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;       // Для чипов до 128 Мбит (16 МБ) используется 24-бит адрес
    sCommand.Address           = address;                    // Целевой адрес во флеш-памяти
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  // Альтернативные байты не используются
    sCommand.DummyCycles       = 2;                          // 2 dummy cycles. In QPI mode depending on the Read Parameter Bits P[5:4]
    sCommand.DataMode          = QSPI_DATA_4_LINES;          // Данные принимаются по 4 линиям, QPI mode
    sCommand.NbData            = size;                       // Размер считываемого блока в байтах
    // sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      // Выключаем Double Data Rate (чтение по переднему фронту)
    // sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   // Отправлять инструкцию при каждом вызове

    // Отправка команды и инициация приема данных
    QSPI_Command(&sCommand);

    // Чтение данных из QSPI в буфер RAM
    QSPI_Receive(pBuffer);
}


// Ожидание окончания внутренней операции (очистки бита BUSY)
void QSPI_WaitForBusy() {
  QSPI_CommandT s_command = {0};
  QSPI_AutoPollingT s_config = {0};

  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = CMD_READ_STATUS_REG1;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  // s_command.DummyCycles       = 0;
  // s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  // s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  // Настройка автоматического опроса регистра статуса (Бит 0 = BUSY)
  s_config.Match           = 0x00; // Ждем, когда BUSY станет 0
  s_config.Mask            = 0x01; // Маска первого бита (BUSY)
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  QSPI_AutoPolling(&s_command, &s_config);
}

// Стирание блока 64 КБ (или сектора 4 КБ)
void QSPI_EraseBlock(uint32_t BlockAddress) {
  QSPI_CommandT s_command = {0};

  QSPI_WriteCmd4L(CMD_WRITE_ENABLE);

  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = CMD_SECTOR_ERASE_4K; // can use: CMD_SECTOR_ERASE_4K CMD_BLOCK_ERASE_32K CMD_BLOCK_ERASE_64K
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS; // 24-битный адрес для W25Q до 128Мбит
  s_command.Address           = BlockAddress;
  // s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  // s_command.DataMode          = QSPI_DATA_NONE;
  // s_command.DummyCycles       = 0;
  // s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  // s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  QSPI_Command(&s_command);

  QSPI_WaitForBusy(); // Стирание блока памяти может занимать до 2 секунд
}

uint32_t regStat1 = 0;
uint32_t regStat2 = 0;
uint32_t regStat3 = 0;
uint32_t regStat4 = 0;

// Запись одной страницы (до 256 байт) в режиме Quad (4 линии данных)
void QSPI_WritePage(uint32_t PageAddress, uint8_t* pData, uint32_t Size) {
    QSPI_CommandT s_command = {0};

    if (Size > 256) Error_Handler(); // Максимальный размер страницы Winbond
    regStat1 = QSPI_GetStatus(1);
    QSPI_WriteCmd4L(CMD_WRITE_ENABLE);
    regStat2 = QSPI_GetStatus(1);

    // QPI mode (4-4-4: Инструкция х4, Адрес х4, Данные х4)
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction       = CMD_PAGE_PROGRAM;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.Address           = PageAddress;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_4_LINES; // Данные идут по 4 линиям (Quad)
    s_command.NbData            = Size;
    // s_command.DummyCycles       = 0;
    // s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    // s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    QSPI_Command(&s_command);
    QSPI_Transmit(pData);
    // regStat3 = QSPI_GetStatus(1);

    // HAL_Delay(1);
    QSPI_WaitForBusy(); // Запись страницы обычно занимает до 3-5 мс
    regStat4 = QSPI_GetStatus(1);
}


void QSPI_EnableMemoryMappedMode(void) {
    QSPI_CommandT s_command = {0};
    QSPI_MemoryMappedT s_mem_mapped_cfg = {0};

    // Настройка команды чтения Flash-памяти
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;    // Команда шлется по 4 линиям, QPI mode
    s_command.Instruction       = CMD_FAST_READ;               // Fast Read, QPI mode
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;        // Адрес передается по 4 линиям, QPI mode
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;        // 3-байтовый адрес (для памяти >16МБ нужно 32_BITS)
    // s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;   // Дополнительные байты не используются
    s_command.DummyCycles       = 2;                           // Количество холостых тактов для стабилизации данных (см. даташит флешки!)
    s_command.DataMode          = QSPI_DATA_4_LINES;           // Чтение данных по 4 линиям
    // s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    // s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;    // Посылать инструкцию при каждом обращении

    // Настройка тайм-аута (для авто-отключения CS, если запросов долго нет)
    // s_mem_mapped_cfg.TimeOutActivation = 0; // QSPI_TIMEOUT_COUNTER_DISABLE;
    // s_mem_mapped_cfg.TimeOutPeriod     = 0;

    // Включение режима маппинга
    QSPI_MemoryMapped(&s_command, &s_mem_mapped_cfg);
}
