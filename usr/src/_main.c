#include <stdlib.h>
#include <stdio.h>
#include <_main.h>
#include <delay.h>
#include <draw.h>
#include <keys.h>
#include <DataBuffer.h>
#include <generator.h>
#include <adc.h>
#include <dac.h>
#include "ft6x36.h"

//extern I2C_HandleTypeDef hi2c1;

void CORECheck();

void FPUCheck();

extern int ii;
extern float time;

const char buildDate[] = __DATE__;
const char buildTime[] = __TIME__;

//touch_point_t touchPoint1;
//touch_point_t touchPoint2;


void mainInitialize() {
    char buf[120];
    sprintf(buf, "\n\nBuild: %s %s\n", buildDate, buildTime);
    DBG_Trace(buf);

    CORECheck();
    FPUCheck();

    DWT_Init();
    LCD_Init();
    LCD_Clear(BLACK);
    KEYS_init();

//    FT6x36(&hi2c1);

    adc1cplt = 0;
    ADC_start();

    GEN_setParams();
    DAC_startSin();

    HAL_TIM_Encoder_Start(&htim8, TIM_CHANNEL_1);
}

u32 ticks =0;

void mainCycle() {
    if ((random() & 7) < 2) HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
//    getPoint(0, &touchPoint1);
//    getPoint(1, &touchPoint2);

    drawScreen();
    KEYS_scan();

    POINT_COLOR = WHITE;
    BACK_COLOR = CLR_BACKGROUND;
    LCD_ShowxNum(0, LINE1_Y, TIM8->CNT, 5, 12, 0x0);
    LCD_ShowxNum(30, LINE1_Y, (u32) button1Count, 5, 12, 0x0);
//    LCD_ShowxNum(60, 214, (u32) ii, 5, 12, 0x01);
//    LCD_ShowxNum(90, 214, (u32) time / 10, 5, 12, 0x01);
//    LCD_ShowxNum(120, 214, (u32) firstHalf, 5, 12, 0x01);

    POINT_COLOR = MAGENTA;
    LCD_ShowxNum(0,  LINE2_Y, ADCElapsedTick, 10, 12, 0x0);

    if (adc1cplt != 0) {
        adc1cplt = 0;
        ADC_start();
    }

    delay_ms(30);
}

#ifdef DEBUG_TRACE_SWO

void SWO_Trace(uint8_t *msg) {
    for (int i = 0; msg[i] != 0; i++) {
        ITM_SendChar(msg[i]);
    }
}

#endif

void FPUCheck(void) {
    char buf[120];
    uint32_t mvfr0;

    sprintf(buf, "%08X %08X %08X\n%08X %08X %08X\n",
            *(volatile uint32_t *) 0xE000EF34,   // FPCCR  0xC0000000
            *(volatile uint32_t *) 0xE000EF38,   // FPCAR
            *(volatile uint32_t *) 0xE000EF3C,   // FPDSCR
            *(volatile uint32_t *) 0xE000EF40,   // MVFR0  0x10110021 vs 0x10110221
            *(volatile uint32_t *) 0xE000EF44,   // MVFR1  0x11000011 vs 0x12000011
            *(volatile uint32_t *) 0xE000EF48);  // MVFR2  0x00000040
    DBG_Trace(buf);

    mvfr0 = *(volatile uint32_t *) 0xE000EF40;

    switch (mvfr0) {
        case 0x00000000 :
            sprintf(buf, "No FPU\n");
            break;
        case 0x10110021 :
            sprintf(buf, "FPU-S Single-precision only\n");
            break;
        case 0x10110221 :
            sprintf(buf, "FPU-D Single-precision and Double-precision\n");
            break;
        default :
            sprintf(buf, "Unknown FPU\n");
    }
    DBG_Trace(buf);
}

void CORECheck(void) {
    char buf[120];
    uint32_t cpuid = SCB->CPUID;
    uint32_t var, pat;

    sprintf(buf, "\nCPUID %08X DEVID %03X REVID %04X\n", cpuid, DBGMCU->IDCODE & 0xFFF, DBGMCU->IDCODE >> 16);
    DBG_Trace(buf);

    pat = (cpuid & 0x0000000F);
    var = (cpuid & 0x00F00000) >> 20;

    if ((cpuid & 0xFF000000) == 0x41000000) // ARM
    {
        switch ((cpuid & 0x0000FFF0) >> 4) {
            case 0xC20 :
                sprintf(buf, "Cortex M0 r%dp%d\n", var, pat);
                break;
            case 0xC60 :
                sprintf(buf, "Cortex M0+ r%dp%d\n", var, pat);
                break;
            case 0xC21 :
                sprintf(buf, "Cortex M1 r%dp%d\n", var, pat);
                break;
            case 0xC23 :
                sprintf(buf, "Cortex M3 r%dp%d\n", var, pat);
                break;
            case 0xC24 :
                sprintf(buf, "Cortex M4 r%dp%d\n", var, pat);
                break;
            case 0xC27 :
                sprintf(buf, "Cortex M7 r%dp%d\n", var, pat);
                break;
            case 0xD21 :
                sprintf(buf, "Cortex M33 r%dp%d\n", var, pat);
                break;

            default :
                sprintf(buf, "Unknown CORE\n");
        }
    } else
        sprintf(buf, "Unknown CORE IMPLEMENTER\n");
    DBG_Trace(buf);
}
