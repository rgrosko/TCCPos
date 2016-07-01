/*
 * sensor.h
 *
 *  Created on: 21/05/2014
 *      Author: Rafael
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include <inttypes.h>
#include <stdbool.h>

#define DHTLIB_OK				0
#define DHTLIB_ERROR_CHECKSUM	-1
#define DHTLIB_ERROR_TIMEOUT	-2
#define DHTLIB_INVALID_VALUE	-999

extern volatile int16_t temp;
extern volatile int16_t umidade;

void Sensor_Init(void);
int Sensor_Read(void);
int16_t Sensor_ReadHDT(void);
int8_t Sensor_CalcOrvalho(void);

#endif /* SENSOR_H_ */
