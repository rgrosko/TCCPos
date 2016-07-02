/*
 * sensor.c
 *
 *  Created on: 21/05/2014
 *      Author: Rafael
 */
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"


#include "sensor.h"

void Sensor_RequestHDT(void);

static uint64_t TIMEOUT = 0;

uint8_t bits[5];  // buffer to receive data
uint64_t ui64Value;

volatile int16_t temp = 0;
volatile int16_t umidade = 0;
volatile double orvalho;


void Sensor_Init()
{
	//habilita porta B e pino 2
	//limitada a porta a nao receber mais de 4mA,
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

    //habilita pino 2, inicialmente como de saida e o habilita em nivel alto
    GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_DIR_MODE_OUT);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
}

int Sensor_Read()
{
    // le valores do sensor
    int rv = Sensor_ReadHDT();
    if (rv != DHTLIB_OK)
    {
    	umidade = DHTLIB_INVALID_VALUE; // valor invalido
    	temp = DHTLIB_INVALID_VALUE; // valor invalido
        return rv;
    }

    // converte valores pra umidade e temperatura
    umidade = bits[0];
    temp = bits[2];

    // testa CHECKSUM
    // bits[1] && bits[3] both 0
    uint8_t sum = bits[0] + bits[2];
    if (bits[4] != sum) return DHTLIB_ERROR_CHECKSUM;

    return DHTLIB_OK;
}

void Sensor_RequestHDT() {
	// requesita amostra
	GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_DIR_MODE_OUT);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);
	//delay
	ui64Value = getSysTick() + (25000*2);
	while (getSysTick() <= ui64Value);

	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
	//delay
	ui64Value = getSysTick() + (5*2);
	while (getSysTick() <= ui64Value);

	//muda pino para entrada, para ler valores do sensor
	GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
}

int16_t Sensor_ReadHDT()
{
    // INIT BUFFERVAR TO RECEIVE DATA
    uint8_t mask = 128;
    uint8_t idx = 0;
    uint8_t i;

    // limpa buffer
    for (i=0; i< 5; i++) bits[i] = 0;

	Sensor_RequestHDT();

    // GET ACKNOWLEDGE or TIMEOUT
	TIMEOUT = getSysTick()+(10000*2);
    while(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2) != GPIO_PIN_2)
        if (getSysTick() >= TIMEOUT)
        	return DHTLIB_ERROR_TIMEOUT;

	TIMEOUT = getSysTick()+(10000*2);
	while(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2) == GPIO_PIN_2)
        if (getSysTick() >= TIMEOUT)
        	return DHTLIB_ERROR_TIMEOUT;

    // READ THE OUTPUT - 40 BITS => 5 BYTES
    for (i=0; i<40; i++)
    {
    	TIMEOUT = getSysTick()+(10000*2);
        while(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2) != GPIO_PIN_2)
        {
            if (getSysTick() >= TIMEOUT)
            	return DHTLIB_ERROR_TIMEOUT;
        }

    	TIMEOUT = getSysTick()+(10000*2);
    	ui64Value = getSysTick();
        while(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2) == GPIO_PIN_2)
        {
            if (getSysTick() >= TIMEOUT)
            	return DHTLIB_ERROR_TIMEOUT;
        }

        if ((getSysTick()-ui64Value) > 40)
			bits[idx] |= mask;
		mask >>= 1;
        if (mask == 0)   // next byte?
        {
            mask = 128;
            idx++;
        }
    }

    return DHTLIB_OK;
}

//calcula se esta formando agua na celula
int8_t Sensor_CalcOrvalho()
{
	// [ (H/100) ^(1/8) ](112 +0,9T) +(0,1T)-112
	orvalho = sqrt(sqrt(sqrt(umidade/100)));
	orvalho += 112;
	orvalho += (0.9*temp);
	orvalho += (0.1*temp)-112;
	if(orvalho<0)
		return true;
	else
		return false;

}
