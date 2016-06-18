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

/**********************************************
 * GLOBAL & MACRO DEFINITIONS
 **********************************************/
#define LEDRED   	0x02
#define LEDBLUE  	0x04
#define LEDVIOLET	0x06
#define LEDGREEN 	0x08
#define LEDYELLOW	0x0A
#define LEDWHITE	0x0E

uint32_t ui32Pulso;
uint8_t ui8Tempo;

void GPIOFIntHandler(void) {
	//CLEAR INTERRUPT FLAG
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4);

	if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)) {
		ui32Pulso++;
		/*GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDGREEN);
		SysCtlDelay(10000000);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDBLUE);
		SysCtlDelay(10000000);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);*/
	}
}

char *IntToStr(int value, char *s, int radix) //-> como funciona essa fun��o? TODO
{
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    unsigned long ulvalue = value;
    char *p = s, *q = s;
    char temp;
    if (radix == 10 && value < 0) {
        *p++ = '-';
        q = p;
        ulvalue = -value;
    }
    do {
        *p++ = digits[ulvalue % radix];
        ulvalue /= radix;
    } while (ulvalue > 0);
    *p-- = '\0';
    while (q < p) {
        temp = *q;
        *q++ = *p;
        *p-- = temp;
    }
    return s;
}
/************
 * MAIN LOOP
 ************/
int main(void) {
	uint32_t ui32Period;
	uint8_t flag;
	unsigned char CPulso[3];
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

	//UNLOCK AND ENABLE SWITCHES: SW1 = PF4 / SW2 = PF0
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	//CONFIGURE GPIO INTERRUPT (PF4 => SW1 PRESS AND RELEASE)
	GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTF_BASE,GPIOFIntHandler);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);

	//ENABLE TIMER 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	//PERIOD: CLOCK_RATE / DESIRED FREQ. => 1/2 INTERRUPT
	ui32Period = (SysCtlClockGet() / 1) / 4;//512;//2; //-> qual � o per�odo?
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

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0X00); 				         	 //LED START DOWN

	GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3);		  							 		 //PD3 = 1 => CLOSE VALVE

	while(1) {

		if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)) {												 //SW2 PRESS
			if(flag == 0x00) {											  							 //START READ CONDITION - F0X00
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDYELLOW);	 		 //LED SIGNAL: YELLOW
				GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0X00);			  						 //PD3 = 0 => OPEN VALVE
				flag = 0x01;
				ui8Tempo = 0;
				ui32Pulso = 0;
				TimerEnable(TIMER0_BASE, TIMER_A);
			} else if (ui8Tempo == 8 && flag == 0x01) {
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDWHITE);           //LED START TX: WHITE
				//ui32Pulso = ui32Pulso/2;
				//memcpy(&CPulso,&ui32Pulso,4);
				//if(flag == 0x01) {
					IntToStr(ui32Pulso,CPulso,10);
					/*UARTCharPut(UART0_BASE,CPulso[3]);
					UARTCharPut(UART0_BASE,CPulso[2]);
					UARTCharPut(UART0_BASE,CPulso[1]);
					UARTCharPut(UART0_BASE,CPulso[0]);*/
					int ind = 0;
					for(ind = 0; ind < 3; ind++)
						UARTCharPut(UART0_BASE,CPulso[ind]);
					UARTCharPut(UART0_BASE,'\r');
					UARTCharPut(UART0_BASE,'\n');
					flag = 0x02;
				//}

			} else if (ui8Tempo >= 16) {
				TimerDisable(TIMER0_BASE, TIMER_A);
				//SysCtlDelay(10000000);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDVIOLET);          //LED END TX: VIOLET
				flag = 0x00;
				SysCtlDelay(10000000);
			}
		} else {																					  //SW2 RELEASE
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,LEDRED);	 		          //LED NO SIGNAL: RED
			GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3);		  							  //PD3 = 1 => CLOSE VALVE
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
