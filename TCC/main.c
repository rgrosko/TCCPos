#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

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
#include "bluetooth.h"
#include "utils.h"
#include "monitorafluxo.h"
#include "i2cmod.h"

#define LEDRED   	0x02
#define LEDBLUE  	0x04
#define LEDVIOLET	0x06
#define LEDGREEN 	0x08
#define LEDYELLOW	0x0A
#define LEDWHITE	0x0E

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
	LCD_Init();
	ADC_Init();
	Bluetooth_Init();
	I2C_Init();
	Timer_Init();
}

void LeSensores() {

	uint32_t rede = ADC_Read(ADC0_BASE);
	uint32_t bateria = ADC_Read(ADC1_BASE);

	if(rede > 0) // TODO - mudar valor de referencia
	{
		LCD_BlackLight_Disable();
		Bluetooth_Disable();
	}
	else
	{
		LCD_BlackLight_Enable();
		Bluetooth_Enable();
	}

	if(lm35Interna == ADC_ERROR_TIMEOUT || lm35Peltier == ADC_ERROR_TIMEOUT)
	{
		SysCtlDelay(halfseg*4);
	}
}

 void main(void) {
	Inicia_Tiva();

	REFTEMPO tempo;
	uint8_t date[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
	StartMonit(date, &tempo);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
//	ResetMem();

//	LCD_BlackLight_Enable();
//	LCD_Clear();
//	LCD_Write("  Bem Vindo!!!", 0);
//	LCD_Write("Medidor de agua!", 1);
//	Delay(2000);
//	LCD_BlackLight_Disable();
//	LCD_Clear();

	 char* recebe;
	 char test;
	 int led = 0;

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDYELLOW);
	Delay(1000);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

	while(1) {
		if(led > 6)
			led = 0;
		Delay(1000);
	//	LeSensores();
	//	ShowDateTime();
		Bluetooth_EnviaDados(1, 2, 3);
		recebe = Bluetooth_RecebeDados();
		test =led + '0';// recebe[7];
		if(test == '1')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDYELLOW);
		else if(test == '2')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDBLUE);
		else if(test == '3')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDGREEN);
		else if(test == '4')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDRED);
		else if(test == '5')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDVIOLET);
		else if(test == '6')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDWHITE);
		led++;
	}
}

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}
