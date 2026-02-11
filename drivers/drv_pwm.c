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
#include <board.h>
#include "drv_pwm.h"

#ifdef RT_USING_PWM

#define SKT_PWM_DEVICE(pwm)    (struct skt_pwm_dev *)(pwm)

static struct skt_pwm_dev pwm_dev0;
static struct skt_pwm_dev pwm_dev1;

struct skt_pwm_dev
{
    struct rt_device_pwm parent;
    rt_uint32_t pwm_periph;
};

static rt_uint8_t skt_pwm_clkdiv(rt_uint32_t pwm_clk, rt_uint32_t period_ns, rt_uint16_t *cycle)
{
    rt_uint32_t div;
    rt_uint32_t best_div = 1;
    rt_uint32_t tick_hz;
    rt_uint64_t ticks;
    rt_uint8_t div_enum = PWM_CLKDIV_1;

    if (period_ns == 0)
    {
        *cycle = 0;
        return PWM_CLKDIV_1;
    }

    for (div_enum = PWM_CLKDIV_1; div_enum <= PWM_CLKDIV_128; div_enum++)
    {
        div = 1U << div_enum;
        tick_hz = pwm_clk / div;
        ticks = ((rt_uint64_t)tick_hz * (rt_uint64_t)period_ns) / 1000000000ULL;
        if (ticks == 0)
        {
            ticks = 1;
        }
        if (ticks <= 0xFFFFU)
        {
            best_div = div;
            *cycle = (rt_uint16_t)ticks;
            return div_enum;
        }
    }

    best_div = 128;
    tick_hz = pwm_clk / best_div;
    ticks = ((rt_uint64_t)tick_hz * (rt_uint64_t)period_ns) / 1000000000ULL;
    if (ticks == 0)
    {
        ticks = 1;
    }
    if (ticks > 0xFFFFU)
    {
        ticks = 0xFFFFU;
    }
    *cycle = (rt_uint16_t)ticks;
    return PWM_CLKDIV_128;
}

static rt_err_t skt_pwm_enable(rt_uint32_t pwm_periph, struct rt_pwm_configuration *cfg, rt_bool_t enable)
{
    PWM_TypeDef *PWMx = (PWM_TypeDef *)pwm_periph;
    rt_uint32_t chA = 0;
    rt_uint32_t chB = 0;

    RT_ASSERT(PWMx != RT_NULL);

    if (cfg->channel == 0)
    {
        chA = 1;
    }
    else if (cfg->channel == 1)
    {
        chB = 1;
    }
    else
    {
        return -RT_EINVAL;
    }

    if (enable)
    {
        PWM_Start(PWMx, chA, chB);
    }
    else
    {
        PWM_Stop(PWMx, chA, chB);
    }

    return RT_EOK;
}

static rt_err_t skt_pwm_get(rt_uint32_t pwm_periph, struct rt_pwm_configuration *cfg)
{
    PWM_TypeDef *PWMx = (PWM_TypeDef *)pwm_periph;
    rt_uint16_t cycle;
    rt_uint16_t hduty;
    rt_uint32_t chn;
    rt_uint32_t tick_hz;
    rt_uint32_t div;

    RT_ASSERT(PWMx != RT_NULL);

    if (cfg->channel == 0)
    {
        chn = PWM_CH_A;
    }
    else if (cfg->channel == 1)
    {
        chn = PWM_CH_B;
    }
    else
    {
        return -RT_EINVAL;
    }

    div = 1U << (PWMG->CLKDIV & 0x7);
    tick_hz = SystemCoreClock / div;
    cycle = PWM_GetCycle(PWMx, chn);
    hduty = PWM_GetHDuty(PWMx, chn);

    cfg->period = (rt_uint32_t)(((rt_uint64_t)cycle * 1000000000ULL) / tick_hz);
    cfg->pulse = (rt_uint32_t)(((rt_uint64_t)hduty * 1000000000ULL) / tick_hz);

    return RT_EOK;
}

