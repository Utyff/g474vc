#include <_main.h>
#include <dwt.h>
#include <DataBuffer.h>
#include "adc.h"

// Recommended ADC clock = 60mHz Max = 170mHz :)
struct ADC_param {
    uint32_t ADC_Prescaler;
    uint32_t ADC_SampleCycles;
    float SampleTime;    // microseconds
};
typedef struct ADC_param ADC_PARAM;

static void ADC1_Init(void);
static void ADC2_Init(void);

#define ADC_Parameters_Size  63
const ADC_PARAM ADC_Parameters[ADC_Parameters_Size] = {
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_2CYCLES_5,  0.06470588f},
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_6CYCLES_5,  0.08823529f},
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_12CYCLES_5,  0.12352941f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_2CYCLES_5,  0.12941176f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_6CYCLES_5,  0.17647059f},
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_24CYCLES_5,  0.19411765f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_12CYCLES_5,  0.24705882f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_2CYCLES_5,  0.25882353f},
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_47CYCLES_5,  0.32941176f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_6CYCLES_5,  0.35294118f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_24CYCLES_5,  0.38823529f},
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_2CYCLES_5,  0.38823529f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_12CYCLES_5,  0.49411765f},
        {ADC_CLOCK_ASYNC_DIV8, ADC_SAMPLETIME_2CYCLES_5,  0.51764706f},
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_6CYCLES_5,  0.52941176f},
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_92CYCLES_5,  0.59411765f},
        {ADC_CLOCK_ASYNC_DIV10, ADC_SAMPLETIME_2CYCLES_5,  0.64705882f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_47CYCLES_5,  0.65882353f},
        {ADC_CLOCK_ASYNC_DIV8, ADC_SAMPLETIME_6CYCLES_5,  0.70588235f},
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_12CYCLES_5,  0.74117647f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_24CYCLES_5,  0.77647059f},
        {ADC_CLOCK_ASYNC_DIV12, ADC_SAMPLETIME_2CYCLES_5,  0.77647059f},
        {ADC_CLOCK_ASYNC_DIV10, ADC_SAMPLETIME_6CYCLES_5,  0.88235294f},
        {ADC_CLOCK_ASYNC_DIV8, ADC_SAMPLETIME_12CYCLES_5,  0.98823529f},
        {ADC_CLOCK_ASYNC_DIV16, ADC_SAMPLETIME_2CYCLES_5,  1.03529412f},
        {ADC_CLOCK_ASYNC_DIV12, ADC_SAMPLETIME_6CYCLES_5,  1.05882353f},
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_24CYCLES_5,  1.16470588f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_92CYCLES_5,  1.18823529f},
        {ADC_CLOCK_ASYNC_DIV10, ADC_SAMPLETIME_12CYCLES_5,  1.23529412f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_47CYCLES_5,  1.31764706f},
        {ADC_CLOCK_ASYNC_DIV16, ADC_SAMPLETIME_6CYCLES_5,  1.41176471f},
        {ADC_CLOCK_ASYNC_DIV12, ADC_SAMPLETIME_12CYCLES_5,  1.48235294f},
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_247CYCLES_5,  1.50588235f},
        {ADC_CLOCK_ASYNC_DIV8, ADC_SAMPLETIME_24CYCLES_5,  1.55294118f},
        {ADC_CLOCK_ASYNC_DIV10, ADC_SAMPLETIME_24CYCLES_5,  1.94117647f},
        {ADC_CLOCK_ASYNC_DIV16, ADC_SAMPLETIME_12CYCLES_5,  1.97647059f},
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_47CYCLES_5,  1.97647059f},
        {ADC_CLOCK_ASYNC_DIV32, ADC_SAMPLETIME_2CYCLES_5,  2.07058824f},
        {ADC_CLOCK_ASYNC_DIV12, ADC_SAMPLETIME_24CYCLES_5,  2.32941176f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_92CYCLES_5,  2.37647059f},
        {ADC_CLOCK_ASYNC_DIV8, ADC_SAMPLETIME_47CYCLES_5,  2.63529412f},
        {ADC_CLOCK_ASYNC_DIV32, ADC_SAMPLETIME_6CYCLES_5,  2.82352941f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_247CYCLES_5,  3.01176471f},
        {ADC_CLOCK_ASYNC_DIV16, ADC_SAMPLETIME_24CYCLES_5,  3.10588235f},
        {ADC_CLOCK_ASYNC_DIV10, ADC_SAMPLETIME_47CYCLES_5,  3.29411765f},
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_92CYCLES_5,  3.56470588f},
        {ADC_CLOCK_ASYNC_DIV32, ADC_SAMPLETIME_12CYCLES_5,  3.95294118f},
        {ADC_CLOCK_ASYNC_DIV12, ADC_SAMPLETIME_47CYCLES_5,  3.95294118f},
        {ADC_CLOCK_ASYNC_DIV8, ADC_SAMPLETIME_92CYCLES_5,  4.75294118f},
        {ADC_CLOCK_ASYNC_DIV16, ADC_SAMPLETIME_47CYCLES_5,  5.27058824f},
        {ADC_CLOCK_ASYNC_DIV10, ADC_SAMPLETIME_92CYCLES_5,  5.94117647f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_247CYCLES_5,  6.02352941f},
        {ADC_CLOCK_ASYNC_DIV32, ADC_SAMPLETIME_24CYCLES_5,  6.21176471f},
        {ADC_CLOCK_ASYNC_DIV12, ADC_SAMPLETIME_92CYCLES_5,  7.12941176f},
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_247CYCLES_5,  9.03529412f},
        {ADC_CLOCK_ASYNC_DIV16, ADC_SAMPLETIME_92CYCLES_5,  9.50588235f},
        {ADC_CLOCK_ASYNC_DIV32, ADC_SAMPLETIME_47CYCLES_5,  10.54117647f},
        {ADC_CLOCK_ASYNC_DIV8, ADC_SAMPLETIME_247CYCLES_5,  12.04705882f},
        {ADC_CLOCK_ASYNC_DIV10, ADC_SAMPLETIME_247CYCLES_5,  15.05882353f},
        {ADC_CLOCK_ASYNC_DIV12, ADC_SAMPLETIME_247CYCLES_5,  18.07058824f},
        {ADC_CLOCK_ASYNC_DIV32, ADC_SAMPLETIME_92CYCLES_5,  19.01176471f},
        {ADC_CLOCK_ASYNC_DIV16, ADC_SAMPLETIME_247CYCLES_5,  24.09411765f},
        {ADC_CLOCK_ASYNC_DIV32, ADC_SAMPLETIME_247CYCLES_5,  48.18823529f},
};

