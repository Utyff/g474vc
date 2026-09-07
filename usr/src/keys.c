#include <stdio.h>
#include <_main.h>
#include <keys.h>
#include <adc.h>
#include "dac.h"
#include <generator.h>


#define DEBOUNCING_CNT 0
#define MAX_ENCODER    255 // max encoder value
#define MID_ENCODER    (MAX_ENCODER/2+1)
#define ENCODER_STEP   2   // counts per step
#define ENCODER1_TIM    TIM8
#define ENCODER2_TIM    TIM5
#define MAX_ACTIONS    3u


uint8_t button1Count = 0;
uint8_t button2Count = 0;
uint8_t button3Count = 0;
uint16_t btns_state = 0;
static uint16_t debounceCnt = 0;


void KEYS_init() {
    /* Start ENCODER_TIM */
    LL_TIM_CC_EnableChannel(ENCODER1_TIM, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(ENCODER1_TIM, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(ENCODER1_TIM);
    ENCODER1_TIM->CNT = MID_ENCODER;
    LL_TIM_CC_EnableChannel(ENCODER2_TIM, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(ENCODER2_TIM, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(ENCODER2_TIM);
    ENCODER2_TIM->CNT = MID_ENCODER;
}

int16_t ENC1_Get() {
    int16_t result = 0;

    int16_t step = (int16_t) (ENCODER1_TIM->CNT - MID_ENCODER);
    if (step >= ENCODER_STEP || step <= -ENCODER_STEP) {
        result = step / (int16_t) ENCODER_STEP;

        ENCODER1_TIM->CNT -= result * ENCODER_STEP;
    }

    return result;
}

int16_t ENC2_Get() {
    int16_t result = 0;

    int16_t step = (int16_t) (ENCODER2_TIM->CNT - MID_ENCODER);
    if (step >= ENCODER_STEP || step <= -ENCODER_STEP) {
        result = step / (int16_t) ENCODER_STEP;

        ENCODER2_TIM->CNT -= result * ENCODER_STEP;
    }

    return result;
}

/**
 * Check buttons and run actions
 */
void KEYS_scan() {
    if (debounceCnt > 0) {
        debounceCnt--;
        return;
    }

    uint32_t st = (~SW5_GPIO_Port->IDR & SW5_Pin) >> 13; // 13 - pin number & BTN1
    st |= (~SW2_GPIO_Port->IDR & SW2_Pin) << 1 ;         // 0 pin number & BTN2
    st |= (~SW3_GPIO_Port->IDR & SW3_Pin);               // 2 pin number & BTN3
    st |= (~SW4_GPIO_Port->IDR & SW4_Pin);               // 3 pin number & BTN4
    st |= (~ENC1_GPIO_Port->IDR & ENC1_Pin) <<3;         // 1 pin number & BTN5
    st |= (~ENC2_GPIO_Port->IDR & ENC2_Pin) <<1;         // 4 pin number & BTN6
    // if button1 change state
    if (st != (btns_state & BUTTON1)) {
        debounceCnt = DEBOUNCING_CNT;
        if ((btns_state & BUTTON1) != 0) {
            button1Count++;
        }
    }
    if (st != (btns_state & BUTTON2)) {
        debounceCnt = DEBOUNCING_CNT;
        if ((btns_state & BUTTON2) != 0) {
            button2Count++;
        }
    }
    if (st != (btns_state & BUTTON3)) {
        debounceCnt = DEBOUNCING_CNT;
        if ((btns_state & BUTTON3) != 0) {
            button3Count++;
        }
    }
    btns_state = st;

    // if encoder has step - do it
    int16_t step1 = ENC1_Get();
    char buf[64];
    if (step1 != 0) {
        sprintf(buf, "enc1: %hi\n", step1);
        DBG_Trace(buf);
    }
    int16_t step2 = ENC2_Get();
    if (step2 != 0) {
        sprintf(buf, "enc2: %hi\n", step2);
        DBG_Trace(buf);
    }

    // choose type of encoder action
    uint8_t action = button1Count % MAX_ACTIONS;
    if (action == 0) {
        ADC_step(step1);
    } else if (action == 1) {
        GEN_step(step1);
    } else {
        DAC_step(step1);
    }
}
