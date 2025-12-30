#include <stdio.h>
#include <_main.h>
#include "generator.h"

/* F7
 * TIM1 Configuration
 * CLK  216 mHz
 * PRE           108 - 1 => 2 MHz
 * COUNT PERIOD  100 - 1 => 20 KHz
 */
/* H7
 * TIM1 Configuration
 * CLK  - 400 mHz
 * AHB2 - 200 mHz
 * PRE           100 - 1 => 2 MHz
 * COUNT PERIOD  100 - 1 => 20 KHz
 */

#define TIMCLK 170000000

uint32_t currentGenParam = 10;
uint32_t tim1Prescaler = 99;
uint32_t tim1Period = 16;
uint32_t tim1Pulse = 7;
uint32_t tim1Freq = 100000;
uint32_t tim1Freq2 = 0;

struct GEN_param {
    uint32_t TIM_Prescaler;
    uint32_t TIM_Period;
    uint32_t Frequency;    // Hz
};
typedef struct GEN_param GEN_PARAM;

#define GEN_Parameters_Size 31
const GEN_PARAM GEN_Parameters[GEN_Parameters_Size] = {
        {0, 16, 10000000},
        {0, 20, 8095238},
        {0, 27, 6071428},
        {0, 41, 4047619},
        {0, 84, 2000000},
        {9, 16, 1000000},
        {0, 211, 801886},
        {0, 282, 600706},
        {0, 424, 400000},
        {9, 84, 200000},
        {99, 16, 100000},
        {0, 2124, 80000},
        {0, 2832, 60007},
        {9, 424, 40000},
        {99, 84, 20000},
        {999, 16, 10000},
        {9, 2124, 8000},
        {0, 28332, 6000},
        {99, 424, 4000},
        {999, 84, 2000},
        {9999, 16, 1000},
        {99, 2124, 800},
        {9, 28332, 600},
        {999, 424, 400},
        {9999, 84, 200},
        {9999, 169, 100},
        {999, 2124, 80},
        {99, 28332, 60},
        {999, 4249, 40},
        {9999, 849, 20},
        {9999, 1699,10}
};


void GEN_step(int16_t step) {
    char msg[200];

    if (step == 0) return;

    if (step > 0) {
        if (currentGenParam > 0) currentGenParam--;
    } else {
        if (currentGenParam < GEN_Parameters_Size-1) currentGenParam++;
    }

    GEN_setParams();

    sprintf(msg, "After step. param: %u, presc: %u, period: %u freq: %u\n",
            currentGenParam, tim1Prescaler, tim1Period, tim1Freq);
    DBG_Trace(msg);
}

/**
 *  made from MX_TIM1_Init()
 *
 */
void GEN_setParams() {
    tim1Prescaler = GEN_Parameters[currentGenParam].TIM_Prescaler;
    tim1Period = GEN_Parameters[currentGenParam].TIM_Period;
    tim1Freq = GEN_Parameters[currentGenParam].Frequency;
    tim1Pulse = tim1Period * 40 / 100;
    tim1Freq2 = TIMCLK / (tim1Prescaler + 1) / (tim1Period + 1);
    if (tim1Freq != tim1Freq2) {
        Error_Handler();
    }

    TIM_OC_InitTypeDef sConfigOC;

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = tim1Prescaler;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = tim1Period;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
        Error_Handler();

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = tim1Pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}
