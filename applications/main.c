#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "drv_gpio.h"

/* 定义测试引脚 (使用板载物理引脚编号) */
#define LED0_PIN 51  /* PB8 */
#define LED1_PIN 52  /* PB9 */
#define LED2_PIN 53  /* PD0 */

/* 外设设备名称 */
#define ADC_DEV_NAME        "adc0"
#define PWM_DEV_NAME        "pwm0"
#define I2C_DEV_NAME        "i2c0"
#define SPI_DEV_NAME        "spi0"
#define CAN_DEV_NAME        "can1"
#define HWTIMER_DEV_NAME    "timer0"
#define UART_DEV_NAME       "uart1"

int main(void)
{
    rt_uint32_t val;
    rt_device_t dev;

    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_kprintf("SWM181 BSP Driver Test Start...\n");

    /* 1. UART 测试 (UART1) */
    dev = rt_device_find(UART_DEV_NAME);
    if (dev)
    {
        rt_device_open(dev, RT_DEVICE_FLAG_RDWR);
        rt_device_write(dev, 0, "UART1 Test OK\n", 14);
    }

    /* 2. ADC 测试 (Channel 1) */
    rt_adc_device_t adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev)
    {
        rt_adc_enable(adc_dev, 1);
        val = rt_adc_read(adc_dev, 1);
        rt_kprintf("ADC Channel 1 Value: %d\n", val);
    }

    /* 3. PWM 测试 (PWM0 Channel 1) */
    struct rt_device_pwm *pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev)
    {
        rt_pwm_set(pwm_dev, 1, 1000000, 500000); /* 1kHz, 50% duty */
        rt_pwm_enable(pwm_dev, 1);
        rt_kprintf("PWM0 Channel 1 Enabled\n");
    }

    /* 4. I2C 测试 (i2c0) */
    struct rt_i2c_bus_device *i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C_DEV_NAME);
    if (i2c_bus)
    {
        rt_kprintf("I2C0 Bus Found\n");
    }

    /* 5. SPI 测试 (spi0) */
    struct rt_spi_device *spi_dev = (struct rt_spi_device *)rt_device_find(SPI_DEV_NAME);
    if (spi_dev)
    {
        rt_kprintf("SPI0 Device Found (Note: needs attach first in real use)\n");
    }

    /* 6. CAN 测试 (can1) */
    rt_device_t can_dev = rt_device_find(CAN_DEV_NAME);
    if (can_dev)
    {
        rt_kprintf("CAN1 Device Found\n");
    }

    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
