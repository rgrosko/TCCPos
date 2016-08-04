/*
 * Impressora.h
 *
 *  Created on: 27/06/2014
 *      Author: Rafael
 */

#ifndef IMPRESSORA_H_
#define IMPRESSORA_H_

void Impressora_Centraliza(void);
void Impressora_Init(void);
void Impressora_Pula_Pagina(void);
void Impressora_ImprimeLinha(char* dado);
void Impressora_Pula_Linha(uint8_t n_linha);
void Impressora_Cut_Paper(void);
void Impressora_Cut_Parc_Paper(void) ;
void Impressora_Sublinhado(int32_t on);
void Impressora_Realce(int32_t realce, int32_t densidade);
void Impressora_Italico(int32_t italico);

#endif /* IMPRESSORA_H_ */
