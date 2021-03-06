
#ifndef I2C_H_
#define I2C_H_

#define MAXFUNC	819

#define ENDFUN	0x0005
#define ENDREG	0x3338
#define COVEND  0x3333

typedef union B16 {
	uint16_t word;
	uint8_t byte[2];
}B16;

typedef struct Dado {
	//uint16_t end;
	//uint16_t mat;
	B16 end;
	B16 mat;
	char nome[14];
	char time[8]; // {MINE,HORAE,MINS,HORAS,DIA,MES,ANO,';'}
}DADO;

void I2C_Init (void);

void DS1307_SetDate (int dia, int mes, int ano);
void DS1307_GetDate (char * date);

void DS1307_SetTime (int hora, int min, int seg);
void DS1307_GetTime (char * time);

void ResetMem(void);
uint16_t EEPROM_GetNrFuncionario(void);
void EEPROM_AdicionaFuncionario(DADO novo);
bool EEPROM_EncontraFuncionario(uint16_t mat, bool del);
void EEPROM_RegistraFuncionarioEntrada(DADO func);
void EEPROM_RegistraFuncionarioSaida(DADO func);
DADO EEPROM_GetDado(uint16_t mat);
DADO EEPROM_GetRegistros(int index);

// Mapeamento da Memoria (32K BYTES) --> mudar colocando entrada saida juntos
// AREA BASE - AREA TOPO (VALOR DECIMAL)  | TIPO DE DADOS DA AREA	| TAMANHO DE DADOS SALVOS     | TAMANHO DA AREA
// 0x00 0x00 - 0x00 0x01 (00000 - 00001)  | NRFUNC 			 		| 2 BYTES DE CABECALHO        |	00002 BYTES
// 0x00 0x02 - 0x00 0x04 (00002 - 00004)  | 3 BYTES DE SEPARACAO   	| #####################       |	00003 BYTES
// 0x00 0x05 - 0x33 0x34 (00005 - 13108)  | MAT NOMEUSUARIO         | 16 BYTES DE 819 FUNCIONARIOS|	13104 BYTES
// 0x33 0x35 - 0x33 0x37 (13109 - 13111)  | 3 BYTES DE SEPARACAO   	| #####################       |	00003 BYTES
// 0x33 0x38 - 0x7F 0xFF (13112 - 32767)  | MAT NOMEUSUARIO DATAHORA| 24 BYTES DE 819 REGISTRO E/S|	19656 BYTES

/*
 * GetFunc = (16*(NRF-1))+ENDFUN
 * GetRegi = (24*(NRF-1))+ENDREG
 * ConvEnd = (RegEnd-ENDFUN)+ENDREG+(8*(NRF-1))
 * Endereco = LOW -> HIGH
 *
 * 00000 0x02
 * 00001 0x00
 * ##### ####
 * 00005 0x01
 * 00006 0x00
 * 00007 'I'
 * ..... ....
 * 00020 'F'
 * 00021 0x02
 * 00022 0x00
 * 00023 'I'
 * ..... ....
 * 00036 'F'
 * ##### ####
 * 13112 0x01
 * 13113 0x00
 * 13114 'I'
 * ..... ....
 * 13127 'F'
 * 13128 MIE
 * 13129 HOE
 * 13130 MIS
 * 13131 HOS
 * 13132 DIA
 * 13133 MES
 * 13134 ANO
 * 13135 ';'
 * 13136 0x02
 * 13137 0x00
 * 13138 'I'
 * ..... ....
 * 13151 'F'
 * 13152 MIE
 * 13153 HOE
 * 13154 MIS
 * 13155 HOS
 * 13156 DIA
 * 13157 MES
 * 13158 ANO
 * 13159 ';'
 * ##### ####
 */


#endif /* I2C_H_ */
