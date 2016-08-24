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

char tmp[20];
int32_t lm35Interna, lm35Peltier = 0;
const uint32_t halfseg = 1666666;//833333;

void GPIO_Init() {
	//INPUTS: PB2 - ENABLE READ / PD1 - COUNT
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);
	//OUTPUTS: OPEN VALVE (PF1|PE3) / CLOSE VALVE (PE2|PD3)
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3);
	//CONFIGURE GPIO INTERRUPT  (PD1 RISE AND DOWN)
	GPIOIntTypeSet(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTD_BASE,GPIODIntHandler);
	GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_1);
}

void Timer_Init() {//MODIFICADO: 18/08/2016
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
	//Timer
	/*SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	uint32_t sysCtlClockGet = SysCtlClockGet() * 5;
	TimerLoadSet(TIMER0_BASE, TIMER_A, sysCtlClockGet - 1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	//count tick
	SysTickPeriodSet(SysCtlClockGet() / 1000000-1);
	SysTickEnable();
	SysTickIntEnable();
	//
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);*/
}

void Inicia_Tiva() {
	// Configura o clock para 50MHz
	SysCtlClockSet(SYSCTL_SYSDIV_2 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ| SYSCTL_OSC_MAIN);
	LCD_Init();
	ADC_Init();
	Bluetooth_Init();
	I2C_Init();
	GPIO_Init();
	Timer_Init();
}

void LeSensores() {

	uint32_t rede = ADC_Read(ADC0_BASE);
	uint32_t bateria = ADC_Read(ADC1_BASE);

	if(rede > 0) // TODO - mudar valor de referencia
	{
		LCD_BlackLight_Disable();
		Bluetooth_Disable();
	}
	else
	{
		LCD_BlackLight_Enable();
		Bluetooth_Enable();
	}

	if(lm35Interna == ADC_ERROR_TIMEOUT || lm35Peltier == ADC_ERROR_TIMEOUT)
	{
		SysCtlDelay(halfseg*4);
	}
}

 void main(void) {
	Inicia_Tiva();

	/*REFTEMPO tempo;
	uint8_t date[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
	StartMonit(date, &tempo);*/
	//VIEW VARS P/ TESTE //////////
	uint8_t data[3];
	uint8_t hora[3];
	DADOANUAL valor_anual;
	DADOMEDIDA valor_diaria;
	//VIEW VARS P/ TESTE //////////
	REFTEMPO referencia;
	uint8_t date[6] = {0x00,0x00,0x0C,0x18,0x08,0x10};
	uint8_t modo_atual = START;
	uint16_t leituras_salvas= 0;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

	LCD_BlackLight_Enable();
	LCD_Clear();
	LCD_Write("  Bem Vindo!!!", 0);
	LCD_Write("Medidor de agua!", 1);
	Delay(2000);
	//LCD_Write("BOOOORAAAAA", 2);
	//LCD_Write("PPPPOOOOORRRA", 3);
//	LCD_BlackLight_Disable();
//	LCD_Clear();

	char* recebe;
	char test;
	int led = 0;

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDBLUE);
	StartMonit(date, &referencia);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

	//TESTES NA PLACA
	DS1307_GetDate(data);
	DS1307_GetTime(hora);
	//->BREAKPOINT: VIEW VARS<-//
	valor_anual = EEPROM_PegaMedia(ANO1 + 1);
	valor_diaria = EEPROM_PegaLeitura(MES1 +3);
	//->BREAKPOINT: VIEW VARS<-//
	Delay(1);

	while(1) {
		if(led > 6)
			led = 0;
		Delay(1000);
	//	LeSensores();
	//	ShowDateTime();
		Bluetooth_EnviaDados(1, 2, 3);
		recebe = Bluetooth_RecebeDados();
		test =led + '0';// recebe[7];
		if(test == '1')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDYELLOW);
		else if(test == '2')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDBLUE);
		else if(test == '3')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDGREEN);
		else if(test == '4')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDRED);
		else if(test == '5')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDVIOLET);
		else if(test == '6')
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDWHITE);
		led++;

		//PREPARA ESTRUTURA COM VALOR INICIAL
		valor_anual.end.word = referencia.end_media.word;
		valor_anual.acu_hi.word = 0x0000;
		valor_anual.acu_lo.word = led;
		valor_anual.cnt_ms.word = valor_anual.cnt_ms.word + 0x0001;
		valor_anual.cnt_ag.word = valor_anual.cnt_ag.word + 0x0002;

		DS1307_GetDate(data);
		DS1307_GetTime(hora);
		valor_diaria.end.word = referencia.end_diaria.word; //+5; //PR�XIMO ENDERE�O PARA SALVAR AP�S UMA HORA DE LEITURAS!
		valor_diaria.datetime[0] = data[0]; //DIA
		valor_diaria.datetime[1] = hora[0]; //HOR
		valor_diaria.datetime[2] = hora[1]; //MIN
		valor_diaria.datetime[3] = hora[2]; //SEG
		valor_diaria.med.word = led + 0x0003;

		EEPROM_SalvaMedia(valor_anual);
		EEPROM_SalvaLeitura(valor_diaria);
		//->BREAKPOINT: VIEW VARS<-//
		valor_anual = EEPROM_PegaMedia(referencia.end_media.word);
		valor_diaria = EEPROM_PegaLeitura(referencia.end_diaria.word);
		//->BREAKPOINT: VIEW VARS<-//
		Delay(1);
	}
}

void Timer0IntHandler(void)
{
	//CLEAR INTERRUPT FLAG
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	tempo_passado++;
}
