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
#include "board.h"
#include "drv_can.h"
#include "drv_gpio.h"

#ifdef RT_USING_CAN

struct swm181_can
{
    struct rt_can_device parent;
    CAN_TypeDef *CANx;
    const char *name;
};

static struct swm181_can can_obj;

static rt_err_t swm181_can_configure(struct rt_can_device *can, struct can_configure *cfg)
{
    struct swm181_can *swm_can = (struct swm181_can *)can;
    CAN_InitStructure init_struct;

    init_struct.Baudrate = cfg->baud_rate;
    init_struct.Mode = (cfg->mode == RT_CAN_MODE_NORMAL) ? CAN_MODE_NORMAL : 
                       (cfg->mode == RT_CAN_MODE_LISTEN) ? CAN_MODE_LISTEN : CAN_MODE_SELFTEST;
    
    init_struct.CAN_SJW = CAN_SJW_1tq;
    init_struct.CAN_BS1 = CAN_BS1_8tq;
    init_struct.CAN_BS2 = CAN_BS2_3tq;
    
    init_struct.FilterMode = CAN_FILTER_32b;
    init_struct.FilterCheck32b = 0x00000000;
    init_struct.FilterMask32b = 0xFFFFFFFF; // Accept all
    
    init_struct.RXNotEmptyIEn = 1;
    init_struct.RXOverflowIEn = 1;
    init_struct.ArbitrLostIEn = 1;
    init_struct.ErrPassiveIEn = 1;

    CAN_Init(swm_can->CANx, &init_struct);
    
    IRQ_Connect(IRQ0_15_CAN, IRQ4_IRQ, 1);
    NVIC_EnableIRQ(IRQ4_IRQ);
    
    CAN_Open(swm_can->CANx);

    return RT_EOK;
}

static rt_err_t swm181_can_control(struct rt_can_device *can, int cmd, void *arg)
{
    struct swm181_can *swm_can = (struct swm181_can *)can;

    switch (cmd)
    {
    case RT_CAN_CMD_SET_BAUD:
    {
        rt_uint32_t baud = (rt_uint32_t)arg;
        CAN_Close(swm_can->CANx);
        CAN_SetBaudrate(swm_can->CANx, baud, CAN_BS1_8tq, CAN_BS2_3tq, CAN_SJW_1tq);
        CAN_Open(swm_can->CANx);
        break;
    }
    case RT_CAN_CMD_SET_MODE:
    {
        rt_uint32_t mode = (rt_uint32_t)arg;
        CAN_Close(swm_can->CANx);
        swm_can->CANx->CR &= ~(CAN_CR_LOM_Msk | CAN_CR_STM_Msk);
        if (mode == RT_CAN_MODE_LISTEN) swm_can->CANx->CR |= (CAN_MODE_LISTEN << CAN_CR_LOM_Pos);
        else if (mode == RT_CAN_MODE_LOOPBACK) swm_can->CANx->CR |= (CAN_MODE_SELFTEST << CAN_CR_STM_Pos);
        CAN_Open(swm_can->CANx);
        break;
    }
    case RT_CAN_CMD_SET_FILTER:
    {
        struct rt_can_filter_config *filter_cfg = (struct rt_can_filter_config *)arg;
        CAN_Close(swm_can->CANx);
        if (filter_cfg->count > 0)
        {
            CAN_SetFilter32b(swm_can->CANx, filter_cfg->items[0].id, filter_cfg->items[0].mask);
        }
        CAN_Open(swm_can->CANx);
        break;
    }
    }

    return RT_EOK;
}

static int swm181_can_sendmsg(struct rt_can_device *can, const void *buf, rt_uint32_t boxno)
{
    struct swm181_can *swm_can = (struct swm181_can *)can;
    struct rt_can_msg *msg = (struct rt_can_msg *)buf;
    uint32_t format = (msg->ide == RT_CAN_STDID) ? CAN_FRAME_STD : CAN_FRAME_EXT;

    if (!CAN_TXBufferReady(swm_can->CANx)) return -RT_EBUSY;

    CAN_Transmit(swm_can->CANx, format, msg->id, msg->data, msg->len, 0);

    return RT_EOK;
}

static int swm181_can_recvmsg(struct rt_can_device *can, void *buf, rt_uint32_t boxno)
{
    struct swm181_can *swm_can = (struct swm181_can *)can;
    struct rt_can_msg *msg = (struct rt_can_msg *)buf;
    CAN_RXMessage rx_msg;

    if (!CAN_RXDataAvailable(swm_can->CANx)) return -1;

    CAN_Receive(swm_can->CANx, &rx_msg);

    msg->id = rx_msg.id;
    msg->ide = (rx_msg.format == CAN_FRAME_STD) ? RT_CAN_STDID : RT_CAN_EXTID;
    msg->rtr = (rx_msg.remote) ? RT_CAN_RTR : RT_CAN_DTR;
    msg->len = rx_msg.size;
    rt_memcpy(msg->data, rx_msg.data, rx_msg.size);

    return RT_EOK;
}

static const struct rt_can_ops swm181_can_ops =
{
    swm181_can_configure,
    swm181_can_control,
    swm181_can_sendmsg,
    swm181_can_recvmsg,
};

void IRQ4_Handler(void)
{
    rt_interrupt_enter();
    uint32_t status = CAN_INTStat(CAN);

    if (status & CAN_IF_RXDA_Msk)
    {
        rt_hw_can_isr(&can_obj.parent, RT_CAN_EVENT_RX_IND | (0 << 8));
    }
    if (status & CAN_IF_TXBR_Msk)
    {
        rt_hw_can_isr(&can_obj.parent, RT_CAN_EVENT_TX_DONE | (0 << 8));
    }
    
    rt_interrupt_leave();
}

int rt_hw_can_init(void)
{
    can_obj.CANx = CAN;
    can_obj.name = "can1";

    PORT_Init(SWM181_PIN_GET_PORT_PTR(BSP_CAN_RX_PIN), SWM181_PIN_GET_PIN_IDX(BSP_CAN_RX_PIN), FUNMUX_CAN_RX, 1);
    PORT_Init(SWM181_PIN_GET_PORT_PTR(BSP_CAN_TX_PIN), SWM181_PIN_GET_PIN_IDX(BSP_CAN_TX_PIN), FUNMUX_CAN_TX, 0);

    return rt_hw_can_register(&can_obj.parent, can_obj.name, &swm181_can_ops, RT_NULL);
}
INIT_DEVICE_EXPORT(rt_hw_can_init);

#endif /* RT_USING_CAN */
