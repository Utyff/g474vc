#ifndef F7_FMC_ADC_H
#define F7_FMC_ADC_H

extern uint32_t ADCStartTick;         // time when start ADC buffer fill
extern uint32_t ADCHalfElapsedTick;   // the last time half buffer fill
extern uint32_t ADCElapsedTick;       // the last time buffer fill

void ADC_start();
void ADC_step(int16_t step);

#endif //F7_FMC_ADC_H
