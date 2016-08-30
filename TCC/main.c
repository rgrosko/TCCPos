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

void GPIO_Init() {
	//INPUTS: PB2 - ENABLE READ / PD1 - COUNT
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);
	//CONFIGURE GPIO INTERRUPT  (PD1 RISE AND DOWN)
	GPIOIntTypeSet(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTD_BASE,GPIODIntHandler);
	GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_1);
}

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

	REFTEMPO referencia;
	uint8_t date[6] = {0x00,0x00,0x0C,0x1B,0x08,0x10};
	uint8_t modo_atual = START;
	uint16_t leituras_salvas= 0;


	LCD_BlackLight_Enable();
	LCD_Clear();
	LCD_Write("    BEM VINDO!!!", 1);
	LCD_Write("  MEDIDOR DE AGUA!", 2);

//	InitSensores();

	Delay(2000);
	LCD_Clear();
	StartMonit(date, &referencia);

	while(1) {
		Delay(500);
//		LeSensores();

//		OpenValve();
//		Delay(5000);
//		CloseValve();
//		Delay(5000);

		recebeDadosBluetooth = Bluetooth_RecebeDados();
		char temp[5];
		char comando;
		int i;
		for(i = 0; i < 5; i++){
			temp[i] = recebeDadosBluetooth[i];
		}
		comando = recebeDadosBluetooth[5];
		if(comando == '1'){
	//	if( strcmp(temp, "atual") == 0 ){
			LCD_Write("       ENVIA", 0);
			LCD_Write("      MEDICAO ", 1);
			LCD_Write("       ATUAL", 2);
			Delay(1000);
			LCD_Clear();
		}
		if(comando == '2'){
	//	if( strcmp(temp, "anter") == 0 ){
			LCD_Write("       ENVIA", 0);
			LCD_Write("        MES ", 1);
			LCD_Write("     ANTERIOR", 2);
			Delay(1000);
			LCD_Clear();
		}
 		if(comando == '3'){
	//	if( strcmp(temp, "histo") == 0 ){
			LCD_Write("       ENVIA ", 1);
			LCD_Write("     HISTORICO", 2);
			Delay(1000);
			LCD_Clear();
		}
		if(comando == '4'){
	//	if( strcmp(temp, "reset") == 0 ){
			LCD_Clear();
			LCD_Write("        RESET", 1);
			LCD_Write("       MEMORIA", 2);
			ResetMem();
			LCD_Clear();
		}
		if(comando == '5'){
	//	if( strcmp(temp, "ajust") == 0 ){
			LCD_Write("  RECEBENDO DADOS", 0);
			//recebe data
			char imprime[15];
			char auxData[6] = "999999";
			Delay(1000);
			while(auxData[0] == '9'){
				Delay(200);
				recebeDadosBluetooth = Bluetooth_RecebeDados();
				for(i = 0; i < 6; i++){
					auxData[i] = recebeDadosBluetooth[i];
				}
			}
			date[5] = ((auxData[0] - 0x30 ) * 10) + (auxData[1] - 0x30);
			date[4] = ((auxData[2] - 0x30 ) * 10) + (auxData[3] - 0x30);
			date[3] = ((auxData[4] - 0x30 ) * 10) + (auxData[5] - 0x30);
			sprintf(imprime, "DATA %d/%d/%d", date[5], date[4], date[3]);
			LCD_Write(imprime, 1);
			//recebe hora
			char auxTime[6] = "999999";
			Delay(1000);
			while(auxTime[0] == '9'){
				Delay(200);
				recebeDadosBluetooth = Bluetooth_RecebeDados();
				for(i = 0; i < 6; i++){
					auxTime[i] = recebeDadosBluetooth[i];
				}
			}
			date[2] = ((auxTime[0] - 0x30 ) * 10) + (auxTime[1] - 0x30);
			date[1] = ((auxTime[2] - 0x30 ) * 10) + (auxTime[3] - 0x30);
			date[0] = ((auxTime[4] - 0x30 ) * 10) + (auxTime[5] - 0x30);
			sprintf(imprime, "HORA %d/%d/%d", date[2], date[1], date[0]);
			LCD_Write(imprime, 2);

			Delay(1000);
			LCD_Clear();
		}
		if(comando == '6'){
	//	if( strcmp(temp, "abrir") == 0 ){
			OpenValve();
			LCD_Write("    ABRE VALVULA", 0);
			LCD_Write("---------/ /--------", 2);
			Delay(1000);
			LCD_Clear();
		}
		if(comando == '7'){
	//	if( strcmp(temp, "fecha") == 0 ){
			CloseValve();
			LCD_Write("   FECHA VALVULA", 0);
			LCD_Write("--------------------", 2);
			Delay(1000);
			LCD_Clear();
		}
		Scan(&referencia, &modo_atual, &tempo_passado, &pulsos_contados, &leituras_salvas);
		if(modo_atual == ENABLED)	{
			TimerEnable(TIMER0_BASE, TIMER_A);
		} else if(modo_atual == RESTART || modo_atual == DISABLED) {
			TimerDisable(TIMER0_BASE, TIMER_A);
			modo_atual = START;
			Delay(1);
		}

		LCD_Process();
	}
}

void Timer0IntHandler(void)
{
	//CLEAR INTERRUPT FLAG
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	tempo_passado++;
}
