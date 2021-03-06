#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

// Esquema de Liga��o do Display LCD -----------------------
// RS - PA2
// EN - PA3
// D4 - PB4
// D5 - PB5
// D6 - PB6
// D7 - PB6 - muda por curto existente na tiva
//----------------------------------------------------------

int process = 0;

void LCD_Inst (unsigned char inst);
void LCD_Data (unsigned char data);
void LCD_EN_Pulse (void);

void LCD_Init (void)
{
	int i;

	// Habilita GPIOA e GPIOC
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	// Programa como sa�da
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);

	// Programa como sa�da
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	// Programa como sa�da
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_6);

	// EN=1
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x08);

	// RS=0
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x00);

	for(i=0;i<3;i++)
	{
		// Escreve 0x30 no Data (PC7-PC4)
		GPIOPinWrite(GPIO_PORTC_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0x30);

		// D� um pulso no EN
		LCD_EN_Pulse();

		// Delay de ~5ms
		SysCtlDelay(83333);
	}

	// Escreve 0x20 no Data (PC7-PC4)
	GPIOPinWrite(GPIO_PORTC_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0x20);

	// D� um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~5ms
	SysCtlDelay(83333);

	// Envia as instru��es 0x01, 0x06 e 0x0c (ver lcd1.pdf p�g3)
	LCD_Inst(0x01);	// Clear display
	LCD_Inst(0x06);	// Increment, Display Shift Off
	LCD_Inst(0x0c);	// Display On, Cursor Underline Off, Cursor Blink Off
}

void LCD_EN_Pulse (void)
{
	// EN=1
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x08);

	// Delay de ~20us
	SysCtlDelay(333);

	// EN=0
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x00);
}

void LCD_Inst (unsigned char inst)
{

	//RS=0
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x00); //2 - RS  3 - E

	// Escreve 4 bits mais significativos de inst no Data (PC7-PC4)
	char temp = (inst >> 4) * 0x10; //UP
	GPIOPinWrite(GPIO_PORTC_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, temp);

	// D� um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~100us
	SysCtlDelay(1667);

	// Escreve 4 bits menos significativos de inst no Data (PC7-PC4)
	temp = (inst & 0x0F) * 0x10; //DOWN
	GPIOPinWrite(GPIO_PORTC_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, temp);

	// D� um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~5ms
	SysCtlDelay(83333);
}

void LCD_Data (unsigned char data)
{
	//RS=1
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x04);

	// Escreve 4 bits mais significativos de inst no Data (PC7-PC4)
	char temp = (data >> 4) * 0x10; //UP
	GPIOPinWrite(GPIO_PORTC_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, temp);

	// D� um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~100us
	SysCtlDelay(1667);

	// Escreve 4 bits menos significativos de inst no Data (PC7-PC4)
	temp = (data & 0x0F) * 0x10; //DOWN
	GPIOPinWrite(GPIO_PORTC_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, temp);

	// D� um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~5ms
	SysCtlDelay(83333);
}

void LCD_WriteCol (char txt[20], int row, int col)
{
	unsigned char address_d = 0;

	switch(row)
	{
		case 0:
			address_d = 0x80;
			break;

		case 1:
			address_d = 0xC0;
			break;

		case 2:
			address_d = 0x94;
			break;

		case 3:
			address_d = 0xD4;
			break;
	}
	address_d = address_d + col;

	// Envia o endere�o inicial do LCD como instru��o
	LCD_Inst(address_d);

	// Envia todos os caracteres da string de entrada
	while(*txt) LCD_Data(*txt++);
}

void LCD_Write (char txt[20], int row)
{
	LCD_WriteCol(txt, row, 0);
}

void LCD_Process ()
{
	if(process == 1)
	{
		LCD_WriteCol("+", 3, 19);
		process = 0;
	}
	else
	{
		LCD_WriteCol("-", 3, 19);
		process = 1;
	}
}

void LCD_Clear (void)
{
	LCD_Inst(0x01);
}

void LCD_BlackLight_Enable (void)
{
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0x40);
}

void LCD_BlackLight_Disable (void)
{
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0x00);
}

char *IntToStr(int value, char *s, int radix)
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

void stringcat(char text[20], int value, int line) {
	char valueTemp[10];
	char textTemp[20];
	strcpy(textTemp, text);
	IntToStr(value, valueTemp, 10);
	strcat(textTemp, valueTemp);
	strcat(textTemp, "C");
	LCD_Write(textTemp, line);
}

