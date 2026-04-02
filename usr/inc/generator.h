#ifndef H750_GENERATOR_H
#define H750_GENERATOR_H

extern uint32_t tim1Prescaler;
extern uint32_t tim1Period;
extern uint32_t tim1Pulse;
extern uint32_t tim1Freq;


void GEN_step(int16_t step);

void GEN_setParams();

#endif //H750_GENERATOR_H
