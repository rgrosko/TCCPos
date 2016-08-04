#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

// Esquema de Ligação do Display LCD -----------------------
// RS - PA2
// EN - PA3
// D4 - PB4
// D5 - PB5
// D6 - PB6
// D7 - PB7
//----------------------------------------------------------

void LCD_Inst (unsigned char inst);
void LCD_Data (unsigned char data);
void LCD_EN_Pulse (void);

void LCD_Init (void)
{
	int i;

	// Habilita GPIOA e GPIOB
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	// Programa PA3 e PA2 como saída
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3);

	// Programa PB7, PB6, PB5 e PB4 como saída
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	// EN=1
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0x08);

	// RS=0
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0x00);

	for(i=0;i<3;i++)
	{
		// Escreve 0x30 no Data (PB7-PB4)
		GPIOPinWrite(GPIO_PORTB_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0x30);

		// Dá um pulso no EN
		LCD_EN_Pulse();

		// Delay de ~5ms
		SysCtlDelay(83333);
	}

	// Escreve 0x20 no Data (PB7-PB4)
	GPIOPinWrite(GPIO_PORTB_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0x20);

	// Dá um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~5ms
	SysCtlDelay(83333);

	// Envia as instruções 0x01, 0x06 e 0x0c (ver lcd1.pdf pág3)
	LCD_Inst(0x01);	// Clear display
	LCD_Inst(0x06);	// Increment, Display Shift Off
	LCD_Inst(0x0c);	// Display On, Cursor Underline Off, Cursor Blink Off
}

void LCD_EN_Pulse (void)
{
	// EN=1
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0x08);

	// Delay de ~20us
	SysCtlDelay(333);

	// EN=0
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0x00);
}

void LCD_Inst (unsigned char inst)
{

	//RS=0
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0x00); //2 - RS  3 - E

	// Escreve 4 bits mais significativos de inst no Data (PB7-PB4)
	char temp = (inst >> 4) * 0x10; //UP
	GPIOPinWrite(GPIO_PORTB_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, temp);

	// Dá um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~100us
	SysCtlDelay(1667);

	// Escreve 4 bits menos significativos de inst no Data (PB7-PB4)
	temp = (inst & 0x0F) * 0x10; //DOWN
	GPIOPinWrite(GPIO_PORTB_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, temp);

	// Dá um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~5ms
	SysCtlDelay(83333);
}

void LCD_Data (unsigned char data)
{
	//RS=1
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0x04);

	// Escreve 4 bits mais significativos de inst no Data (PB7-PB4)
	char temp = (data >> 4) * 0x10; //UP
	GPIOPinWrite(GPIO_PORTB_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, temp);

	// Dá um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~100us
	SysCtlDelay(1667);

	// Escreve 4 bits menos significativos de inst no Data (PB7-PB4)
	temp = (data & 0x0F) * 0x10; //DOWN
	GPIOPinWrite(GPIO_PORTB_BASE , GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, temp);

	// Dá um pulso no EN
	LCD_EN_Pulse();

	// Delay de ~5ms
	SysCtlDelay(83333);
}

void LCD_Write (char * txt, int row, int col)
{
	unsigned char address_d = 0;

	switch(row)
	{
		case 0:
			address_d = 0x80 + col;
			break;

		case 1:
			address_d = 0xC0 + col;
			break;

		case 2:
			address_d = 0x94 + col;
			break;

		case 3:
			address_d = 0xD4 + col;
			break;
	}

	// Envia o endereço inicial do LCD como instrução
	LCD_Inst(address_d);

	// Envia todos os caracteres da string de entrada
	while(*txt) LCD_Data(*txt++);
}

void LCD_Clear (void)
{
	LCD_Inst(0x01);
}

void LCD_Cursor (uint32_t on)
{
	if(on == 1)
		LCD_Inst(0x0F);
	else
		LCD_Inst(0x0C);
}

char *IntToStr(int value, char *out)
{
	uint32_t radix = 10;
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    unsigned long ulvalue = value;
    char *p = out, *q = out;
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
    return out;
}


