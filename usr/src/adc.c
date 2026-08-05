#include <_main.h>
#include <dwt.h>
#include <DataBuffer.h>
#include "adc.h"

// ADC clock freq (Hz)
// ADC clock: DS recommended - 60 MHz, works MAX - 113.3 MHz
#define ADC_CLOCK 113333333.f
// RM0440 page 633
// 8 bit. TSAR timings depending on resolution
#define CONV_TICS 8.5f

typedef struct {
    uint32_t ADC_Prescaler;
    uint32_t ADC_SampleCycles;
} ADC_PARAM;

#define ADC_Parameters_Size 63
static const ADC_PARAM ADC_Parameters[ADC_Parameters_Size] = {
    {LL_ADC_CLOCK_ASYNC_DIV1, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV1, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV1, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV2, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV2, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV1, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV2, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV4, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV1, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV4, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV2, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV6, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV4, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV8, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV6, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV1, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV10, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV2, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV8, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV6, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV4, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV12, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV10, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV8, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV16, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV12, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV6, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV2, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV10, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV4, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV16, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV12, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV1, LL_ADC_SAMPLINGTIME_247CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV8, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV10, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV16, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV6, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV32, LL_ADC_SAMPLINGTIME_2CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV12, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV4, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV8, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV32, LL_ADC_SAMPLINGTIME_6CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV2, LL_ADC_SAMPLINGTIME_247CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV16, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV10, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV6, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV32, LL_ADC_SAMPLINGTIME_12CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV12, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV8, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV16, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV10, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV4, LL_ADC_SAMPLINGTIME_247CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV32, LL_ADC_SAMPLINGTIME_24CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV12, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV6, LL_ADC_SAMPLINGTIME_247CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV16, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV32, LL_ADC_SAMPLINGTIME_47CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV8, LL_ADC_SAMPLINGTIME_247CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV10, LL_ADC_SAMPLINGTIME_247CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV12, LL_ADC_SAMPLINGTIME_247CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV32, LL_ADC_SAMPLINGTIME_92CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV16, LL_ADC_SAMPLINGTIME_247CYCLES_5},
    {LL_ADC_CLOCK_ASYNC_DIV32, LL_ADC_SAMPLINGTIME_247CYCLES_5}
};

typedef struct {
    ADC_TypeDef *master;
    uint32_t msChannel;
    ADC_TypeDef *slave;
    uint32_t slChannel;
    DMA_TypeDef *dma;
    uint32_t dmaChannel;
    uint8_t *buffer;
} ChannelParam;

static const ChannelParam chParams[2] = {
    {.master = ADC1, .msChannel = LL_ADC_CHANNEL_1, .slave = ADC2, .slChannel = LL_ADC_CHANNEL_3, .dma = DMA2, .dmaChannel = LL_DMA_CHANNEL_1, .buffer = samplesBuffer},
    {.master = ADC3, .msChannel = LL_ADC_CHANNEL_1, .slave = ADC4, .slChannel = LL_ADC_CHANNEL_3, .dma = DMA1, .dmaChannel = LL_DMA_CHANNEL_3, .buffer = samplesBuffer1}
};

uint8_t ADC_param = 2;
static uint32_t ADC_Prescaler = LL_ADC_CLOCK_ASYNC_DIV2;
static uint32_t ADC_SampleTime = LL_ADC_SAMPLINGTIME_2CYCLES_5;
float ADC_MeasureTime = 0;

uint16_t ScreenTime = 0; // index in ScreenTimes
uint16_t ScreenTime_adj = 0; // 0-9 shift in ScreenTime
const float ScreenTimes[] = {100, 200, 500, 1000, 2000, 5000, 10000, 20000}; // sweep screen, microseconds

uint8_t ADCworks;
uint32_t ADCStartTick; // time when start ADC buffer fill
uint32_t ADCHalfElapsedTick; // the last time half buffer fill
uint32_t ADCElapsedTick; // the last time buffer fill

static void initCH(uint8_t);
static void startCH(uint8_t);
static void stopCH(uint8_t);
static void initMaster(uint8_t);
static void initSlave(uint8_t);
static float ADC_calcSampleTime();


