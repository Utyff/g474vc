#include <stdio.h>
#include <_main.h>
#include <keys.h>
#include <adc.h>
#include <dac.h>
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
int16_t stepEnc1 = 0;
int16_t stepEnc2 = 0;
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

    uint16_t st = ~SW2_GPIO_Port->IDR & SW2_Pin;   // E0 pin number & BTN1
    st |= ~ENC1_GPIO_Port->IDR & ENC1_Pin;         // E1 pin number & BTN2
    st |= ~SW3_GPIO_Port->IDR  & SW3_Pin;          // E2 pin number & BTN3
    st |= ~SW4_GPIO_Port->IDR  & SW4_Pin;          // E3 pin number & BTN4
    st |= ~ENC2_GPIO_Port->IDR & ENC2_Pin;         // E4 pin number & BTN5
    st |= ~SW5_GPIO_Port->IDR  & SW5_Pin;          // E5 pin number & BTN6

    uint16_t change = st ^ btns_state;             // bit will set 1 if button change state
    if (change!=0) {
        // if button1 change state
        if (change & BUTTON1) {
            debounceCnt = DEBOUNCING_CNT;
            if ((btns_state & BUTTON1) != 0) {
                button1Count++;
            }
        }
        if (change & BUTTON2) {
            debounceCnt = DEBOUNCING_CNT;
            if ((btns_state & BUTTON2) != 0) {
                button2Count++;
            }
        }
        if (change & BUTTON3) {
            debounceCnt = DEBOUNCING_CNT;
            if ((btns_state & BUTTON3) != 0) {
                button3Count++;
            }
        }
        if (change & BUTTON6) {
            debounceCnt = DEBOUNCING_CNT;
            if ((btns_state & BUTTON6) != 0) {
                if (activeChannel == 0) activeChannel = 1;
                else activeChannel = 0;
            }
        }
        btns_state = st;
    }

    // if encoder has step - do it
    stepEnc1 = ENC1_Get();
    char buf[64];
    if (stepEnc1 != 0) {
        sprintf(buf, "enc1: %hi\n", stepEnc1);
        DBG_Trace(buf);

        // choose type of encoder action
        uint8_t action = button1Count % MAX_ACTIONS;
        if (action == 0) {
            ADC_step(stepEnc1);
        } else if (action == 1) {
            GEN_step(stepEnc1);
        } else {
            DAC_step(stepEnc1);
        }
    }
    stepEnc2 = ENC2_Get();
    if (stepEnc2 != 0) {
        sprintf(buf, "enc2: %hi\n", stepEnc2);
        DBG_Trace(buf);
        if (activeChannel == 0) {
            if (stepEnc2 > 0) {
                if (ch1Shift < 97) {
                    ch1Shift += 3;
                }
            } else {
                if (ch1Shift > 2) {
                    ch1Shift -= 3;
                }
            }
        } else {
            if (stepEnc2 > 0) {
                if (ch2Shift < 100) {
                    ch2Shift += 3;
                }
            } else {
                if (ch2Shift > 97) {
                    ch2Shift -= 3;
                }
            }
        }
        setShift();
    }
}
