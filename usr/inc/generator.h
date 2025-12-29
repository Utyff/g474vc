#ifndef F7_FMC_GENERATOR_H
#define F7_FMC_GENERATOR_H

extern uint32_t tim1Prescaler;
extern uint32_t tim1Period;
extern uint32_t tim1Pulse;
extern uint32_t tim1Freq;
extern uint32_t tim1Freq2;

void GEN_step(int16_t step);

void GEN_setParams();

#endif //F7_FMC_GENERATOR_H
