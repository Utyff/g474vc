#include <_main.h>
#include <stm32g4xx_hal.h>


/**
 * @brief Инициализация интерфейса FMC (Flexible Managed Controller) для работы
 * с внешним дисплеем через SRAM-подобный интерфейс.
 *
 * Настраивает банк NORSRAM1 контроллера FMC для 16-битного параллельного
 * подключения TFT-дисплея. Использует timings для синхронизации записи
 * без ожидания готовности (асинхронный режим, режим доступа A).
 *
 * Физическое подключение:
 *   D0..D15  → PD0-1, PD8-15, PE7-15  (данные дисплея)
 *   FMC_NE1  → PD7                    (CS дисплея)
 *   FMC_NWE  → PD5                    (WR / Write Enable)
 *   FMC_NOE  → PD4                    (RD / Read Enable)
 *   FMC_A18  → PD13                   (RS / DC — Register Select / Data Command)
 */
void FMC_Init(void) {
    SRAM_HandleTypeDef hsram1;
    FMC_NORSRAM_TimingTypeDef Timing = {0};

    // Включение тактирования блока FMC (аналог __HAL_RCC_FMC_CLK_ENABLE())
    SET_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN);

    // --- Настройка структуры SRAM-устройства ---
    hsram1.Instance = FMC_NORSRAM_DEVICE;
    hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;

    // Инициализация параметров банка NORSRAM
    hsram1.Init.NSBank = FMC_NORSRAM_BANK1;                        // Банк 1 (NOR/SRAM Bank 1 → NE1)
    hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;     // Без мультиплексирования данных/адреса
    hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;                 // Тип памяти — SRAM (подходит и для LCD)
    hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;    // 16-битная шина данных
    hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;   // Без burst-доступа
    hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW; // Полярность сигнала ожидания
    hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;      // Время ожидания до начала передачи
    hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;       // Разрешить запись
    hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;              // Сигнал ожидания отключён
    hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;          // Без расширенного режима (读写同参数)
    hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;  // Асинхронное ожидание отключено
    hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;              // Burst-запись отключена
    hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;  // Непрерывное тактирование только в синхронном режиме
    hsram1.Init.WriteFifo = FMC_WRITE_FIFO_DISABLE;                // Write FIFO отключено
    hsram1.Init.NBLSetupTime = 0;                                  // NBL0/NBL1 setup time
    hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;                     // Страницы не используются
    hsram1.Init.MaxChipSelectPulse = DISABLE;                      // Импульс CS не генерируется

    // --- Настройка таймингов доступа (режим A — без ожидания) ---
    Timing.AddressSetupTime = 5;           // 5 тактов FMC между CS и адресом/данными
    Timing.AddressHoldTime = 15;           // 15 тактов — адрес удерживается (не используется в режиме A)
    Timing.DataSetupTime = 7;              // 7 тактов FMC между CS и данными (для записи)
    Timing.DataHoldTime = 0;               // Данные не удерживаются после конца передачи
    Timing.BusTurnAroundDuration = 15;     // 15 тактов — разворот шины (не используется)
    Timing.CLKDivision = 16;               // Делитель тактирования (не используется в асинхронном режиме)
    Timing.DataLatency = 17;               // Задержка данных (не используется)
    Timing.AccessMode = FMC_ACCESS_MODE_A; // Режим доступа A: данные сразу вместе с адресом

    // --- Инициализация интерфейса ---
    // Запись параметров управления в регистры FMC_NORSRAMCR
    FMC_NORSRAM_Init(FMC_NORSRAM_DEVICE, &hsram1.Init);
    // Запись таймингов в регистр FMC_NORSRAMTimingCR
    FMC_NORSRAM_Timing_Init(FMC_NORSRAM_DEVICE, &Timing, FMC_NORSRAM_BANK1);
    // Включение банка NORSRAM1 (установка бита EN в FMC_NORSRAMB1_CR)
    __FMC_NORSRAM_ENABLE(FMC_NORSRAM_DEVICE, FMC_NORSRAM_BANK1);
}
