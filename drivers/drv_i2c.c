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
#include "drv_i2c.h"

#ifdef RT_USING_I2C

#define SKT_I2C_DEVICE(i2c_bus)    (struct skt_i2c_bus *)(i2c_bus)

static struct skt_i2c_bus i2c_bus0;
static struct skt_i2c_bus i2c_bus1;

struct skt_i2c_bus
{
    struct rt_i2c_bus_device parent;
    rt_uint32_t i2c_periph;
};

static rt_uint32_t skt_i2c_msg_addr(const struct rt_i2c_msg *msg)
{
    rt_uint32_t addr = msg->addr & 0x7FU;
    rt_uint32_t rw = (msg->flags & RT_I2C_RD) ? 1U : 0U;
    return (addr << 1) | rw;
}

static rt_ssize_t skt_i2c_master_xfer(struct rt_i2c_bus_device *bus, struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    struct skt_i2c_bus *i2c_bus = SKT_I2C_DEVICE(bus);
    I2C_TypeDef *I2Cx;
    rt_uint32_t i;

    RT_ASSERT(i2c_bus != RT_NULL);
    I2Cx = (I2C_TypeDef *)i2c_bus->i2c_periph;
    RT_ASSERT(I2Cx != RT_NULL);

    for (i = 0; i < num; i++)
    {
        struct rt_i2c_msg *msg = &msgs[i];
        rt_uint32_t j;
        rt_bool_t ignore_nack = (msg->flags & RT_I2C_IGNORE_NACK) ? RT_TRUE : RT_FALSE;

        if ((msg->flags & RT_I2C_NO_START) == 0)
        {
            rt_uint8_t ack = I2C_Start(I2Cx, (rt_uint8_t)skt_i2c_msg_addr(msg));
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

const static struct rt_i2c_bus_device_ops skt_i2c_ops =
{
    skt_i2c_master_xfer,
    RT_NULL,
    RT_NULL
};

int rt_hw_i2c_init(void)
{
    rt_err_t ret = RT_EOK;
    I2C_InitStructure init_struct;

    i2c_bus0.parent.ops = &skt_i2c_ops;
    i2c_bus1.parent.ops = &skt_i2c_ops;

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

    i2c_bus0.i2c_periph = (rt_uint32_t)I2C0;
    I2C_Init((I2C_TypeDef *)i2c_bus0.i2c_periph, &init_struct);
    I2C_Open((I2C_TypeDef *)i2c_bus0.i2c_periph);
    ret = rt_i2c_bus_device_register(&i2c_bus0.parent, "i2c0");
    if (ret != RT_EOK)
    {
        return ret;
    }

    i2c_bus1.i2c_periph = (rt_uint32_t)I2C1;
    I2C_Init((I2C_TypeDef *)i2c_bus1.i2c_periph, &init_struct);
    I2C_Open((I2C_TypeDef *)i2c_bus1.i2c_periph);
    ret = rt_i2c_bus_device_register(&i2c_bus1.parent, "i2c1");

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_i2c_init);

#endif /* RT_USING_I2C */
