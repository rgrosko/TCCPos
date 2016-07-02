/*
 * bluetooth.h
 *
 *  Created on: 29/05/2014
 *      Author: Rafael
 */


#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

void Bluetooth_Init(void);
void Bluetooth_EnviaDados(uint32_t valor, uint32_t data, uint32_t time);
char* Bluetooth_RecebeDados(void);

#endif /* BLUETOOTH_H_ */
