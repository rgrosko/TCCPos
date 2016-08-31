#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"

#include "i2cmod.h"
#include "utils.h"

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

void DS1307_GetDate (uint8_t* date)
{
	uint8_t tmp, ano, mes, dia;

	tmp = DS1307_ReadReg(4);
	dia = (tmp&0x0F)+10*((tmp>>4)&0x03);

	tmp = DS1307_ReadReg(5);
	mes = (tmp&0x0F)+10*((tmp>>4)&0x01);

	tmp = DS1307_ReadReg(6);
	ano = (tmp&0x0F)+10*((tmp>>4)&0x0F);

	date[0] = dia;
	date[1] = mes;
	date[2] = ano;	
}

void DS1307_GetTime (uint8_t * time)
{
	uint8_t tmp, hora, min, seg;

	tmp = DS1307_ReadReg(0);
	seg = (tmp&0x0F)+10*((tmp>>4)&0x07);

	tmp = DS1307_ReadReg(1);
	min = (tmp&0x0F)+10*((tmp>>4)&0x07);

	tmp = DS1307_ReadReg(2);
	hora = (tmp&0x0F)+10*((tmp>>4)&0x03);

	time[0] = hora;
	time[1] = min;
	time[2] = seg;
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

	for(i=0; i < 26891; i++)
	{
		EEPROM_Write(ind.byte[1],ind.byte[0],0x00);
		ind.word++;
	}
}

B16 GetMesAno(uint16_t mem_pos) {
	B16 mes_ano, end;
	end.word = mem_pos;
	mes_ano.byte[1] = EEPROM_Read(end.byte[1],end.byte[1]); //MES
	end.word++;
	mes_ano.byte[0] = EEPROM_Read(end.byte[1],end.byte[1]); //ANO
	return mes_ano;
}

DADOMEDIDA LeSequencia(uint16_t mes, uint16_t inc) {
	B16 end;
	DADOMEDIDA lido;
	end.word = mes;
	uint8_t mes = EEPROM_Read(end.byte[1],end.byte[0]);
	end.word = end.word + 3 + (inc*6);
	lido = EEPROM_PegaLeitura(end.byte[1],end.byte[0]);
	return lido;
}

void EEPROM_Define(uint16_t addr, uint8_t data) {
	B16 aux;
	aux.word = addr;
	EEPROM_Write(aux.byte[1],aux.byte[0],data);
}

void EEPROM_MudaSemestre(uint8_t mes_inicial, uint8_t ano_inicial) {
	B16 endereco;
	uint8_t aux = 0, ind = 0, sem = 0;
	uint8_t mesoff[6];
	uint8_t anooff[6];
	
	sem = EEPROM_Read(0x00,0x02);//lê o semestre de um dos meses (é o mesmo para todos)
	if(sem == 0x01) sem++;
	else sem = 0x01;
	
	//CALCULA OFFSET DE ESPAÇOS DA MEMÓRIA - ÁREA DE LEITURAS DIÁRIAS
	if(mes_inicial <= 6) {
		for(aux = 0; aux < 6; aux++) {
			mesoff[aux] = aux;
			anooff[aux] = 0;
		}
	} else {
		aux = mes_inicial - 7;
		for(ind = 0; ind < 6; ind++) {				
			if(ind + aux < 6) {
				mesoff[ind] = ind + aux;
				anooff[aux] = 0;					
			} else {
				mesoff[ind] = ind + aux + (251 - mes_inicial);
				if(ano_inicial < 99) 
					anooff[aux] = 1;
				else
					anooff[aux] = 157;
			}				
		}		
	}		
	//DEFINE OS VALORES DOS 6 MESES(ATUAL E PRÓXIMOS 5)
	endereco.word = MES1;    //MES
	EEPROM_Define(endereco.word, mes_inicial + mesoff[0]);
	endereco.word = MES1+1;  //ANO
	EEPROM_Define(endereco.word, ano_inicial + anooff[0]);
	endereco.word = MES1+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, sem);
    //atual->end_diaria = MES1+3;
	
	endereco.word = MES2;    //MES
	EEPROM_Define(endereco.word, mes_inicial + mesoff[1]);
	endereco.word = MES2+1;  //ANO
	EEPROM_Define(endereco.word, ano_inicial + anooff[1]);
	endereco.word = MES2+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, sem);	
	//atual->end_proxmes = MES2;
	
	endereco.word = MES3;    //MES
	EEPROM_Define(endereco.word, mes_inicial + mesoff[2]);
	endereco.word = MES3+1;  //ANO
	EEPROM_Define(endereco.word, ano_inicial + anooff[2]);
	endereco.word = MES3+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, sem);
	
	endereco.word = MES4;    //MES
	EEPROM_Define(endereco.word, mes_inicial + mesoff[3]);
	endereco.word = MES4+1;  //ANO
	EEPROM_Define(endereco.word, ano_inicial + anooff[3]);
	endereco.word = MES4+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, sem);	
	
	endereco.word = MES5;    //MES
	EEPROM_Define(endereco.word, mes_inicial + mesoff[4]);
	endereco.word = MES5+1;  //ANO
	EEPROM_Define(endereco.word, ano_inicial + anooff[4]);
	endereco.word = MES5+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, sem);
	
	endereco.word = MES6;    //MES
	EEPROM_Define(endereco.word, mes_inicial + mesoff[5]);
	endereco.word = MES6+1;  //ANO
	EEPROM_Define(endereco.word, ano_inicial + anooff[5]);
	endereco.word = MES6+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, sem);				
}

