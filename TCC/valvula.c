#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#include "valvula.h"
#include "utils.h"

void Valvula_Init (void)
{
	//UNLOCK AND ENABLE SWITCHES: SW1 = PF4 / SW2 = PF0
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

	// Habilita GPIO
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	//OUTPUTS: OPEN VALVE (PA2|PE3) / CLOSE VALVE (PF4|PF1)
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);
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
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x10); //CLOSE1
	Delay(5);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02); //CLOSE2
	//DELAY-------------------------------------------
	Delay(50);
	//RESETA------------------------------------------
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00); //CLOSE1
	Delay(5);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00); //CLOSE2
}
