/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_gpio.h"

#ifdef RT_USING_PIN

static void swm181_pin_mode(struct rt_device *device, rt_base_t pin, rt_uint8_t mode)
{
    int dir = 0;
    int pull_up = 0;
    int pull_down = 0;
    int open_drain = 0;

    /* Configure GPIO_InitStructure */
    switch (mode)
    {
    case PIN_MODE_OUTPUT:
        /* output setting */
        dir = 1;
        break;
    case PIN_MODE_INPUT:
        /* input setting: not pull. */
        dir = 0;
        break;
    case PIN_MODE_INPUT_PULLUP:
        /* input setting: pull up. */
        dir = 0;
        pull_up = 1;
        break;
    case PIN_MODE_INPUT_PULLDOWN:
        /* input setting: pull down. */
        dir = 0;
        pull_down = 1;
        break;
    case PIN_MODE_OUTPUT_OD:
        /* output setting: od. */
        dir = 1;
        open_drain = 1;
        break;
    }

    GPIO_Init(GPIOB, pin, dir, pull_up, pull_down, open_drain);
}

static void swm181_pin_write(struct rt_device *device, rt_base_t pin, rt_uint8_t value)
{
    if (value)
    {
        GPIO_AtomicSetBit(GPIOB, pin);
    }
    else
    {
        GPIO_AtomicClrBit(GPIOB, pin);
    }
}

static int swm181_pin_read(struct rt_device *device, rt_base_t pin)
{
    return (int)GPIO_GetBit(GPIOB, pin);
}

static rt_err_t swm181_pin_attach_irq(struct rt_device *device, rt_base_t pin,
                                      rt_uint8_t mode, void (*hdr)(void *args), void *args)
{
    rt_err_t ret = RT_EOK;

    /* Todo:attach hdr to pin ISR */

    return ret;
}

static rt_err_t swm181_pin_detach_irq(struct rt_device *device, rt_base_t pin)
{
    rt_err_t ret = RT_EOK;

    /* Todo:detach hdr from pin ISR */

    return ret;
}

static rt_err_t swm181_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint8_t enabled)
{
    rt_err_t ret = RT_EOK;

    /* Todo:enable pin ISR */

    return ret;
}

const static struct rt_pin_ops swm181_pin_ops =
    {
        swm181_pin_mode,
        swm181_pin_write,
        swm181_pin_read,

        swm181_pin_attach_irq,
        swm181_pin_detach_irq,
        swm181_pin_irq_enable};

int rt_hw_pin_init(void)
{
    rt_err_t ret = RT_EOK;

    ret = rt_device_pin_register("pin", &swm181_pin_ops, RT_NULL);

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_pin_init);

#endif /* RT_USING_PIN */
