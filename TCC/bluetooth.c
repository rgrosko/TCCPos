/*
 * bluetooth.c
 *
 *  Created on: 29/05/2014
 *      Author: Rafael
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"

#include "bluetooth.h"

void Bluetooth_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB0_U1RX);
	GPIOPinConfigure(GPIO_PB1_U1TX);
	GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600, UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE);
}

void Bluetooth_EnviaValor(uint32_t valor) {
	unsigned char temp[5];
	sprintf(temp,"%d",(valor));
	int i = 0;
	for (i = 0; i < 3; i++) {
		UARTCharPut(UART1_BASE, temp[i]);
	}
}

void Bluetooth_EniaDados(uint32_t valor, uint32_t data, uint32_t time)
{
	//envia data
	UARTCharPut(UART1_BASE, ' ');
	Bluetooth_EnviaValor(valor);
}

char* Bluetooth_RecebeDados(void)
{
	char *dado;
	int index = 0;
	while(UARTCharsAvail(UART1_BASE)){
		dado[index] = UARTCharGet(UART1_BASE);
		index++;
	}
	return dado;
}