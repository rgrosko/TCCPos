#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

#include "i2c_mod.h" 
#include "MonitoraFluxo.h"

/**********************************************
 * GLOBAL (INTERRUPTS VARS) & MACRO DEFINITIONS
 **********************************************/
#define LEDRED   	0x02
#define LEDBLUE  	0x04
#define LEDVIOLET	0x06
#define LEDGREEN 	0x08
#define LEDYELLOW	0x0A
#define LEDWHITE	0x0E

#define START		0x00
#define ENABLED		0x01
#define MIDDLE		0X02
#define RESTART		0x04
#define DISABLED    0x08
		
uint8_t tempo_passado;			
uint16_t pulsos_contados;	
	
void GPIODIntHandler(void) {
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4);  //TEST ONLY: (PF4/SW1 PRESS AND RELEASE) 
	/*
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_1);  
	if(!GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)) { //IF ENABLED (B2 == 0)
	*/
	if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)) { //TEST ONLY: IF ENABLED (PF0/SW2 PRESSED)
		pulsos_contados++;
	}
}

void IniciaTiva() {	
	/***************
	* CONFIG BLOCK
	****************/
	//clock 50MHz:
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	//ENABLE LEDs (PF1: RED | PF2: BLUE | PF3: GREEN)
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	//INPUTS: PB2 - ENABLE READ / PD1 - COUNT
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);	
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);		
	//OUTPUTS: OPEN VALVE (PF1|PE3) / CLOSE VALVE (PE2|PD3)
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3);

	//UNLOCK AND ENABLE SWITCHES: SW1 = PF4 / SW2 = PF0
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	//CONFIGURE GPIO INTERRUPT  (PD1 RISE AND DOWN)
	/*GPIOIntTypeSet(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTD_BASE,GPIODIntHandler);
	GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_1);*/
	//TEST ONLY: PF4 => SW1 PRESS AND RELEASE)
	GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTF_BASE,GPIOFIntHandler);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
	

	//ENABLE TIMER 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	//PERIOD: CLOCK_RATE / DESIRED FREQ. => 1/2 INTERRUPT
	ui32Period = (SysCtlClockGet() / 1) / 2;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

	//ENABLE TIMER INTERRUPT
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();
}

void main(void)
{	
    //VIEW VARS////////////////////
	uint8_t data[3];
	uint8_t hora[3];
	DADOANUAL valor_anual;
	DADOMEDIDA valor_diaria;
	//VIEW VARS////////////////////
	REFTEMPO* referencia;           //POINTER-> VALORES ALTERADOS NAS FUNÇÕES
	uint8_t* modo_atual;			//POINTER-> VALORES ALTERADOS NAS FUNÇÕES		
	uint16_t* leituras_salvas;		//POINTER-> VALORES ALTERADOS NAS FUNÇÕES
	uint8_t agora[6] = {0x00,0x00,0x0C,0x05,0x08,0x10};
	/*agora[0] = 0x00; //SEG 
	agora[1] = 0x00; //MIN
	agora[2] = 0x0C; //HOR
	agora[3] = 0x05; //DIA 
	agora[4] = 0x08; //MES
	agora[5] = 0x10; //ANO*/
	
	IniciaTiva();
	StartMonit(agora,referencia);
    *modo_Atual = START;
	tempo_passado = 0;
	pulsos_contados = 0;
	TimerDisable(TIMER0_BASE, TIMER_A);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00); 				         	 //LED START DOWN
	Close_Valve();
	
	//TESTES NA PLACA
	DS1307_GetDate(data);
	DS1307_GetTime(hora);
	//->BREAKPOINT: VIEW VARS<-//
	valor_anual = EEPROM_PegaMedia(ANO1 + 1);
	valor_diaria = EEPROM_PegaLeitura(MES1 +3);
	//->BREAKPOINT: VIEW VARS<-//
		
	while(1) {
		Scan(referencia, modo_atual, &tempo_passado, &pulsos_contados, leituras_salvas);
		if(*modo_atual == START) 
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDYELLOW);	 		 //LED SIGNAL: YELLOW
		else if(*modo_atual == ENABLED)	{	
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDGREEN);           //LED START TX: WHITE		
			TimerEnable(TIMER0_BASE, TIMER_A);	
		} else if(*modo_atual == MIDDLE) {
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDVIOLET);          //LED END TX: VIOLET			
		} else if(*modo_atual == RESTART || *modo_atual == DISABLED) {
			if(*modo_atual == DISABLED)
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,LEDRED);	 		 //LED NO SIGNAL: RED
			else
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,LEDBLUE);	 	 //LED NO SIGNAL: BLUE
			TimerDisable(TIMER0_BASE, TIMER_A);
			*modo_atual = START;
			Delay(1);
			//->BREAKPOINT: VIEW VARS<-//
			valor_anual = EEPROM_PegaMedia(referencia.end_media);
			valor_diaria = EEPROM_PegaLeitura(referencia.end_diaria);
			//->BREAKPOINT: VIEW VARS<-//
		}		
	}
}

void Timer0IntHandler(void) {
	//CLEAR INTERRUPT FLAG
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	tempo_passado++;	
}
