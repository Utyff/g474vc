#include <_main.h>

#include "dwt.h"

static const uint16_t sin32[] = {
    0,
    40,
    157,
    346,
    601,
    911,
    1265,
    1649,
    2048,
    2447,
    2831,
    3185,
    3495,
    3750,
    3939,
    4056,
    4095,
    4056,
    3939,
    3750,
    3495,
    3185,
    2831,
    2447,
    2048,
    1649,
    1265,
    911,
    601,
    346,
    157,
    40
};

/* G4
 * TIM4 Configuration
 * CLK  - 170 mHz
 * APB2 - 170 mHz
 * PRE           2 + 1 => 56.66 mHz
 * COUNT PERIOD  10 + 1 => 5.1515 mHz
 * DAC table size 32 => 0.1609 mHz
 * MAX DAC timer ~ 5 mHz. Otherwise, a conflict occurs between the DMA DAC and the ADC.
 */
static uint32_t prescaler = 2;
uint32_t autoreload = 10;


static void DAC_Activate(void) {
    /* Enable DAC channel */
    LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
    DWT_Delay_us(LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US);
    /* Enable DAC channel DMA request */
    LL_DAC_EnableDMAReq(DAC1, LL_DAC_CHANNEL_1);
    /* Enable DAC channel trigger */
    LL_DAC_EnableTrigger(DAC1, LL_DAC_CHANNEL_1);
}

static void DAC_Deativate(void) {
    LL_DAC_DisableTrigger(DAC1, LL_DAC_CHANNEL_1);
    LL_DAC_DisableDMAReq(DAC1, LL_DAC_CHANNEL_1);
    LL_DAC_Disable(DAC1, LL_DAC_CHANNEL_1);
    DWT_Delay_us(LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US);
}

static void startTimer() {
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

    TIM_InitStruct.Prescaler = prescaler;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = autoreload;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM4, &TIM_InitStruct);

    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = autoreload - 1;
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);

    /* Start tim4 ch1 */
    /* Enable output channel 1 */
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
    /* Enable counter */
    LL_TIM_EnableCounter(TIM4);
    /* Force update generation */
    LL_TIM_GenerateEvent_UPDATE(TIM4);
}

void DAC_startSin() {

    startTimer();

    /*##-2- Enable DAC selected channel and associated DMA #############################*/
    /* Set DMA transfer addresses of source and destination */
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2,
                           (uint32_t) &sin32,
                           LL_DAC_DMA_GetRegAddr(DAC1, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_12BITS_RIGHT_ALIGNED),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    /* Set DMA transfer size */
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, 32);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

    DAC_Activate();
}

static void DAC_stop() {
    DAC_Deativate();

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_TIM_DisableCounter(TIM4);
    LL_TIM_CC_DisableChannel(TIM4, LL_TIM_CHANNEL_CH1);
}

void DAC_step(const int16_t step) {
    if (step == 0) {
        return;
    }
    if (step < 0) {
        if (autoreload > 2) {
            autoreload -= 1;
        }
    } else {
        if (autoreload < 255) {
            autoreload += 1;
        }
    }
    DAC_stop();
    DAC_startSin();
}
