/*
 * keypad.c
 *
 *  Created on: 23/06/2014
 *      Author: Rafael
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

#include "keypad.h"
#include "lcd.h"

uint32_t TimerDelay(uint32_t ms);

#define NOTUSENUMBER 0x01
#define TIMERON 0x02
#define TIMEROFF 0x03
uint32_t currentState, numberState, colPosition, disableTimer;
uint32_t ui32Period;
char *letter;
char name[16];

struct KEYPAD
{
	char *letter;
	uint32_t next;
};

struct KEYPAD b2[3] =
{
		{"A", 1},
		{"B", 2},
		{"C", 0}
};

struct KEYPAD b3[3] =
{
		{"D", 1},
		{"E", 2},
		{"F", 0}
};

struct KEYPAD b4[3] =
{
		{"G", 1},
		{"H", 2},
		{"I", 0}
};

struct KEYPAD b5[3] =
{
		{"J", 1},
		{"K", 2},
		{"L", 0}
};

struct KEYPAD b6[3] =
{
		{"M", 1},
		{"N", 2},
		{"O", 0}
};

struct KEYPAD b7[4] =
{
		{"P", 1},
		{"Q", 2},
		{"R", 3},
		{"S", 0}
};

struct KEYPAD b8[3] =
{
		{"T", 1},
		{"U", 2},
		{"V", 0}
};

struct KEYPAD b9[4] =
{
		{"W", 1},
		{"X", 2},
		{"Y", 3},
		{"Z", 0}
};

struct KEYPAD b0[2] =
{
		{" ", 1},
		{".", 0}
};

uint32_t TimerDelay(uint32_t s)
{
	uint32_t delay = s / (3.0 / SysCtlClockGet());
	return delay;
}

void ResetTimer() {
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER0_BASE, TIMER_A);

	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
	TimerEnable(TIMER0_BASE, TIMER_A);
}

void NextLetter() {
	if (numberState != NOTUSENUMBER)
	{
		strcat(name, letter);
		colPosition++;
	}
}

void KEYPAD_Init(void)
{
	ui32Period = TimerDelay(4);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	currentState = 0;
	numberState = NOTUSENUMBER;
	disableTimer = TIMEROFF;
}

void KEYPAD_DeleteLetter(void)
{
	if(colPosition > 0 && numberState == NOTUSENUMBER)
		colPosition--;
	else if(numberState != NOTUSENUMBER)
		numberState = NOTUSENUMBER;
	strncpy(name, name, colPosition);

	ResetTimer();
	letter = " ";
	LCD_Write(letter, 1, KEYPAD_GetColPosition());
}

void KEYPAD_NewName(void)
{
	strcpy(name,"");
	colPosition = 0;
	letter = 0;
	numberState = NOTUSENUMBER;
	currentState = 0;
	disableTimer = TIMERON;
}

char *KEYPAD_GetName(void)
{
	NextLetter();
	disableTimer = TIMEROFF;
	return name;
}

char *KEYPAD_GetLetter(uint32_t number)
{
	if(number != numberState)
	{
		NextLetter();
		numberState = number;
		currentState = 0;
	}
	ResetTimer();
	switch(number)
	{
	case 2:
		letter = b2[currentState].letter;
		currentState = b2[currentState].next;
		break;
	case 3:
		letter = b3[currentState].letter;
		currentState = b3[currentState].next;
		break;
	case 4:
		letter = b4[currentState].letter;
		currentState = b4[currentState].next;
		break;
	case 5:
		letter = b5[currentState].letter;
		currentState = b5[currentState].next;
		break;
	case 6:
		letter = b6[currentState].letter;
		currentState = b6[currentState].next;
		break;
	case 7:
		letter = b7[currentState].letter;
		currentState = b7[currentState].next;
		break;
	case 8:
		letter = b8[currentState].letter;
		currentState = b8[currentState].next;
		break;
	case 9:
		letter = b9[currentState].letter;
		currentState = b9[currentState].next;
		break;
	case 0:
		letter = b0[currentState].letter;
		currentState = b0[currentState].next;
		break;
	}
	LCD_Write(letter, 1, KEYPAD_GetColPosition());
	return letter;
}

void TimerEnd(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	if(disableTimer == TIMEROFF) return;

	NextLetter();
	letter = " ";
	LCD_Write(letter, 1, KEYPAD_GetColPosition());

	currentState = 0;
	numberState = NOTUSENUMBER;
}

void KEYPAD_SetColPosition(uint32_t position)
{
	colPosition = position;
}

uint32_t KEYPAD_GetColPosition(void)
{
	return colPosition;
}
