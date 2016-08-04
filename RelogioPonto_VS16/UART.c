/*
 * UART.c
 *
 *  Created on: 26/06/2014
 *      Author: Rafael
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"

#include "UART.h"
#include "i2c.h"

char dado[1];

void UART_SetDateTime(void);
void UART_AddUser(void);
void UART_RemoveUser(void);
int32_t GetMatriculaUART(void);
void SendUART(char* tmp);

void UART_Init(void)
{
	// Habilita os perifericos UART0 e GPIOA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	// Configura PA0 e PA1 como RX e TX da UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configura a UART para funcionar a 115200 bps, 8N1
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));

	// Habilita as interrupcoes do processador
    IntMasterEnable();

	// Habilita a interrupcao da UART0
    IntEnable(INT_UART0);

	// Habilita interrupcao por recepcao
    UARTIntEnable(UART0_BASE, UART_INT_RX);

    // Habilita a FIFO
    UARTFIFOEnable(UART0_BASE);
}

void ReadUART() {
	dado[0] = UARTCharGet(UART0_BASE);
}

void SendUART(char* tmp) {
	uint8_t dadoTemp;
	while (*tmp)
	{
		dadoTemp = *tmp++;
		UARTCharPut(UART0_BASE, dadoTemp);
	}
	UARTCharPut(UART0_BASE, '\n');
}

void UART_Send(void)
{
	ReadUART();
	char temp[32];
	uint16_t nr;
	DADO reg;
	int index;

	nr = EEPROM_GetNrFuncionario();
	if(nr)
	{
		for(index = 1; index < (nr+1); index++)
		{
			reg = EEPROM_GetRegistros(index);
			if(reg.mat.word)
			{
				sprintf(temp, "%s;%02d/%02d/%02d;%02d:%02d;%02d:%02d", reg.nome, reg.time[4],reg.time[5],reg.time[6], reg.time[1],reg.time[0], reg.time[3],reg.time[2]);
				SendUART(temp);
			}
		}
	}
	SendUART("#end#");
}

void UART_SetDateTime(void)
{
	int32_t input = 0, i, index = 0;
	int32_t dataHora[5];
	ReadUART();
	while(dado[0] != '#')
	{
		for(i = 0; i < 2; i++)
		{
			if (atoi(dado) != 0 || (strcmp(dado, "0") == 0))
			{
				if (input == 0)
					input = atoi(dado);
				else
					input = input * 10 + atoi(dado);
			}
			ReadUART();
		}
		dataHora[index++] = input;
		input = 0;
	}

	DS1307_SetDate(dataHora[0],dataHora[1],dataHora[2]);
	DS1307_SetTime(dataHora[3],dataHora[4],00);
	SendUART("#ok#");
}

int32_t GetMatriculaUART(void) {
	int32_t number = 0;
	while (atoi(dado) != 0 || (strcmp(dado, "0") == 0)) {
		if (number == 0)
			number = atoi(dado);
		else
			number = number * 10 + atoi(dado);

		ReadUART();
	}
	return number;
}

void UART_AddUser(void)
{
	DADO input;
	ReadUART();

	input.mat.word = GetMatriculaUART();

	strcpy(input.nome, "");

	while(dado[0] != '#')
	{
		strcat(input.nome, dado);
		ReadUART();
	}

	EEPROM_AdicionaFuncionario(input);
	SendUART("#ok#");
}

void UART_RemoveUser(void)
{
	ReadUART();

	int32_t matricula = GetMatriculaUART();

	if(!EEPROM_EncontraFuncionario(matricula,true))
		SendUART("#error#");
	else
		SendUART("#ok#");
}

void UART_Receive(void)
{
	uint32_t status;

	// Lê o status da interrupcao
    status = UARTIntStatus(UART0_BASE, true);
    // Limpa o status da interrupcao
    UARTIntClear(UART0_BASE, status);

    // Enquanto houver bytes disponiveis na serial
    while(UARTCharsAvail(UART0_BASE))
    {
    	ReadUART();
		if(dado[0] == '#')
		{
			ReadUART();
			switch(dado[0])
			{
			case 'D':
				UART_SetDateTime();
				break;
			case 'A':
				UART_AddUser();
				break;
			case 'R':
				UART_RemoveUser();
				break;
			case 'I':
				UART_Send();
				break;
			}
		}

    }
}
