#ifndef _DATABUFFER_H
#define _DATABUFFER_H

#include "lcd.h"

#define BUF_SIZE 256
extern u8 samplesBuffer[BUF_SIZE*2];

extern u8 firstHalf; // first or second half of buffer writing

#endif //_DATABUFFER_H
