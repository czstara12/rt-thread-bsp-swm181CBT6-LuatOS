# SWM181 BSP 说明

标签： SYNWIT、Cortex-M0、SWM181、国产MCU

---

此仓库需要克隆在`rt-thread目录/bsp/synwit/`下使用

```sh
# 快速开始
git clone -b v5.1.0 https://github.com/RT-Thread/rt-thread.git
git clone https://github.com/czstara12/rt-thread-bsp-swm181CBT6-LuatOS.git rt-thread/bsp/synwit/swm181CBT6-LuatOS
cd bsp/synwit/swm181CBT6-LuatOS
scons --target=mdk5
# 或者scons
```

不能编译请添加环境变量

```sh
export RTT_EXEC_PATH=[GCC路径]
export RTT_CC=gcc
export RTT_ROOT=[RTT仓库路径]
```

## 简介

本文档为SWM181开发板提供的 BSP (板级支持包) 说明。

通过阅读快速上手章节开发者可以快速地上手该 BSP，将 RT-Thread 运行在开发板上。

![board](figures/board.png)

## 芯片介绍

SWM181 系列 32 位 MCU（以下简称 SWM181）内嵌 ARM® CortexTM-M0 内核，凭借其出色的性能以及高可靠性、低功耗、代码密度大等突出特点，可应用于工业控制、电机控制、白色家电等多种领域。 SWM181 支持片上包含精度为 1%以内的 24MHz、48MHz 时钟，并提供最大为 248K 字节的 FLASH 和最大 16K 字节的 SRAM。此外，芯片支持 ISP（在系统编程）操作及 IAP （在应用编程），用户可自定义 BOOT 程序。 SWM181外设串行总线包括 1 个 CAN 接口，多个 UART 接口、SPI 通信接口（支持主/ 从选择）及 I2C 接口（支持主/从选择），此外还具有 1 个 32 位看门狗定时器，4 组 32 位通用定时器（其中 1 组支持 Hall 接口），4 组（8 通道）PWM 控制模块，1 个 8 通道 12 位、 1MSPS 的逐次逼近型 ADC 模块，1 个 6 通道、支持单端及差分输入的 16 位 SIGMA-DELTA ADC，32 位除法模块，段码式液晶驱动模块，角度计算模块，以及 3 路比较器模块，同时提供欠压检测及低电压复位功能。

SWM181CBT6采用32 位 ARM® Cortex™-M0 内核，拥有24 位系统定时器，工作频率最高 48MHz，硬件单周期乘法 ，集成嵌套向量中断控制器（NVIC），提供最多 32 个、4 级可配置优先级的中断 ，通过 SWD 接口烧录，内置 LDO，供电电压范围 2.3V（2.7V）至 3.6V 。

**SRAM 存储器** ： 16KB

**FLASH 存储器**：64KB/120KB/248KB，支持用户定制 ISP（在系统编程）更新用户程序，支持自定义 BOOT 程序。

**串行接口**，UART*4，具有独立 8 字节 FIFO，最高支持主时钟 16 分频

SPI*2，具有 8 字节独立 FIFO，支持 SPI、SSI 协议，支持 master/slave 模式

I2C*2，支持 8 位、10 位地址方式，支持 master/slave 模式

CAN*1，支持协议 2.0A(11bit 标识符)和 2.0B（29bit 标识符）

**PORTCON 控制模块**，支持 UART/I2C/COUNTER/PWM/CAN 功能引脚定义置任意 IO。

**PWM 控制模块** ，8 通道 16 位 PWM 产生器 ，可设置高电平结束或周期开始两种条件触发中断，具有普通、互补、中心对称等多种输出模式 ， 死区控制 ， 由硬件完成与 ADC 的交互。

**定时器模块** ，4 路 32 位通用定时器，可做计数器使用，1 路支持 HALL 接口。

**32 位看门狗定时器**，溢出后可配置触发中断或复位芯片。

**内置低功耗定时器模块**，使用内部 32KB 时钟，休眠计数并自唤醒 。

**DMA 模块** ，支持 SAR ADC/SIGMA-DELTA ADC/CAN 模块与 SRAM 间数据搬运 。

**除法器模块**，支持 32 位整数除法、整数求余、整数或小数开方运算 ，除法实现约 30 个时钟周期，整数开方约 16 个时钟周期，小数开方约 30 个时钟周期。

**旋转坐标计算模块**，已知角度计算 sin/cos/arctan 值，结果为 14bit，可保证 11bit 有效值。

**LCD 驱动模块**，工作电压范围：2.4V~3.6V，静态电流：＜1uA ，最大支持 4*32 段 LCD 面板 ，支持 1/4duty+1/3bias 或 1/3duty+1/2bias 可选。

**GPIO**，最多可达 56 个 GPIO，可配置 4 种 IO 模式，上拉输入，下拉输入，推挽输出，开漏输出。

**灵活的中断配置**，触发类型设置（边沿检测、电平检测） 触发电平设置（高电平、低电平、双电平）

