/*
 * FuncoesMenu.c
 *
 *  Created on: 27/06/2014
 *      Author: Rafael
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils/ustdlib.h"

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

#include "lcd.h"
#include "teclado.h"
#include "keypad.h"
#include "i2c.h"
#include "UART.h"
#include "Impressora.h"
#include "FuncoesMenu.h"

void SetDateTime(void);
uint16_t GetMatricula(void);
void CadastroNovoUsuario(void);
void RemoveUsuario(void);
void ImprimeExtrato(DADO reg, bool entrada);
void ImprimeRelatorio(void);
void ImprimeOpcao(uint32_t menuCurrent);

struct DHM
{
	int32_t linha;
	int32_t coluna;
	int32_t next;
};

struct DHM dhm[5] =
{
		{LCDL0,PDIA,DMES}, 	//DIA: l=0 ; c=0 ; n=1
		//X . / . . / . .
		//. . : . .
		{LCDL0,PMES,DANO}, 	//MES: l=0 ; c=3 ; n=2
		//. . / X . / . .
		//. . : . .
		{LCDL0,PANO,DHOR}, 	//ANO: l=0 ; c=6 ; n=3
		//. . / . . / X .
		//. . : . .
		{LCDL1,PHORA,DMIN}, //HOR: l=1 ; c=0 ; n=4
		//. . / . . / . .
		//X . : . .
		{LCDL1,PMINU,DEND}  //MIN: l=1 ; c=3 ; n=5
		//. . / . . / . .
		//. . : X .
};

struct MENU
{
	char opcao[20];
	int32_t next[3];//Anterior, Proximo, Seleciona
};

struct MENU menu[9] =
{
		{"1-Data/Hora", 		{MEXIT,		MADD,		FTIMER}},
		{"2-Add usuario", 		{MTIMER,	MREMOVE,	FADD}},
		{"3-Remove usuario", 	{MADD,		MPRINT,		FREMOVE}},
		{"4-Imprime relat.", 	{MREMOVE,	MEXIT,		FPRINT}},
		{"5-Sair", 				{MPRINT,	MTIMER,		EXIT}},
		{"", 					{MTIMER,	EXIT,		EXIT}}, // Rotina Configuracao de Data e Hora
		{"", 					{MADD,		EXIT,		EXIT}}, // Rotina Adicionar Funcionario
		{"", 					{MREMOVE,	EXIT,		EXIT}}, // Rotina Remover Funcionario
		{"", 					{MPRINT,	EXIT,		EXIT}}  // Rotina Imprimir Relatorio

};


//Rotina Configuracao de Data e Hora (DHM)
void SetDateTime() {
	LCD_Clear();
	LCD_Write("DD/MM/AA", 0, 0);
	LCD_Write("HH:MM:00", 1, 0);

	uint32_t dhmCurrent = DDIA;
	int32_t input, index, posicao;
	int32_t dataHora[5];

	while(dhmCurrent != DEND)
	{
		input = index = 0;
		posicao = dhm[dhmCurrent].coluna;
		while(index != 2)
		{
			tecla = Teclado_GetBotao();
			if (atoi(tecla) != 0 || (strcmp(tecla, "0") == 0))
			{
				LCD_Write(tecla, dhm[dhmCurrent].linha, posicao++);
				if (input == 0)
					input = atoi(tecla);
				else
					input = input * 10 + atoi(tecla);
				index++;
			}
		}
		dataHora[dhmCurrent] = input;
		dhmCurrent = dhm[dhmCurrent].next;
	}

	DS1307_SetDate(dataHora[0],dataHora[1],dataHora[2]);
	DS1307_SetTime(dataHora[3],dataHora[4],00);

	tecla = "";
}

uint16_t GetMatricula() {
	uint32_t posicao = 0;
	uint16_t matricula = 0;

	LCD_Clear();
	LCD_Write("Digite o codigo:", 0, 0);
	LCD_Write("###", 1, 0);
	while (posicao < 3) {
		tecla = Teclado_GetBotao();
		if (atoi(tecla) != 0 || (strcmp(tecla, "0") == 0)) {
			LCD_Write(tecla, 1, posicao++);
			if (matricula == 0)
				matricula = (uint16_t)atoi(tecla);
			else
				matricula = matricula * 10 + (uint16_t)atoi(tecla);
		}
	}
	Delay(1000);
	LCD_Clear();
	return matricula;
}

void CadastroNovoUsuario()
{
	DADO input;

	do{input.mat.word = GetMatricula();}while(EEPROM_EncontraFuncionario(input.mat.word,false));

	LCD_Cursor(1);
	KEYPAD_NewName();
	LCD_Write("Digite o nome:", 0, 0);
	while(1){
		tecla = Teclado_GetBotao();
		if (atoi(tecla) != 0 || (strcmp(tecla, "0") == 0))
			KEYPAD_GetLetter(atoi(tecla));
		else if(strcmp(tecla,"D") == 0){
			strcpy(input.nome,KEYPAD_GetName());
			break;
		}else if(strcmp(tecla,"#") == 0){
			KEYPAD_DeleteLetter();
		}
	}
	LCD_Cursor(0);

	EEPROM_AdicionaFuncionario(input);
}

void RemoveUsuario()
{
	uint16_t mat;
	mat = GetMatricula();

	if(!EEPROM_EncontraFuncionario(mat,true))
	{
		LCD_Clear();
		LCD_Write("  Matricula  ", 0, 0);
		LCD_Write(" inexistente ", 1, 0);
		Delay(2000);
		LCD_Clear();
	}
	else
	{
		LCD_Clear();
		LCD_Write("  Matricula  ", 0, 0);
		LCD_Write("  removida ", 1, 0);
		Delay(2000);
		LCD_Clear();
	}

}

void ImprimeExtrato(DADO reg, bool entrada)
{
	char temp[20];

	LCD_Clear();
	LCD_Write(" Imprimindo...  ", 0, 0);
	Impressora_ImprimeLinha("----------------------------------");
	Impressora_Realce(1,4);
	Impressora_ImprimeLinha("Relogio Ponto - EXTRATO");
	Impressora_Realce(0,0);

	strcpy(temp, "Usuario: ");
	strcat(temp, reg.nome);
	Impressora_ImprimeLinha(temp);

	if(entrada)
		Impressora_ImprimeLinha("ENTRADA");
	else
		Impressora_ImprimeLinha("SAIDA");

	DS1307_GetDate(temp);
	Impressora_ImprimeLinha(temp);
	DS1307_GetTime(temp);
	Impressora_ImprimeLinha(temp);

	Impressora_ImprimeLinha("----------------------------------");
	Impressora_Cut_Paper();
	LCD_Clear();

}

void FuncoesMenu_SalvaEntrada()
{
	uint16_t matricula = GetMatricula();
	DADO func;

	func = EEPROM_GetDado(matricula);
	if(func.end.word)
	{
		ImprimeExtrato(func,true);
		EEPROM_RegistraFuncionarioEntrada(func);
	}
	else
	{
		LCD_Clear();
		LCD_Write("  Matricula  ", 0, 0);
		LCD_Write(" inexistente ", 1, 0);
		Delay(2000);
		LCD_Clear();
	}
}

void FuncoesMenu_SalvaSaida()
{
	uint16_t matricula = GetMatricula();
	DADO func;

	func = EEPROM_GetDado(matricula);
	if(func.end.word)
	{
		ImprimeExtrato(func,false);
		EEPROM_RegistraFuncionarioSaida(func);
	}
	else
	{
		LCD_Clear();
		LCD_Write("  Matricula  ", 0, 0);
		LCD_Write(" inexistente ", 1, 0);
		Delay(2000);
		LCD_Clear();
	}
}

void ImprimeRelatorio()
{
	char temp[20];
	uint16_t nr;
	DADO reg;
	int index;

	nr = EEPROM_GetNrFuncionario();
	if(nr)
	{
		LCD_Clear();
		LCD_Write(" Imprimindo...  ", 0, 0);
		for(index = 1; index < (nr+1); index++)
		{
			reg = EEPROM_GetRegistros(index);
			if(reg.mat.word)
			{
				Impressora_ImprimeLinha("----------------------------------");
				Impressora_Realce(1,4);
				Impressora_ImprimeLinha("Relogio Ponto");
				Impressora_Realce(0,0);

				strcpy(temp, "Usuario: ");
				strcat(temp, reg.nome);
				Impressora_ImprimeLinha(temp);
				sprintf(temp, "Dia: %02d/%02d/%02d", reg.time[4],reg.time[5],reg.time[6]);
				Impressora_ImprimeLinha(temp);
				sprintf(temp, "Entrada: %02d:%02d", reg.time[1],reg.time[0]);
				Impressora_ImprimeLinha(temp);
				sprintf(temp, "Saida: %02d:%02d", reg.time[3],reg.time[2]);
				Impressora_ImprimeLinha(temp);

				Impressora_ImprimeLinha("----------------------------------");
			}
		}
		Impressora_Cut_Paper();
		LCD_Clear();
	}
	else
	{
		Impressora_ImprimeLinha("----------------------------------");
		Impressora_Realce(1,4);
		Impressora_ImprimeLinha("Relogio Ponto");
		Impressora_Realce(0,0);
		Impressora_ImprimeLinha("Memoria Vazia");
		Impressora_ImprimeLinha("----------------------------------");
		Impressora_Cut_Paper();
	}
}

void ImprimeOpcao(uint32_t menuCurrent) {
	LCD_Clear();
	LCD_Write("Configuracoes", 0, 0);
	LCD_Write(menu[menuCurrent].opcao, 1, 0);
}

void FuncoesMenu_Menu() {
	int32_t menuCurrent = 0, reimprime = 0;
	ImprimeOpcao(menuCurrent);

	while(1){
		switch(*Teclado_GetBotao())
		{
		case '*':
			menuCurrent = menu[menuCurrent].next[0];
			reimprime = 1;
			break;
		case '#':
			menuCurrent = menu[menuCurrent].next[1];
			reimprime = 1;
			break;
		case 'D':
			menuCurrent = menu[menuCurrent].next[2];
			reimprime = 1;
			break;
		}
		if(menuCurrent == EXIT)
			break;
		else if(menuCurrent >= FTIMER) { // VERIFICA SE EH UMA FUNCAO
			switch(menuCurrent){
			case 5:
				SetDateTime();
				break;
			case 6:
				CadastroNovoUsuario();
				break;
			case 7:
				RemoveUsuario();
				break;
			case 8:
				ImprimeRelatorio();
				break;
			}
			menuCurrent = menu[menuCurrent].next[0];
		}
		else if(reimprime == 1){
			ImprimeOpcao(menuCurrent);
			reimprime = 0;
		}
	}
	LCD_Clear();
}
