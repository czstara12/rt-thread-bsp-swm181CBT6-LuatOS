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
#include <board.h>
#include "drv_pwm.h"
#include "drv_gpio.h"

#ifdef RT_USING_PWM

struct swm181_pwm
{
    struct rt_device_pwm parent;
    PWM_TypeDef *PWMx;
    const char *name;
    uint32_t ch1_pin;
    uint32_t ch2_pin;
};

static rt_uint8_t swm181_pwm_clkdiv(rt_uint32_t pwm_clk, rt_uint32_t period_ns, rt_uint16_t *cycle)
{
    rt_uint32_t div;
    rt_uint64_t ticks;
    rt_uint8_t div_enum;

    if (period_ns == 0)
    {
        *cycle = 0;
        return PWM_CLKDIV_1;
    }

    for (div_enum = PWM_CLKDIV_1; div_enum <= PWM_CLKDIV_128; div_enum++)
    {
        div = 1U << div_enum;
        ticks = ((rt_uint64_t)(pwm_clk / div) * (rt_uint64_t)period_ns) / 1000000000ULL;
        if (ticks <= 0xFFFFU)
        {
            *cycle = (rt_uint16_t)ticks;
            return div_enum;
        }
    }

    *cycle = 0xFFFFU;
    return PWM_CLKDIV_128;
}

static rt_err_t swm181_pwm_control(struct rt_device_pwm *device, int cmd, void *arg)
{
    struct swm181_pwm *pwm = (struct swm181_pwm *)device->parent.user_data;
    struct rt_pwm_configuration *cfg = (struct rt_pwm_configuration *)arg;
    rt_uint32_t chn;
    
    if (cfg->channel == 1) chn = PWM_CH_A;
    else if (cfg->channel == 2) chn = PWM_CH_B;
    else return -RT_EINVAL;

    switch (cmd)
    {
    case PWM_CMD_ENABLE:
        if (cfg->channel == 1) PWM_Start(pwm->PWMx, 1, 0);
        else PWM_Start(pwm->PWMx, 0, 1);
        break;
    case PWM_CMD_DISABLE:
        if (cfg->channel == 1) PWM_Stop(pwm->PWMx, 1, 0);
        else PWM_Stop(pwm->PWMx, 0, 1);
        break;
    case PWM_CMD_SET:
    {
        rt_uint16_t cycle = 0;
        rt_uint16_t hduty = 0;
        rt_uint8_t div_enum;
        
        div_enum = swm181_pwm_clkdiv(SystemCoreClock, cfg->period, &cycle);
        PWMG->CLKDIV = div_enum;
        
        hduty = (rt_uint16_t)(((rt_uint64_t)(SystemCoreClock / (1U << div_enum)) * (rt_uint64_t)cfg->pulse) / 1000000000ULL);
        if (hduty > cycle) hduty = cycle;

        PWM_SetCycle(pwm->PWMx, chn, cycle);
        PWM_SetHDuty(pwm->PWMx, chn, hduty);
        break;
    }
    case PWM_CMD_GET:
    {
        rt_uint32_t div = 1U << (PWMG->CLKDIV & 0x7);
        rt_uint32_t tick_hz = SystemCoreClock / div;
        rt_uint16_t cycle = PWM_GetCycle(pwm->PWMx, chn);
        rt_uint16_t hduty = PWM_GetHDuty(pwm->PWMx, chn);

        cfg->period = (rt_uint32_t)(((rt_uint64_t)cycle * 1000000000ULL) / tick_hz);
        cfg->pulse = (rt_uint32_t)(((rt_uint64_t)hduty * 1000000000ULL) / tick_hz);
        break;
    }
    default:
        return -RT_EINVAL;
    }

    return RT_EOK;
}

static const struct rt_pwm_ops swm181_pwm_ops =
{
    swm181_pwm_control
};

static struct swm181_pwm pwm_objs[] = {
#ifdef BSP_USING_PWM0
    { .PWMx = PWM0, .name = "pwm0", .ch1_pin = BSP_PWM0_CH1_PIN, .ch2_pin = BSP_PWM0_CH2_PIN },
#endif
#ifdef BSP_USING_PWM1
    { .PWMx = PWM1, .name = "pwm1", .ch1_pin = BSP_PWM1_CH1_PIN, .ch2_pin = BSP_PWM1_CH2_PIN },
#endif
#ifdef BSP_USING_PWM2
    { .PWMx = PWM2, .name = "pwm2", .ch1_pin = BSP_PWM2_CH1_PIN, .ch2_pin = BSP_PWM2_CH2_PIN },
#endif
#ifdef BSP_USING_PWM3
    { .PWMx = PWM3, .name = "pwm3", .ch1_pin = BSP_PWM3_CH1_PIN, .ch2_pin = BSP_PWM3_CH2_PIN },
#endif
};

int rt_hw_pwm_init(void)
{
    PWM_InitStructure init_struct;
    int i;

    init_struct.clk_div = PWM_CLKDIV_1;
    init_struct.mode = PWM_MODE_INDEP;
    init_struct.cycleA = 1000;
    init_struct.hdutyA = 0;
    init_struct.deadzoneA = 0;
    init_struct.initLevelA = 0;
    init_struct.cycleB = 1000;
    init_struct.hdutyB = 0;
    init_struct.deadzoneB = 0;
    init_struct.initLevelB = 0;
    init_struct.HEndAIEn = 0;
    init_struct.NCycleAIEn = 0;
    init_struct.HEndBIEn = 0;
    init_struct.NCycleBIEn = 0;

    for (i = 0; i < sizeof(pwm_objs) / sizeof(pwm_objs[0]); i++)
    {
        struct swm181_pwm *pwm = &pwm_objs[i];
        uint32_t funcA = 0, funcB = 0;

        if (pwm->PWMx == PWM0) { funcA = FUNMUX_PWM0A_OUT; funcB = FUNMUX_PWM0B_OUT; }
        else if (pwm->PWMx == PWM1) { funcA = FUNMUX_PWM1A_OUT; funcB = FUNMUX_PWM1B_OUT; }
        else if (pwm->PWMx == PWM2) { funcA = FUNMUX_PWM2A_OUT; funcB = FUNMUX_PWM2B_OUT; }
        else if (pwm->PWMx == PWM3) { funcA = FUNMUX_PWM3A_OUT; funcB = FUNMUX_PWM3B_OUT; }

        PORT_Init(SWM181_PIN_GET_PORT_PTR(pwm->ch1_pin), SWM181_PIN_GET_PIN_IDX(pwm->ch1_pin), funcA, 0);
        PORT_Init(SWM181_PIN_GET_PORT_PTR(pwm->ch2_pin), SWM181_PIN_GET_PIN_IDX(pwm->ch2_pin), funcB, 0);

        PWM_Init(pwm->PWMx, &init_struct);
        rt_device_pwm_register(&pwm->parent, pwm->name, &swm181_pwm_ops, pwm);
    }

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_pwm_init);

#endif /* RT_USING_PWM */
