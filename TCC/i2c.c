#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"

#include "i2c.h"

#define DS1307_ADDRESS		0x68
#define EEPROM_ADDRESS		0x50

void DS1307_WriteReg (uint8_t address, uint8_t reg);
uint8_t DS1307_ReadReg (uint8_t address);

void EEPROM_Write(uint8_t addHigh, uint8_t addLow, uint8_t data);
uint8_t EEPROM_Read (uint8_t addHigh, uint8_t addLow);

void I2C_Init (void)
{
    // Habilita perifericos GPIOA e I2C1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    // Inicializa o Clock do I2C1
	I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);

    // Programa PA6 como I2C1 SCL
	GPIOPinConfigure(GPIO_PA6_I2C1SCL);
	GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);

    // Programa PA7 como I2C1 SDA
	GPIOPinConfigure(GPIO_PA7_I2C1SDA);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
}

void DS1307_WriteReg (uint8_t address, uint8_t reg)
{
	//--------------------INICIALIZACAO-------------------------------------------------
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	// Configura o endereco do dispositivo I2C e informa que vai escrever
	I2CMasterSlaveAddrSet(I2C1_BASE, DS1307_ADDRESS, false);
	//--------------------ENDERECAMENTO-------------------------------------------------
	// Envia um byte contendo o endereco dentro do dispositivo
	I2CMasterDataPut(I2C1_BASE, address);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	//--------------------ENVIO DE DADO-------------------------------------------------
    // Envia o reg para o device
	I2CMasterDataPut(I2C1_BASE, reg);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
}

uint8_t DS1307_ReadReg (uint8_t address)
{
	//--------------------INICIALIZACAO-------------------------------------------------
	uint8_t data;
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	// Configura o endereco do dispositivo I2C e informa que vai escrever
	I2CMasterSlaveAddrSet(I2C1_BASE, DS1307_ADDRESS, false);
	//--------------------ENDERECAMENTO-------------------------------------------------
	// Envia um byte contendo o endereco dentro do dispositivo
	I2CMasterDataPut(I2C1_BASE, address);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	//--------------------RECEBE O DADO-------------------------------------------------
	// Configura o endereco do dispositivo I2C e informa que vai ler
	I2CMasterSlaveAddrSet(I2C1_BASE, DS1307_ADDRESS, true);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	// Lê um byte do dispositivo e armazena em data
	data = I2CMasterDataGet(I2C1_BASE);
    return (data);
}

void DS1307_SetDate (int dia, int mes, int ano)
{
	uint8_t H, L;

	L = dia % 10;
	H = dia / 10;
	DS1307_WriteReg(4, (H<<4)|L);

	L = mes % 10;
	H = mes / 10;
	DS1307_WriteReg(5, (H<<4)|L);

	L = ano % 10;
	H = ano / 10;
	DS1307_WriteReg(6, (H<<4)|L);

	DS1307_WriteReg(7, 0x90);
}

void DS1307_SetTime (int hora, int min, int seg)
{
	uint8_t H, L;

	L = seg % 10;
	H = seg / 10;
	DS1307_WriteReg(0, (H<<4)|L);

	L = min % 10;
	H = min / 10;
	DS1307_WriteReg(1, (H<<4)|L);

	L = hora % 10;
	H = hora / 10;
	DS1307_WriteReg(2, (H<<4)|L);

	DS1307_WriteReg(7, 0x90);
}

void DS1307_GetDate (char * date)
{
	uint8_t tmp, ano, mes, dia;

	tmp = DS1307_ReadReg(4);
	dia = (tmp&0x0F)+10*((tmp>>4)&0x03);

	tmp = DS1307_ReadReg(5);
	mes = (tmp&0x0F)+10*((tmp>>4)&0x01);

	tmp = DS1307_ReadReg(6);
	ano = (tmp&0x0F)+10*((tmp>>4)&0x0F);

	sprintf(date, "%02d/%02d/%02d", dia, mes, ano);
}

void DS1307_GetTime (char * time)
{
	uint8_t tmp, hora, min, seg;

	tmp = DS1307_ReadReg(0);
	seg = (tmp&0x0F)+10*((tmp>>4)&0x07);

	tmp = DS1307_ReadReg(1);
	min = (tmp&0x0F)+10*((tmp>>4)&0x07);

	tmp = DS1307_ReadReg(2);
	hora = (tmp&0x0F)+10*((tmp>>4)&0x03);

	sprintf(time, "%02d:%02d:%02d", hora, min, seg);
}


