#include <_main.h>
#include <dwt.h>
#include <DataBuffer.h>
#include "adc.h"

// max ADC clock = ??mHz; Recommended ADC clock = 60mHz
struct ADC_param {
    uint32_t ADC_Prescaler;
    uint32_t ADC_SampleTime;
    float SampleTime;    // microseconds
};
typedef struct ADC_param ADC_PARAM;

uint32_t currentAdcParam = 0;

#define ADC_Parameters_Size  8
const ADC_PARAM ADC_Parameters[ADC_Parameters_Size] = {
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_2CYCLES_5,  0.f},
        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_3CYCLES_5,  0.f},
//        {ADC_CLOCK_ASYNC_DIV1, ADC_SAMPLETIME_6CYCLES_5,  0.f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_2CYCLES_5,  0.f},
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_3CYCLES_5,  0.f},
//        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_6CYCLES_5,  0.f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_2CYCLES_5,  0.f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_3CYCLES_5,  0.f},
//        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_6CYCLES_5,  0.f}
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_2CYCLES_5,  0.f},
        {ADC_CLOCK_ASYNC_DIV6, ADC_SAMPLETIME_3CYCLES_5,  0.f},
//        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_6CYCLES_5,  0.f}
};

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
    ADC_SampleTime = ADC_Parameters[currentAdcParam].ADC_SampleTime;

    adc_err = HAL_ADC_Stop_DMA(&hadc1);
//    adc_err = HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t *) samplesBuffer, BUF_SIZE / 2);
    if (adc_err != HAL_OK) {
        Error_Handler();
    }

    ADC_ChannelConfTypeDef sConfig;

    /**Common config
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
    hadc1.Init.NbrOfDiscConversion = 1;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    /**Configure Regular Channel
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

    adc_err = HAL_ADC_Start_DMA(&hadc1, (uint32_t *) samplesBuffer, BUF_SIZE);
//    adc_err = HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t *) samplesBuffer, BUF_SIZE / 2);
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
//    ADC_SampleTime = ADC_Parameters[currentAdcParam].ADC_SampleTime;
    return;

/*    if (step == 0) return;
    if (step > 0) ADC_step_up();
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
    ADC_SampleTime = ADC_Parameters[i].ADC_SampleTime;

    // set X scale
    scaleX = ADC_Parameters[i].ScreenTime / time;
//*/
    ADC_start();
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
