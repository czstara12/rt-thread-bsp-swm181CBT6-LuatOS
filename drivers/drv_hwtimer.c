/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-12     Gemini       first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "drv_hwtimer.h"

#ifdef RT_USING_HWTIMER

struct swm181_hwtimer
{
    rt_hwtimer_t parent;
    TIMR_TypeDef *TIMRx;
    IRQn_Type irqn;
    uint32_t periph_irq;
    const char *name;
};

static rt_err_t swm181_hwtimer_control(rt_hwtimer_t *timer, rt_uint32_t cmd, void *args)
{
    struct swm181_hwtimer *hwtimer = (struct swm181_hwtimer *)timer;

    switch (cmd)
    {
    case HWTIMER_CTRL_FREQ_SET:
        // SWM181 timers run at SystemCoreClock. Fixed frequency.
        return -RT_ERROR;
    case HWTIMER_CTRL_STOP:
        TIMR_Stop(hwtimer->TIMRx);
        break;
    case HWTIMER_CTRL_MODE_SET:
        // Modes are handled in start
        break;
    }
    return RT_EOK;
}

static rt_uint32_t swm181_hwtimer_count_get(rt_hwtimer_t *timer)
{
    struct swm181_hwtimer *hwtimer = (struct swm181_hwtimer *)timer;
    // SWM181 timer is down counter
    return TIMR_GetPeriod(hwtimer->TIMRx) - TIMR_GetCurValue(hwtimer->TIMRx);
}

static void swm181_hwtimer_init(rt_hwtimer_t *timer, rt_uint32_t state)
{
    struct swm181_hwtimer *hwtimer = (struct swm181_hwtimer *)timer;
    if (state)
    {
        TIMR_Init(hwtimer->TIMRx, TIMR_MODE_TIMER, 0xFFFFFFFF, 1);
        IRQ_Connect(hwtimer->periph_irq, hwtimer->irqn, 1);
        NVIC_EnableIRQ(hwtimer->irqn);
    }
    else
    {
        TIMR_Stop(hwtimer->TIMRx);
        NVIC_DisableIRQ(hwtimer->irqn);
    }
}

static rt_err_t swm181_hwtimer_start(rt_hwtimer_t *timer, rt_uint32_t cnt, rt_hwtimer_mode_t mode)
{
    struct swm181_hwtimer *hwtimer = (struct swm181_hwtimer *)timer;
    
    TIMR_Stop(hwtimer->TIMRx);
    TIMR_SetPeriod(hwtimer->TIMRx, cnt);
    TIMR_INTClr(hwtimer->TIMRx);
    TIMR_Start(hwtimer->TIMRx);

    return RT_EOK;
}

static void swm181_hwtimer_stop(rt_hwtimer_t *timer)
{
    struct swm181_hwtimer *hwtimer = (struct swm181_hwtimer *)timer;
    TIMR_Stop(hwtimer->TIMRx);
}

static const struct rt_hwtimer_ops swm181_hwtimer_ops =
{
    swm181_hwtimer_init,
    swm181_hwtimer_start,
    swm181_hwtimer_stop,
    swm181_hwtimer_count_get,
    swm181_hwtimer_control
};

static struct swm181_hwtimer hwtimer_objs[] = {
#ifdef BSP_USING_HWTIMER0
    { .TIMRx = TIMR0, .irqn = IRQ5_IRQ, .periph_irq = IRQ0_15_TIMR0, .name = "timer0" },
#endif
#ifdef BSP_USING_HWTIMER1
    { .TIMRx = TIMR1, .irqn = IRQ6_IRQ, .periph_irq = IRQ0_15_TIMR1, .name = "timer1" },
#endif
#ifdef BSP_USING_HWTIMER2
    { .TIMRx = TIMR2, .irqn = IRQ7_IRQ, .periph_irq = IRQ0_15_TIMR2, .name = "timer2" },
#endif
#ifdef BSP_USING_HWTIMER3
    { .TIMRx = TIMR3, .irqn = IRQ8_IRQ, .periph_irq = IRQ0_15_TIMR3, .name = "timer3" },
#endif
};

#ifdef BSP_USING_HWTIMER0
void IRQ5_Handler(void)
{
    rt_interrupt_enter();
    TIMR_INTClr(TIMR0);
    rt_device_hwtimer_isr(&hwtimer_objs[0].parent);
    rt_interrupt_leave();
}
#endif

#ifdef BSP_USING_HWTIMER1
void IRQ6_Handler(void)
{
    rt_interrupt_enter();
    TIMR_INTClr(TIMR1);
    int idx = 0;
#ifdef BSP_USING_HWTIMER0
    idx++;
#endif
    rt_device_hwtimer_isr(&hwtimer_objs[idx].parent);
    rt_interrupt_leave();
}
#endif

int rt_hw_hwtimer_init(void)
{
    int i;
    
    for (i = 0; i < sizeof(hwtimer_objs) / sizeof(hwtimer_objs[0]); i++)
    {
        hwtimer_objs[i].parent.info = &(struct rt_hwtimer_info){SystemCoreClock, SystemCoreClock, 0xFFFFFFFF, HWTIMER_CNTMODE_DW};
        hwtimer_objs[i].parent.ops  = &swm181_hwtimer_ops;
        rt_device_hwtimer_register(&hwtimer_objs[i].parent, hwtimer_objs[i].name, &hwtimer_objs[i]);
    }

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_hwtimer_init);

#endif /* RT_USING_HWTIMER */
