/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"
#include "drv_gpio.h"

#ifdef RT_USING_PIN

#define SWM181_PIN_GET_PORT(pin)  ((GPIO_TypeDef *)(GPIOA_BASE + ((pin) >> 4) * 0x1000))
#define SWM181_PIN_GET_PIN(pin)   ((pin) & 0x0F)

struct swm181_pin_irq
{
    rt_base_t pin;
    rt_uint8_t mode;
    void (*hdr)(void *args);
    void *args;
};

static struct swm181_pin_irq pin_irq_table[16 * 5];
static rt_uint8_t port_irq_refcnt[5];

static rt_int32_t swm181_pin_to_index(rt_base_t pin)
{
    rt_int32_t port = (pin >> 4) & 0xFF;
    rt_int32_t bit = pin & 0x0F;

    if (port < 0 || port >= 5)
    {
        return -1;
    }

    return port * 16 + bit;
}

static rt_uint32_t swm181_exti_mode(rt_uint8_t mode)
{
    switch (mode)
    {
    case PIN_IRQ_MODE_RISING:
        return EXTI_RISE_EDGE;
    case PIN_IRQ_MODE_FALLING:
        return EXTI_FALL_EDGE;
    case PIN_IRQ_MODE_RISING_FALLING:
        return EXTI_BOTH_EDGE;
    case PIN_IRQ_MODE_HIGH_LEVEL:
        return EXTI_HIGH_LEVEL;
    case PIN_IRQ_MODE_LOW_LEVEL:
        return EXTI_LOW_LEVEL;
    default:
        return EXTI_FALL_EDGE;
    }
}

static IRQn_Type swm181_port_irqn(rt_uint8_t port)
{
    switch (port)
    {
    case 0:
        return (IRQn_Type)IRQ16_IRQ;
    case 1:
        return (IRQn_Type)IRQ17_IRQ;
    case 2:
        return (IRQn_Type)IRQ18_IRQ;
    case 3:
        return (IRQn_Type)IRQ19_IRQ;
    case 4:
        return (IRQn_Type)IRQ20_IRQ;
    default:
        return (IRQn_Type)IRQ20_IRQ;
    }
}

static rt_uint32_t swm181_port_periph_irqsrc(rt_uint8_t port)
{
    switch (port)
    {
    case 0:
        return IRQ16_31_GPIOA;
    case 1:
        return IRQ16_31_GPIOB;
    case 2:
        return IRQ16_31_GPIOC;
    case 3:
        return IRQ16_31_GPIOD;
    case 4:
        return IRQ16_31_GPIOE;
    default:
        return IRQ16_31_GPIOA;
    }
}

static void swm181_gpio_port_isr(rt_uint8_t port)
{
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(GPIOA_BASE + port * 0x1000);
    rt_uint32_t pending;
    rt_uint32_t i;

    pending = gpio_port->INTSTAT & gpio_port->INTEN;
    for (i = 0; i < 16; i++)
    {
        if (pending & (1UL << i))
        {
            rt_int32_t idx = (rt_int32_t)port * 16 + (rt_int32_t)i;
            if (pin_irq_table[idx].hdr)
            {
                pin_irq_table[idx].hdr(pin_irq_table[idx].args);
            }
            EXTI_Clear(gpio_port, i);
        }
    }
}

void IRQ16_Handler(void)
{
    rt_interrupt_enter();
    swm181_gpio_port_isr(0);
    rt_interrupt_leave();
}

void IRQ17_Handler(void)
{
    rt_interrupt_enter();
    swm181_gpio_port_isr(1);
    rt_interrupt_leave();
}

void IRQ18_Handler(void)
{
    rt_interrupt_enter();
    swm181_gpio_port_isr(2);
    rt_interrupt_leave();
}

void IRQ19_Handler(void)
{
    rt_interrupt_enter();
    swm181_gpio_port_isr(3);
    rt_interrupt_leave();
}

void IRQ20_Handler(void)
{
    rt_interrupt_enter();
    swm181_gpio_port_isr(4);
    rt_interrupt_leave();
}

