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
#include "drv_i2c.h"
#include "drv_gpio.h"

#ifdef RT_USING_I2C

#if !defined(BSP_USING_I2C0) && !defined(BSP_USING_I2C1)
#error "Please define at least one BSP_USING_I2Cx"
#endif

struct swm181_i2c_bus
{
    struct rt_i2c_bus_device parent;
    I2C_TypeDef *I2Cx;
    const char *name;
    uint32_t scl_pin;
    uint32_t sda_pin;
};

static rt_ssize_t swm181_i2c_master_xfer(struct rt_i2c_bus_device *bus, struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    struct swm181_i2c_bus *i2c_bus = (struct swm181_i2c_bus *)bus;
    I2C_TypeDef *I2Cx = i2c_bus->I2Cx;
    rt_uint32_t i;

    for (i = 0; i < num; i++)
    {
        struct rt_i2c_msg *msg = &msgs[i];
        rt_uint32_t j;
        rt_bool_t ignore_nack = (msg->flags & RT_I2C_IGNORE_NACK) ? RT_TRUE : RT_FALSE;

        if ((msg->flags & RT_I2C_NO_START) == 0)
        {
            rt_uint32_t addr = msg->addr & 0x7FU;
            rt_uint32_t rw = (msg->flags & RT_I2C_RD) ? 1U : 0U;
            rt_uint8_t ack = I2C_Start(I2Cx, (uint8_t)((addr << 1) | rw));
            if (!ack && !ignore_nack)
            {
                I2C_Stop(I2Cx);
                return i;
            }
        }

        if (msg->flags & RT_I2C_RD)
        {
            for (j = 0; j < msg->len; j++)
            {
                rt_uint8_t ack;
                if (msg->flags & RT_I2C_NO_READ_ACK)
                {
                    ack = 0;
                }
                else
                {
                    ack = ((j + 1U) == msg->len) ? 0 : 1;
                }
                msg->buf[j] = I2C_Read(I2Cx, ack);
            }
        }
        else
        {
            for (j = 0; j < msg->len; j++)
            {
                rt_uint8_t ack = I2C_Write(I2Cx, msg->buf[j]);
                if (!ack && !ignore_nack)
                {
                    I2C_Stop(I2Cx);
                    return i;
                }
            }
        }

        if ((msg->flags & RT_I2C_NO_STOP) == 0)
        {
            I2C_Stop(I2Cx);
        }
    }

    return num;
}

static const struct rt_i2c_bus_device_ops swm181_i2c_ops =
{
    swm181_i2c_master_xfer,
    RT_NULL,
    RT_NULL
};

static struct swm181_i2c_bus i2c_objs[] = {
#ifdef BSP_USING_I2C0
    { .I2Cx = I2C0, .name = "i2c0", .scl_pin = BSP_I2C0_SCL_PIN, .sda_pin = BSP_I2C0_SDA_PIN },
#endif
#ifdef BSP_USING_I2C1
    { .I2Cx = I2C1, .name = "i2c1", .scl_pin = BSP_I2C1_SCL_PIN, .sda_pin = BSP_I2C1_SDA_PIN },
#endif
};

int rt_hw_i2c_init(void)
{
    I2C_InitStructure init_struct;
    int i;

    init_struct.Master = 1;
    init_struct.Addr7b = 1;
    init_struct.MstClk = 100000;
    init_struct.MstIEn = 0;
    init_struct.SlvAddr = 0;
    init_struct.SlvAddrMask = 0;
    init_struct.SlvRxEndIEn = 0;
    init_struct.SlvTxEndIEn = 0;
    init_struct.SlvSTADetIEn = 0;
    init_struct.SlvSTODetIEn = 0;
    init_struct.SlvRdReqIEn = 0;
    init_struct.SlvWrReqIEn = 0;

    for (i = 0; i < sizeof(i2c_objs) / sizeof(i2c_objs[0]); i++)
    {
        struct swm181_i2c_bus *bus = &i2c_objs[i];
        uint32_t scl_func = (bus->I2Cx == I2C0) ? FUNMUX_I2C0_SCL : FUNMUX_I2C1_SCL;
        uint32_t sda_func = (bus->I2Cx == I2C0) ? FUNMUX_I2C0_SDA : FUNMUX_I2C1_SDA;

        PORT_Init(SWM181_PIN_GET_PORT_PTR(bus->scl_pin), SWM181_PIN_GET_PIN_IDX(bus->scl_pin), scl_func, 1);
        PORT_Init(SWM181_PIN_GET_PORT_PTR(bus->sda_pin), SWM181_PIN_GET_PIN_IDX(bus->sda_pin), sda_func, 1);

        bus->parent.ops = &swm181_i2c_ops;
        I2C_Init(bus->I2Cx, &init_struct);
        I2C_Open(bus->I2Cx);
        rt_i2c_bus_device_register(&bus->parent, bus->name);
    }

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_i2c_init);

#endif /* RT_USING_I2C */
