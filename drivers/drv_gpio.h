/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-12     Gemini       first version
 */

#ifndef DRV_GPIO_H__
#define DRV_GPIO_H__

#include "board.h"

/* SWM181CBT6-LuatOS Board Pin Mapping */
#define __GET_PIN_A0  30
#define __GET_PIN_A1  31
#define __GET_PIN_A2  47
#define __GET_PIN_A3  48
#define __GET_PIN_A4  6
#define __GET_PIN_A5  5
#define __GET_PIN_A6  2
#define __GET_PIN_A7  1
#define __GET_PIN_A8  21
#define __GET_PIN_A9  22
#define __GET_PIN_A10 28
#define __GET_PIN_A11 29
#define __GET_PIN_A12 13
#define __GET_PIN_A13 12
#define __GET_PIN_A14 11
#define __GET_PIN_A15 8

#define __GET_PIN_B1  33
#define __GET_PIN_B2  34
#define __GET_PIN_B3  35
#define __GET_PIN_B4  36
#define __GET_PIN_B5  37
#define __GET_PIN_B6  39
#define __GET_PIN_B7  40
#define __GET_PIN_B8  51
#define __GET_PIN_B9  52

#define __GET_PIN_C2  45
#define __GET_PIN_C3  44
#define __GET_PIN_C4  42
#define __GET_PIN_C5  41
#define __GET_PIN_C6  15
#define __GET_PIN_C7  14

#define __GET_PIN_D0  53
#define __GET_PIN_D1  20
#define __GET_PIN_D2  19
#define __GET_PIN_D3  16

#define __GET_PIN_E4  7

#define GET_PIN(PORTx, PIN)  __GET_PIN_##PORTx##PIN

/* Helper macros for driver internal use (Physical Pin -> MCU index) */
#define SWM181_PIN_GET_PORT_PTR(pin) swm181_get_port_ptr(pin)
#define SWM181_PIN_GET_PIN_IDX(pin)  swm181_get_pin_idx(pin)

PORT_TypeDef *swm181_get_port_ptr(uint32_t board_pin);
uint32_t swm181_get_pin_idx(uint32_t board_pin);
int rt_hw_pin_init(void);

#endif