static rt_err_t skt_pwm_set(rt_uint32_t pwm_periph, struct rt_pwm_configuration *cfg)
{
    PWM_TypeDef *PWMx = (PWM_TypeDef *)pwm_periph;
    rt_uint16_t cycle = 0;
    rt_uint16_t hduty = 0;
    rt_uint32_t chn;
    rt_uint32_t tick_hz;
    rt_uint32_t div;
    rt_uint8_t div_enum;

    RT_ASSERT(PWMx != RT_NULL);

    if (cfg->period == 0 || cfg->pulse > cfg->period)
    {
        return -RT_EINVAL;
    }

    if (cfg->channel == 0)
    {
        chn = PWM_CH_A;
    }
    else if (cfg->channel == 1)
    {
        chn = PWM_CH_B;
    }
    else
    {
        return -RT_EINVAL;
    }

    div_enum = skt_pwm_clkdiv(SystemCoreClock, cfg->period, &cycle);
    PWMG->CLKDIV = div_enum;
    div = 1U << div_enum;
    tick_hz = SystemCoreClock / div;

    hduty = (rt_uint16_t)(((rt_uint64_t)tick_hz * (rt_uint64_t)cfg->pulse) / 1000000000ULL);
    if (hduty > cycle)
    {
        hduty = cycle;
    }

    PWM_SetCycle(PWMx, chn, cycle);
    PWM_SetHDuty(PWMx, chn, hduty);

    return RT_EOK;
}

static rt_err_t skt_pwm_control(struct rt_device_pwm *device, int cmd, void *arg)
{
    rt_err_t ret = RT_EOK;
    struct skt_pwm_dev *pwm = SKT_PWM_DEVICE(device->parent.user_data);
    struct rt_pwm_configuration *cfg = (struct rt_pwm_configuration *)arg;

    RT_ASSERT(pwm != RT_NULL);

    switch (cmd)
    {
    case PWM_CMD_ENABLE:

        ret = skt_pwm_enable(pwm->pwm_periph, cfg, RT_TRUE);
        break;
    case PWM_CMD_DISABLE:

        ret = skt_pwm_enable(pwm->pwm_periph, cfg, RT_FALSE);
        break;
    case PWM_CMD_SET:

        ret = skt_pwm_set(pwm->pwm_periph, cfg);
        break;
    case PWM_CMD_GET:

        ret = skt_pwm_get(pwm->pwm_periph, cfg);
        break;
    default:
        ret = RT_EINVAL;
        break;
    }

    return ret;
}

const static struct rt_pwm_ops skt_pwm_ops =
{
    skt_pwm_control
};

int rt_hw_pwm_init(void)
{
    rt_err_t ret = RT_EOK;
    PWM_InitStructure init_struct;

    init_struct.clk_div = PWM_CLKDIV_1;
    init_struct.mode = PWM_MODE_INDEP;
    init_struct.cycleA = 1;
    init_struct.hdutyA = 0;
    init_struct.deadzoneA = 0;
    init_struct.initLevelA = 0;
    init_struct.cycleB = 1;
    init_struct.hdutyB = 0;
    init_struct.deadzoneB = 0;
    init_struct.initLevelB = 0;
    init_struct.HEndAIEn = 0;
    init_struct.NCycleAIEn = 0;
    init_struct.HEndBIEn = 0;
    init_struct.NCycleBIEn = 0;

    pwm_dev0.pwm_periph = (rt_uint32_t)PWM0;
    PWM_Init((PWM_TypeDef *)pwm_dev0.pwm_periph, &init_struct);
    ret = rt_device_pwm_register(&pwm_dev0.parent, "pwm0", &skt_pwm_ops, &pwm_dev0);
    if (ret != RT_EOK)
    {
        return ret;
    }

    pwm_dev1.pwm_periph = (rt_uint32_t)PWM1;
    PWM_Init((PWM_TypeDef *)pwm_dev1.pwm_periph, &init_struct);
    ret = rt_device_pwm_register(&pwm_dev1.parent, "pwm1", &skt_pwm_ops, &pwm_dev1);

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_pwm_init);

#endif /* RT_USING_PWM */
