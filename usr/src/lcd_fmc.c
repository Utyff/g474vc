#include <_main.h>
#include <stm32g4xx_hal.h>


void FMC_Init(void) {
    SRAM_HandleTypeDef hsram1;
    FMC_NORSRAM_TimingTypeDef Timing = {0};

    // __HAL_RCC_FMC_CLK_ENABLE()
    SET_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN);

    // Perform the SRAM1 memory initialization sequence
    hsram1.Instance = FMC_NORSRAM_DEVICE;
    hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    // hsram1.Init
    hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
    hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram1.Init.WriteFifo = FMC_WRITE_FIFO_DISABLE;
    hsram1.Init.NBLSetupTime = 0;
    hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
    hsram1.Init.MaxChipSelectPulse = DISABLE;
    // Timing
    Timing.AddressSetupTime = 5;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = 7;
    Timing.DataHoldTime = 0;
    Timing.BusTurnAroundDuration = 15;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;

    // Initialize SRAM control Interface
    (void) FMC_NORSRAM_Init(FMC_NORSRAM_DEVICE, &hsram1.Init);
    // Initialize SRAM timing Interface
    (void) FMC_NORSRAM_Timing_Init(FMC_NORSRAM_DEVICE, &Timing, FMC_NORSRAM_BANK1);
    // Enable the NORSRAM device
    __FMC_NORSRAM_ENABLE(FMC_NORSRAM_DEVICE, FMC_NORSRAM_BANK1);
}
