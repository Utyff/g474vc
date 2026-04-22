#include <stdio.h>
#include <_main.h>
#include "generator.h"

/* H7
 * TIM1 Configuration
 * CLK  - 400 mHz
 * AHB2 - 200 mHz
 * PRE           100 - 1 => 2 MHz
 * COUNT PERIOD  100 - 1 => 20 KHz
 */

struct GEN_param {
    uint32_t TIM_Prescaler;
    uint32_t TIM_Period;
    uint32_t Frequency;    // Hz
};
typedef struct GEN_param GEN_PARAM;

#define GEN_Parameters_Size 26
const GEN_PARAM GEN_Parameters[GEN_Parameters_Size] = {
        {0, 19, 10000000},
        {0, 24, 8000000},
        {0, 32, 6060606},
        {0, 49, 4000000},
        {0, 99, 2000000},
        {9, 19, 1000000},
        {9, 24, 800000},
        {8, 36, 600600},
        {9, 49, 400000},
        {9, 99, 200000},
        {99, 19, 100000},
        {99, 24, 80000},
        {32, 100, 60006},
        {99, 49, 40000},
        {99, 99, 20000},
        {999, 19, 10000},
        {999, 24, 8000},
        {329, 100, 6000},
        {999, 49, 4000},
        {999, 99, 2000},
        {9999, 19, 1000},
        {9999, 24, 800},
        {3299, 100, 600},
        {9999, 49, 400},
        {9999, 99, 200},
        {9999, 199, 100}
};

uint32_t currentGenParam = 9;
uint32_t tim1Prescaler = 9;
uint32_t tim1Period = 99;
uint32_t tim1Pulse = 40;
uint32_t tim1Freq = 0;

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


void GEN_setParams() {
    tim1Prescaler = GEN_Parameters[currentGenParam].TIM_Prescaler;
    tim1Period = GEN_Parameters[currentGenParam].TIM_Period;
    tim1Freq = GEN_Parameters[currentGenParam].Frequency;
    tim1Pulse = tim1Period * 40 / 100;

    LL_TIM_SetPrescaler(TIM1, tim1Prescaler);
    LL_TIM_SetAutoReload(TIM1, tim1Period);
    LL_TIM_OC_SetCompareCH1(TIM1, tim1Pulse);

    // Start tim1 ch1
    // Enable output channel 1
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    // Enable the TIM main Output
    LL_TIM_EnableAllOutputs(TIM1);
    // Enable counter
    LL_TIM_EnableCounter(TIM1);
}
