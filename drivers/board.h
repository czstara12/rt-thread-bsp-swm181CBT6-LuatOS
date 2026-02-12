/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-5-30      Bernard      the first version
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtthread.h>
#include "SWM181.h"

#define BOARD_SRAM_SIZE 0x4000
#define BOARD_SRAM_END (void *)(0x20000000 + BOARD_SRAM_SIZE)

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section = "HEAP"
#define HEAP_BEGIN (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN ((void *)&__bss_end)
#endif

/* 堆结束地址：预留 3KB 给系统栈 (2KB) 和 异常处理/冗余 (1KB) */
#define HEAP_END (void *)(0x20000000 + BOARD_SRAM_SIZE - 3072)

void rt_hw_board_init(void);

#endif