void ADC_start() {
    if (ADCworks != 0) {
        return;
    }
    ADCworks = 1;

    ADC_Prescaler = ADC_Parameters[ADC_param].ADC_Prescaler;
    ADC_SampleTime = ADC_Parameters[ADC_param].ADC_SampleCycles;
    ADC_MeasureTime = ADC_calcSampleTime();

    stopCH(0);
    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
    DWT_Delay_tics(10);

    initCH(0);
    startCH(0);

    stopCH(1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    DWT_Delay_tics(10);

    initCH(1);
    startCH(1);

    ADCStartTick = DWT_Get_Current_Tick();
}

static void initCH(const uint8_t ch) {
    initMaster(ch);
    initSlave(ch);
}

static void startCH(const uint8_t ch) {
    LL_ADC_ClearFlag_ADRDY(chParams[ch].slave);
    LL_ADC_Enable(chParams[ch].slave);
    LL_ADC_ClearFlag_ADRDY(chParams[ch].master);
    LL_ADC_Enable(chParams[ch].master);
    while (!LL_ADC_IsActiveFlag_ADRDY(chParams[ch].slave)) {}
    LL_ADC_ClearFlag_ADRDY(chParams[ch].slave);
    while (!LL_ADC_IsActiveFlag_ADRDY(chParams[ch].master)) {}
    LL_ADC_ClearFlag_ADRDY(chParams[ch].master);
    LL_ADC_ClearFlag_EOSMP(chParams[ch].master);
    LL_ADC_ClearFlag_OVR(chParams[ch].master);

    // Set DMA transfer addresses of source and destination
    LL_DMA_ConfigAddresses(chParams[ch].dma, chParams[ch].dmaChannel,
                           (uint32_t) &(__LL_ADC_COMMON_INSTANCE(chParams[ch].master)->CDR),
                           (uint32_t) chParams[ch].buffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    // Set DMA transfer size
    LL_DMA_SetDataLength(chParams[ch].dma, chParams[ch].dmaChannel, BUF_SIZE / 2);
    // Enable DMA transfer interruption: transfer complete & error
    LL_DMA_ClearFlag_TC3(DMA1);
    LL_DMA_ClearFlag_HT3(DMA1);
    LL_DMA_ClearFlag_TE3(DMA1);
    LL_DMA_EnableIT_TC(chParams[ch].dma, chParams[ch].dmaChannel);
    LL_DMA_EnableIT_TE(chParams[ch].dma, chParams[ch].dmaChannel);
    LL_DMA_EnableChannel(chParams[ch].dma, chParams[ch].dmaChannel);

    LL_ADC_REG_StartConversion(chParams[ch].master);
}

static void stopCH(const uint8_t ch) {
    LL_ADC_REG_StopConversion(chParams[ch].master);
    while (LL_ADC_REG_IsConversionOngoing(chParams[ch].master)) {}
    LL_ADC_ClearFlag_EOS(chParams[ch].master);
    LL_ADC_ClearFlag_EOSMP(chParams[ch].master);
    LL_ADC_ClearFlag_OVR(chParams[ch].master);

    if (LL_ADC_IsEnabled(chParams[ch].master)) {
        LL_ADC_Disable(chParams[ch].master);
        while (LL_ADC_IsEnabled(chParams[ch].master)) {}
    }

    if (LL_ADC_IsEnabled(chParams[ch].slave)) {
        LL_ADC_Disable(chParams[ch].slave);
        while (LL_ADC_IsEnabled(chParams[ch].slave)) {}
    }
}

#define DMAMUX_REQ(__ADCx__)  (((__ADCx__) == ADC1) ? (LL_DMAMUX_REQ_ADC1) : (LL_DMAMUX_REQ_ADC3))

static void initMaster(const uint8_t ch) {
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
    LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

    // DMA Init
    LL_DMA_SetPeriphRequest(chParams[ch].dma, chParams[ch].dmaChannel, DMAMUX_REQ(chParams[ch].master));
    LL_DMA_SetDataTransferDirection(chParams[ch].dma, chParams[ch].dmaChannel, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetChannelPriorityLevel(chParams[ch].dma, chParams[ch].dmaChannel, LL_DMA_PRIORITY_VERYHIGH);
    LL_DMA_SetMode(chParams[ch].dma, chParams[ch].dmaChannel, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(chParams[ch].dma, chParams[ch].dmaChannel, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(chParams[ch].dma, chParams[ch].dmaChannel, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(chParams[ch].dma, chParams[ch].dmaChannel, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(chParams[ch].dma, chParams[ch].dmaChannel, LL_DMA_MDATAALIGN_HALFWORD);

    MODIFY_REG(chParams[ch].master->CFGR, ADC_CFGR_RES | ADC_CFGR_ALIGN | ADC_CFGR_AUTDLY,
               LL_ADC_RESOLUTION_8B | LL_ADC_DATA_ALIGN_RIGHT | LL_ADC_LP_MODE_NONE);

    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
    ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_LIMITED;
    ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_PRESERVED;
    LL_ADC_REG_Init(chParams[ch].master, &ADC_REG_InitStruct);
    LL_ADC_SetGainCompensation(chParams[ch].master, 0);
    LL_ADC_SetOverSamplingScope(chParams[ch].master, LL_ADC_OVS_DISABLE);

    // Common config
    ADC_CommonInitStruct.CommonClock = ADC_Prescaler;
    ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_DUAL_REG_INTERL;
    ADC_CommonInitStruct.MultiDMATransfer = LL_ADC_MULTI_REG_DMA_UNLMT_RES8_6B;
    ADC_CommonInitStruct.MultiTwoSamplingDelay = LL_ADC_MULTI_TWOSMP_DELAY_2CYCLES; // TODO
    LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(chParams[ch].master), &ADC_CommonInitStruct);
    CLEAR_BIT(__LL_ADC_COMMON_INSTANCE(chParams[ch].master)->CCR, ADC_CCR_DMACFG); // cube ll bug

    // Configure Regular Channel
    LL_ADC_REG_SetSequencerRanks(chParams[ch].master, LL_ADC_REG_RANK_1, chParams[ch].msChannel);
    LL_ADC_SetChannelSamplingTime(chParams[ch].master, chParams[ch].msChannel, ADC_SampleTime);
    LL_ADC_SetChannelSingleDiff(chParams[ch].master, chParams[ch].msChannel, LL_ADC_SINGLE_ENDED);
}

static void initSlave(const uint8_t ch) {
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};

    MODIFY_REG(chParams[ch].slave->CFGR, ADC_CFGR_RES | ADC_CFGR_ALIGN | ADC_CFGR_AUTDLY,
               LL_ADC_RESOLUTION_8B | LL_ADC_DATA_ALIGN_RIGHT | LL_ADC_LP_MODE_NONE);

    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
    ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
    ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_PRESERVED;
    LL_ADC_REG_Init(chParams[ch].slave, &ADC_REG_InitStruct);
    LL_ADC_SetGainCompensation(chParams[ch].slave, 0);
    LL_ADC_SetOverSamplingScope(chParams[ch].slave, LL_ADC_OVS_DISABLE);

    // Configure Regular Channel
    LL_ADC_REG_SetSequencerRanks(chParams[ch].slave, LL_ADC_REG_RANK_1, chParams[ch].slChannel);
    LL_ADC_SetChannelSamplingTime(chParams[ch].slave, chParams[ch].slChannel, ADC_SampleTime);
    LL_ADC_SetChannelSingleDiff(chParams[ch].slave, chParams[ch].slChannel, LL_ADC_SINGLE_ENDED);
}


void ADC_step(const int16_t step) {
    if (step == 0) return;
    if (step > 0) {
        if (ADC_param < ADC_Parameters_Size - 1) ADC_param++;
    } else {
        if (ADC_param > 0) ADC_param--;
    }
}

// Sample time (ns)
float ADC_calcSampleTime() {
    float presc = 0;
    float sampling = 0;

    switch (ADC_Prescaler) {
        case LL_ADC_CLOCK_ASYNC_DIV1:
            presc = 1;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV2:
            presc = 2;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV4:
            presc = 4;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV6:
            presc = 6;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV8:
            presc = 8;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV10:
            presc = 10;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV12:
            presc = 12;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV16:
            presc = 16;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV32:
            presc = 32;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV64:
            presc = 64;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV128:
            presc = 128;
            break;
        case LL_ADC_CLOCK_ASYNC_DIV256:
            presc = 256;
            break;
        default:
            Error_Handler();
    }

    switch (ADC_SampleTime) {
        case LL_ADC_SAMPLINGTIME_2CYCLES_5:
            sampling = 2.5f;
            break;
        case LL_ADC_SAMPLINGTIME_6CYCLES_5:
            sampling = 6.5f;
            break;
        case LL_ADC_SAMPLINGTIME_12CYCLES_5:
            sampling = 12.5f;
            break;
        case LL_ADC_SAMPLINGTIME_24CYCLES_5:
            sampling = 24.5f;
            break;
        case LL_ADC_SAMPLINGTIME_47CYCLES_5:
            sampling = 47.5f;
            break;
        case LL_ADC_SAMPLINGTIME_92CYCLES_5:
            sampling = 92.5f;
            break;
        case LL_ADC_SAMPLINGTIME_247CYCLES_5:
            sampling = 247.5f;
            break;
        case LL_ADC_SAMPLINGTIME_640CYCLES_5:
            sampling = 640.5f;
            break;
        default:
            Error_Handler();
    }

    // Sample time (ns). Interleave mode.
    return (CONV_TICS + sampling) *1000000000.f / 2 / (ADC_CLOCK / presc);
}
