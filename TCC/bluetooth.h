/*
 * bluetooth.h
 *
 *  Created on: 29/05/2014
 *      Author: Rafael
 */


#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

void Bluetooth_Init(void);
void Bluetooth_EnviaDados(uint32_t valor, uint8_t * data, uint8_t * time);
void Bluetooth_EnviaMedicao(uint32_t valor);
char* Bluetooth_RecebeDados(void);
void Bluetooth_Enable(void);
void Bluetooth_Disable(void);

#endif /* BLUETOOTH_H_ */