void EEPROM_Write(uint8_t addHigh, uint8_t addLow, uint8_t data)
{
	//--------------------INICIALIZACAO-------------------------------------------------
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	// Configura o endereco do dispositivo I2C e informa que vai escrever
	I2CMasterSlaveAddrSet(I2C1_BASE, EEPROM_ADDRESS, false);
	//--------------------ENDERECAMENTO-------------------------------------------------
	// Envia um byte contendo a parte alta do endereco dentro do dispositivo
	I2CMasterDataPut(I2C1_BASE, addHigh);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	// Envia um byte contendo a parte baixa do endereco dentro do dispositivo
	I2CMasterDataPut(I2C1_BASE, addLow);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	//--------------------ENVIO DE DADO-------------------------------------------------
	// Envia o dado para o device
	I2CMasterDataPut(I2C1_BASE, data);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	Delay(2);
}

uint8_t EEPROM_Read (uint8_t addHigh, uint8_t addLow)
{
	//--------------------INICIALIZACAO-------------------------------------------------
	uint8_t data;
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	// Configura o endereco do dispositivo I2C e informa que vai escrever
	I2CMasterSlaveAddrSet(I2C1_BASE, EEPROM_ADDRESS, false);
	//--------------------ENDERECAMENTO-------------------------------------------------
	// Envia um byte contendo a parte alta do endereco dentro do dispositivo
	I2CMasterDataPut(I2C1_BASE, addHigh);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	// Envia um byte contendo a parte baixa do endereco dentro do dispositivo
	I2CMasterDataPut(I2C1_BASE, addLow);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	//--------------------RECEBE O DADO-------------------------------------------------
	// Configura o endereco do dispositivo I2C e informa que vai ler
	I2CMasterSlaveAddrSet(I2C1_BASE, EEPROM_ADDRESS, true);
	I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
	// Aguarda enquanto o Master estiver ocupado
	while(I2CMasterBusy(I2C1_BASE));
	// Lê um byte do dispositivo e armazena em data
	data = I2CMasterDataGet(I2C1_BASE);
	return (data);
}

void ResetMem()
{
	unsigned int i;
	B16 ind;
	ind.word = 0x0000;

	for(i=0;i<2;i++)
	{
		EEPROM_Write(ind.byte[1],ind.byte[0],0x00);
		ind.word++;
	}
}

uint16_t EEPROM_GetNrFuncionario(void)
{
	B16 nr;
	nr.byte[0] = EEPROM_Read (0x00,0x00);
	nr.byte[1] = EEPROM_Read (0x00,0x01);
	return nr.word;
}

DADO EEPROM_GetDado(uint16_t mat)
{
	DADO requisitado;
	B16 end,nr,rmat;
	int index;

	requisitado.mat.word = mat;
	nr.word = EEPROM_GetNrFuncionario();
	if(!nr.word)
	{
		requisitado.end.word = 0;
		return requisitado;
	}

	end.word = 0;
	for(index = 1; index < (nr.word+1); index++)
	{
		requisitado.end.word = (16*(index-1))+ENDFUN;

		rmat.byte[0] = EEPROM_Read (requisitado.end.byte[1],requisitado.end.byte[0]);
		requisitado.end.word++;
		rmat.byte[1] = EEPROM_Read (requisitado.end.byte[1],requisitado.end.byte[0]);

		if(rmat.word == mat)
		{
			end.word = requisitado.end.word+1;
			requisitado.end.word--;
			break;
		}
	}

	if(!end.word)
	{
		requisitado.end = end;
		return requisitado;
	}

	for(index = 0; index < 14; index++)
	{
		requisitado.nome[index] = EEPROM_Read(end.byte[1],end.byte[0]);
		end.word++;
	}

	return requisitado;
}

void EEPROM_AdicionaFuncionario(DADO novo)
{
	B16 end_escrita, end_relatorio;
	uint8_t aux;
	int index;

	novo.end.word = EEPROM_GetNrFuncionario();
	if(!novo.end.word)
		end_escrita.word = ENDFUN;
	else
		end_escrita.word = (16*novo.end.word) + ENDFUN;

	end_relatorio.word = (24*novo.end.word) + ENDREG;

	for(index = 0; index < 16; index++)
	{
		if(index == 0)
			aux = novo.mat.byte[0];
		else if (index == 1)
			aux = novo.mat.byte[1];
		else
			aux = novo.nome[index-2];

		EEPROM_Write(end_escrita.byte[1], end_escrita.byte[0], aux);
		end_escrita.word++;

		EEPROM_Write(end_relatorio.byte[1], end_relatorio.byte[0], aux);
		end_relatorio.word++;
	}

	novo.end.word++;
	EEPROM_Write(0x00,0x00, novo.end.byte[0]);
	EEPROM_Write(0x00,0x01, novo.end.byte[1]);
}

