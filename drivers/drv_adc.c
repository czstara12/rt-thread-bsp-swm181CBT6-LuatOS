/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtdevice.h>
#include <rtthread.h>
#include "board.h"
#include "drv_adc.h"

#ifdef RT_USING_ADC

#define SKT_ADC_DEVICE(adc_dev)    (struct skt_adc_dev *)(adc_dev)

static struct skt_adc_dev adc_dev0;

struct skt_adc_dev
{
    struct rt_adc_device parent;
    rt_uint32_t adc_periph;
    rt_uint32_t channel;
    rt_uint32_t chn_mask;
};

static rt_err_t skt_adc_enabled(struct rt_adc_device *device, rt_int8_t channel, rt_bool_t enabled)
{
    rt_err_t ret = RT_EOK;
    struct skt_adc_dev *adc = SKT_ADC_DEVICE(device);
    rt_uint32_t chn;

    RT_ASSERT(adc != RT_NULL);
    if (channel < 0 || channel > 7)
    {
        return -RT_EINVAL;
    }
    chn = 1U << channel;

    if (enabled)
    {
        adc->chn_mask |= chn;
        ADC_ChnSelect((ADC_TypeDef *)adc->adc_periph, adc->chn_mask);
        ADC_Open((ADC_TypeDef *)adc->adc_periph);
    }
    else
    {
        adc->chn_mask &= ~chn;
        ADC_ChnSelect((ADC_TypeDef *)adc->adc_periph, adc->chn_mask);
        if (adc->chn_mask == 0)
        {
            ADC_Close((ADC_TypeDef *)adc->adc_periph);
        }
    }
    return ret;
}
static rt_err_t skt_adc_convert(struct rt_adc_device *device, rt_int8_t channel, rt_uint32_t *value)
{
    rt_err_t ret = RT_EOK;
    struct skt_adc_dev *adc = SKT_ADC_DEVICE(device);
    rt_uint32_t chn;
    ADC_TypeDef *ADCx;
    rt_uint32_t timeout = 0x100000;

    RT_ASSERT(adc != RT_NULL);
    if (channel < 0 || channel > 7)
    {
        return -RT_EINVAL;
    }
    chn = 1U << channel;
    ADCx = (ADC_TypeDef *)adc->adc_periph;
    RT_ASSERT(ADCx != RT_NULL);

    if (value)
    {
        ADC_ChnSelect(ADCx, chn);
        ADC_Open(ADCx);
        ADC_Start(ADCx);
        while (!ADC_IsEOC(ADCx, chn) && timeout--)
        {
        }
        *value = ADC_Read(ADCx, chn);
    }

    return ret;
}

const static struct rt_adc_ops skt_adc_ops =
{
    skt_adc_enabled,
    skt_adc_convert
};

int rt_hw_adc_init(void)
{
    rt_err_t ret = RT_EOK;
    ADC_InitStructure init_struct;

    adc_dev0.adc_periph = (rt_uint32_t)ADC;
    adc_dev0.chn_mask = 0;

    init_struct.clk_src = ADC_CLKSRC_HRC_DIV4;
    init_struct.channels = 0;
    init_struct.samplAvg = ADC_AVG_SAMPLE1;
    init_struct.trig_src = ADC_TRIGSRC_SW;
    init_struct.Continue = 0;
    init_struct.EOC_IEn = 0;
    init_struct.OVF_IEn = 0;
    ADC_Init((ADC_TypeDef *)adc_dev0.adc_periph, &init_struct);
    ADC_Open((ADC_TypeDef *)adc_dev0.adc_periph);

    ret = rt_hw_adc_register(&adc_dev0.parent, "adc0", &skt_adc_ops, &adc_dev0);

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_adc_init);

#endif /* RT_USING_ADC */
