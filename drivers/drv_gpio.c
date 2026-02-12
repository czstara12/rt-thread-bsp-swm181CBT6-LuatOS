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
#include "drv_gpio.h"

#ifdef RT_USING_PIN

/* Board Pin (1-53) to MCU Pin Index (0-79) mapping */
/* PA: 0-15, PB: 16-31, PC: 32-47, PD: 48-63, PE: 64-79 */
static const rt_int16_t board_to_mcu_table[] = {
    -1, // 0
    7, 6, -1, -1, 5, 4, 68, 15, -1, -1, // 1-10
    14, 13, 12, 39, 38, 51, -1, -1, 50, 49, // 11-20
    8, 9, -1, -1, -1, -1, -1, 10, 11, 0,    // 21-30
    1, -1, 17, 18, 19, 20, 21, -1, 22, 23,  // 31-40
    37, 36, -1, 35, 34, -1, 2, 3, -1, -1,   // 41-50
    24, 25, 48                              // 51-53
};

struct swm181_pin_irq
{
    rt_base_t pin;
    rt_uint8_t mode;
    void (*hdr)(void *args);
    void *args;
};

static struct swm181_pin_irq pin_irq_table[80];
static rt_uint8_t port_irq_refcnt[5];

static rt_int32_t get_mcu_pin(rt_base_t board_pin)
{
    if (board_pin < 1 || board_pin > 53) return -1;
    return board_to_mcu_table[board_pin];
}

PORT_TypeDef *swm181_get_port_ptr(uint32_t board_pin)
{
    rt_int32_t mcu_pin = get_mcu_pin(board_pin);
    if (mcu_pin < 0) return RT_NULL;
    return (PORT_TypeDef *)(PORTA_BASE + (mcu_pin >> 4) * 0x1000);
}

uint32_t swm181_get_pin_idx(uint32_t board_pin)
{
    rt_int32_t mcu_pin = get_mcu_pin(board_pin);
    if (mcu_pin < 0) return 0;
    return (mcu_pin & 0x0F);
}

static rt_uint32_t swm181_exti_mode(rt_uint8_t mode)
{
    switch (mode)
    {
    case PIN_IRQ_MODE_RISING: return EXTI_RISE_EDGE;
    case PIN_IRQ_MODE_FALLING: return EXTI_FALL_EDGE;
    case PIN_IRQ_MODE_RISING_FALLING: return EXTI_BOTH_EDGE;
    case PIN_IRQ_MODE_HIGH_LEVEL: return EXTI_HIGH_LEVEL;
    case PIN_IRQ_MODE_LOW_LEVEL: return EXTI_LOW_LEVEL;
    default: return EXTI_FALL_EDGE;
    }
}

static void swm181_gpio_port_isr(rt_uint8_t port)
{
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(GPIOA_BASE + port * 0x1000);
    rt_uint32_t pending = gpio_port->INTSTAT & gpio_port->INTEN;
    for (rt_uint32_t i = 0; i < 16; i++)
    {
        if (pending & (1UL << i))
        {
            rt_int32_t idx = port * 16 + i;
            if (pin_irq_table[idx].hdr) pin_irq_table[idx].hdr(pin_irq_table[idx].args);
            EXTI_Clear(gpio_port, i);
        }
    }
}

void IRQ16_Handler(void) { rt_interrupt_enter(); swm181_gpio_port_isr(0); rt_interrupt_leave(); }
void IRQ17_Handler(void) { rt_interrupt_enter(); swm181_gpio_port_isr(1); rt_interrupt_leave(); }
void IRQ18_Handler(void) { rt_interrupt_enter(); swm181_gpio_port_isr(2); rt_interrupt_leave(); }
void IRQ19_Handler(void) { rt_interrupt_enter(); swm181_gpio_port_isr(3); rt_interrupt_leave(); }
void IRQ20_Handler(void) { rt_interrupt_enter(); swm181_gpio_port_isr(4); rt_interrupt_leave(); }

