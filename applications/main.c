#include "SWM181.h"

#include "rtthread.h"
#include <rtdevice.h>
#include <board.h>

#define LED0_PIN 8
#define LED1_PIN 9

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
	while (1)
	{
		rt_pin_write(LED1_PIN, PIN_HIGH);
		rt_thread_delay(300);
		rt_pin_write(LED1_PIN, PIN_LOW);
		rt_thread_delay(300);
	}
}
void TaskPWM(void *arg)
{
	GPIO_Init(GPIOD, PIN0, 1, 0, 0, 0); // 调试指示信号
	while (1)
	{
		GPIO_InvBit(GPIOD, PIN0);
		rt_thread_delay(500);
	}
}
