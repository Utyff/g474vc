#include "main.h"
#include "qspi.h"


/**
  * @brief  Структура, описывающая команду QSPI.
  * Каждая фаза передачи (инструкция, адрес, дополнительные байты, данные)
  * включается, если соответствующее поле *Mode не равно *_NONE;
  * количество линий выбирается значением QSPI_*_x_LINES.
  */
typedef struct {
  uint32_t Instruction;        /* Команда (опкод) для отправки (8 бит, 0x00..0xFF) */
  uint32_t Address;            /* Адрес для отправки (от 1 до 4 байт, зависит от AddressSize) */
  uint32_t AlternateBytes;     /* Дополнительные байты (от 1 до 4, зависит от AlternateBytesSize) */
  uint32_t AddressSize;        /* Размер адреса (см. QSPI_ADDRESS_x_BITS) */
  uint32_t AlternateBytesSize; /* Размер дополнительных байт (см. QSPI_ALTERNATE_BYTES_x_BITS) */
  uint32_t DummyCycles;        /* Количество dummy-циклов (0..31) */
  uint32_t InstructionMode;    /* Режим передачи инструкции (см. QSPI_INSTRUCTION_*_LINES) */
  uint32_t AddressMode;        /* Режим передачи адреса (см. QSPI_ADDRESS_*_LINES) */
  uint32_t AlternateByteMode;  /* Режим передачи доп. байт (см. QSPI_ALTERNATE_BYTES_*_LINES) */
  uint32_t DataMode;           /* Режим передачи данных (используется и для dummy-циклов) */
  uint32_t NbData;             /* Количество передаваемых байт (0 — длина до конца памяти) */
  uint32_t DdrMode;            /* Режим Double Data Rate для адресной, доп. и данных фаз */
  uint32_t DdrHoldHalfCycle;   /* DDR hold: задержка вывода данных на 1/4 такта QSPI в DDR режиме */
  uint32_t SIOOMode;           /* Режим SIOO: инструкция один раз или при каждой операции */
} QSPI_CommandT;

/**
  * @brief  Структура настроек режима Automatic Polling (автоматический опрос).
  */
typedef struct
{
  uint32_t Match;              /* Значение, с которым сравнивается замаскированный регистр статуса */
  uint32_t Mask;               /* Маска, применяемая к байтам статуса */
  uint32_t Interval;           /* Количество тактов между двумя считываниями при автоматическом опросе */
  uint32_t StatusBytesSize;    /* Количество байт статуса для чтения (1..4) */
  uint32_t MatchMode;          /* Режим срабатывания (см. QSPI_MATCH_MODE_*) */
  uint32_t AutomaticStop;      /* Останавливать ли опрос при совпадении (QSPI_AUTOMATIC_STOP_ENABLE) */
} QSPI_AutoPollingT;

/**
  * @brief  Структура настроек режима Memory-Mapped.
  */
typedef struct
{
  uint32_t TimeOutPeriod;      /* Таймаут в тактах: при заполненном FIFO на это время CS освобождается */
  uint32_t TimeOutActivation;  /* Вкл/выкл счетчика таймаута освобождения CS (см. QSPI_TIMEOUT_COUNTER_*) */
} QSPI_MemoryMappedT;


static JedecId jedecId = {0};      // W25Q128: manId = EFh memTypeId = 60h capacityId = 18h
static ManufacturerId manId = {0}; // W25Q128: manId = EFh devId = 17h
static uint32_t statRegs = 0;      // Текущее значение регистров статуса (3 байта, см. QSPI_GetStatus)


/**
 * @brief Ожидает установки указанного флага в статусном регистре QSPI (SR).
 * Функция блокирующая, завершается только когда флаг установлен.
 * @param Flag — ожидаемый флаг (бит QUADSPI_SR_*).
 */
__STATIC_INLINE void QSPI_WaitFlagSet(const uint32_t Flag) {
    while (READ_BIT(QUADSPI->SR, Flag)==RESET){}
}

/**
 * @brief Ожидает сброса указанного флага в статусном регистре QSPI (SR).
 * Функция блокирующая, завершается только когда флаг сброшен.
 * @param Flag — ожидаемый флаг (бит QUADSPI_SR_*).
 */