bool EEPROM_EncontraFuncionario(uint16_t mat, bool del)
{
	B16 end, nr, rmat, end_relatorio;
	uint16_t aux;
	int index;

	nr.word = EEPROM_GetNrFuncionario();
	if(!nr.word)
		return false;

	for(index = 1; index < (nr.word+1); index++)
	{
		end.word = (16*(index-1))+ENDFUN;

		rmat.byte[0] = EEPROM_Read(end.byte[1],end.byte[0]);
		end.word++;
		rmat.byte[1] = EEPROM_Read(end.byte[1],end.byte[0]);

		if(rmat.word == mat)
		{
			if(del == true)
			{
				end.word--;
				aux = end.word - ENDFUN;
				end_relatorio.word =  end.word + COVEND + (aux/2);

				EEPROM_Write(end.byte[1],end.byte[0], 0x00);
				end.word++;
				EEPROM_Write(end.byte[1],end.byte[0], 0x00);

				EEPROM_Write(end_relatorio.byte[1],end_relatorio.byte[0], 0x00);
				end_relatorio.word++;
				EEPROM_Write(end_relatorio.byte[1],end_relatorio.byte[0], 0x00);
			}

			return true;
		}
	}

	return false;
}

void EEPROM_RegistraFuncionarioEntrada(DADO atualiza)
{
	uint8_t tmp;
	uint16_t aux;
	int index;

	//Dia
	tmp = DS1307_ReadReg(4);
	atualiza.time[4] = (tmp&0x0F)+10*((tmp>>4)&0x03);
	//Mes
	tmp = DS1307_ReadReg(5);
	atualiza.time[5] = (tmp&0x0F)+10*((tmp>>4)&0x01);
	//Ano
	tmp = DS1307_ReadReg(6);
	atualiza.time[6] = (tmp&0x0F)+10*((tmp>>4)&0x0F);
	//Minuto entrada
	tmp = DS1307_ReadReg(1);
	atualiza.time[0] = (tmp&0x0F)+10*((tmp>>4)&0x07);
	//Hora entrada
	tmp = DS1307_ReadReg(2);
	atualiza.time[1] = (tmp&0x0F)+10*((tmp>>4)&0x03);
	//Limpa minuto e hora saida
	atualiza.time[2] = atualiza.time[3] = 0x00;

	atualiza.time[7] = ';';

	aux = atualiza.end.word - ENDFUN;
	atualiza.end.word = atualiza.end.word + COVEND + (aux/2) + 16;

	for(index = 0; index < 8; index++)
	{
		tmp = atualiza.time[index];

		EEPROM_Write(atualiza.end.byte[1], atualiza.end.byte[0],tmp);
		atualiza.end.word++;
	}
}

void EEPROM_RegistraFuncionarioSaida(DADO atualiza)
{
	uint16_t aux;
	uint8_t tmp;
	int index;

	//Dia
	tmp = DS1307_ReadReg(4);
	atualiza.time[4] = (tmp&0x0F)+10*((tmp>>4)&0x03);
	//Mes
	tmp = DS1307_ReadReg(5);
	atualiza.time[5] = (tmp&0x0F)+10*((tmp>>4)&0x01);
	//Ano
	tmp = DS1307_ReadReg(6);
	atualiza.time[6] = (tmp&0x0F)+10*((tmp>>4)&0x0F);
	//Minuto saida
	tmp = DS1307_ReadReg(1);
	atualiza.time[2] = (tmp&0x0F)+10*((tmp>>4)&0x07);
	//Hora saida
	tmp = DS1307_ReadReg(2);
	atualiza.time[3] = (tmp&0x0F)+10*((tmp>>4)&0x03);

	atualiza.time[7] = ';';

	aux = atualiza.end.word - ENDFUN;
	atualiza.end.word = atualiza.end.word + COVEND + (aux/2) + 16;

	for(index = 0; index < 8; index++)
	{
		tmp = atualiza.time[index];

		if(index != 0 && index != 1)// Pula escrita de minuto e hora de entrada
			EEPROM_Write(atualiza.end.byte[1], atualiza.end.byte[0],tmp);

		atualiza.end.word++;
	}
}

DADO EEPROM_GetRegistros(int index)
{
	DADO requisitado;
	int pos;

	//endereco
	requisitado.end.word = (24*(index-1))+ENDREG;;

	//matricula
	requisitado.mat.byte[0] = EEPROM_Read(requisitado.end.byte[1],requisitado.end.byte[0]);
	requisitado.end.word++;
	requisitado.mat.byte[1] = EEPROM_Read(requisitado.end.byte[1],requisitado.end.byte[0]);
	if(!requisitado.mat.word)
		return requisitado;

	requisitado.end.word++;

	//nome
	for(pos = 0; pos < 14; pos++)
	{
		requisitado.nome[pos] = EEPROM_Read(requisitado.end.byte[1],requisitado.end.byte[0]);
		requisitado.end.word++;
	}

	//tempo entrada e saida
	for(pos = 0; pos < 8; pos++)
	{
		requisitado.time[pos] = EEPROM_Read(requisitado.end.byte[1],requisitado.end.byte[0]);
		requisitado.end.word++;
	}

	return requisitado;
}
