/*
 * keypad.h
 *
 *  Created on: 23/06/2014
 *      Author: Rafael
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

void KEYPAD_Init(void);
char *KEYPAD_GetLetter(uint32_t number);

void KEYPAD_SetColPosition(uint32_t position);
uint32_t KEYPAD_GetColPosition(void);
char *KEYPAD_GetName(void);
void KEYPAD_NewName(void);
void KEYPAD_DeleteLetter(void);

#endif /* KEYPAD_H_ */