static void swm181_pin_mode(struct rt_device *device, rt_base_t pin, rt_uint8_t mode)
{
    rt_int32_t mcu_pin = get_mcu_pin(pin);
    if (mcu_pin < 0) return;
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(GPIOA_BASE + (mcu_pin >> 4) * 0x1000);
    int dir = 0, pull_up = 0, pull_down = 0, open_drain = 0;
    switch (mode)
    {
    case PIN_MODE_OUTPUT: dir = 1; break;
    case PIN_MODE_INPUT: dir = 0; break;
    case PIN_MODE_INPUT_PULLUP: dir = 0; pull_up = 1; break;
    case PIN_MODE_INPUT_PULLDOWN: dir = 0; pull_down = 1; break;
    case PIN_MODE_OUTPUT_OD: dir = 1; open_drain = 1; break;
    }
    GPIO_Init(gpio_port, mcu_pin & 0x0F, dir, pull_up, pull_down, open_drain);
}

static void swm181_pin_write(struct rt_device *device, rt_base_t pin, rt_uint8_t value)
{
    rt_int32_t mcu_pin = get_mcu_pin(pin);
    if (mcu_pin < 0) return;
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(GPIOA_BASE + (mcu_pin >> 4) * 0x1000);
    if (value) GPIO_AtomicSetBit(gpio_port, mcu_pin & 0x0F);
    else GPIO_AtomicClrBit(gpio_port, mcu_pin & 0x0F);
}

static int swm181_pin_read(struct rt_device *device, rt_base_t pin)
{
    rt_int32_t mcu_pin = get_mcu_pin(pin);
    if (mcu_pin < 0) return 0;
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(GPIOA_BASE + (mcu_pin >> 4) * 0x1000);
    return (int)GPIO_GetBit(gpio_port, mcu_pin & 0x0F);
}

static rt_err_t swm181_pin_attach_irq(struct rt_device *device, rt_base_t pin,
                                      rt_uint8_t mode, void (*hdr)(void *args), void *args)
{
    rt_int32_t mcu_pin = get_mcu_pin(pin);
    if (mcu_pin < 0) return -RT_EINVAL;
    pin_irq_table[mcu_pin].pin = pin;
    pin_irq_table[mcu_pin].mode = mode;
    pin_irq_table[mcu_pin].hdr = hdr;
    pin_irq_table[mcu_pin].args = args;
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(GPIOA_BASE + (mcu_pin >> 4) * 0x1000);
    EXTI_Init(gpio_port, mcu_pin & 0x0F, swm181_exti_mode(mode));
    return RT_EOK;
}

static rt_err_t swm181_pin_detach_irq(struct rt_device *device, rt_base_t pin)
{
    rt_int32_t mcu_pin = get_mcu_pin(pin);
    if (mcu_pin < 0) return -RT_EINVAL;
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(GPIOA_BASE + (mcu_pin >> 4) * 0x1000);
    EXTI_Close(gpio_port, mcu_pin & 0x0F);
    pin_irq_table[mcu_pin].hdr = RT_NULL;
    return RT_EOK;
}

static rt_err_t swm181_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint8_t enabled)
{
    rt_int32_t mcu_pin = get_mcu_pin(pin);
    if (mcu_pin < 0) return -RT_EINVAL;
    rt_uint8_t port = mcu_pin >> 4;
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(GPIOA_BASE + port * 0x1000);
    if (enabled)
    {
        if (port_irq_refcnt[port] == 0)
        {
            uint32_t irqsrc = (port == 0) ? IRQ16_31_GPIOA : (port == 1) ? IRQ16_31_GPIOB : (port == 2) ? IRQ16_31_GPIOC : (port == 3) ? IRQ16_31_GPIOD : IRQ16_31_GPIOE;
            IRQ_Connect(irqsrc, IRQ16_IRQ + port, 2);
        }
        port_irq_refcnt[port]++;
        EXTI_Open(gpio_port, mcu_pin & 0x0F);
    }
    else
    {
        EXTI_Close(gpio_port, mcu_pin & 0x0F);
        if (port_irq_refcnt[port] > 0)
        {
            if (--port_irq_refcnt[port] == 0) NVIC_DisableIRQ(IRQ16_IRQ + port);
        }
    }
    return RT_EOK;
}

const static struct rt_pin_ops swm181_pin_ops = {
    swm181_pin_mode, swm181_pin_write, swm181_pin_read, swm181_pin_attach_irq, swm181_pin_detach_irq, swm181_pin_irq_enable
};

int rt_hw_pin_init(void)
{
    return rt_device_pin_register("pin", &swm181_pin_ops, RT_NULL);
}
INIT_BOARD_EXPORT(rt_hw_pin_init);

#endif
