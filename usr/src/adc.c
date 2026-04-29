#include <_main.h>
#include <dwt.h>
#include <DataBuffer.h>
#include "adc.h"

// ADC clock freq (Hz)
#define ADC_CLOCK 68000000.f
// RM0440 page 633
// 8 bit. TSAR timings depending on resolution
#define CONV_TICS 8.5f

// Recommended ADC clock = 60mHz Max = 170mHz :)
struct ADC_param {
    uint32_t ADC_Prescaler;
    uint32_t ADC_SampleCycles;
    float SampleTime;    // microseconds
};
typedef struct ADC_param ADC_PARAM;

#define ADC_Parameters_Size 6
const ADC_PARAM ADC_Parameters[ADC_Parameters_Size] = {
        {LL_ADC_CLOCK_ASYNC_DIV1,  LL_ADC_SAMPLINGTIME_2CYCLES_5,  0.f},
        {LL_ADC_CLOCK_ASYNC_DIV1,  LL_ADC_SAMPLINGTIME_6CYCLES_5,  0.f},
        {LL_ADC_CLOCK_ASYNC_DIV2,  LL_ADC_SAMPLINGTIME_2CYCLES_5,  0.f},
        {LL_ADC_CLOCK_ASYNC_DIV1,  LL_ADC_SAMPLINGTIME_12CYCLES_5, 0.f},
        {LL_ADC_CLOCK_ASYNC_DIV2,  LL_ADC_SAMPLINGTIME_6CYCLES_5,  0.f},
        {LL_ADC_CLOCK_ASYNC_DIV4,  LL_ADC_SAMPLINGTIME_2CYCLES_5,  0.f}
};

uint32_t ADC_Prescaler = LL_ADC_CLOCK_ASYNC_DIV2;
uint32_t ADC_SampleTime = LL_ADC_SAMPLINGTIME_2CYCLES_5;
float    ADC_MeasureTime = 0;
uint8_t  ADC_param = 3;

uint16_t ScreenTime = 0;      // index in ScreenTimes
uint16_t ScreenTime_adj = 0;  // 0-9 shift in ScreenTime
const float ScreenTimes[] = {100, 200, 500, 1000, 2000, 5000, 10000, 20000};  // sweep screen, microseconds

uint8_t  ADCworks;
uint32_t ADCStartTick;         // time when start ADC buffer fill
uint32_t ADCHalfElapsedTick;   // the last time half buffer fill
uint32_t ADCElapsedTick;       // the last time buffer fill

static void ADC2_Init(void);
float ADC_calcSampleTime();


void ADC_start() {

    if (ADCworks != 0) {
        return;
    }
    ADCworks = 1;

    ADC2_Init();

    LL_ADC_ClearFlag_ADRDY(ADC2);
    LL_ADC_Enable(ADC2);
    while (!LL_ADC_IsActiveFlag_ADRDY(ADC2)) {}
    LL_ADC_ClearFlag_ADRDY(ADC2);

    // Set DMA transfer addresses of source and destination
    LL_DMA_ConfigAddresses(DMA2, LL_DMA_CHANNEL_1,
                           (uint32_t) &(ADC2->DR),
                           (uint32_t)&samplesBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    // Set DMA transfer size
    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, BUF_SIZE);
    // Enable DMA transfer interruption: transfer error
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);

    LL_ADC_REG_StartConversion(ADC2);

    ADCStartTick = DWT_Get_Current_Tick();
}


static void ADC2_Init(void) {

  if (LL_ADC_IsEnabled(ADC2)) {
      LL_ADC_REG_StopConversion(ADC2);
      while (LL_ADC_REG_IsConversionOngoing(ADC2)) {}

      LL_ADC_Disable(ADC2);
      while (LL_ADC_IsDisableOngoing(ADC2)) {}
      while (LL_ADC_IsEnabled(ADC2)) {}

      LL_ADC_DisableInternalRegulator(ADC2);
      LL_mDelay(1);
      LL_ADC_EnableDeepPowerDown(ADC2);
      LL_mDelay(1);

      LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
      LL_mDelay(1);
  }

  // ADC2 DMA Init
  LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_ADC2);
  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetChannelPriorityLevel(DMA2, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_VERYHIGH);
  LL_DMA_SetMode(DMA2, LL_DMA_CHANNEL_1, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA2, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA2, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_BYTE);

  MODIFY_REG(ADC2->CFGR, ADC_CFGR_RES | ADC_CFGR_ALIGN | ADC_CFGR_AUTDLY,
             LL_ADC_RESOLUTION_8B | LL_ADC_DATA_ALIGN_RIGHT | LL_ADC_LP_MODE_NONE);
  // Common config
  // MODIFY_REG(ADC12_COMMON->CCR,
  //            ADC_CCR_CKMODE | ADC_CCR_PRESC | ADC_CCR_DUAL | ADC_CCR_MDMA | ADC_CCR_DELAY,
  //            ADC_Prescaler | LL_ADC_MULTI_INDEPENDENT);
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
  ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_PRESERVED;
  LL_ADC_REG_Init(ADC2, &ADC_REG_InitStruct);
  LL_ADC_SetGainCompensation(ADC2, 0);
  LL_ADC_SetOverSamplingScope(ADC2, LL_ADC_OVS_DISABLE);
  ADC_CommonInitStruct.CommonClock = ADC_Prescaler;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC2), &ADC_CommonInitStruct);

  /* Disable ADC deep power down (enabled by default after reset state) */
  LL_ADC_DisableDeepPowerDown(ADC2);
  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC2);
  /* Delay for ADC internal voltage regulator stabilization. */
  /* Compute number of CPU cycles to wait for, from delay in us. */
  /* Note: Variable divided by 2 to compensate partially */
  /* CPU processing cycles (depends on compilation optimization). */
  /* Note: If system core clock frequency is below 200kHz, wait time */
  /* is only a few CPU processing cycles. */
  uint32_t wait_loop_index;
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
      wait_loop_index--;
  }

  // Configure Regular Channel
  LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_3);
  LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_CHANNEL_3, ADC_SampleTime);
  LL_ADC_SetChannelSingleDiff(ADC2, LL_ADC_CHANNEL_3, LL_ADC_SINGLE_ENDED);
}


void ADC_step(int16_t step) {
    if (step == 0) return;
    if (step > 0) {
        if (ADC_param < ADC_Parameters_Size-1) ADC_param++;
    } else {
        if (ADC_param > 0) ADC_param--;
    }
    ADC_Prescaler = ADC_Parameters[ADC_param].ADC_Prescaler;
    ADC_SampleTime = ADC_Parameters[ADC_param].ADC_SampleCycles;
    ADC_MeasureTime = ADC_calcSampleTime();
}

// return time for 1 measuring. (ns)
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

    return (CONV_TICS + sampling) * 1/(ADC_CLOCK/presc /1000000.f) * 1000.f /2.f ;
}
