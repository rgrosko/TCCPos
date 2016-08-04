/*
 * Teclado.c
 *
 *  Created on: 17/05/2014
 *      Author: Rafael
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#include "teclado.h"

void Teclado_EscreveLinha(uint32_t row);
uint32_t Teclado_LeColuma(void);
char *Teclado_PegaValor(uint32_t row, uint32_t column);


void Teclado_Init(void)
{
	//Habiliata as portas que serao usadas para leitura e escrita no teclado
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	//Habilita pinos que serao usadas para escrever nas linhas do teclado
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6);

	//Habilita os pinos  1, 2 e 3 da porta E como entrada, que serao usadas para ler as columnas do teclado
	GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_DIR_MODE_IN);
	GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_7);
	GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPD);
	GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);
	GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_FALLING_EDGE);
	GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_7);

	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;

	GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_DIR_MODE_IN);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7);
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPD);
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7);
	GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_FALLING_EDGE);
	GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7);

	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPD);
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);

}

void Delay(uint32_t ms)
{
	uint32_t delay = (ms / 1000.0) / (3.0 / SysCtlClockGet());
	SysCtlDelay(delay);
}

void Teclado_Clear() {
	//limpas os pinos de entrada do teclado
	GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_RISING_EDGE);
	GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);

	GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_RISING_EDGE);
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7);

	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
}

char* Teclado_GetBotao()
{
	//Identifica o botao presionado
	//Primeiro escreve em uma linha
	// e le todas as colunas
	//se alguma coluna esta diferente de zero
	//pega o valor correspondente
	uint32_t x, column = 0;
	char *letter = "";
	for(x=1;x<=4;x++){
		Teclado_EscreveLinha(x);
		column = Teclado_LeColuma();
		if(column != 0){
			letter = Teclado_PegaValor(x, column);
			break;
		}
	}
	Teclado_Clear();
	if(strcmp(letter, "") != 0)
		Delay(250);
	return letter;
}

void Teclado_EscreveLinha(uint32_t row)
{
	//dada uma linha habilita seu pino correspondente
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0x00);
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, 0x00);
	switch(row)
	{
		case 1:
			GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3);
			break;
		case 2:
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
			break;
		case 3:
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);
			break;
		case 4:
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_PIN_6);
			break;
	}
}

uint32_t Teclado_LeColuma(void)
{
	//le todas os pinos relacionados as colunas,
	//e se alguma estiver habiliatda retorna um valor correspondente
	if (GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == GPIO_PIN_7) 	 { return 1; }
	else if (GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_6) == GPIO_PIN_6) { return 2; }
	else if (GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_7) == GPIO_PIN_7) { return 3; }
	else if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == GPIO_PIN_4) { return 4; }
	else return 0;
}

char *Teclado_PegaValor(uint32_t row, uint32_t column)
{
	//retorna o valor correspondente de uma linha X Columa
	if(row == 1 && column == 1) { return "1"; }
	else if(row == 1 && column == 2) { return "2"; }
	else if(row == 1 && column == 3) { return "3"; }
	else if(row == 1 && column == 4) { return "A"; }

	else if(row == 2 && column == 1) { return "4"; }
	else if(row == 2 && column == 2) { return "5"; }
	else if(row == 2 && column == 3) { return "6"; }
	else if(row == 2 && column == 4) { return "B"; }

	else if(row == 3 && column == 1) { return "7"; }
	else if(row == 3 && column == 2) { return "8"; }
	else if(row == 3 && column == 3) { return "9"; }
	else if(row == 3 && column == 4) { return "C"; }

	else if(row == 4 && column == 1) { return "*"; }
	else if(row == 4 && column == 2) { return "0"; }
	else if(row == 4 && column == 3) { return "#"; }
	else  return "D";
}
