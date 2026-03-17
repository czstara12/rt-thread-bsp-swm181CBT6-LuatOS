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

/* Unified MCU pin encoding: high nibble is port index, low nibble is pin number. */
#define SWM181_PIN(port, pin)        ((((port) & 0x0F) << 4) | ((pin) & 0x0F))
#define SWM181_PIN_PORT(pin)         (((pin) >> 4) & 0x0F)
#define SWM181_PIN_IDX(pin)          ((pin) & 0x0F)

#define SWM181_PORT_A                0
#define SWM181_PORT_B                1
#define SWM181_PORT_C                2
#define SWM181_PORT_D                3
#define SWM181_PORT_E                4

#define __SWM181_PORT_A              SWM181_PORT_A
#define __SWM181_PORT_B              SWM181_PORT_B
#define __SWM181_PORT_C              SWM181_PORT_C
#define __SWM181_PORT_D              SWM181_PORT_D
#define __SWM181_PORT_E              SWM181_PORT_E

/* BSP fast path: encode pin directly. Application code should prefer rt_pin_get("PXn"). */
#define GET_PIN(PORTx, PIN)          SWM181_PIN(__SWM181_PORT_##PORTx, PIN)

/* All BSP pin interfaces use the encoded MCU pin. */
#define SWM181_PIN_GET_PORT_PTR(pin) swm181_pin_get_port_ptr(pin)
#define SWM181_PIN_GET_PIN_IDX(pin)  swm181_pin_get_pin_idx(pin)

PORT_TypeDef *swm181_pin_get_port_ptr(uint32_t pin);
uint32_t swm181_pin_get_pin_idx(uint32_t pin);
rt_int32_t swm181_board_pin_to_mcu_pin(uint32_t board_pin);
int rt_hw_pin_init(void);

#endif
