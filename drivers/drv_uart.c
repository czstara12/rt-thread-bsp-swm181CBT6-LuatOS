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
#include "drv_uart.h"

#ifdef RT_USING_SERIAL

#define swm181_UART_DEVICE(uart) (struct swm181_uart_dev *)(uart)

static struct swm181_uart_dev uart0_dev;

struct swm181_uart_dev
{
    struct rt_serial_device parent;
};

void IRQ0_Handler(void)
{
    /* read interrupt status and clear it */
    if (UART_INTRXThresholdStat(UART0) || UART_INTTimeoutStat(UART0)) /* rx ind */
    {
        rt_hw_serial_isr(&uart0_dev.parent, RT_SERIAL_EVENT_RX_IND);
    }

    if (0) /* tx done */
    {
        rt_hw_serial_isr(&uart0_dev.parent, RT_SERIAL_EVENT_TX_DONE);
    }
}

/*
 * UART interface
 */
static rt_err_t swm181_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    rt_err_t ret = RT_EOK;

    RT_ASSERT(serial != RT_NULL);

    serial->config = *cfg;

    /* Init UART Hardware(uart->uart_periph) */
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
    UART_Init(UART0, &UART_initStruct);
    IRQ_Connect(IRQ0_15_UART0, IRQ0_IRQ, 1);
    NVIC_DisableIRQ(IRQ0_IRQ);
    UART_Open(UART0);
    return ret;
}

static rt_err_t swm181_uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    rt_err_t ret = RT_EOK;

    rt_ubase_t ctrl_arg = (rt_ubase_t)arg;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* Disable the UART Interrupt */
        NVIC_DisableIRQ(IRQ0_IRQ);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        /* install interrupt */
        /* Enable the UART Interrupt */
        NVIC_EnableIRQ(IRQ0_IRQ);
        break;

    case RT_DEVICE_CTRL_CONFIG:
        if (ctrl_arg == RT_DEVICE_FLAG_DMA_RX)
        {
            /* No DMA */
        }
        break;
    }

    return ret;
}

static int swm181_uart_putc(struct rt_serial_device *serial, char c)
{
    /* FIFO status, contain valid data */
    while (UART_IsTXFIFOFull(UART0) == 1)
        ;
    UART_WriteByte(UART0, c);

    return 1;
}

static int swm181_uart_getc(struct rt_serial_device *serial)
{
    int ch;

    ch = -1;

    if (UART_IsRXFIFOEmpty(UART0) == 0)
    {
        uint32_t data;
        if (UART_ReadByte(UART0, &data) == 0)
        {
            ch = (int)data;
        }
    }

    return ch;
}

static rt_ssize_t swm181_uart_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    return 0;
}

const static struct rt_uart_ops _uart_ops =
    {
        swm181_uart_configure,
        swm181_uart_control,
        swm181_uart_putc,
        swm181_uart_getc,
        swm181_uart_dma_transmit};

/*
 * UART Initiation
 */
int rt_hw_uart_init(void)
{
    struct serial_configure serial_cfg = RT_SERIAL_CONFIG_DEFAULT;
    rt_err_t ret = RT_EOK;

    PORT_Init(PORTA, PIN0, FUNMUX_UART0_RXD, 1);
    PORT_Init(PORTA, PIN1, FUNMUX_UART0_TXD, 0);

    uart0_dev.parent.ops = &_uart_ops;
    uart0_dev.parent.config = serial_cfg;

    ret = rt_hw_serial_register(&uart0_dev.parent,
                                "uart0",
                                RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                                &uart0_dev);

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);

#endif /* RT_USING_SERIAL */
