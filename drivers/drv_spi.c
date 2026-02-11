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
#include <board.h>
#include "drv_spi.h"

#ifdef RT_USING_SPI

#define SKT_SPI_DEVICE(spi_bus)    (struct skt_spi_bus *)(spi_bus)

static struct skt_spi_bus spi_bus0;
static struct skt_spi_bus spi_bus1;

struct skt_spi_bus
{
    struct rt_spi_bus parent;
    rt_uint32_t spi_periph;
};

static rt_uint8_t skt_spi_clkdiv(rt_uint32_t max_hz)
{
    rt_uint32_t div;
    rt_uint32_t enum_div;

    if (max_hz == 0)
    {
        return SPI_CLKDIV_512;
    }

    div = SystemCoreClock / max_hz;
    if (div <= 4)
    {
        enum_div = SPI_CLKDIV_4;
    }
    else if (div <= 8)
    {
        enum_div = SPI_CLKDIV_8;
    }
    else if (div <= 16)
    {
        enum_div = SPI_CLKDIV_16;
    }
    else if (div <= 32)
    {
        enum_div = SPI_CLKDIV_32;
    }
    else if (div <= 64)
    {
        enum_div = SPI_CLKDIV_64;
    }
    else if (div <= 128)
    {
        enum_div = SPI_CLKDIV_128;
    }
    else if (div <= 256)
    {
        enum_div = SPI_CLKDIV_256;
    }
    else
    {
        enum_div = SPI_CLKDIV_512;
    }

    return (rt_uint8_t)enum_div;
}

static rt_err_t skt_spi_configure(struct rt_spi_device *device,
                                  struct rt_spi_configuration *configuration)
{
    rt_err_t ret = RT_EOK;
    struct skt_spi_bus *bus = SKT_SPI_DEVICE(device->bus);
    SPI_TypeDef *SPIx;
    SPI_InitStructure init_struct;

    RT_ASSERT(bus != RT_NULL);
    SPIx = (SPI_TypeDef *)bus->spi_periph;
    RT_ASSERT(SPIx != RT_NULL);

    if (configuration->data_width < 4 || configuration->data_width > 16)
    {
        return -RT_EINVAL;
    }

    init_struct.FrameFormat = SPI_FORMAT_SPI;
    init_struct.WordSize = configuration->data_width;
    init_struct.Master = (configuration->mode & RT_SPI_SLAVE) ? 0 : 1;
    init_struct.clkDiv = skt_spi_clkdiv(configuration->max_hz);
    init_struct.SampleEdge = (configuration->mode & RT_SPI_CPHA) ? SPI_SECOND_EDGE : SPI_FIRST_EDGE;
    init_struct.IdleLevel = (configuration->mode & RT_SPI_CPOL) ? SPI_HIGH_LEVEL : SPI_LOW_LEVEL;
    init_struct.RXHFullIEn = 0;
    init_struct.TXEmptyIEn = 0;
    init_struct.TXCompleteIEn = 0;

    SPI_Init(SPIx, &init_struct);
    SPI_Open(SPIx);

    return ret;
}

static rt_ssize_t skt_spi_xfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    struct skt_spi_bus *bus = SKT_SPI_DEVICE(device->bus);
    struct rt_spi_configuration *cfg = &device->config;
    rt_base_t cs_pin = device->cs_pin;
    SPI_TypeDef *SPIx;
    rt_uint32_t i;
    rt_uint8_t cs_active = (cfg->mode & RT_SPI_CS_HIGH) ? PIN_HIGH : PIN_LOW;
    rt_uint8_t cs_inactive = (cfg->mode & RT_SPI_CS_HIGH) ? PIN_LOW : PIN_HIGH;
    const rt_uint8_t *send = (const rt_uint8_t *)message->send_buf;
    rt_uint8_t *recv = (rt_uint8_t *)message->recv_buf;

    RT_ASSERT(bus != RT_NULL);
    SPIx = (SPI_TypeDef *)bus->spi_periph;
    RT_ASSERT(SPIx != RT_NULL);

    if ((cfg->mode & RT_SPI_NO_CS) == 0 && cs_pin != -1)
    {
        rt_pin_mode(cs_pin, PIN_MODE_OUTPUT);
        if (message->cs_take)
        {
            rt_pin_write(cs_pin, cs_active);
        }
    }

    if (cfg->data_width <= 8)
    {
        for (i = 0; i < message->length; i++)
        {
            rt_uint8_t out = send ? send[i] : 0xFF;
            rt_uint8_t in = (rt_uint8_t)SPI_ReadWrite(SPIx, out);
            if (recv)
            {
                recv[i] = in;
            }
        }
    }
    else
    {
        const rt_uint16_t *send16 = (const rt_uint16_t *)message->send_buf;
        rt_uint16_t *recv16 = (rt_uint16_t *)message->recv_buf;
        rt_uint32_t cnt = message->length / 2;

        for (i = 0; i < cnt; i++)
        {
            rt_uint16_t out = send16 ? send16[i] : 0xFFFF;
            rt_uint16_t in = (rt_uint16_t)SPI_ReadWrite(SPIx, out);
            if (recv16)
            {
                recv16[i] = in;
            }
        }
    }

    if ((cfg->mode & RT_SPI_NO_CS) == 0 && cs_pin != -1)
    {
        if (message->cs_release)
        {
            rt_pin_write(cs_pin, cs_inactive);
        }
    }

    return (rt_ssize_t)message->length;
}

const static struct rt_spi_ops skt_spi_ops =
{
    skt_spi_configure,
    skt_spi_xfer
};

int rt_hw_spi_init(void)
{
    rt_err_t ret = RT_EOK;

    spi_bus0.spi_periph = (rt_uint32_t)SPI0;
    spi_bus1.spi_periph = (rt_uint32_t)SPI1;

    ret = rt_spi_bus_register(&spi_bus0.parent, "spi0", &skt_spi_ops);
    if (ret != RT_EOK)
    {
        return ret;
    }
    ret = rt_spi_bus_register(&spi_bus1.parent, "spi1", &skt_spi_ops);

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_spi_init);

#endif /* RT_USING_SPI */
