#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

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
#include "driverlib/systick.h"

#include "lcd.h"
#include "adc.h"
#include "bluetooth.h"
#include "utils.h"
#include "monitorafluxo.h"
#include "i2cmod.h"
#include "utils.h"
#include "valvula.h"

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
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_1);
	if(!GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)) { //IF ENABLED (B2 == 0)
		pulsos_contados++;
	}
}
/************************************************/

int32_t rede, bateria = 0;
char* recebeDadosBluetooth;
const uint32_t halfseg = 1666666;//833333;


void Timer_Init() {
	uint32_t ui32Period = 0;
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

void Inicia_Tiva() {
	// Configura o clock para 50MHz
	SysCtlClockSet(SYSCTL_SYSDIV_2 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ| SYSCTL_OSC_MAIN);
	LCD_Init();
	ADC_Init();
	Bluetooth_Init();
	I2C_Init();
	Valvula_Init();
	Timer_Init();
}

void LeSensores() {

	rede = ADC_Read(ADC0_BASE);
//	bateria = ADC_Read(ADC1_BASE);

	if(rede <= 18)
		LCD_BlackLight_Disable();
	else
		LCD_BlackLight_Enable();

	if(rede == ADC_ERROR_TIMEOUT || bateria == ADC_ERROR_TIMEOUT)
	{
		LCD_Clear();
		LCD_Write("ERRO ADC!", 1);
		SysCtlDelay(halfseg*4);
	}

}

void InitSensores() {
	rede = 0;
	bateria = 0;

	int i = 0;
	for (i = 0; i < 10; i++) {
		rede = ADC_Read(ADC0_BASE);
		bateria = ADC_Read(ADC1_BASE);
		LCD_Process();
	}
}

 void main(void) {
	Inicia_Tiva();


	LCD_BlackLight_Enable();
	LCD_Clear();
	LCD_Write("  Bem Vindo!!!", 0);
	LCD_Write("Medidor de agua!", 1);

	InitSensores();

	Delay(2000);
	LCD_Clear();

	while(1) {
		Delay(500);

		LeSensores();

		recebeDadosBluetooth = Bluetooth_RecebeDados();
		char temp[5];
		int i;
		for(i = 0; i < 5; i++){
			temp[i] = recebeDadosBluetooth[i];
		}
		if( strcmp(temp, "atual") == 0 ){}
		if( strcmp(temp, "anter") == 0 ){}
		if( strcmp(temp, "histo") == 0 ){}

		LCD_Process();
	}
}

void Timer0IntHandler(void)
{
	//CLEAR INTERRUPT FLAG
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	tempo_passado++;
}
