#include <stdio.h>
#include <_main.h>
#include "generator.h"

/* G4
 * TIM1 Configuration
 * CLK  - 170 mHz
 * AHB2 - 170 mHz
 * PRE           100 - 1 => 1.7 MHz
 * COUNT PERIOD  100 - 1 => 17 KHz
 */

struct GEN_param {
    uint32_t TIM_Prescaler;
    uint32_t TIM_Period;
    uint32_t Frequency;    // Hz
};
typedef struct GEN_param GEN_PARAM;

#define GEN_Parameters_Size 31
static const GEN_PARAM GEN_Parameters[GEN_Parameters_Size] = {
    {0, 16, 10000000},
    {0, 20, 8095238},
    {0, 27, 6071429},
    {0, 41, 4047619},
    {0, 84, 2000000},
    {9, 16, 1000000},
    {0, 211, 801887},
    {0, 282, 600707},
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
    {9999, 1699, 10}
};

uint32_t currentGenParam = 0;
uint32_t tim1Prescaler;
uint32_t tim1Period;
uint32_t tim1Pulse;
uint32_t tim1Freq;

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
    LL_TIM_EnableCounter(TIM1);
}
