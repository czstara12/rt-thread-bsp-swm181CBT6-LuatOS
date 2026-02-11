#include "SWM181.h"

#include "rtthread.h"
#include <rtdevice.h>
#include <board.h>
#include "drv_gpio.h"

#define LED0_PIN GET_PIN(B, 8)
#define LED1_PIN GET_PIN(B, 9)
#define LED2_PIN GET_PIN(D, 0)

void TaskADC(void *arg);
void TaskPWM(void *arg);

struct rt_thread taskADC;
static uint8_t stackADC[512];
static uint8_t priorityADC = 4;

struct rt_thread taskPWM;
static uint8_t stackPWM[512];
static uint8_t priorityPWM = 5;

int main(void)
{
	// SystemInit();
	PORT_Init(PORTA, PIN2, PORTA_PIN2_SWCLK, 1);
	PORT_Init(PORTA, PIN3, PORTA_PIN3_SWDIO, 1);

	rt_thread_init(&taskADC, "ADC", TaskADC, RT_NULL, stackADC, sizeof(stackADC), priorityADC, 20);
	rt_thread_startup(&taskADC);

	rt_thread_init(&taskPWM, "PWM", TaskPWM, RT_NULL, stackPWM, sizeof(stackPWM), priorityPWM, 20);
	rt_thread_startup(&taskPWM);

	rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

	while (1)
	{
		rt_pin_write(LED0_PIN, PIN_HIGH);
		rt_thread_delay(200);
		rt_pin_write(LED0_PIN, PIN_LOW);
		rt_thread_delay(200);
	}
}

void TaskADC(void *arg)
{
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	rt_adc_device_t adc_dev = (rt_adc_device_t)rt_device_find("adc0");
	if (adc_dev)
	{
		rt_adc_enable(adc_dev, 0);
	}
	while (1)
	{
		if (adc_dev)
		{
			(void)rt_adc_read(adc_dev, 0);
		}
		rt_pin_write(LED1_PIN, PIN_HIGH);
		rt_thread_delay(300);
		rt_pin_write(LED1_PIN, PIN_LOW);
		rt_thread_delay(300);
	}
}
void TaskPWM(void *arg)
{
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT); // ����ָʾ�ź�
	struct rt_device_pwm *pwm_dev = (struct rt_device_pwm *)rt_device_find("pwm0");
	if (pwm_dev)
	{
		rt_pwm_set(pwm_dev, 0, 1000000, 500000);
		rt_pwm_enable(pwm_dev, 0);
	}
	while (1)
	{
		rt_pin_write(LED2_PIN, !rt_pin_read(LED2_PIN));
		rt_thread_delay(500);
	}
}
