#ifndef H750_DAC_H
#define H750_DAC_H

extern uint32_t autoreload;

void DAC_startTriangle();
void DAC_startSin();
void DAC_step(int16_t);

#endif //H750_DAC_H
