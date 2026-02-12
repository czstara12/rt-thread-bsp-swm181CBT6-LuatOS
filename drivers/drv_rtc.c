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
#include "drv_rtc.h"

#ifdef RT_USING_RTC

static rt_rtc_dev_t swm181_rtc_dev;
static time_t swm181_rtc_base_sec;
static rt_tick_t swm181_rtc_base_tick;

static rt_err_t swm181_rtc_init(void)
{
    swm181_rtc_base_sec = 0;
    swm181_rtc_base_tick = rt_tick_get();
    return RT_EOK;
}

static rt_err_t swm181_rtc_get_secs(time_t *sec)
{
    rt_tick_t tick;

    if (sec == RT_NULL)
    {
        return -RT_EINVAL;
    }

    tick = rt_tick_get();
    *sec = swm181_rtc_base_sec + (time_t)((tick - swm181_rtc_base_tick) / RT_TICK_PER_SECOND);

    return RT_EOK;
}

static rt_err_t swm181_rtc_set_secs(time_t *sec)
{
    if (sec == RT_NULL)
    {
        return -RT_EINVAL;
    }

    swm181_rtc_base_sec = *sec;
    swm181_rtc_base_tick = rt_tick_get();

    return RT_EOK;
}

static const struct rt_rtc_ops swm181_rtc_ops =
{
    .init = swm181_rtc_init,
    .get_secs = swm181_rtc_get_secs,
    .set_secs = swm181_rtc_set_secs,
    .get_alarm = RT_NULL,
    .set_alarm = RT_NULL,
    .get_timeval = RT_NULL,
    .set_timeval = RT_NULL,
};

int rt_hw_rtc_init(void)
{
    swm181_rtc_dev.ops = &swm181_rtc_ops;
    return rt_hw_rtc_register(&swm181_rtc_dev, "rtc", RT_DEVICE_FLAG_RDWR, RT_NULL);
}
INIT_DEVICE_EXPORT(rt_hw_rtc_init);

#endif /* RT_USING_RTC */
