#ifndef MONITORAFLUXO_H_
#define MONITORAFLUXO_H_

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

void StartMonit(uint8_t datetime[6], REFTEMPO* atual); //INICIALIZAÇÃO 
void OpenValve(void);            //SINAL PARA ABRIR A VÁLVULA: MANIPULA RELÉS, DURAÇÃO DE 50ms
void CloseValve(void);           //SINAL PARA FECHAR A VÁLVULA: MANIPULA RELÉS, DURAÇÃO DE 50ms
//VERIFICA PASSAGEM DE TEMPO PARA ESCREVER NA MEMÓRIA
void CheckToSave(uint16_t* leitura_acumulada, REFTEMPO* atual); 
//MONITORAMENTO ATRAVÉS DOS SINAIS E VARIÁVEIS EXTERNAS
void Scan(REFTEMPO* atual, uint8_t* flag, uint8_t* tempo, uint16_t* pulsos, uint16_t* leituras);

#endif