#include <_main.h>

const uint16_t sin32[] = {
        0,
        40,
        157,
        346,
        601,
        911,
        1265,
        1649,
        2048,
        2447,
        2831,
        3185,
        3495,
        3750,
        3939,
        4056,
        4095,
        4056,
        3939,
        3750,
        3495,
        3185,
        2831,
        2447,
        2048,
        1649,
        1265,
        911,
        601,
        346,
        157,
        40
};

uint8_t sin32_2[32] = {
};


void DAC_Activate(void)
{
    /* Enable DAC channel */
    LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);

    /* Delay for DAC channel voltage settling time from DAC channel startup.    */
    /* Compute number of CPU cycles to wait for, from delay in us.              */
    /* Note: Variable divided by 2 to compensate partially                      */
    /*       CPU processing cycles (depends on compilation optimization).       */
    /* Note: If system core clock frequency is below 200kHz, wait time          */
    /*       is only a few CPU processing cycles.                               */
    __IO uint32_t wait_loop_index = ((LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US * (SystemCoreClock / (100000 * 2))) / 10);
    while(wait_loop_index != 0)
    {
        wait_loop_index--;
    }

    /* Enable DAC channel DMA request */
    LL_DAC_EnableDMAReq(DAC1, LL_DAC_CHANNEL_1);
    /* Enable interruption DAC channel1 underrun */
    LL_DAC_EnableIT_DMAUDR1(DAC1);
    /* Enable DAC channel trigger */
    /* Note: DAC channel conversion can start from trigger enable:              */
    /*       - if DAC channel trigger source is set to SW:                      */
    /*         DAC channel conversion will start after trig order               */
    /*         using function "LL_DAC_TrigSWConversion()".                      */
    /*       - if DAC channel trigger source is set to external trigger         */
    /*         (timer, ...):                                                    */
    /*         DAC channel conversion can start immediately                     */
    /*         (after next trig order from external trigger)                    */
    LL_DAC_EnableTrigger(DAC1, LL_DAC_CHANNEL_1);
}

void DAC_startSin() {
    for(int i=0; i<32; i++) {
        sin32_2[i] = (sin32[i]>>4);
    }
    /* Start tim4 ch1 */
    /* Enable output channel 1 */
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
    /* Enable counter */
    LL_TIM_EnableCounter(TIM4);
    /* Force update generation */
    LL_TIM_GenerateEvent_UPDATE(TIM4);

    /*##-2- Enable DAC selected channel and associated DMA #############################*/
    /* Set DMA transfer addresses of source and destination */
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2,
                           (uint32_t)&sin32_2,
                           LL_DAC_DMA_GetRegAddr(DAC1, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_8BITS_RIGHT_ALIGNED),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    /* Set DMA transfer size */
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, 32);
    /* Enable DMA transfer interruption: transfer error */
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

    DAC_Activate();
}
