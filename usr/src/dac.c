#include <_main.h>

extern DAC_HandleTypeDef hdac1;

const uint16_t sin32[] = {
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

uint8_t sin32_2[32] = {
};

#define GEN_Parameters_Size 12
const uint32_t DAC_Parameters[GEN_Parameters_Size] = {0, 1, 3, 5, 7, 9, 19, 21, 23, 25, 27, 29};
uint32_t currentDacParam = 2;
uint32_t tim4Prescaler = 3;


void DAC_startSin() {
    for(int i=0; i<32; i++) {
        sin32_2[i] = (sin32[i]>>4);
    }
    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);

    /*##-2- Enable DAC selected channel and associated DMA #############################*/
    if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)sin32_2, 32, DAC_ALIGN_8B_R) != HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
}


void DAC_setParams() {
    tim4Prescaler = DAC_Parameters[currentDacParam];

    TIM_OC_InitTypeDef sConfigOC;

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = tim4Prescaler;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 9;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
        Error_Handler();

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 4;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();

    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
}

void DAC_step(int16_t step) {
    char msg[200];

    if (step == 0) return;

    if (step > 0) {
        if (currentDacParam > 0) currentDacParam--;
    } else {
        if (currentDacParam < GEN_Parameters_Size-1) currentDacParam++;
    }

    DAC_setParams();

    sprintf(msg, "DAC step. param: %u, presc: %u\n", currentDacParam, tim4Prescaler);
    DBG_Trace(msg);
}
