/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#ifndef DRV_GPIO_H__
#define DRV_GPIO_H__

#include "board.h"

#define __SWM181_PORT(port)  (GPIO##port##_BASE)
#define GET_PIN(PORTx,PIN)  ((int)(((int)__SWM181_PORT(PORTx) - (int)GPIOA_BASE) / (0x1000) * 16 + PIN))

int rt_hw_pin_init(void);

#endif
