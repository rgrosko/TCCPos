#ifndef MONITORAFLUXO_H_
#define MONITORAFLUXO_H_

#include "i2cmod.h"

typedef struct RefTempo {
	uint8_t hora;
	uint8_t minu;
	uint8_t mes;
	uint8_t ano;
	
    B16     end_diaria;
	B16 	end_proxmes;
    B16     end_media;	
	B16 	end_proxano;
}REFTEMPO;

void StartMonit(uint8_t datetime[6], REFTEMPO* atual); //INICIALIZACAO
//VERIFICA PASSAGEM DE TEMPO PARA ESCREVER NA MEMORIA
void CheckToSave(uint16_t* leitura_acumulada, REFTEMPO* atual); 
//MONITORAMENTO ATRAVES DOS SINAIS E VARIAVEIS EXTERNAS
void Scan(REFTEMPO* atual, uint8_t* flag, uint8_t* tempo, uint16_t* pulsos, uint16_t* leituras);

#endif
