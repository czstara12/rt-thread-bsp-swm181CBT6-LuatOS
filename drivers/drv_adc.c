/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-12     Gemini       first version
 */

#include <rtdevice.h>
#include <rtthread.h>
#include "board.h"
#include "drv_adc.h"

#ifdef RT_USING_ADC

struct swm181_adc
{
    struct rt_adc_device parent;
    ADC_TypeDef *ADCx;
    rt_uint32_t chn_mask;
};

static struct swm181_adc adc_obj;

static rt_err_t swm181_adc_enabled(struct rt_adc_device *device, rt_int8_t channel, rt_bool_t enabled)
{
    struct swm181_adc *adc = (struct swm181_adc *)device;
    
    if (channel < 0 || channel > 7)
        return -RT_EINVAL;

    if (enabled)
    {
        switch (channel)
        {
        case 0: PORT_Init(PORTE, PIN4, PORTE_PIN4_ADC_CH0, 0); break;
        case 1: PORT_Init(PORTA, PIN15, PORTA_PIN15_ADC_CH1, 0); break;
        case 2: PORT_Init(PORTA, PIN14, PORTA_PIN14_ADC_CH2, 0); break;
        case 3: PORT_Init(PORTA, PIN13, PORTA_PIN13_ADC_CH3, 0); break;
        case 4: PORT_Init(PORTA, PIN12, PORTA_PIN12_ADC_CH4, 0); break;
        case 5: PORT_Init(PORTC, PIN7, PORTC_PIN7_ADC_CH5, 0); break;
        case 6: PORT_Init(PORTC, PIN6, PORTC_PIN6_ADC_CH6, 0); break;
        // Channel 7 might be internal
        }
        
        adc->chn_mask |= (1U << channel);
        ADC_ChnSelect(adc->ADCx, adc->chn_mask);
        ADC_Open(adc->ADCx);
    }
    else
    {
        adc->chn_mask &= ~(1U << channel);
        ADC_ChnSelect(adc->ADCx, adc->chn_mask);
        if (adc->chn_mask == 0)
            ADC_Close(adc->ADCx);
    }
    
    return RT_EOK;
}

static rt_err_t swm181_adc_convert(struct rt_adc_device *device, rt_int8_t channel, rt_uint32_t *value)
{
    struct swm181_adc *adc = (struct swm181_adc *)device;
    rt_uint32_t chn_bit;
    rt_uint32_t timeout = 0x10000;

    if (channel < 0 || channel > 7)
        return -RT_EINVAL;

    chn_bit = (1U << channel);
    
    if (value)
    {
        ADC_ChnSelect(adc->ADCx, chn_bit);
        ADC_Open(adc->ADCx);
        ADC_Start(adc->ADCx);
        while (!ADC_IsEOC(adc->ADCx, chn_bit) && timeout--);
        if (timeout == 0) return -RT_ETIMEOUT;
        *value = ADC_Read(adc->ADCx, chn_bit);
    }

    return RT_EOK;
}

static const struct rt_adc_ops swm181_adc_ops =
{
    swm181_adc_enabled,
    swm181_adc_convert
};

int rt_hw_adc_init(void)
{
    ADC_InitStructure init_struct;

    adc_obj.ADCx = ADC;
    adc_obj.chn_mask = 0;

    init_struct.clk_src = ADC_CLKSRC_HRC_DIV4;
    init_struct.channels = 0;
    init_struct.samplAvg = ADC_AVG_SAMPLE1;
    init_struct.trig_src = ADC_TRIGSRC_SW;
    init_struct.Continue = 0;
    init_struct.EOC_IEn = 0;
    init_struct.OVF_IEn = 0;
    
    ADC_Init(adc_obj.ADCx, &init_struct);

    return rt_hw_adc_register(&adc_obj.parent, "adc0", &swm181_adc_ops, RT_NULL);
}
INIT_DEVICE_EXPORT(rt_hw_adc_init);

#endif /* RT_USING_ADC */
