#include <stdint.h>

#ifndef I2C_H_
#define I2C_H_

#define MES1    0x0000
#define MES2	0x1173
#define MES3    0x22E6
#define MES4	0x345A
#define MES5	0x45CD
#define MES6    0x5740

#define ANO1	0x68B2
#define ANO2	0x68BB
#define ANO3    0x68C4
#define ANO4	0x68CD
#define ANO5	0x68D6
#define ANO6    0x68DF
#define ANO7	0x68E8
#define ANO8	0x68F1
#define ANO9    0x68FA
#define ANOA    0x6903

typedef union B16 {
	uint16_t word;
	uint8_t byte[2];
}B16;

typedef struct DadoMedida {
	B16 end;			 // ENDLO|ENDHI	=> ENDERECO REFERENCIA
	uint8_t datetime[4]; // DIA|HORA|MIN|SEG 
	B16 med;			 // MEDLO|MEDHI => MEDIDA DE FLUXO DA HORA	
}DADOMEDIDA;

typedef struct DadoAnual {
	B16 end;			 // ENDLO|ENDHI	 => ENDERECO REFERENCIA
	B16 acu_hi;			 // ACULO|ACUHI  => ACUMULADOR MEDIDAS (2 MSBy) 
	B16 acu_lo;			 // ACULO|ACUHI  => ACUMULADOR MEDIDAS (2 LSBy)
	B16 cnt_ms;			 // CNTLO|CNTHI	 => CONTA MEDIDAS FEITAS
	B16 cnt_ag;		     // CNTLO|CNTHI	 => CONTA FALTA DE AGUA
}DADOANUAL;

void I2C_Init (void);
void Delay(uint32_t ms); //COPIADO DE TECLADO

void DS1307_SetDate (int dia, int mes, int ano);
void DS1307_GetDate (uint8_t* date);

void DS1307_SetTime (int hora, int min, int seg);
void DS1307_GetTime (uint8_t* time);

void ResetMem(void);
void EEPROM_Define(uint16_t addr, uint8_t data);
void EEPROM_MudaSemestre(uint8_t mes_inicial, uint8_t ano_inicial);
void EEPROM_SalvaMedia(DADOANUAL dado_de_ano);
void EEPROM_SalvaLeitura(DADOMEDIDA dado_de_dia);
DADOANUAL EEPROM_PegaMedia(uint16_t addr);
DADOMEDIDA EEPROM_PegaLeitura(uint16_t addr);
/*void EEPROM_PegaMedia(DADOANUAL* lido);
void EEPROM_PegaLeitura(DADOMEDIDA* lido);*/
void EEPROM_Incrementa(uint16_t addr);
/*******************************************************************************************************************
// MAPA DA MEMORIA (32K BYTES) /////////////////////////////////////////////////////////////////////////////////////
// AREA BASE - AREA TOPO (VALOR DECIMAL) | TIPO DE DADOS DA AREA	| TAMANHO DE DADOS SALVOS     | TAMANHO DA AREA/

// 0x00 0x00 - 0x00 0x02 (00000 - 00002) | MES1 DO ANO/SEMESTRE x/x | 3 BYTES PARA IDENTIFCADOR   |   00003 BYTES //	
// 0x00 0x03 - 0x11 0x72 (00003 - 04466) | LEITURAS POR HORA NO MES | 6 BYTES DE DADOS            |   04464 BYTES //

// 0x11 0x73 - 0x11 0x75 (04467 - 04469) | MES2 DO ANO/SEMESTRE x/x | 3 BYTES PARA IDENTIFCADOR   |   00003 BYTES //
// 0x11 0x76 - 0x22 0xE5 (04470 - 08933) | LEITURAS POR HORA NO MES | 6 BYTES DE DADOS			  |   04464 BYTES //

// 0x22 0xE6 - 0x22 0xE8 (08934 - 08936) | MES3 DO ANO/SEMESTRE x/x | 3 BYTES PARA IDENTIFCADOR	  |   00003 BYTES //	
// 0x22 0xE9 - 0x34 0x58 (08937 - 13400) | LEITURAS POR HORA NO MES | 6 BYTES DE DADOS 			  |   04464 BYTES //

// 0x34 0x59 - 0x34 0x5B (13401 - 13403) | MES4 DO ANO/SEMESTRE x/x | 3 BYTES PARA IDENTIFCADOR   |   00003 BYTES //	
// 0x34 0x5C - 0x45 0xCB (13404 - 17867) | LEITURAS POR HORA NO MES | 6 BYTES DE DADOS            |   04464 BYTES //

// 0x45 0xCC - 0x45 0xCE (17868 - 17870) | MES5 DO ANO/SEMESTRE x/x | 3 BYTES PARA IDENTIFCADOR   |   00003 BYTES //
// 0x45 0xCF - 0x57 0x3E (17871 - 22334) | LEITURAS POR HORA NO MES | 6 BYTES DE DADOS			  |   04464 BYTES //

// 0x57 0x3F - 0x57 0x41 (22335 - 22337) | MES6 DO ANO/SEMESTRE x/x | 3 BYTES PARA IDENTIFCADOR	  |   00003 BYTES //	
// 0x57 0x42 - 0x68 0xB1 (22338 - 26801) | LEITURAS POR HORA NO MES | 6 BYTES DE DADOS 			  |   04464 BYTES //

// 0x68 0xB2   			 (26802) 		 | 			ANO 1	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xB3 - 0x68 0xBA (26803 - 26810) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x68 0xBB 			 (26811) 		 | 			ANO 2	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xBC - 0x68 0xC3 (26812 - 26819) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x68 0xC4 			 (26820) 		 | 			ANO 3	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xC5 - 0x68 0xCC (26821 - 26828) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x68 0xCD   			 (26829) 		 | 			ANO 4	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xCE - 0x68 0xD5 (26830 - 26837) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x68 0xD6 			 (26838) 		 | 			ANO 5	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xD7 - 0x68 0xDE (26839 - 26846) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x68 0xDF 			 (26847) 		 | 			ANO 6	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xE0 - 0x68 0xE7 (26848 - 26855) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x68 0xE8   			 (26856) 		 | 			ANO 7	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xE9 - 0x68 0xF0 (26857 - 26864) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x68 0xF1 			 (26865) 		 | 			ANO 8	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xF2 - 0x68 0xF9 (26866 - 26873) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x68 0xFA 			 (26874) 		 | 			ANO 9	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x68 0xFB - 0x69 0x02 (26875 - 26882) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //

// 0x69 0x03 			 (26883) 		 | 			ANO A	  		| 1 BYTES PARA IDENTIFCADOR	  |   00001 BYTES //
// 0x69 0x04 - 0x69 0x0B (26884 - 26891) | ACUMULADORES E CONTADORES| 8 BYTES DE DADOS 		      |   00008 BYTES //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*******************************************************************************************************************/

#endif /* I2C_H_ */
