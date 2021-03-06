#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"

#include "utils.h"

volatile unsigned long long sys_us = 0;

unsigned long long getSysTick()
{
	return sys_us;
}

///
/// increment the elapsed seconds, minutes and hours if needed
void SysTick_Handler(void) {
	sys_us++; // update clock since booted in uS.
}

void Delay(uint32_t ms)
{
	uint32_t delay = (ms / 1000.0) / (3.0 / SysCtlClockGet());
	SysCtlDelay(delay);
}
