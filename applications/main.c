#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "drv_gpio.h"

#define LED0_PIN 51  /* PB8 */

int main(void)
{
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_kprintf("RT-Thread Blinky Debug Mode...\n");

    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
