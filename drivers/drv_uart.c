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
#include "drv_uart.h"

#ifdef RT_USING_SERIAL

#if !defined(BSP_USING_UART0) && !defined(BSP_USING_UART1) && \
    !defined(BSP_USING_UART2) && !defined(BSP_USING_UART3)
#error "Please define at least one BSP_USING_UARTx"
#endif

struct swm181_uart
{
    UART_TypeDef *UARTx;
    IRQn_Type irqn;
    uint32_t periph_irq;
    const char *name;
};

static struct swm181_uart_device
{
    struct swm181_uart *uart_info;
    struct rt_serial_device serial;
} uart_obj[] = {
#ifdef BSP_USING_UART0
    {
        .uart_info = &(struct swm181_uart){UART0, IRQ0_IRQ, IRQ0_15_UART0, "uart0"},
    },
#endif
#ifdef BSP_USING_UART1
    {
        .uart_info = &(struct swm181_uart){UART1, IRQ1_IRQ, IRQ0_15_UART1, "uart1"},
    },
#endif
#ifdef BSP_USING_UART2
    {
        .uart_info = &(struct swm181_uart){UART2, IRQ2_IRQ, IRQ0_15_UART2, "uart2"},
    },
#endif
#ifdef BSP_USING_UART3
    {
        .uart_info = &(struct swm181_uart){UART3, IRQ3_IRQ, IRQ0_15_UART3, "uart3"},
    },
#endif
};

static rt_err_t swm181_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct swm181_uart *uart = ((struct swm181_uart_device *)serial)->uart_info;
    UART_InitStructure UART_initStruct;

    UART_initStruct.Baudrate = cfg->baud_rate;
    
    switch (cfg->data_bits)
    {
    case DATA_BITS_8:
        UART_initStruct.DataBits = UART_DATA_8BIT;
        break;
    case DATA_BITS_9:
        UART_initStruct.DataBits = UART_DATA_9BIT;
        break;
    default:
        UART_initStruct.DataBits = UART_DATA_8BIT;
        break;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        UART_initStruct.Parity = UART_PARITY_NONE;
        break;
    case PARITY_ODD:
        UART_initStruct.Parity = UART_PARITY_ODD;
        break;
    case PARITY_EVEN:
        UART_initStruct.Parity = UART_PARITY_EVEN;
        break;
    default:
        UART_initStruct.Parity = UART_PARITY_NONE;
        break;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        UART_initStruct.StopBits = UART_STOP_1BIT;
        break;
    case STOP_BITS_2:
        UART_initStruct.StopBits = UART_STOP_2BIT;
        break;
    default:
        UART_initStruct.StopBits = UART_STOP_1BIT;
        break;
    }

    UART_initStruct.RXThreshold = 3;
    UART_initStruct.RXThresholdIEn = 1;
    UART_initStruct.TXThreshold = 3;
    UART_initStruct.TXThresholdIEn = 0;
    UART_initStruct.TimeoutTime = 10;
    UART_initStruct.TimeoutIEn = 1;

    UART_Init(uart->UARTx, &UART_initStruct);
    
    IRQ_Connect(uart->periph_irq, uart->irqn, 1);
    NVIC_DisableIRQ(uart->irqn);
    
    UART_Open(uart->UARTx);

    return RT_EOK;
}

static rt_err_t swm181_uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct swm181_uart *uart = ((struct swm181_uart_device *)serial)->uart_info;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        NVIC_DisableIRQ(uart->irqn);
        UART_INTRXThresholdDis(uart->UARTx);
        UART_INTTimeoutDis(uart->UARTx);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        UART_INTRXThresholdEn(uart->UARTx);
        UART_INTTimeoutEn(uart->UARTx);
        NVIC_EnableIRQ(uart->irqn);
        break;
    }

    return RT_EOK;
}

static int swm181_uart_putc(struct rt_serial_device *serial, char c)
{
    struct swm181_uart *uart = ((struct swm181_uart_device *)serial)->uart_info;

    while (UART_IsTXFIFOFull(uart->UARTx));
    UART_WriteByte(uart->UARTx, c);

    return 1;
}