**12 位 8 通道高精度 SAR ADC**，采样率高达 1MSPS，支持 single/scan 两种模式，独立结果寄存器，提供独立 FIFO，可由软件/PWM/TIMER 触发，支持 DMA，**16 位 6 通道高精度 SIGMA-DELTA ADC** ，采样率 16KSPS，单通道快速模式可达 50KSPS，支持 single/scan 两种模式 ，独立结果寄存器，提供独立 FIFO ，可由软件或 TIMER 触发

**3 路模拟比较器**，可以灵活选择片内、片外参考电压，比较结果可以触发中断通知 MCU 进行处理，欠压检测，支持 2.7V 欠压检测，支持欠压中断和复位选择

**时钟源**，24MHz、48MHz 精度可达 1%的片内时钟源，32KHZ 片内时钟源，片外 2～32Mhz 片外晶振，

**其他**，自定义 BOOT 程序

**低功耗**， 正常模式：30mA@48MHZ，浅睡眠：70uA，深睡眠：5uA

**环境**
* 工作温度：-40℃～85℃
* 保存温度：-50℃～150℃
* 湿度等级：MSL3

**封装**
* QFN40
* LQFP48
* LQFP64

**应用范围**
* 仪器仪表
* 工业控制
* 电机驱动
* 白色家电
* 可穿戴设备

详细信息见数据手册，[华芯微特SWM181数据手册V2.07.pdf](https://cdn.openluat-luatcommunity.openluat.com/attachment/20220721163511464_华芯微特SWM181数据手册V2.07.pdf)

## 编译说明

本 BSP 为开发者提供 MDK5 工程。下面以 MDK5 开发环境为例，介绍如何将系统运行起来。

双击 project.uvprojx 文件，打开 MDK5 工程，编译并下载程序到开发板。

> 工程默认配置使用 Jlink 仿真器下载程序，在通过 Jlink 连接开发板到 PC 的基础上，点击下载按钮即可下载程序到开发板

推荐熟悉 RT_Thread 的用户使用[env工具](https://www.rt-thread.org/page/download.html)，可以在console下进入到 `bsp/synwit/swm181CBT6-LuatOS`目录中，运行以下命令：

`scons`

来编译这个板级支持包。如果编译正确无误，会产生rtthread.elf、rtthread.bin文件。其中 rtthread.bin 可以烧写到设备中运行。

## 烧写及执行

### 硬件连接

- 使用 USB C-Type 数据线连接开发板到 PC（注意：需要下载安装串口驱动支持 CH340 芯片，使用 MDK5 需要安装 SWM181 相关的 pack）。

  >  USB B-Type 数据线用于串口通讯，同时供电

- 使用 swd 连接开发板到 PC （需要 Jlink 驱动）

将串口 1 引脚为：`[PA2/PA3]`和 USB串口CH340 相连，串口配置方式为115200-N-8-1。

当使用 [env工具](https://www.rt-thread.org/page/download.html) 正确编译产生出rtthread.bin映像文件后，可以使用 ISP 的方式来烧写到设备中。

**建议使用 keil 软件直接下载**。ISP 下载较复杂。

### 运行结果

如果编译 & 烧写无误，当复位设备后，会在串口上看到板子上的蓝色LED闪烁。串口打印RT-Thread的启动logo信息：

```
 \ | /
- RT -     Thread Operating System
 / | \     4.0.0 build Dec 11 2018
 2006 - 2018 Copyright by rt-thread team
msh />
```
## 外设支持

本 BSP 目前对外设的支持情况如下：

| **片上外设**        | **支持情况**  | **备注**                              |
| :----------------- | :----------: | :----------------------------------- |
| GPIO               |     支持     | PA0, PA1... PP23 ---> PIN: 0, 1...100（中断/IRQ 未实现） |
| UART               |     支持     | UART0 基础收发可用（DMA/部分中断路径待完善） |
| ADC                |   支持(基础)  | adc0 单次轮询转换（通道引脚复用需按硬件配置） |
| TIM                |     ?     | TIM0/1/2/3/4/5                        |
| I2C                |   支持(基础)  | i2c0/i2c1 主机轮询（仅 7bit 地址） |
| PWM                |   支持(基础)  | pwm0/pwm1，通道 0/1 映射 A/B |
| RTC                |   支持(软)  | 基于系统 tick 的软 RTC（掉电不保持） |
| SPI                |   支持(基础)  | spi0/spi1 主机轮询（软件 CS） |
| WDT                |   支持(基础)  | wdt 超时/喂狗/启停（超时时间按秒配置） |
| CRC                |     ?     | CRC                                   |
| SDIO               |     ?     | SDIO                                  |
| SRAM               |     ?     | SRAM                                  |
| NOR FLASH          |     ?     | NOR FLASH                             |
| CAN                |    暂不支持   |                                       |

## 维护人信息

- [czstara12](https://github.com/czstara12)
- [邮箱](czstara12@3939831.xyz)

## 参考资料

* [RT-Thread 文档中心](https://www.rt-thread.org/document/site/)
* [SWM181数据手册](https://www.synwit.cn/uploads/soft/20220722/1-220H21TRIJ.pdf)
* [LuatOSwiki](https://wiki.luatos.com/chips/swm181/index.html)

# TODO

1. 完成对所有外设的支持
