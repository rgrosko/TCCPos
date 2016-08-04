/*
 * Impressora.cpp
 *
 *  Created on: 27/06/2014
 *      Author: Rafael
 */



#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "Impressora.h"
#include "teclado.h"

char* dado;

void Impressora_Centraliza(void) {
	// Alinha
	UARTCharPut(UART1_BASE, 0x1B);
	UARTCharPut(UART1_BASE, 0x61);
	UARTCharPut(UART1_BASE, 0x31);
}


void Impressora_Init(void)
{
	// Habilita os perifericos UART1 e GPIOB
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	// Configura PB0 e PB1 como RX e TX da UART0
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configura a UART para funcionar a 9600bps, 8N1
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));

	Impressora_Centraliza();
}


void Impressora_Pula_Pagina(void) {
	uint32_t i = 0;
	for(i =0; i< 10; i++){
		UARTCharPut(UART1_BASE, '\n');
	}
}

void Impressora_ImprimeLinha(char* dado) {
	while (*dado) UARTCharPut(UART1_BASE, *dado++);
	UARTCharPut(UART1_BASE, '\n');
}

void Impressora_Pula_Linha(uint8_t n_linha){
	uint8_t i;
	for(i=1; i<n_linha; i++){
		UARTCharPut(UART1_BASE, 0x0A);	//LF
	}
}


void Impressora_Cut_Paper(void) {
	Impressora_Pula_Linha(5);

	// Aciona Guilhotina
	UARTCharPut(UART1_BASE, 0x1B);	//ESC
	UARTCharPut(UART1_BASE, 0x77);	//w
}

void Impressora_Cut_Parc_Paper(void) {
	Impressora_Pula_Linha(6);

	// Efetua o corte parcial do Papel
	UARTCharPut(UART1_BASE, 0x1B);	//ESC
	UARTCharPut(UART1_BASE, 0x6D);	//m
}

void Impressora_Sublinhado(int32_t on) {
	//Ativa modo de Sublinhado
	Delay(500);
	UARTCharPut(UART1_BASE, 0x1B);	//ESC
	UARTCharPut(UART1_BASE, 0x2D);	//-

	if(on == 1)
		UARTCharPut(UART1_BASE, 0x31);	//1 - ON
	else
		UARTCharPut(UART1_BASE, 0x30);	//0 - OFF

	Delay(500);
}

void Impressora_Realce(int32_t realce, int32_t densidade) {
	//Ativa modo de Realce
	UARTCharPut(UART1_BASE, 0x1B);	//ESC

	if(realce == 1)
		UARTCharPut(UART1_BASE, 0x45);	// E-1- Habilita modo Realce
	else
		UARTCharPut(UART1_BASE, 0x46);	// F-0- Desabilita modo Realce

	Delay(500);

	// Alta Densidade
	UARTCharPut(UART1_BASE, 0x1B);	//ESC
	UARTCharPut(UART1_BASE, 0x4E);	//N
	if(densidade == 1)
		UARTCharPut(UART1_BASE, 4);		//4 -Altissíma
	else
		UARTCharPut(UART1_BASE, 2);		//Normal
}

void Impressora_Italico(int32_t italico) {
	// Ativa modo Italico
	UARTCharPut(UART1_BASE, 0x1B); //ESC
	if(italico == 1)
		UARTCharPut(UART1_BASE, 0x34); // 4-1 - Habilita
	else
		UARTCharPut(UART1_BASE, 0x35); // 5-0 - Desabilita
}

