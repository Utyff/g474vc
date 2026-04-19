#ifndef MY_MAIN_H
#define MY_MAIN_H

#include "main.h"

typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef __IO uint32_t vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t vu8;

extern uint8_t DMA1_0_busy;

void mainInitialize();

void mainCycle();

void UART_Transmit(const char *msg);

#define DEBUG_TRACE_UART

#if defined(DEBUG_TRACE_SWO)

void SWO_Trace(uint8_t* msg);
#define DBG_Trace(msg) SWO_Trace((uint8_t*)(msg))

#elif defined(DEBUG_TRACE_UART)

#include "string.h"

#define DBG_Trace(msg) UART_Transmit(msg)

#elif defined(DEBUG_TRACE_NONE)

#define DBG_Trace(msg)

#else
#error Define DUBUG_TRACE destination
#endif

#endif //MY_MAIN_H