uint32_t currentAdcParam = 0;
uint32_t ADC_Prescaler = ADC_CLOCK_ASYNC_DIV1;
uint32_t ADC_SampleTime = ADC_SAMPLETIME_2CYCLES_5;

uint16_t ScreenTime = 0;      // index in ScreenTimes
uint16_t ScreenTime_adj = 0;  // 0-9 shift in ScreenTime
const float ScreenTimes[] = {100, 200, 500, 1000, 2000, 5000, 10000, 20000};  // sweep screen, microseconds

uint32_t ADCStartTick;         // time when start ADC buffer fill
uint32_t ADCHalfElapsedTick;   // the last time half buffer fill
uint32_t ADCElapsedTick;       // the last time buffer fill

/**
 * Copy of MX_ADC1_Init()
 */
HAL_StatusTypeDef adc_err=0;
void ADC_start() {
    ADC_Prescaler = ADC_Parameters[currentAdcParam].ADC_Prescaler;
    ADC_SampleTime = ADC_Parameters[currentAdcParam].ADC_SampleCycles;

    if (hadc1.State != HAL_ADC_STATE_READY) {
        HAL_ADCEx_MultiModeStop_DMA(&hadc1);
//      HAL_ADC_Stop_DMA(&hadc1);
        hadc1.State = HAL_ADC_STATE_READY; // TODO don't force state
    }

    ADC1_Init();
    ADC2_Init();

//  adc_err = HAL_ADC_Start_DMA(&hadc1, (uint32_t *) samplesBuffer, BUF_SIZE);
    adc_err = HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t *) samplesBuffer, BUF_SIZE / 2);
    if (adc_err != HAL_OK) {
        Error_Handler();
    }
    ADCStartTick = DWT_Get_Current_Tick();
}

uint32_t halfCount = 0;
uint32_t cpltCount = 0;

/**
  * @brief  Conversion complete callback in non-blocking mode
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
    ADCHalfElapsedTick = DWT_Elapsed_Tick(ADCStartTick);
    halfCount++;
    firstHalf = 0;
    /* Invalidate Data Cache to get the updated content of the SRAM on the first half of the ADC converted data buffer: 32 bytes */
//    SCB_InvalidateDCache_by_Addr((uint32_t *) &samplesBuffer[0], BUF_SIZE);
}

/**
  * @brief  Conversion DMA half-transfer callback in non-blocking mode
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    ADCElapsedTick = DWT_Elapsed_Tick(ADCStartTick);
    cpltCount++;
    adc1cplt = 1;
    /* Invalidate Data Cache to get the updated content of the SRAM on the second half of the ADC converted data buffer: 32 bytes */
