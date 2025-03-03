CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m0)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(CMAKE_C_COMPILER "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin/arm-none-eabi-gcc.exe")
SET(CMAKE_ASM_COMPILER "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin/arm-none-eabi-gcc.exe")
SET(CMAKE_C_FLAGS " -mcpu=cortex-m0 -mthumb -ffunction-sections -fdata-sections -g -Wall -O0 -gdwarf-2 -g")
SET(CMAKE_ASM_FLAGS " -c -mcpu=cortex-m0 -mthumb -ffunction-sections -fdata-sections -x assembler-with-cpp -Wa,-mimplicit-it=thumb  -gdwarf-2")
SET(CMAKE_C_COMPILER_WORKS TRUE)

SET(CMAKE_CXX_COMPILER "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin/arm-none-eabi-g++.exe")
SET(CMAKE_CXX_FLAGS " -mcpu=cortex-m0 -mthumb -ffunction-sections -fdata-sections -g -Wall -O0 -gdwarf-2 -g")
SET(CMAKE_CXX_COMPILER_WORKS TRUE)

SET(CMAKE_OBJCOPY "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin/arm-none-eabi-objcopy.exe")
SET(CMAKE_SIZE "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin/arm-none-eabi-size.exe")

SET(CMAKE_EXE_LINKER_FLAGS " -mcpu=cortex-m0 -mthumb -ffunction-sections -fdata-sections -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,Reset_Handler -T ${CMAKE_SOURCE_DIR}/drivers/linker_scripts/link.lds")

SET(CMAKE_CXX_STANDARD 14)
PROJECT(rtthread C CXX ASM)
INCLUDE_DIRECTORIES(
	applications
	../../../components/libc/compilers/newlib
	../../../components/libc/compilers/common/include
	../../../libcpu/arm/common
	../../../libcpu/arm/cortex-m0
	../../../components/drivers/include
	drivers
	../../../components/finsh
	.
	../../../include
	libraries/CMSIS/CoreSupport
	libraries/CMSIS/DeviceSupport
	libraries/SWM181_StdPeriph_Driver
	../../../components/libc/posix/io/epoll
	../../../components/libc/posix/io/eventfd
	../../../components/libc/posix/io/poll
	../../../components/libc/posix/ipc
)

ADD_DEFINITIONS(
	-DRT_USING_LIBC
	-DRT_USING_NEWLIBC
	-D_POSIX_C_SOURCE=1
	-D__RTTHREAD__
)

SET(PROJECT_SOURCES
	applications/main.c
	../../../components/libc/compilers/common/cctype.c
	../../../components/libc/compilers/common/cstdlib.c
	../../../components/libc/compilers/common/cstring.c
	../../../components/libc/compilers/common/ctime.c
	../../../components/libc/compilers/common/cunistd.c
	../../../components/libc/compilers/common/cwchar.c
	../../../components/libc/compilers/newlib/syscalls.c
	../../../libcpu/arm/common/div0.c
	../../../libcpu/arm/common/showmem.c
	../../../libcpu/arm/cortex-m0/context_gcc.S
	../../../libcpu/arm/cortex-m0/cpuport.c
	../../../components/drivers/core/device.c
	../../../components/drivers/ipc/completion.c
	../../../components/drivers/ipc/dataqueue.c
	../../../components/drivers/ipc/pipe.c
	../../../components/drivers/ipc/ringblk_buf.c
	../../../components/drivers/ipc/ringbuffer.c
	../../../components/drivers/ipc/waitqueue.c
	../../../components/drivers/ipc/workqueue.c
	../../../components/drivers/misc/pin.c
	../../../components/drivers/serial/serial.c
	drivers/drv_uart.c
	drivers/drv_gpio.c
	drivers/board.c
	../../../components/finsh/msh.c
	../../../components/finsh/msh_parse.c
	../../../components/finsh/cmd.c
	../../../components/finsh/shell.c
	../../../src/clock.c
	../../../src/components.c
	../../../src/idle.c
	../../../src/ipc.c
	../../../src/irq.c
	../../../src/kservice.c
	../../../src/mem.c
	../../../src/mempool.c
	../../../src/object.c
	../../../src/scheduler_up.c
	../../../src/thread.c
	../../../src/timer.c
	libraries/SWM181_StdPeriph_Driver/SWM181_spi.c
	libraries/SWM181_StdPeriph_Driver/SWM181_sleep.c
	libraries/SWM181_StdPeriph_Driver/SWM181_wdt.c
	libraries/SWM181_StdPeriph_Driver/SWM181_can.c
	libraries/SWM181_StdPeriph_Driver/SWM181_exti.c
	libraries/SWM181_StdPeriph_Driver/SWM181_timr.c
	libraries/CMSIS/DeviceSupport/system_SWM181.c
	libraries/SWM181_StdPeriph_Driver/SWM181_i2c.c
	libraries/CMSIS/DeviceSupport/startup/gcc/startup_SWM181.s
	libraries/SWM181_StdPeriph_Driver/SWM181_gpio.c
	libraries/SWM181_StdPeriph_Driver/SWM181_div.c
	libraries/SWM181_StdPeriph_Driver/SWM181_cache.c
	libraries/SWM181_StdPeriph_Driver/SWM181_cmp.c
	libraries/SWM181_StdPeriph_Driver/SWM181_flash.c
	libraries/SWM181_StdPeriph_Driver/SWM181_uart.c
	libraries/SWM181_StdPeriph_Driver/SWM181_port.c
	libraries/SWM181_StdPeriph_Driver/SWM181_cordic.c
	libraries/SWM181_StdPeriph_Driver/SWM181_sdadc.c
	libraries/SWM181_StdPeriph_Driver/SWM181_adc.c
	libraries/SWM181_StdPeriph_Driver/SWM181_slcd.c
	libraries/SWM181_StdPeriph_Driver/SWM181_pwm.c
	libraries/SWM181_StdPeriph_Driver/SWM181_dma.c
	libraries/SWM181_StdPeriph_Driver/SWM181_irqmux.c
)

LINK_DIRECTORIES(
)

LINK_LIBRARIES(
	c
	m
)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf ${PROJECT_SOURCES})
ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD 
COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf)