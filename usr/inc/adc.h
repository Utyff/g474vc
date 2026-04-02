#ifndef H750_ADC_H
#define H750_ADC_H

extern uint8_t  ADCworks;
extern uint32_t ADCStartTick;         // time when start ADC buffer fill
extern uint32_t ADCHalfElapsedTick;   // the last time half buffer fill
extern uint32_t ADCElapsedTick;       // the last time buffer fill
extern float    ADC_MeasureTime;

void ADC_start();
void ADC_step(int16_t step);

#endif //H750_ADC_H