static void swm181_pin_mode(struct rt_device *device, rt_base_t pin, rt_uint8_t mode)
{
    int dir = 0;
    int pull_up = 0;
    int pull_down = 0;
    int open_drain = 0;
    
    GPIO_TypeDef *gpio_port = SWM181_PIN_GET_PORT(pin);
    uint32_t gpio_pin = SWM181_PIN_GET_PIN(pin);

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

    GPIO_Init(gpio_port, gpio_pin, dir, pull_up, pull_down, open_drain);
}

static void swm181_pin_write(struct rt_device *device, rt_base_t pin, rt_uint8_t value)
{
    GPIO_TypeDef *gpio_port = SWM181_PIN_GET_PORT(pin);
    uint32_t gpio_pin = SWM181_PIN_GET_PIN(pin);

    if (value)
    {
        GPIO_AtomicSetBit(gpio_port, gpio_pin);
    }
    else
    {
        GPIO_AtomicClrBit(gpio_port, gpio_pin);
    }
}

static int swm181_pin_read(struct rt_device *device, rt_base_t pin)
{
    GPIO_TypeDef *gpio_port = SWM181_PIN_GET_PORT(pin);
    uint32_t gpio_pin = SWM181_PIN_GET_PIN(pin);
    
    return (int)GPIO_GetBit(gpio_port, gpio_pin);
}

static rt_err_t swm181_pin_attach_irq(struct rt_device *device, rt_base_t pin,
                                      rt_uint8_t mode, void (*hdr)(void *args), void *args)
{
    rt_err_t ret = RT_EOK;
    rt_int32_t index = swm181_pin_to_index(pin);
    GPIO_TypeDef *gpio_port;
    rt_uint32_t gpio_pin;

    if (index < 0)
    {
        return -RT_EINVAL;
    }

    gpio_port = SWM181_PIN_GET_PORT(pin);
    gpio_pin = SWM181_PIN_GET_PIN(pin);

    pin_irq_table[index].pin = pin;
    pin_irq_table[index].mode = mode;
    pin_irq_table[index].hdr = hdr;
    pin_irq_table[index].args = args;

    EXTI_Init(gpio_port, gpio_pin, swm181_exti_mode(mode));

    return ret;
}

static rt_err_t swm181_pin_detach_irq(struct rt_device *device, rt_base_t pin)
{
    rt_err_t ret = RT_EOK;
    rt_int32_t index = swm181_pin_to_index(pin);
    GPIO_TypeDef *gpio_port;
    rt_uint32_t gpio_pin;

    if (index < 0)
    {
        return -RT_EINVAL;
    }

    gpio_port = SWM181_PIN_GET_PORT(pin);
    gpio_pin = SWM181_PIN_GET_PIN(pin);

    EXTI_Close(gpio_port, gpio_pin);

    pin_irq_table[index].pin = -1;
    pin_irq_table[index].mode = 0;
    pin_irq_table[index].hdr = RT_NULL;
    pin_irq_table[index].args = RT_NULL;

    return ret;
}

static rt_err_t swm181_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint8_t enabled)
{
    rt_err_t ret = RT_EOK;
    rt_int32_t index = swm181_pin_to_index(pin);
    rt_uint8_t port;
    GPIO_TypeDef *gpio_port;
    rt_uint32_t gpio_pin;
    IRQn_Type irqn;

    if (index < 0)
    {
        return -RT_EINVAL;
    }

    port = (rt_uint8_t)((pin >> 4) & 0xFF);
    gpio_port = SWM181_PIN_GET_PORT(pin);
    gpio_pin = SWM181_PIN_GET_PIN(pin);

    if (enabled)
    {
        if (port_irq_refcnt[port] == 0)
        {
            irqn = swm181_port_irqn(port);
            IRQ_Connect(swm181_port_periph_irqsrc(port), (rt_uint32_t)irqn, 2);
        }
        port_irq_refcnt[port]++;
        EXTI_Open(gpio_port, gpio_pin);
    }
    else
    {
        EXTI_Close(gpio_port, gpio_pin);
        if (port_irq_refcnt[port] > 0)
        {
            port_irq_refcnt[port]--;
            if (port_irq_refcnt[port] == 0)
            {
                NVIC_DisableIRQ(swm181_port_irqn(port));
            }
        }
    }

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