static int swm181_uart_getc(struct rt_serial_device *serial)
{
    struct swm181_uart *uart = ((struct swm181_uart_device *)serial)->uart_info;
    uint32_t data;

    if (UART_IsRXFIFOEmpty(uart->UARTx))
        return -1;

    if (UART_ReadByte(uart->UARTx, &data) == 0)
        return (int)data;

    return -1;
}

static const struct rt_uart_ops swm181_uart_ops =
{
    swm181_uart_configure,
    swm181_uart_control,
    swm181_uart_putc,
    swm181_uart_getc,
    RT_NULL,
};

static void swm181_uart_isr(struct rt_serial_device *serial)
{
    struct swm181_uart *uart = ((struct swm181_uart_device *)serial)->uart_info;

    if (UART_INTRXThresholdStat(uart->UARTx) || UART_INTTimeoutStat(uart->UARTx))
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
    }

    if (UART_INTTXDoneStat(uart->UARTx))
    {
        UART_INTTXDoneDis(uart->UARTx);
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_TX_DONE);
    }
}

#ifdef BSP_USING_UART0
void IRQ0_Handler(void)
{
    rt_interrupt_enter();
    swm181_uart_isr(&uart_obj[0].serial);
    rt_interrupt_leave();
}
#endif

#ifdef BSP_USING_UART1
void IRQ1_Handler(void)
{
    rt_interrupt_enter();
    /* We need to find the correct index in uart_obj */
    int idx = 0;
#ifdef BSP_USING_UART0
    idx++;
#endif
    swm181_uart_isr(&uart_obj[idx].serial);
    rt_interrupt_leave();
}
#endif

#ifdef BSP_USING_UART2
void IRQ2_Handler(void)
{
    rt_interrupt_enter();
    int idx = 0;
#ifdef BSP_USING_UART0
    idx++;
#endif
#ifdef BSP_USING_UART1
    idx++;
#endif
    swm181_uart_isr(&uart_obj[idx].serial);
    rt_interrupt_leave();
}
#endif

#ifdef BSP_USING_UART3
void IRQ3_Handler(void)
{
    rt_interrupt_enter();
    int idx = 0;
#ifdef BSP_USING_UART0
    idx++;
#endif
#ifdef BSP_USING_UART1
    idx++;
#endif
#ifdef BSP_USING_UART2
    idx++;
#endif
    swm181_uart_isr(&uart_obj[idx].serial);
    rt_interrupt_leave();
}
#endif

int rt_hw_uart_init(void)
{
    struct serial_configure serial_cfg = RT_SERIAL_CONFIG_DEFAULT;
    int i;

#ifdef BSP_USING_UART0
    PORT_Init(PORTA, PIN0, FUNMUX_UART0_RXD, 1);
    PORT_Init(PORTA, PIN1, FUNMUX_UART0_TXD, 0);
#endif
#ifdef BSP_USING_UART1
    PORT_Init(PORTA, PIN10, FUNMUX_UART1_RXD, 1);
    PORT_Init(PORTA, PIN11, FUNMUX_UART1_TXD, 0);
#endif
#ifdef BSP_USING_UART2
    PORT_Init(PORTB, PIN10, FUNMUX_UART2_RXD, 1);
    PORT_Init(PORTB, PIN11, FUNMUX_UART2_TXD, 0);
#endif
#ifdef BSP_USING_UART3
    PORT_Init(PORTC, PIN10, FUNMUX_UART3_RXD, 1);
    PORT_Init(PORTC, PIN11, FUNMUX_UART3_TXD, 0);
#endif

    for (i = 0; i < sizeof(uart_obj) / sizeof(uart_obj[0]); i++)
    {
        uart_obj[i].serial.ops = &swm181_uart_ops;
        uart_obj[i].serial.config = serial_cfg;

        rt_hw_serial_register(&uart_obj[i].serial,
                                uart_obj[i].uart_info->name,
                                RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                                &uart_obj[i]);
    }

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);

#endif /* RT_USING_SERIAL */
