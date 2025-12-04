#include <stdlib.h>
#include <stdio.h>
#include <_main.h>
#include <delay.h>
#include <draw.h>
#include <keys.h>
#include <DataBuffer.h>
#include <generator.h>
#include <adc.h>


void CORECheck();

void FPUCheck();

extern int ii;
extern float time;


void mainInitialize() {
    DWT_Init();
    LCD_Init();

    for (int i = 0; i < BUF_SIZE * 2; i++) {
        samplesBuffer[i] = i;
    }
//    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) samplesBuffer, BUF_SIZE);
//    ADC_setParams();

//    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    //GEN_setParams();

//    HAL_TIM_Encoder_Start(&htim8, TIM_CHANNEL_1);
//    KEYS_init();
    //initScreenBuf();

//    CORECheck();
//    FPUCheck();
}

static u16 color = 1;

void mainCycle() {
//    drawScreen();
//    KEYS_scan();

    if ((random() & 7) < 1) HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
#ifdef LED2_Pin
    if ((random() & 7) < 1) HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
#endif
//    if ((random() & 7) < 1) HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    if (HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin)==GPIO_PIN_SET ) {
        HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
    }

//    POINT_COLOR = WHITE;
//    BACK_COLOR = BLACK;
//    LCD_ShowxNum(0, 214, TIM8->CNT, 5, 12, 0x01);
//    LCD_ShowxNum(30, 214, (u32) button1Count, 5, 12, 0x01);
//    LCD_ShowxNum(60, 214, (u32) ii, 5, 12, 0x01);
//    LCD_ShowxNum(90, 214, (u32) time / 10, 5, 12, 0x01);
//    LCD_ShowxNum(120, 214, (u32) firstHalf, 5, 12, 0x01);

    u32 t0 = DWT_Get_Current_Tick();
    LCD_Clear(color);
    u32 ticks = DWT_Elapsed_Tick(t0);
    POINT_COLOR = YELLOW;
    LCD_ShowxNum(130, 227, ticks / DWT_IN_MICROSEC, 8, 12, 9);
//    t00 = t0;
//    ticks0 = ticks;

    color = color << 1;
    if (color == 0) {
        color = 1;
    }
    POINT_COLOR = BLACK;
    LCD_ShowxNum(0, 214, color, 10, 12, 0x0);
    delay_ms(300);
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
        case 0x10110021 :
            sprintf(buf, "FPU-S Single-precision only\n");
            break;
        case 0x10110221 :
            sprintf(buf, "FPU-D Single-precision and Double-precision\n");
            break;
        default :
            sprintf(buf, "Unknown FPU");
    }
    DBG_Trace(buf);
}

void CORECheck(void) {
    char buf[120];
    uint32_t cpuid = SCB->CPUID;
    uint32_t var, pat;

    sprintf(buf, "\n\nCPUID %08X DEVID %03X DEVREV %03X\n", cpuid, DBGMCU->IDCODE & 0xFFF, DBGMCU->IDCODE >> 16);
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

            default :
                sprintf(buf, "Unknown CORE");
        }
    } else
        sprintf(buf, "Unknown CORE IMPLEMENTER");
    DBG_Trace(buf);
}
