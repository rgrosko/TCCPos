#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#include "valvula.h"
#include "utils.h"

void Valvula_Init (void)
{
	// Habilita GPIO
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Programa como saída
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);

	// Programa como saída
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	// Programa como saída
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7);
}

void OpenValve(void) {
	//ACIONA------------------------------------------
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0x04); //OPEN1
	Delay(5);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0x08); //OPEN2
	//DELAY-------------------------------------------
	Delay(50);
	//RESETA------------------------------------------
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0x00); //OPEN1
	Delay(5);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0x00); //OPEN2
}

void CloseValve(void) {
	//ACIONA------------------------------------------
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x08); //CLOSE1
	Delay(5);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02); //CLOSE2
	//DELAY-------------------------------------------
	Delay(50);
	//RESETA------------------------------------------
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00); //CLOSE1
	Delay(5);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00); //CLOSE2
}