void EEPROM_SalvaMedia(DADOANUAL dado_de_ano) {
	EEPROM_Write(dado_de_ano.end.byte[1],dado_de_ano.end.byte[0],dado_de_ano.acu_hi.byte[1]);
	dado_de_ano.end.word++;
	EEPROM_Write(dado_de_ano.end.byte[1],dado_de_ano.end.byte[0],dado_de_ano.acu_hi.byte[0]);
	dado_de_ano.end.word++;
	EEPROM_Write(dado_de_ano.end.byte[1],dado_de_ano.end.byte[0],dado_de_ano.acu_lo.byte[1]);
	dado_de_ano.end.word++;
	EEPROM_Write(dado_de_ano.end.byte[1],dado_de_ano.end.byte[0],dado_de_ano.acu_lo.byte[0]);
	dado_de_ano.end.word++;
	EEPROM_Write(dado_de_ano.end.byte[1],dado_de_ano.end.byte[0],dado_de_ano.cnt_ms.byte[1]);
	dado_de_ano.end.word++;
	EEPROM_Write(dado_de_ano.end.byte[1],dado_de_ano.end.byte[0],dado_de_ano.cnt_ms.byte[0]);
	dado_de_ano.end.word++;
	EEPROM_Write(dado_de_ano.end.byte[1],dado_de_ano.end.byte[0],dado_de_ano.cnt_ag.byte[1]);
	dado_de_ano.end.word++;
	EEPROM_Write(dado_de_ano.end.byte[1],dado_de_ano.end.byte[0],dado_de_ano.cnt_ag.byte[0]);
}

void EEPROM_SalvaLeitura(DADOMEDIDA dado_de_dia) {
	EEPROM_Write(dado_de_dia.end.byte[1],dado_de_dia.end.byte[0],dado_de_dia.datetime[0]);
	dado_de_dia.end.word++;
	EEPROM_Write(dado_de_dia.end.byte[1],dado_de_dia.end.byte[0],dado_de_dia.datetime[1]);
	dado_de_dia.end.word++;
	EEPROM_Write(dado_de_dia.end.byte[1],dado_de_dia.end.byte[0],dado_de_dia.datetime[2]);
	dado_de_dia.end.word++;
	EEPROM_Write(dado_de_dia.end.byte[1],dado_de_dia.end.byte[0],dado_de_dia.datetime[3]);
	dado_de_dia.end.word++;
	EEPROM_Write(dado_de_dia.end.byte[1],dado_de_dia.end.byte[0],dado_de_dia.med.byte[1]);
	dado_de_dia.end.word++;
	EEPROM_Write(dado_de_dia.end.byte[1],dado_de_dia.end.byte[0],dado_de_dia.med.byte[0]);
}

DADOANUAL EEPROM_PegaMedia(uint16_t addr) {
//void EEPROM_PegaMedia(DADOANUAL* lido) {
	DADOANUAL lido;
	lido.end.word = addr;
	lido.acu_hi.byte[1] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.acu_hi.byte[0] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.acu_lo.byte[1] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.acu_lo.byte[0] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.cnt_ms.byte[1] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.cnt_ms.byte[0] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.cnt_ag.byte[1] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.cnt_ag.byte[0] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word = addr;
	return lido;
}

void EEPROM_Incrementa(uint16_t addr) {
	B16 endereco, contador;
	
	endereco.word = addr;
	contador.byte[1] = EEPROM_Read(endereco.byte[1],endereco.byte[0]);
	endereco.word++;
	contador.byte[0] = EEPROM_Read(endereco.byte[1],endereco.byte[0]);
	
	contador.word++;
	
	endereco.word--;
	EEPROM_Write(endereco.byte[1],endereco.byte[0],contador.byte[1]);
	endereco.word++;
	EEPROM_Write(endereco.byte[1],endereco.byte[0],contador.byte[0]);
}

DADOMEDIDA EEPROM_PegaLeitura(uint16_t addr) {
//void EEPROM_PegaLeitura(DADOMEDIDA* lido) {
	DADOMEDIDA lido;
	lido.end.word = addr;
	lido.datetime[0] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.datetime[1] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.datetime[2] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.datetime[3] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.med.byte[1] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.med.byte[0] = EEPROM_Read(lido.end.byte[1],lido.end.byte[0]);
	lido.end.word++;
	lido.end.word = addr;
	return lido;
}
