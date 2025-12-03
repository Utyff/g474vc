#include <_main.h>
#include <dwt.h>
#include <graph.h>
#include <DataBuffer.h>
#include "adc.h"


struct ADC_param {
    uint32_t ADC_Prescaler;
    uint32_t ADC_SampleTime;
    float SampleTime;    // microseconds
    float ScreenTime;    // microseconds
};
typedef struct ADC_param ADC_PARAM;

#define ADC_Parameters_Size  3 // 31
const ADC_PARAM ADC_Parameters[ADC_Parameters_Size] = {
        {ADC_CLOCK_ASYNC_DIV2, ADC_SAMPLETIME_2CYCLES_5,  0.4074074f,  130.37037f},
        {ADC_CLOCK_ASYNC_DIV4, ADC_SAMPLETIME_6CYCLES_5,  0.4259259f,  136.29630f} };
        /*
        {ADC_CLOCK_SYNC_PCLK_DIV6, ADC_SAMPLETIME_3CYCLES,   0.6111111f,  195.55556f},
        {ADC_CLOCK_SYNC_PCLK_DIV2, ADC_SAMPLETIME_28CYCLES,  0.6666667f,  213.33333f},
        {ADC_CLOCK_SYNC_PCLK_DIV8, ADC_SAMPLETIME_3CYCLES,   0.8148148f,  260.74074f},
        {ADC_CLOCK_SYNC_PCLK_DIV4, ADC_SAMPLETIME_15CYCLES,  0.8518519f,  272.59259f},
        {ADC_CLOCK_SYNC_PCLK_DIV2, ADC_SAMPLETIME_56CYCLES,  1.1851852f,  379.25926f},
        {ADC_CLOCK_SYNC_PCLK_DIV6, ADC_SAMPLETIME_15CYCLES,  1.2777778f,  408.88889f},
        {ADC_CLOCK_SYNC_PCLK_DIV4, ADC_SAMPLETIME_28CYCLES,  1.3333333f,  426.66667f},
        {ADC_CLOCK_SYNC_PCLK_DIV2, ADC_SAMPLETIME_84CYCLES,  1.7037037f,  545.18519f},
        {ADC_CLOCK_SYNC_PCLK_DIV6, ADC_SAMPLETIME_28CYCLES,  2.0000000f,  640.00000f},
        {ADC_CLOCK_SYNC_PCLK_DIV2, ADC_SAMPLETIME_112CYCLES, 2.2222222f,  711.11111f},
        {ADC_CLOCK_SYNC_PCLK_DIV4, ADC_SAMPLETIME_56CYCLES,  2.3703704f,  758.51852f},
        {ADC_CLOCK_SYNC_PCLK_DIV8, ADC_SAMPLETIME_28CYCLES,  2.6666667f,  853.33333f},
        {ADC_CLOCK_SYNC_PCLK_DIV2, ADC_SAMPLETIME_144CYCLES, 2.8148148f,  900.74074f},
        {ADC_CLOCK_SYNC_PCLK_DIV4, ADC_SAMPLETIME_84CYCLES,  3.4074074f,  1090.37037f},
        {ADC_CLOCK_SYNC_PCLK_DIV6, ADC_SAMPLETIME_56CYCLES,  3.5555556f,  1137.77778f},
        {ADC_CLOCK_SYNC_PCLK_DIV4, ADC_SAMPLETIME_112CYCLES, 4.4444444f,  1422.22222f},
        {ADC_CLOCK_SYNC_PCLK_DIV8, ADC_SAMPLETIME_56CYCLES,  4.7407407f,  1517.03704f},
        {ADC_CLOCK_SYNC_PCLK_DIV6, ADC_SAMPLETIME_84CYCLES,  5.1111111f,  1635.55556f},
        {ADC_CLOCK_SYNC_PCLK_DIV4, ADC_SAMPLETIME_144CYCLES, 5.6296296f,  1801.48148f},
        {ADC_CLOCK_SYNC_PCLK_DIV6, ADC_SAMPLETIME_112CYCLES, 6.6666667f,  2133.33333f},
        {ADC_CLOCK_SYNC_PCLK_DIV8, ADC_SAMPLETIME_84CYCLES,  6.8148148f,  2180.74074f},
        {ADC_CLOCK_SYNC_PCLK_DIV6, ADC_SAMPLETIME_144CYCLES, 8.4444444f,  2702.22222f},
        {ADC_CLOCK_SYNC_PCLK_DIV8, ADC_SAMPLETIME_112CYCLES, 8.8888889f,  2844.44444f},
        {ADC_CLOCK_SYNC_PCLK_DIV2, ADC_SAMPLETIME_480CYCLES, 9.0370370f,  2891.85185f},
        {ADC_CLOCK_SYNC_PCLK_DIV8, ADC_SAMPLETIME_144CYCLES, 11.2592593f, 3602.96296f},
        {ADC_CLOCK_SYNC_PCLK_DIV4, ADC_SAMPLETIME_480CYCLES, 18.0740741f, 5783.70370f},
        {ADC_CLOCK_SYNC_PCLK_DIV6, ADC_SAMPLETIME_480CYCLES, 27.1111111f, 8675.55556f},
        {ADC_CLOCK_SYNC_PCLK_DIV8, ADC_SAMPLETIME_480CYCLES, 36.1481481f, 11567.40741f}
}; //*/

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
void ADC_setParams() {

    ADC_ChannelConfTypeDef sConfig;

    /**Common config
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_Prescaler;
    hadc1.Init.Resolution = ADC_RESOLUTION_8B;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.NbrOfDiscConversion = 1;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
//    hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
//    hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
//    hadc1.Init.BoostMode = ENABLE;
    hadc1.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    /**Configure Regular Channel
    */
    /**Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SampleTime;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) samplesBuffer, BUF_SIZE-4);

    ADCStartTick = DWT_Get_Current_Tick();
}

uint32_t halfCount =0;
uint32_t cpltCount =10;
/**
  * @brief  Conversion complete callback in non-blocking mode
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
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
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    cpltCount++;
    firstHalf = 1;
    /* Invalidate Data Cache to get the updated content of the SRAM on the second half of the ADC converted data buffer: 32 bytes */
//    SCB_InvalidateDCache_by_Addr((uint32_t *) &samplesBuffer[BUF_SIZE/2], BUF_SIZE);
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
int ii;

void ADC_step(int16_t step) {
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
    ii = i;
    ADC_Prescaler = ADC_Parameters[i].ADC_Prescaler;
    ADC_SampleTime = ADC_Parameters[i].ADC_SampleTime;

    // set X scale
    scaleX = ADC_Parameters[i].ScreenTime / time;
//*/
    ADC_setParams();
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
