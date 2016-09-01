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


int32_t rede, bateria = 0;
int index;
char* recebeDadosBluetooth;
uint8_t tempo_passado;
uint16_t pulsos_contados;
REFTEMPO referencia;
uint8_t date[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t modo_atual = START, aberta = 0x00;
uint16_t leituras_salvas= 0;
const uint32_t halfseg = 1666666;//833333;

void GPIODIntHandler(void) {
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_1);
	if(!GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)) { //IF ENABLED (B2 == 0)
		pulsos_contados++;
	}
}
/************************************************/



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
	bateria = ADC_Read(ADC1_BASE);

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

void ShowDateTime() {
	uint8_t tmp[3];
	char imprime[20];
	//LCD_Clear();

	DS1307_GetDate(tmp);
	sprintf(imprime, "%02d/%02d/%02d", tmp[0], tmp[1], tmp[2]);
	LCD_WriteCol(imprime, 0, 0);

	DS1307_GetTime(tmp);
	sprintf(imprime, "%02d:%02d", tmp[0], tmp[1]);
	LCD_WriteCol(imprime, 0, 12);
}

void EnviaValorAtual(const REFTEMPO* referencia) {
	LCD_Clear();

	LCD_Write("       ENVIA", 0);
	LCD_Write("       ULTIMO", 1);
	LCD_Write("       VALOR", 2);

	DADOMEDIDA ultimo;
	uint16_t mes_atual = referencia->end_proxmes.word - 0x1173; //TODO revisar
	if (referencia->end_diaria.word - 3 != mes_atual)
	ultimo = EEPROM_PegaLeitura(referencia->end_diaria.word - 6);
	else
	ultimo.med.word = 0;

	Bluetooth_EnviaMedicao(ultimo.med.word);

	Delay(1000);
	LCD_Clear();
}

void EnviaMediaAnual(const REFTEMPO* referencia) {
	LCD_Clear();

	LCD_Write("       ENVIA", 0);
	LCD_Write("       MEDIA ", 1);
	LCD_Write("       ANUAL", 2);

	DADOANUAL media;
	media = EEPROM_PegaMedia(referencia->end_proxano.word - 9);
	uint32_t valor = ((media.acu_hi.word * 65536) + media.acu_lo.word)
			/ media.cnt_ms.word;
	Bluetooth_EnviaMedicao(valor);

	Delay(1000);
	LCD_Clear();
}

void EnviaHistorico(){
	LCD_Clear();

	LCD_Write("       ENVIA ", 1);
	LCD_Write("     HISTORICO", 2);

	B16 cal;
	uint16_t mem_mes = MES1;
	uint8_t day[3];
	uint8_t hour[3];
	DADOMEDIDA hist;
	int ind_mes;
	for(ind_mes = 0; ind_mes < 6; ind_mes++) {
		mem_mes = mem_mes + (ind_mes*0x1173);
		cal = GetMesAno(mem_mes);
		int ler;
		for(ler = 0; ler < 744; ler++) {
			hist = LeSequencia((ind_mes*0x1173), ler);
			day[0] = hist.datetime[0];
			day[1] = cal.byte[1];
			day[2] = cal.byte[0];
			hour[0] = hist.datetime[1];
			hour[1] = hist.datetime[2];
			hour[2] = hist.datetime[3];
			Bluetooth_EnviaDados(hist.med.word, day, hour);
		}
	}

	Delay(1000);
	LCD_Clear();
}

void ResetMemoria() {
	LCD_Clear();

	LCD_Write("        RESET", 1);
	LCD_Write("       MEMORIA", 2);

	ResetMem();

	LCD_Clear();
}

void AbreValvula() {
	LCD_Clear();

	OpenValve();

	LCD_Write("    ABRE VALVULA", 0);
	LCD_Write("---------/ /--------", 2);

	Delay(1000);
	LCD_Clear();
}

void FechaValvula() {
	LCD_Clear();

	CloseValve();

	LCD_Write("   FECHA VALVULA", 0);
	LCD_Write("--------------------", 2);

	Delay(1000);
	LCD_Clear();
}

