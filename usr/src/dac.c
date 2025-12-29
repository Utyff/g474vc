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

void DAC_startSin() {
    for(int i=0; i<32; i++) {
        sin32_2[i] = (sin32[i]>>4);
    }
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

    /*##-2- Enable DAC selected channel and associated DMA #############################*/
    if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)sin32_2, 32, DAC_ALIGN_8B_R) != HAL_OK) {
        Error_Handler();
    }
}