//    SCB_InvalidateDCache_by_Addr((uint32_t *) &samplesBuffer[BUF_SIZE/2], BUF_SIZE);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
    Error_Handler();
}

void ADC_step_up() {
    if (ScreenTime_adj < 9)
        ScreenTime_adj++;
    else if (ScreenTime < sizeof(ScreenTimes) / sizeof(ScreenTimes[0]) - 2) // last value forbidden to assign
        ScreenTime_adj = 0, ScreenTime++;
}


void ADC_step_down() {
    if (ScreenTime_adj > 0)
        ScreenTime_adj--;
    else if (ScreenTime > 0)
        ScreenTime_adj = 9, ScreenTime--;
}


float ADC_getTime() {
    float time = ScreenTimes[ScreenTime];
    // next time always exist because last forbidden to assign
    float adj = (ScreenTimes[ScreenTime + 1] - time) * ScreenTime_adj / 10;
    time += adj;
    return time;
}

s16 sStep;
float time;

void ADC_step(int16_t step) {
    if (step == 0) return;
    if (step > 0) {
        if (++currentAdcParam >= ADC_Parameters_Size) currentAdcParam = ADC_Parameters_Size - 1;
    } else {
        if (currentAdcParam-- == 0) currentAdcParam = 0;
    }
//    ADC_Prescaler = ADC_Parameters[currentAdcParam].ADC_Prescaler;
//    ADC_SampleCycles = ADC_Parameters[currentAdcParam].ADC_SampleCycles;
    return;

/*  if (step > 0) ADC_step_up();
    else ADC_step_down();
    sStep = step;

    time = ADC_getTime(); // get screen sweep time

    // looking last parameters set with ScreenTime less than required time
    int i = 1;
    while (ADC_Parameters[i].ScreenTime < time) {
        i++;
        if (i >= ADC_Parameters_Size) break;
    }

    i--;
    currentAdcParam = i;
    ADC_Prescaler = ADC_Parameters[i].ADC_Prescaler;
    ADC_SampleCycles = ADC_Parameters[i].ADC_SampleCycles;

    // set X scale
    scaleX = ADC_Parameters[i].ScreenTime / time;
//*/
//    ADC_start();
}

/*uint16_t ICount = 0;

// dma2 stream 0 irq handler
void DMA2_Stream0_IRQHandler() {
    ICount++;
    // Test on DMA Stream HalfTransfer Complete interrupt
    if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_HTIF0)) {
        // Clear Stream0 HalfTransfer
        DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_HTIF0);

        // count time for half circle
        ADCHalfElapsedTick = DWT_Elapsed_Tick(ADCStartTick);
        half = 0;
    }

    // Test on DMA Stream Transfer Complete interrupt
    if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0)) {
        // Clear Stream0 Transfer Complete
        DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);

        // count time for one circle
        ADCElapsedTick = DWT_Elapsed_Tick(ADCStartTick);
        ADCStartTick = DWT_Get_Current_Tick();
        half = 1;
    }
} //*/

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void ADC1_Init(void) {

//    if (HAL_ADC_DeInit(&hadc1) != HAL_OK)  {
//        Error_Handler();
//    }

    ADC_MultiModeTypeDef multimode = {0};
    ADC_ChannelConfTypeDef sConfig = {0};

    /** Common config
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_Prescaler;
    hadc1.Init.Resolution = ADC_RESOLUTION_8B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.GainCompensation = 0;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    /** Configure the ADC multi-mode
    */
    multimode.Mode = ADC_DUALMODE_INTERL;
    multimode.DMAAccessMode = ADC_DMAACCESSMODE_8_6_BITS;
    multimode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_2CYCLES;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SampleTime;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void ADC2_Init(void) {

//    if (HAL_ADC_DeInit(&hadc2) != HAL_OK)  {
//        Error_Handler();
//    }

    ADC_ChannelConfTypeDef sConfig = {0};

    /** Common config
    */
    hadc2.Instance = ADC2;
    hadc2.Init.ClockPrescaler = ADC_Prescaler;
    hadc2.Init.Resolution = ADC_RESOLUTION_8B;
    hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc2.Init.GainCompensation = 0;
    hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc2.Init.LowPowerAutoWait = DISABLE;
    hadc2.Init.ContinuousConvMode = ENABLE;
    hadc2.Init.NbrOfConversion = 1;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.DMAContinuousRequests = DISABLE;
    hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc2.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc2) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SampleTime;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}
