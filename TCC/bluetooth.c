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

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	//Programa UART
	GPIOPinConfigure(GPIO_PB0_U1RX);
	GPIOPinConfigure(GPIO_PB1_U1TX);
	GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Programa como saída
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);

	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600, UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE);
}

void Bluetooth_EnviaValor(char* identificador, uint32_t valor) {
	UARTCharPut(UART1_BASE, identificador[0]);

	char temp[6] = "\0";
	sprintf(temp,"%d",(valor));
	int i = 0;
	for (i = 0; i < 6; i++) {
		UARTCharPut(UART1_BASE, temp[i]);
	}
	UARTCharPut(UART1_BASE, '\n');
}

void Bluetooth_EnviaDados(uint32_t valor, uint8_t * data, uint8_t * time)
{
	//envia data
	uint32_t temp;
	temp = ((data[0] * 10000) + (data[1] * 100)) + data[2];
	Bluetooth_EnviaValor("D", temp);

	temp = ((time[0] * 10000) + (time[1] * 100)) + time[2];
	Bluetooth_EnviaValor("T", temp);

	Bluetooth_EnviaValor("M", valor);
}

void Bluetooth_EnviaMedicao(uint32_t valor)
{
	Bluetooth_EnviaValor("M", valor);
}

void Bluetooth_Enable(void)
{
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0x20);
}

void Bluetooth_Disable(void)
{
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0x00);
}

char* Bluetooth_RecebeDados(void)
{
	char dado[9]= "00000000";
	char aux;
	int index = 0;
	while(UARTCharsAvail(UART1_BASE)){
		aux = UARTCharGet(UART1_BASE);
		dado[index] = (aux - 0x30) + '0';
	    index++;
	}
	dado[8] = '\0';
	return dado;
}
