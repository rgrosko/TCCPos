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

#include "lcd.h"
#include "teclado.h"
#include "i2c.h"
#include "keypad.h"
#include "UART.h"
#include "Impressora.h"
#include "FuncoesMenu.h"

char tmp[20];

void IniciaTiva() {
	// Configura o clock para 50MHz
	SysCtlClockSet(	SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ| SYSCTL_OSC_MAIN);
	LCD_Init();
	Teclado_Init();
	I2C_Init();
	KEYPAD_Init();
	UART_Init();
	Impressora_Init();
}

void ShowDateTime() {
	DS1307_GetDate(tmp);
	LCD_Write(tmp, 0, 0);
	DS1307_GetTime(tmp);
	LCD_Write(tmp, 1, 0);
}

void main(void)
{
	IniciaTiva();
	//LCD_Write("Inicializando", 0, 0);
	ResetMem();

	LCD_Clear();
	LCD_Write("  Bem Vindo!!!", 0, 0);
	LCD_Write(" Relogio Ponto!", 1, 0);
	Delay(2000);
	LCD_Clear();

	while(1) {

		ShowDateTime();

		tecla = Teclado_GetBotao();
		if(strcmp(tecla, "C") == 0)
		{
			FuncoesMenu_Menu();
		}else if(strcmp(tecla, "A") == 0)
		{
			FuncoesMenu_SalvaEntrada();
		}
		else if(strcmp(tecla, "B") == 0)
		{
			FuncoesMenu_SalvaSaida();
		}
	}
}