__STATIC_INLINE void QSPI_WaitFlagReset(const uint32_t Flag) {
    while (READ_BIT(QUADSPI->SR, Flag)!=RESET){}
}


/**
 * @brief Настройка периферии QSPI: источник тактирования (SYSCLK),
 * пороговый уровень FIFO, предделитель такта QSPI, сдвиг выборки данных,
 * выбор чипа флеш, размер флеш-памяти и режим такта в состоянии покоя.
 */
void QSPI_Init(void) {
    LL_RCC_SetQUADSPIClockSource(LL_RCC_QUADSPI_CLKSOURCE_SYSCLK);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_QSPI);

    // Пороговый уровень FIFO (FIFO Threshold)
    MODIFY_REG(QUADSPI->CR, QUADSPI_CR_FTHRES, (QSPI_FIFO_THRESHOLD - 1U) << QUADSPI_CR_FTHRES_Pos);

    QSPI_WaitFlagReset(QUADSPI_SR_BUSY); // Дождаться, пока периферия будет свободна

    // Предделитель, сдвиг выборки данных, выбор чипа, режим dual-flash
    MODIFY_REG(QUADSPI->CR, (QUADSPI_CR_PRESCALER | QUADSPI_CR_SSHIFT | QUADSPI_CR_FSEL | QUADSPI_CR_DFM),
               ((QSPI_CLOCK_PRESCALER << QUADSPI_CR_PRESCALER_Pos) |
                   QSPI_SAMPLE_SHIFTING | QSPI_FLASH_ID | QSPI_DUAL_FLASH));

    /* Размер флеш-памяти, CS High Time and Clock Mode */
    MODIFY_REG(QUADSPI->DCR, (QUADSPI_DCR_FSIZE | QUADSPI_DCR_CSHT | QUADSPI_DCR_CKMODE),
               ((QSPI_FLASH_SIZE << QUADSPI_DCR_FSIZE_Pos) | QSPI_CHIP_SELECT_HIGH_TIME | QSPI_CLOCK_MODE));

    // Включение периферии
    SET_BIT(QUADSPI->CR, QUADSPI_CR_EN);
}


/**
  * @brief Запрограммировать регистры QSPI для передачи данных.
  * В соответствии с включенными фазами (instruction, address, alternate bytes, data)
  * в структуре cmd заполняются регистры CCR, AR, ABR, DLR.
  * @param cmd — указатель на структуру с параметрами команды.
  * @param FunctionalMode функциональный режим работы:
  *        QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE — косвенная запись;
  *        QSPI_FUNCTIONAL_MODE_INDIRECT_READ — косвенное чтение;
  *        QSPI_FUNCTIONAL_MODE_AUTO_POLLING — автоматический опрос;
  *        QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED — режим маппинга памяти.
 */
