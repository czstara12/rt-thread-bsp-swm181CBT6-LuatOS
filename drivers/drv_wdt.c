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
#include "drv_wdt.h"

#ifdef RT_USING_WDT

static struct rt_watchdog_device wdt_dev;
static rt_uint32_t wdt_timeout_sec = 5;

static rt_err_t skt_wdt_init(rt_watchdog_t *wdt)
{
    rt_err_t ret = RT_EOK;

    RT_UNUSED(wdt);
    WDT_Init(WDT, (rt_uint32_t)((rt_uint64_t)SystemCoreClock * (rt_uint64_t)wdt_timeout_sec * 2ULL), WDT_MODE_RESET);
    return ret;
}

static rt_err_t skt_wdt_control(rt_watchdog_t *wdt, int cmd, void *arg)
{
    rt_err_t ret = RT_EOK;

    RT_UNUSED(wdt);
    switch (cmd)
    {
    case RT_DEVICE_CTRL_WDT_GET_TIMEOUT:
        if (arg)
        {
            *(rt_uint32_t *)arg = wdt_timeout_sec;
        }
        break;
    case RT_DEVICE_CTRL_WDT_SET_TIMEOUT:

        if (arg == RT_NULL)
        {
            return -RT_EINVAL;
        }
        wdt_timeout_sec = *(rt_uint32_t *)arg;
        WDT_Init(WDT, (rt_uint32_t)((rt_uint64_t)SystemCoreClock * (rt_uint64_t)wdt_timeout_sec * 2ULL), WDT_MODE_RESET);
        break;
    case RT_DEVICE_CTRL_WDT_START:

        WDT_Start(WDT);
        break;
    case RT_DEVICE_CTRL_WDT_STOP:

        WDT_Stop(WDT);
        break;

    case RT_DEVICE_CTRL_WDT_KEEPALIVE:

        WDT_Feed(WDT);
        break;
    default:

        ret = -RT_EINVAL;
        break;
    }

    return ret;
}

const static struct rt_watchdog_ops skt_wdt_ops =
{
    skt_wdt_init,
    skt_wdt_control
};

int rt_hw_wdt_init(void)
{
    rt_err_t ret = RT_EOK;

    wdt_dev.ops = &skt_wdt_ops;

    ret = rt_hw_watchdog_register(&wdt_dev, "wdt", RT_DEVICE_FLAG_STANDALONE, RT_NULL);

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_wdt_init);

#endif /* RT_USING_WDT */
