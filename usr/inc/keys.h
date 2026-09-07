#ifndef KEYS_H
#define KEYS_H

#include "_main.h"

// buttons bits
#define BUTTON1 0x01
#define BUTTON2 0x02
#define BUTTON3 0x04
#define BUTTON4 0x08
#define BUTTON5 0x10
#define BUTTON6 0x20

extern uint8_t button1Count;
extern uint8_t button2Count;
extern uint8_t button3Count;
extern uint16_t btns_state;

void KEYS_init();
void KEYS_scan();
int16_t ENC1_Get();
int16_t ENC2_Get();

#endif //KEYS_H