void AjustaHora(){
	LCD_Clear();

	LCD_Write("  RECEBENDO DADOS", 0);

	//recebe data
	char imprime[15];
	char auxData[6] = "999999";
	Delay(1000);
	while(auxData[0] == '9'){
		Delay(200);
		recebeDadosBluetooth = Bluetooth_RecebeDados();
		for(index = 0; index < 6; index++){
			auxData[index] = recebeDadosBluetooth[index];
		}
	}
	date[3] = ((auxData[0] - 0x30 ) * 10) + (auxData[1] - 0x30);
	date[4] = ((auxData[2] - 0x30 ) * 10) + (auxData[3] - 0x30);
	date[5] = ((auxData[4] - 0x30 ) * 10) + (auxData[5] - 0x30);

	sprintf(imprime, "DATA %02d/%02d/%02d", date[3], date[4], date[5]);
	LCD_Write(imprime, 1);

	//recebe hora
	char auxTime[6] = "999999";
	Delay(1000);
	while(auxTime[0] == '9'){
		Delay(200);
		recebeDadosBluetooth = Bluetooth_RecebeDados();
		for(index = 0; index < 6; index++){
			auxTime[index] = recebeDadosBluetooth[index];
		}
	}
	date[2] = ((auxTime[0] - 0x30 ) * 10) + (auxTime[1] - 0x30);
	date[1] = ((auxTime[2] - 0x30 ) * 10) + (auxTime[3] - 0x30);
	date[0] = ((auxTime[4] - 0x30 ) * 10) + (auxTime[5] - 0x30);

	sprintf(imprime, "HORA %02d:%02d:%02d", date[2], date[1], date[0]);
	LCD_Write(imprime, 2);

	Delay(1000);

	//HORA - MINUTO - SEGUNDO
	DS1307_SetTime(date[2],date[1],date[0]);
	//DIA - MES - ANO
	DS1307_SetDate(date[3],date[4],date[5]);

	LCD_Clear();
}

void ImprimeVolts(){
	LCD_Clear();

	LCD_Write("FONTES DE ENERGIAS", 0);

	char imprime[10];

	float volt = bateria / 1.67;
	sprintf(imprime, "BATERIA = %.2f V", volt); // 9.0v -> 2.18V
	LCD_Write(imprime, 1);

	volt = rede / 2;
	sprintf(imprime, "REDE = %.2f V", volt); //12.4V -> 2.80V
	LCD_Write(imprime, 2);

	Delay(1000);
	LCD_Clear();
}

void ImprimeStatusAgua(uint8_t aberta) {
	if (aberta == 0x01)
		LCD_Write("AGUA OK", 3);

	if (aberta == 0x00)
		LCD_Write("FALTA DE AGUA", 3);
}

 void main(void) {
	Inicia_Tiva();

	LCD_BlackLight_Enable();
	LCD_Clear();
	LCD_Write("    BEM VINDO!!!", 1);
	LCD_Write("  MEDIDOR DE AGUA!", 2);

	InitSensores();

	Delay(2000);
	LCD_Clear();
	StartMonit(date, &referencia);

	while(1) {
		Delay(250);
		LeSensores();

		recebeDadosBluetooth = Bluetooth_RecebeDados();
		char comando;
//		char temp[5];
//		for(i = 0; i < 5; i++){
//			temp[i] = recebeDadosBluetooth[i];
//		}
		comando = recebeDadosBluetooth[5];
		if(comando == '1'){
	//	if( strcmp(temp, "atual") == 0 ){
			EnviaValorAtual(&referencia);
		}
		if(comando == '2'){
	//	if( strcmp(temp, "anter") == 0 ){
			EnviaMediaAnual(&referencia);
		}
 		if(comando == '3'){
	//	if( strcmp(temp, "histo") == 0 ){
			EnviaHistorico();
		}
		if(comando == '4'){
	//	if( strcmp(temp, "reset") == 0 ){
			ResetMemoria();
		}
		if(comando == '5'){
	//	if( strcmp(temp, "ajust") == 0 ){
			AjustaHora();
		}
		if(comando == '6'){
	//	if( strcmp(temp, "abrir") == 0 ){
			AbreValvula();
		}
		if(comando == '7'){
	//	if( strcmp(temp, "fecha") == 0 ){
			FechaValvula();
		}
		if(comando == '8'){
	//	if( strcmp(temp, "volts") == 0 ){
			ImprimeVolts();
		}

		ShowDateTime();
		ImprimeStatusAgua(aberta);
		Scan(&referencia, &modo_atual, &tempo_passado, &pulsos_contados, &leituras_salvas);

		if(modo_atual == ENABLED)	{
			TimerEnable(TIMER0_BASE, TIMER_A);
		} else if(modo_atual == RESTART || modo_atual == DISABLED) {
			TimerDisable(TIMER0_BASE, TIMER_A);
			modo_atual = START;
			Delay(1);
		}

		if(modo_atual != DISABLED) {
			if(aberta == 0x00) {
				OpenValve();
				aberta = 0x01;
			}
		} else {
			if(aberta == 0x01) {
				CloseValve();
				aberta = 0x00;
			}
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
