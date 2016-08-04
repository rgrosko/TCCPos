/*
 * FuncoesMenu.h
 *
 *  Created on: 27/06/2014
 *      Author: Rafael
 */

#ifndef FUNCOESMENU_H_
#define FUNCOESMENU_H_

// Maquina de Estados Menu
// MENUS - M
#define MTIMER	0x00
#define MADD	0x01
#define MREMOVE	0x02
#define MPRINT	0x03
#define MEXIT	0x04
//FUNCOES - F
#define FTIMER	0x05
#define FADD	0x06
#define FREMOVE	0x07
#define FPRINT	0x08
//
#define EXIT	0x09

// Rotinas
//	 Maquina de Estados DataHoraManager (DHM)
//	 Estados
#define DDIA	0x00
#define DMES	0x01
#define DANO	0x02
#define DHOR	0x03
#define DMIN	0x04
#define DEND	0x05
//	 Output
#define LCDL0	0x00
#define LCDL1	0x01
#define PDIA	0x00
#define PMES	0x03
#define PANO	0x06
#define PHORA	0x00
#define PMINU	0x03

char *tecla;

void FuncoesMenu_Menu(void);
void FuncoesMenu_SalvaSaida(void);
void FuncoesMenu_SalvaEntrada(void);
void Delay(uint32_t ms);

#endif /* FUNCOESMENU_H_ */
