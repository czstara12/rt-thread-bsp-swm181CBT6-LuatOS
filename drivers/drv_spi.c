/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-12     Gemini       first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_spi.h"
#include "drv_gpio.h"

#ifdef RT_USING_SPI

#if !defined(BSP_USING_SPI0) && !defined(BSP_USING_SPI1)
#error "Please define at least one BSP_USING_SPIx"
#endif

struct swm181_spi_bus
{
    struct rt_spi_bus parent;
    SPI_TypeDef *SPIx;
    const char *name;
    uint32_t miso_pin;
    uint32_t mosi_pin;
    uint32_t sclk_pin;
};

static rt_uint8_t swm181_spi_clkdiv(rt_uint32_t max_hz)
{
    rt_uint32_t div;

    if (max_hz == 0)
        return SPI_CLKDIV_512;

    div = SystemCoreClock / max_hz;
    if (div <= 4) return SPI_CLKDIV_4;
    if (div <= 8) return SPI_CLKDIV_8;
    if (div <= 16) return SPI_CLKDIV_16;
    if (div <= 32) return SPI_CLKDIV_32;
    if (div <= 64) return SPI_CLKDIV_64;
    if (div <= 128) return SPI_CLKDIV_128;
    if (div <= 256) return SPI_CLKDIV_256;
    
    return SPI_CLKDIV_512;
}

static rt_err_t swm181_spi_configure(struct rt_spi_device *device,
                                  struct rt_spi_configuration *configuration)
{
    struct swm181_spi_bus *bus = (struct swm181_spi_bus *)device->bus;
    SPI_TypeDef *SPIx = bus->SPIx;
    SPI_InitStructure init_struct;

    if (configuration->data_width < 4 || configuration->data_width > 16)
        return -RT_EINVAL;

    init_struct.FrameFormat = SPI_FORMAT_SPI;
    init_struct.WordSize = configuration->data_width;
    init_struct.Master = (configuration->mode & RT_SPI_SLAVE) ? 0 : 1;
    init_struct.clkDiv = swm181_spi_clkdiv(configuration->max_hz);
    init_struct.SampleEdge = (configuration->mode & RT_SPI_CPHA) ? SPI_SECOND_EDGE : SPI_FIRST_EDGE;
    init_struct.IdleLevel = (configuration->mode & RT_SPI_CPOL) ? SPI_HIGH_LEVEL : SPI_LOW_LEVEL;
    init_struct.RXHFullIEn = 0;
    init_struct.TXEmptyIEn = 0;
    init_struct.TXCompleteIEn = 0;

    SPI_Init(SPIx, &init_struct);
    SPI_Open(SPIx);

    return RT_EOK;
}

static rt_ssize_t swm181_spi_xfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    struct swm181_spi_bus *bus = (struct swm181_spi_bus *)device->bus;
    struct rt_spi_configuration *cfg = &device->config;
    rt_base_t cs_pin = device->cs_pin;
    SPI_TypeDef *SPIx = bus->SPIx;
    rt_uint32_t i;
    rt_uint8_t cs_active = (cfg->mode & RT_SPI_CS_HIGH) ? PIN_HIGH : PIN_LOW;
    rt_uint8_t cs_inactive = (cfg->mode & RT_SPI_CS_HIGH) ? PIN_LOW : PIN_HIGH;

    if ((cfg->mode & RT_SPI_NO_CS) == 0 && cs_pin != -1)
    {
        rt_pin_mode(cs_pin, PIN_MODE_OUTPUT);
        if (message->cs_take)
            rt_pin_write(cs_pin, cs_active);
    }

    if (cfg->data_width <= 8)
    {
        const rt_uint8_t *send = (const rt_uint8_t *)message->send_buf;
        rt_uint8_t *recv = (rt_uint8_t *)message->recv_buf;
        for (i = 0; i < message->length; i++)
        {
            rt_uint8_t out = send ? send[i] : 0xFF;
            rt_uint8_t in = (rt_uint8_t)SPI_ReadWrite(SPIx, out);
            if (recv) recv[i] = in;
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
            if (recv16) recv16[i] = in;
        }
    }

    if ((cfg->mode & RT_SPI_NO_CS) == 0 && cs_pin != -1)
    {
        if (message->cs_release)
            rt_pin_write(cs_pin, cs_inactive);
    }

    return (rt_ssize_t)message->length;
}

static const struct rt_spi_ops swm181_spi_ops =
{
    swm181_spi_configure,
    swm181_spi_xfer
};

static struct swm181_spi_bus spi_objs[] = {
#ifdef BSP_USING_SPI0
    { .SPIx = SPI0, .name = "spi0", .miso_pin = BSP_SPI0_MISO_PIN, .mosi_pin = BSP_SPI0_MOSI_PIN, .sclk_pin = BSP_SPI0_SCLK_PIN },
#endif
#ifdef BSP_USING_SPI1
    { .SPIx = SPI1, .name = "spi1", .miso_pin = BSP_SPI1_MISO_PIN, .mosi_pin = BSP_SPI1_MOSI_PIN, .sclk_pin = BSP_SPI1_SCLK_PIN },
#endif
};

int rt_hw_spi_init(void)
{
    int i;

    for (i = 0; i < sizeof(spi_objs) / sizeof(spi_objs[0]); i++)
    {
        struct swm181_spi_bus *bus = &spi_objs[i];
        uint32_t miso_func = 0, mosi_func = 0, sclk_func = 0;

        if (bus->SPIx == SPI0) {
            /* SPI0 物理引脚映射逻辑 (使用板载编号 1-53) */
            /* MISO: PA9(22) -> FUNC 2, PA13(12) -> FUNC 4 */
            if (bus->miso_pin == 22) miso_func = 2; else if (bus->miso_pin == 12) miso_func = 4;
            /* MOSI: PA10(28) -> FUNC 2, PA14(11) -> FUNC 4 */
            if (bus->mosi_pin == 28) mosi_func = 2; else if (bus->mosi_pin == 11) mosi_func = 4;
            /* SCLK: PA11(29) -> FUNC 2, PA15(8) -> FUNC 4 */
            if (bus->sclk_pin == 29) sclk_func = 2; else if (bus->sclk_pin == 8) sclk_func = 4;
        } else {
            /* SPI1 物理引脚映射逻辑 */
            /* MISO: PC5(41) -> FUNC 4 */
            if (bus->miso_pin == 41) miso_func = 4;
            /* MOSI: PC6(15) -> FUNC 4 */
            if (bus->mosi_pin == 15) mosi_func = 4;
            /* SCLK: PC7(14) -> FUNC 4 */
            if (bus->sclk_pin == 14) sclk_func = 4;
        }

        PORT_Init(SWM181_PIN_GET_PORT_PTR(bus->miso_pin), SWM181_PIN_GET_PIN_IDX(bus->miso_pin), miso_func, 1);
        PORT_Init(SWM181_PIN_GET_PORT_PTR(bus->mosi_pin), SWM181_PIN_GET_PIN_IDX(bus->mosi_pin), mosi_func, 0);
        PORT_Init(SWM181_PIN_GET_PORT_PTR(bus->sclk_pin), SWM181_PIN_GET_PIN_IDX(bus->sclk_pin), sclk_func, 0);

        rt_spi_bus_register(&bus->parent, bus->name, &swm181_spi_ops);
    }

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_spi_init);

#endif /* RT_USING_SPI */
