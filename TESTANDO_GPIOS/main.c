/********************
 * LIBRARIES
 ********************/
#include <stdint.h>
#include <stdbool.h>

#define PART_TM4C123GH6PM
//#define TARGET_IS_BLIZZARD_RB1

#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

/********************
 * MACRO DEFINITIONS
 ********************/

/************
 * MAIN LOOP
 ************/
uint32_t ui32Pulso;
uint8_t ui8Tempo;
int main(void) {
	uint32_t ui32Period;
	uint8_t flag;
	unsigned char CPulso[4];
	/***************
	* CONFIG BLOCK
	****************/
	//clock 50MHz:
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	//ENABLE LEDs (PF1: RED | PF2: BLUE | PF3: GREEN)
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	//INPUTS: PD1 - ENABLE READ / PD2 - COUNT
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	//GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);
	//GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3);

	// ENABLE UART0 e GPIOA
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	//  PA0/RX_UART0 & PA1/TX_UART0
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// SERIAL: 115200 , 8N1
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
		(UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));

	//UNLOCK AND ENABLE SWITCHES
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	//CONFIGURE GPIO INTERRUPT
	GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_BOTH_EDGES);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);

	//ENABLE TIMER 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	//PERIOD: CLOCK_RATE / DESIRED FREQ. => 1/2 INTERRUPT
	ui32Period = (SysCtlClockGet() / 1) / 512;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

	//ENABLE TIMER INTERRUPT
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();

	// ENABLE THE RUN OF TIMER
	//TimerEnable(TIMER0_BASE, TIMER_A);

	//UART FIFO
	UARTFIFOEnable(UART0_BASE);

	/**********************
	* FOREVER LOOP BLOCK
	***********************/
	flag = 0x00;
	ui8Tempo = 0;
	ui32Pulso = 0;
	TimerDisable(TIMER0_BASE, TIMER_A);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1); 				     //LED START: RED
	GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0X00);
	while(1) {
		if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)) {
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1|GPIO_PIN_2);	 //LED SIGNAL: YELLOW
			GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0X00);			  // OPEN VALVE
			if(flag == 0x00) {											  //START READ CONDITION - F0X00
				flag = 0x01;
				ui8Tempo = 0;
				ui32Pulso = 0;
				TimerEnable(TIMER0_BASE, TIMER_A);
			} else if (ui8Tempo >= 8){
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);               //LED START TX: DOWN
				ui32Pulso = ui32Pulso/2;
				memcpy(&CPulso,&ui32Pulso,4);
				UARTCharPut(UART0_BASE,CPulso[3]);
				UARTCharPut(UART0_BASE,CPulso[2]);
				UARTCharPut(UART0_BASE,CPulso[1]);
				UARTCharPut(UART0_BASE,CPulso[0]);
			} else if (ui8Tempo >= 16) {
				TimerDisable(TIMER0_BASE, TIMER_A);
				SysCtlDelay(10000000);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1);         //LED END TX: UP RED
				flag = 0x00;
				SysCtlDelay(10000000);
			}
		} else {
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1|GPIO_PIN_3);	 //LED NO SIGNAL: RED
			GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3);		  // CLOSE VALVE
			ui8Tempo = 0;
			ui32Pulso = 0;
			flag = 0x00;
		}
	}
}


void Timer0IntHandler(void) {
	//CLEAR INTERRUPT FLAG
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	ui8Tempo++;
}

void GPIOFIntHandler(void) {
	//CLEAR INTERRUPT FLAG
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))  ui32Pulso++;
	SysCtlDelay(10000000);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	SysCtlDelay(10000000);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1);
}