static void QSPI_Config(QSPI_CommandT *cmd, uint32_t FunctionalMode) {

  if (cmd->DataMode != QSPI_DATA_NONE && FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
    /* Запись DLR: количество данных для чтения/записи */
    WRITE_REG(QUADSPI->DLR, (cmd->NbData - 1U));
  }

  if (cmd->InstructionMode != QSPI_INSTRUCTION_NONE) {
    if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE) {
      /* Настройка QSPI: регистр ABR — значение дополнительных байт */
      WRITE_REG(QUADSPI->ABR, cmd->AlternateBytes);

      if (cmd->AddressMode != QSPI_ADDRESS_NONE) {
        /*--- Команда с инструкцией, адресом и дополнительными байтами ---*/
        /* Настройка QSPI: регистр CCR — все параметры коммуникации */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateBytesSize | cmd->AlternateByteMode |
                    cmd->AddressSize | cmd->AddressMode | cmd->InstructionMode |
                    cmd->Instruction | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
            /* Настройка QSPI: регистр AR — значение адреса */
            WRITE_REG(QUADSPI->AR, cmd->Address);
        }
      } else {
        /*--- Команда с инструкцией и дополнительными байтами ---*/
        /* Настройка QSPI: регистр CCR — все параметры коммуникации */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateBytesSize | cmd->AlternateByteMode |
                    cmd->AddressMode | cmd->InstructionMode |
                    cmd->Instruction | FunctionalMode));

        /* Очистка регистра AR */
        CLEAR_REG(QUADSPI->AR);
      }
    } else {
      if (cmd->AddressMode != QSPI_ADDRESS_NONE) {
        /*--- Команда с инструкцией и адресом ---*/
        /* Настройка QSPI: регистр CCR — все параметры коммуникации */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateByteMode | cmd->AddressSize | cmd->AddressMode |
                    cmd->InstructionMode | cmd->Instruction | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
          /* Настройка QSPI: регистр AR — значение адреса */
          WRITE_REG(QUADSPI->AR, cmd->Address);
        }
      } else {
        /*--- Команда только с инструкцией ---*/
        /* Настройка QSPI: регистр CCR — все параметры коммуникации */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateByteMode | cmd->AddressMode |
                    cmd->InstructionMode | cmd->Instruction | FunctionalMode));

        /* Очистка регистра AR */
        CLEAR_REG(QUADSPI->AR);
      }
    }
  } else {
    if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE) {
      /* Настройка QSPI: регистр ABR — значение дополнительных байт */
      WRITE_REG(QUADSPI->ABR, cmd->AlternateBytes);

      if (cmd->AddressMode != QSPI_ADDRESS_NONE) {
        /*--- Команда с адресом и дополнительными байтами ---*/
        /* Настройка QSPI: регистр CCR — все параметры коммуникации */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateBytesSize | cmd->AlternateByteMode |
                    cmd->AddressSize | cmd->AddressMode |
                    cmd->InstructionMode | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
          /* Настройка QSPI: регистр AR — значение адреса */
          WRITE_REG(QUADSPI->AR, cmd->Address);
        }
      } else {
        /*--- Команда только с дополнительными байтами ---*/
        /* Настройка QSPI: регистр CCR — все параметры коммуникации */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateBytesSize | cmd->AlternateByteMode |
                    cmd->AddressMode | cmd->InstructionMode | FunctionalMode));

        /* Очистка регистра AR */
        CLEAR_REG(QUADSPI->AR);
      }
    } else {
      if (cmd->AddressMode != QSPI_ADDRESS_NONE) {
        /*--- Команда только с адресом ---*/
        /* Настройка QSPI: регистр CCR — все параметры коммуникации */
        WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                    cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                    cmd->AlternateByteMode | cmd->AddressSize |
                    cmd->AddressMode | cmd->InstructionMode | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED) {
          /* Настройка QSPI: регистр AR — значение адреса */
          WRITE_REG(QUADSPI->AR, cmd->Address);
        }
      } else {
        /*--- Команда только с фазой данных ---*/
        if (cmd->DataMode != QSPI_DATA_NONE) {
          /* Настройка QSPI: регистр CCR — все параметры коммуникации */
          WRITE_REG(QUADSPI->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                      cmd->DataMode | (cmd->DummyCycles << QUADSPI_CCR_DCYC_Pos) |
                      cmd->AlternateByteMode | cmd->AddressMode |
                      cmd->InstructionMode | FunctionalMode));

          /* Очистка регистра AR */
          CLEAR_REG(QUADSPI->AR);
        }
      }
    }
  }
}


/**
  * @brief Отправить пакет данных в блокирующем режиме.
  * Количество байт определяется заранее записанным в DLR значением.
  * Используется только в режиме Indirect Write Mode.
  * @param pData указатель на буфер с данными.
  */
