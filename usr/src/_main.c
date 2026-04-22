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


void CORECheck();

void FPUCheck();

extern int ADC_param;
extern float time;

const char buildDate[] = __DATE__;
const char buildTime[] = __TIME__;
uint8_t DMA1_0_busy;
// touch_point_t touchPoint1;
// touch_point_t touchPoint2;


void UART_start() {
    DMA1_0_busy = 0;
}

void mainInitialize() {
    UART_start();
    char buf[120];
    sprintf(buf, "\n\nBuild: %s %s\n", buildDate, buildTime);
    DBG_Trace(buf);

    CORECheck();
    FPUCheck();

    DWT_Init();
    // LCD_Init();
    // LCD_Clear(BLACK);
    KEYS_init();

    // ADCworks = 0;
    // ADC_start();

    GEN_setParams();
    // DAC_startSin();
}

u32 ticks = 0;

void mainCycle() {

    if ((random() & 7) < 2) GPIOB->ODR ^= LED1_Pin;
    if ((random() & 7) < 2) GPIOB->ODR ^= LED2_Pin;
    if ((random() & 7) < 2) GPIOB->ODR ^= LED3_Pin;
    // getPoint(0, &touchPoint1);
    // getPoint(1, &touchPoint2);

    // if (ADCworks == 0) {
    //     drawScreen();
    //     ADC_start();
    // }

    KEYS_scan();

    // POINT_COLOR = CYAN;
    // BACK_COLOR = CLR_BACKGROUND;
    // LCD_ShowxNum(0, LINE1_Y, TIM8->CNT, 5, 12, 0x0);
    // LCD_ShowxNum(30, LINE1_Y, button1Count, 5, 12, 0x0);
    // LCD_ShowxNum(80, LINE1_Y, tim1Freq, 8, 12, 0x0);
    // LCD_ShowxNum(90, LINE2_Y, (u32) ADC_MeasureTime * FRAME_STEP, 7, 12, 0x0);
    // LCD_ShowxNum(120, LINE2_Y, (u32) ADC_param, 5, 12, 0x0);

    // POINT_COLOR = MAGENTA;
    // LCD_ShowxNum(200,  LINE2_Y, ADCElapsedTick, 10, 12, 0x0);

    delay_ms(30);
}

void UART_Transmit(const char *msg) {
    static char __ALIGNED(4) txBuffer[250];
    const uint32_t txBufferSize = strlen(msg);

    while (DMA1_0_busy){}
    DMA1_0_busy = 1;

    stpcpy(txBuffer, msg);
    // SCB_CleanDCache_by_Addr((uint32_t*)txBuffer, txBufferSize);

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    // Channel 1 = TX
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_1,
                           (uint32_t) txBuffer,
                           (uint32_t) &(USART1->TDR),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, txBufferSize);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_USART_EnableDMAReq_TX(USART1);
    LL_USART_EnableDirectionTx(USART1);
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
