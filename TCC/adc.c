/*
 * adc.c
 *
 *  Created on: 17/05/2014
 *      Author: Rafael
 */

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"

#include "adc.h"
#include "utils.h"

static uint64_t TIMEOUT = 0;

void ADC_Init()
{
	//Habilita ADCs
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
	//Habilita porta E e pinos 1 e 5 para usar no ADC
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_5);


	//Configura ADC 0
	ADCHardwareOversampleConfigure(ADC0_BASE, 64);
	ADCSequenceDisable(ADC0_BASE, 0);
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	//configura ADC 0 para ler o pino 0
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH3|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 0);
	ADCIntClear(ADC0_BASE, 0);

	//configura ADC 1
//	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
	ADCHardwareOversampleConfigure(ADC1_BASE, 64);
	ADCSequenceDisable(ADC1_BASE, 0);
	ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	//configura ADC 1 para ler o pino 5
	ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH8 |ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC1_BASE, 0);
}

//Realiza a leitura do sensor, baseado no ADC selecionado
int32_t ADC_Read(uint32_t sensor)
{
	uint32_t valor = 0;
	uint32_t ui32ADC0Value[1];


	ADCProcessorTrigger(sensor, 0);

	TIMEOUT = getSysTick()+10000;
	while (!ADCIntStatus(sensor, 0, false)) {
		if (getSysTick() >= TIMEOUT)
			return ADC_ERROR_TIMEOUT;
	}

	ADCSequenceDataGet(sensor, 0, ui32ADC0Value);
	valor = ui32ADC0Value[0] / 124;
	return valor;
}