void QSPI_Transmit(uint8_t *pData) {
  __IO uint32_t *data_reg = &QUADSPI->DR;

  if (pData != 0) {
    // Считать количество байт из DLR
    uint32_t TxXferCount = READ_REG(QUADSPI->DLR) + 1U;
    // uint32_t TxXferSize = READ_REG(QUADSPI->DLR) + 1U;
    uint8_t *pTxBuffPtr = pData;

    // Перевести CCR в режим Indirect Write
    MODIFY_REG(QUADSPI->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

    while (TxXferCount > 0U) {
      // Ждем флаг FTF (FIFO Threshold), отправлять данные можно по мере освобождения места
      QSPI_WaitFlagSet(QUADSPI_SR_FTF);

      *((__IO uint8_t *) data_reg) = *pTxBuffPtr;
      pTxBuffPtr++;
      TxXferCount--;
    }

    // Ждем флаг TC (Transfer Complete) — возврат в idle
    QSPI_WaitFlagSet(QUADSPI_SR_TCF);
    // Сбросить флаг Transfer Complete
    // __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
    WRITE_REG(QUADSPI->FCR, QUADSPI_SR_TCF);
  }
}


/**
  * @brief Получить данные в блокирующем режиме.
  * Количество байт определяется заранее записанным в DLR значением.
  * Используется только в режиме Indirect Read Mode.
  * @param pData указатель на буфер для получения данных.
  */
void QSPI_Receive(uint8_t *pData) {
  uint32_t addr_reg = READ_REG(QUADSPI->AR);
  __IO uint32_t *data_reg = &QUADSPI->DR;

  if (pData != 0) {
    // Считать количество байт из DLR
    uint32_t RxXferCount = READ_REG(QUADSPI->DLR) + 1U;
    uint8_t *pRxBuffPtr = pData;

    // Перевести CCR в режим Indirect Read
    MODIFY_REG(QUADSPI->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_READ);

    // Перезаписав адрес в AR, инициализировать передачу
    WRITE_REG(QUADSPI->AR, addr_reg);

    while (RxXferCount > 0U) {
      // Ждем флаг FTF или TC (FIFO Threshold / Transfer Complete) — чтение принятых данных
      QSPI_WaitFlagSet(QUADSPI_SR_FTF | QUADSPI_SR_TCF);

      *pRxBuffPtr = *((__IO uint8_t *) data_reg);
      pRxBuffPtr++;
      RxXferCount--;
    }

    // Ждем флаг TC (Transfer Complete) — возврат в idle
    QSPI_WaitFlagSet(QUADSPI_SR_TCF);
    // Сбросить флаг Transfer Complete
    // __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
    WRITE_REG(QUADSPI->FCR, QUADSPI_SR_TCF);
  }
}


/**
  * @brief Настроить автоматический опрос (Automatic Polling) в блокирующем режиме.
  * QSPI автоматически читает статусные байты с заданным шагом и сравнивает их с
  * ожидаемым значением (Match & Mask). Функция завершается при совпадении.
  * Используется только в режиме Automatic Polling Mode.
  * @param cmd — указатель на структуру с параметрами команды.
  * @param cfg — указатель на структуру с параметрами опроса.
  */
void QSPI_AutoPolling(QSPI_CommandT *cmd, QSPI_AutoPollingT const *cfg) {

    QSPI_WaitFlagReset(QUADSPI_SR_BUSY);

    WRITE_REG(QUADSPI->PSMAR, cfg->Match);
    WRITE_REG(QUADSPI->PSMKR, cfg->Mask);
    WRITE_REG(QUADSPI->PIR, cfg->Interval);

    /* Настройка CR: режим сравнения и принудительная остановка
    (иначе в блокирующем режиме будет бесконечный цикл) */
    MODIFY_REG(QUADSPI->CR, (QUADSPI_CR_PMM | QUADSPI_CR_APMS), (cfg->MatchMode | QSPI_AUTOMATIC_STOP_ENABLE));

    cmd->NbData = cfg->StatusBytesSize;
    QSPI_Config(cmd, QSPI_FUNCTIONAL_MODE_AUTO_POLLING);

    QSPI_WaitFlagSet(QUADSPI_SR_SMF);

    // __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_SM);
    WRITE_REG(QUADSPI->FCR, QUADSPI_SR_SMF);
}


/**
  * @brief Записать и отправить команду в QSPI.
  * Конфигурация всегда выполняется в режиме Indirect Write;
  * если фаза данных отсутствует (DataMode == QSPI_DATA_NONE), команда
  * отправляется сразу и функция ожидает флаг TC (Transfer Complete).
  * Используется в режимах Indirect Read/Write.
  * @param cmd — указатель на структуру с параметрами команды.
  */
void QSPI_Command(QSPI_CommandT *cmd) {

    /* Ждать сброса флага BUSY */
    QSPI_WaitFlagReset(QUADSPI_SR_BUSY);// RESET

    /* Заполнить регистры конфигурации */
    QSPI_Config(cmd, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

    if (cmd->DataMode == QSPI_DATA_NONE) {
        /* Если фазы нет, передача начинается сразу после конфигурации,
        поэтому ждем флаг TC (возврат в idle) */
        QSPI_WaitFlagSet(QUADSPI_SR_TCF); // SET

        // __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
        WRITE_REG(QUADSPI->FCR, QUADSPI_SR_TCF);
    }
}

/**
  * @brief Перевести QSPI в режим Memory-Mapped.
  * После вызова флеш-память доступна чтениями через регистр DR:
  * QSPI сам шлёт команду, адрес и dummy-циклы при каждом обращении.
  * Используется только в режиме Memory mapped Mode.
  * @param cmd — указатель на структуру с параметрами команды.
  * @param cfg — указатель на структуру с настройками тайм-аута (освобождение CS).
  */
void QSPI_MemoryMapped(QSPI_CommandT *cmd, QSPI_MemoryMappedT const *cfg) {

    QSPI_WaitFlagReset(QUADSPI_SR_BUSY);

    /* Настройка CR: включить счётчик тайм-аута */
    MODIFY_REG(QUADSPI->CR, QUADSPI_CR_TCEN, cfg->TimeOutActivation);

    if (cfg->TimeOutActivation == QUADSPI_CR_TCEN) {

      /* Настройка LPTR: значение тайм-аута для режима энергосбережения */
      WRITE_REG(QUADSPI->LPTR, cfg->TimeOutPeriod);

      WRITE_REG(QUADSPI->FCR, QUADSPI_SR_TOF);
      SET_BIT(QUADSPI->CR, QUADSPI_CR_TOIE);
    }

    QSPI_Config(cmd, QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED);
}


/**
 * @brief Отправить простую команду без фазы данных (например, Write Enable, Enter QPI).
 * Инструкция передается по 1 линии (SPI).
 * @param cmd код команды (8 бит).
 */
static void QSPI_WriteCmd(const uint8_t cmd) {
    QSPI_CommandT s_command = {0};
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = cmd;
    // s_command.AddressMode       = QSPI_ADDRESS_NONE;       // адрес не передаётся
    // s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; // доп. байты не используются
    // s_command.DataMode          = QSPI_DATA_NONE;          // фаза данных отсутствует
    // s_command.DummyCycles       = 0;
    // s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    // s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    QSPI_Command(&s_command);
}

/**
 * @brief Отправить простую команду без фазы данных в режиме 4 линий (QPI).
 * Инструкция передается по 4 линиям.
 * @param cmd код команды (8 бит).
 */
static void QSPI_WriteCmd4L(const uint8_t cmd) {
    QSPI_CommandT s_command = {0};
    s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction = cmd;

    QSPI_Command(&s_command);
}

/**
 * @brief Отладочная статика: значения статуса вокруг установки QE (см. QSPI_Enable_QPI_Mode).
 */
static uint8_t stat1 = 0; // SR2, прочитанный до установки QE
static uint8_t stat2 = 0; // SR2, подготовленный к записи (с битом QE)
static uint8_t stat3 = 0; // SR2, прочитанный сразу после записи
static uint8_t stat4 = 0; // SR2, прочитанный после задержки

/**
 * @brief Включить режим QPI (Quad I/O) во флеш-памяти Winbond W25Qxx.
 * Если флеш уже работает в режиме QPI (Manufacturer ID = 0xEF, прочитанный
 * по 4 линиям) — ничего не делает. Иначе: при необходимости устанавливает
 * бит QE (Quad Enable) в Status Register 2 и отправляет CMD_ENTER_QPI (0x38).
 * @note После входа в QPI все команды должны идти по 4 линиям.
 */
void QSPI_Enable_QPI_Mode(void) {
    QSPI_CommandT s_command = {0};
    uint8_t reg2 = 0;

    // QSPI_WriteCmd4L(CMD_EXIT_QPI);
    // Запросить Manufacturer ID по 4 линиям
    QSPI_Read_Manf_Device_ID(1);
    if (manId.manId == WINBOND_CODE) {
        return; // QPI уже активен (Winbond отвечает 0xEF)
    }

    // Читаем Status Register 2 по 1 линии, чтобы узнать состояние бита QE
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

    // Если бит QE (в SR2 у Winbond — бит 1) не установлен — устанавливаем его
    if ((reg2 & 0x02) == 0) {
        stat2 = 0x51;
        // Перед записью в регистр статуса ОБЯЗАТЕЛЬНО шлем Write Enable
        QSPI_WriteCmd(CMD_WRITE_ENABLE);

        reg2 |= 0x02; // Устанавливаем бит QE

        // Команда записи в Status Register 2
        s_command.Instruction = CMD_WRITE_STATUS_REG2;
        QSPI_Command(&s_command);
        QSPI_Transmit(&reg2);

        // Читаем Status Register 2
        s_command.Instruction = CMD_READ_STATUS_REG2;
        QSPI_Command(&s_command);
        QSPI_Receive(&stat3);

        // Ждем завершения внутренней записи (бит BUSY в SR1)
        LL_mDelay(1);

        // Читаем Status Register 2 повторно (проверка результата)
        s_command.Instruction = CMD_READ_STATUS_REG2;
        QSPI_Command(&s_command);
        QSPI_Receive(&stat4);
    }

    // Отправляем команду перевода Flash в режим QPI (команда еще по 1 линии)
    QSPI_WriteCmd(CMD_ENTER_QPI);
}


/**
 * @brief Прочитать JEDEC ID флеш-памяти (команда 0x9F).
 * ID читается по 4 линиям и сохраняется в статическую структуру jedecId.
 */
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
 * @brief Прочитать Manufacturer/Device ID флеш-памяти (команда 0x90).
 * Команда 0x90 требует 24-битный адресный цикл (0x00).
 * При Winbond в режиме QPI мануфактурный код в первом байте = 0xEF.
 * @param linesNum 0 — по 1 линии (SPI), 1 — по 4 линиям (QPI).
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
 * @brief Прочитать три регистра статуса Winbond W25Qxx.
 * Регистры читаются последовательно: 0x05 (SR1), 0x35 (SR2), 0x15 (SR3).
 * @param linesNum 0 — по 1 линии (SPI), 1 — по 4 линиям (QPI).
 * @return Сводное 32-битное значение статусов (SR1 | SR2<<8 | SR3<<16):
 *         SR1: S7 SRP, S6 SEC, S5 TB, S4 BP2, S3 BP1, S2 BP0, S1 WEL, S0 BUSY
 *         SR2: S15 SUS, S14 CMP, S13 LB3, S12 LB2, S11 LB1, S10 R, S9 QE, S8 SRL
 *         SR3: S23 HOLD/RST, S22 DRV1, S21 DRV0, S20 R, S19 R, S18 WPS, S17 R, S16 R
 */
uint32_t QSPI_GetStatus(uint8_t linesNum) {
  QSPI_CommandT s_command = {0};
  uint8_t reg = 0;

  // 25Q64JVIQ — JEDEC ID: 0x00 0x40 0x18
  // Настройка команды чтения status register (0x05)
  s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction = CMD_READ_STATUS_REG1; // Чтение SR1
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

  // Отправка команды
  QSPI_Command(&s_command);

  // Чтение SR1
  QSPI_Receive(&reg);
  statRegs = reg;

  s_command.Instruction = CMD_READ_STATUS_REG2; // Чтение SR2
  // Отправка команды
  QSPI_Command(&s_command);

  // Прочтение SR2
  QSPI_Receive(&reg);
  statRegs |= reg << 8;

  s_command.Instruction = CMD_READ_STATUS_REG3; // Чтение SR3
  // Отправка команды
  QSPI_Command(&s_command);

  // Прочтение SR3
  QSPI_Receive(&reg);

  statRegs |= reg << 16;

  return statRegs;
}

/**
  * @brief Чтение блока данных из QSPI Flash (Winbond).
  * Используется команда Fast Read Quad I/O (0x0B): инструкция, адрес (24 бита)
  * и данные — все по 4 линиям.
  * @param address — внутренний адрес во флеш-памяти (например, 0x000000).
  * @param pBuffer — указатель на буфер в RAM для сохранения данных.
  * @param size    — количество байт для чтения.
  */
void QSPI_ReadBlock(uint32_t address, uint8_t *pBuffer, uint32_t size) {

    QSPI_CommandT sCommand = {0};

    // Настройка параметров команды чтения
    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;   // Команда отправляется по 4 линиям, QPI mode
    sCommand.Instruction       = CMD_FAST_READ;              // Команда Fast Read Quad I/O (0x0B), QPI mode
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;       // Адрес отправляется по 4 линиям, QPI mode
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;       // Для чипов до 128 Мбит (16 МБ) используется 24-битный адрес
    sCommand.Address           = address;                    // Целевой адрес во флеш-памяти
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  // Альтернативные байты не используются
    sCommand.DummyCycles       = 2;                          // 2 dummy-цикла (в QPI зависит от Read Parameter Bits P[5:4])
    sCommand.DataMode          = QSPI_DATA_4_LINES;          // Данные принимаются по 4 линиям, QPI mode
    sCommand.NbData            = size;                       // Размер считываемого блока в байтах
    // sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      // Выключаем Double Data Rate (чтение по переднему фронту)
    // sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   // Отправлять инструкцию при каждом вызове

    // Отправка команды и инициация приема данных
    QSPI_Command(&sCommand);

    // Чтение данных из QSPI в буфер RAM
    QSPI_Receive(pBuffer);
}


/**
 * @brief Ожидание завершения внутренней операции во флеш-памяти.
 * Автоматический опрос SR1: ждем, пока бит BUSY (S0) не станет 0.
 */
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
  s_config.Interval        = 0x10; // Шаг опроса в тактах
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  QSPI_AutoPolling(&s_command, &s_config);
}

/**
 * @brief Стирание блока 64 КБ (или сектора 4 КБ).
 * Сейчас используется команда Sector Erase 4 КБ (0x20);
 * доступны также CMD_BLOCK_ERASE_32K (0x52) и CMD_BLOCK_ERASE_64K (0xD8).
 * Перед стиранием автоматически отправляется Write Enable.
 * @param BlockAddress — стартовый адрес стираемого блока (должен быть выровнен по его размеру).
 */
void QSPI_EraseBlock(uint32_t BlockAddress) {
  QSPI_CommandT s_command = {0};

  QSPI_WriteCmd4L(CMD_WRITE_ENABLE);

  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = CMD_SECTOR_ERASE_4K; // Можно использовать: CMD_SECTOR_ERASE_4K, CMD_BLOCK_ERASE_32K, CMD_BLOCK_ERASE_64K
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

uint32_t regStat1 = 0; // SR1 до Write Enable (отладка)
uint32_t regStat2 = 0; // SR1 после Write Enable (отладка)
uint32_t regStat3 = 0; // SR1 после передачи данных (отладка)
uint32_t regStat4 = 0; // SR1 после завершения записи (отладка)

/**
 * @brief Записать страницу (до 256 байт) во флеш-память.
 * Режим QPI: инструкция, адрес (24 бита) и данные — все по 4 линиям.
 * Перед записью автоматически отправляется Write Enable, после передачи
 * функция ждет завершения внутренней операции (обычно 3-5 мс).
 * @param PageAddress — стартовый адрес страницы (должен быть выровнен по 256 байт).
 * @param pData       — указатель на буфер с данными.
 * @param Size        — количество байт (максимум 256). При превышении вызывается Error_Handler().
 */
void QSPI_WritePage(uint32_t PageAddress, uint8_t* pData, uint32_t Size) {
    QSPI_CommandT s_command = {0};

    if (Size > 256) Error_Handler(); // Максимальный размер страницы Winbond
    regStat1 = QSPI_GetStatus(1);
    QSPI_WriteCmd4L(CMD_WRITE_ENABLE);
    regStat2 = QSPI_GetStatus(1);

    // QPI mode (4-4-4: инструкция x4, адрес x4, данные x4)
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


/**
 * @brief Включить режим Memory-Mapped для флеш-памяти.
 * Настройка команды чтения Fast Read Quad I/O (0x0B) с 24-битным адресом
 * и 2 dummy-циклами; после включения периферия сама шлет команду и адрес
 * при каждом обращении к DR (см. QSPI_MemoryMapped).
 * @note Таймаут освобождения CS настраивается через QSPI_MemoryMappedT, но сейчас не включен.
 */
void QSPI_EnableMemoryMappedMode(void) {
    QSPI_CommandT s_command = {0};
    QSPI_MemoryMappedT s_mem_mapped_cfg = {0};

    // Настройка команды чтения Flash-памяти
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;    // Команда шлется по 4 линиям, QPI mode
    s_command.Instruction       = CMD_FAST_READ;               // Fast Read Quad I/O (0x0B), QPI mode
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;        // Адрес передается по 4 линиям, QPI mode
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;        // 3-байтовый адрес (для памяти >16МБ нужно QSPI_ADDRESS_32_BITS)
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
