#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "utils/ustdlib.h"

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"

#include "lcd.h"
#include "adc.h"
#include "sensor.h"
#include "bluetooth.h"
#include "i2c.h"
#include "utils.h"

char tmp[20];
int32_t lm35Interna, lm35Peltier = 0;
const uint32_t halfseg = 1666666;//833333;

void Timer_Init() {
	//Timer
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	uint32_t sysCtlClockGet = SysCtlClockGet() * 5;
	TimerLoadSet(TIMER0_BASE, TIMER_A, sysCtlClockGet - 1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	//count tick
	SysTickPeriodSet(SysCtlClockGet() / 1000000-1);
	SysTickEnable();
	SysTickIntEnable();
	//
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);
}

void Inicia_Tiva() {
	// Configura o clock para 50MHz
	SysCtlClockSet(SYSCTL_SYSDIV_2 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ| SYSCTL_OSC_MAIN);
//	LCD_Init();
	ADC_Init();
//	Bluetooth_Init();
//	I2C_Init();
	Timer_Init();
}

void ShowDateTime() {
	DS1307_GetDate(tmp);
	LCD_Write(tmp, 0);
	DS1307_GetTime(tmp);
	LCD_Write(tmp, 1);
}

void LeSensores() {

	lm35Peltier = ADC_Read(ADC0_BASE);
	lm35Interna = ADC_Read(ADC1_BASE);

	if(lm35Interna == ADC_ERROR_TIMEOUT || lm35Peltier == ADC_ERROR_TIMEOUT)
	{
		SysCtlDelay(halfseg*4);
	}
}

void main(void) {
	Inicia_Tiva();
//	ResetMem();

//	LCD_Clear();
//	LCD_Write("  Bem Vindo!!!", 0);
//	LCD_Write("Medidor de agua!", 1);
//	Delay(2000);
//	LCD_Clear();

	while(1) {
		LeSensores();
		//ShowDateTime();
	}
}

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}
