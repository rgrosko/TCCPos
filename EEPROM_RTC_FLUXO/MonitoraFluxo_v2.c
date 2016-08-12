//#include <stdint.h> -> PARA OS TIPOS uint8_t, uint16_t

#include "i2c_mod.h" 
//#include "bluetooth.h"
#include "MonitoraFluxo.h"

void StartMonit(uint8_t datetime[6], REFTEMPO* atual) {
	B16 endereco;
	uint8_t aux = 0, ind = 0;
	uint8_t mesoff[6];
	uint8_t anooff[6];
	uint8_t decoff[10];
	
	//HORA - MINUTO - SEGUNDO
	DS1307_SetTime(datetime[2],datetime[1],datetime[0]);
	atual->hora = datetime[2];
	atual->minu = datetime[1];
	//DIA - MÊS - ANO
	DS1307_SetDate(datetime[3],datetime[4],datetime[5]);
	atual->mes = datetime[4];
	atual->ano = datetime[5];
	//LIMPA MEMÓRIA
	ResetMem();	
	//CALCULA OFFSET DE ESPAÇOS DA MEMÓRIA - ÁREA DE LEITURAS DIÁRIAS
	if(datetime[5] <= 6) {
		for(aux = 0; aux < 6; aux++) {
			mesoff[aux] = aux;
			anooff[aux] = 0;
		}
	} else {
		aux = datetime[5] - 7;
		for(ind = 0; ind < 6; ind++) {				
			if(ind + aux < 6) {
				mesoff[ind] = ind + aux;
				anooff[aux] = 0;					
			} else {
				mesoff[ind] = ind + aux + (251 - datetime[5]);
				if(datetime[6] < 99) 
					anooff[aux] = 1;
				else
					anooff[aux] = 157;
			}				
		}		
	}		
	//DEFINE OS VALORES DOS 6 MESES(ATUAL E PRÓXIMOS 5)
	endereco.word = MES1;    //MES
	EEPROM_Define(endereco.word, datetime[5] + mesoff[0]);
	endereco.word = MES1+1;  //ANO
	EEPROM_Define(endereco.word, datetime[6] + anooff[0]);
	endereco.word = MES1+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, 0x01);
    atual->end_diaria = MES1+3;
	
	endereco.word = MES2;    //MES
	EEPROM_Define(endereco.word, datetime[5] + mesoff[1]);
	endereco.word = MES2+1;  //ANO
	EEPROM_Define(endereco.word, datetime[6] + anooff[1]);
	endereco.word = MES2+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, 0x01);	
	atual->end_proxmes = MES2;
	
	endereco.word = MES3;    //MES
	EEPROM_Define(endereco.word, datetime[5] + mesoff[2]);
	endereco.word = MES3+1;  //ANO
	EEPROM_Define(endereco.word, datetime[6] + anooff[2]);
	endereco.word = MES3+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, 0x01);
	
	endereco.word = MES4;    //MES
	EEPROM_Define(endereco.word, datetime[5] + mesoff[3]);
	endereco.word = MES4+1;  //ANO
	EEPROM_Define(endereco.word, datetime[6] + anooff[3]);
	endereco.word = MES4+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, 0x01);	
	
	endereco.word = MES5;    //MES
	EEPROM_Define(endereco.word, datetime[5] + mesoff[4]);
	endereco.word = MES5+1;  //ANO
	EEPROM_Define(endereco.word, datetime[6] + anooff[4]);
	endereco.word = MES5+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, 0x01);
	
	endereco.word = MES6;    //MES
	EEPROM_Define(endereco.word, datetime[5] + mesoff[5]);
	endereco.word = MES6+1;  //ANO
	EEPROM_Define(endereco.word, datetime[6] + anooff[5]);
	endereco.word = MES6+2;  //PRIMEIRA LEITURA
	EEPROM_Define(endereco.word, 0x01);				
	
	//DEFINE OS VALORES DOS 10 ANOS (ATUAL E PRÓXIMOS 9)
	//CALCULA OFFSET DE ESPAÇOS DA MEMÓRIA - ÁREA DE LEITURAS DIÁRIAS
	if(datetime[6] < 90) {
		for(aux = 0; aux < 10; aux++) {
			decoff[aux] = aux;
		}
	} else {
		aux = datetime[6] - 90;
		for(ind = 0; ind < 10; ind++) {				
			if(ind + aux < 10) {
				decoff[ind] = ind + aux;					
			} else {
				decoff[ind] = ind + aux + (246 - datetime[6]);				
			}				
		}		
	}
	endereco = ANO1;
	EEPROM_Define(endereco.word, datetime[5] + decoff[0]);	
	atual->end_media = ANO1 + 1;
	
	endereco = ANO2;
	EEPROM_Define(endereco.word, datetime[5] + decoff[1]);	
	atual->end_proxano = ANO2;
	
	endereco = ANO3;
	EEPROM_Define(endereco.word, datetime[5] + decoff[2]);	
	
	endereco = ANO4;
	EEPROM_Define(endereco.word, datetime[5] + decoff[3]);	
	
	endereco = ANO5;
	EEPROM_Define(endereco.word, datetime[5] + decoff[4]);	
	
	endereco = ANO6;
	EEPROM_Define(endereco.word, datetime[5] + decoff[5]);	
	
	endereco = ANO7;
	EEPROM_Define(endereco.word, datetime[5] + decoff[6]);
	
	endereco = ANO8;
	EEPROM_Define(endereco.word, datetime[5] + decoff[7]);	
	
	endereco = ANO9;
	EEPROM_Define(endereco.word, datetime[5] + decoff[8]);
	
	endereco = ANOA;
	EEPROM_Define(endereco.word, datetime[5] + decoff[9]);	
} 

void OpenValve(void) {
	//ACIONA------------------------------------------
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0x04); //OPEN1	
	Delay(5);//DEFINIDO EM I2C
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0x08); //OPEN2
	//DELAY-------------------------------------------
	Delay(50);//DEFINIDO EM I2C
	//RESETA------------------------------------------
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0x00); //OPEN1	
	Delay(5);//DEFINIDO EM I2C
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0x00); //OPEN2
} 
            
void CloseValve(void) {
	//ACIONA------------------------------------------
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x08); //CLOSE1	
	Delay(5);//DEFINIDO EM I2C
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02); //CLOSE2
	//DELAY-------------------------------------------
	Delay(50);//DEFINIDO EM I2C
	//RESETA------------------------------------------
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00); //CLOSE1	
	Delay(5);//DEFINIDO EM I2C
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00); //CLOSE2
} 
  
void CheckToSave(uint16_t* leitura_acumulada, REFTEMPO* atual) {
	DADOMEDIDA leitura_hora;
	DADOANUAL leitura_media;
	uint8_t time[3]; //0) HORA - MINUTO - SEGUNDO
	uint8_t data[3]; //0) DIA - MES - ANO
	
	DS1307_GetTime(time);	
	if(time[2] > atual->hora && time[1] >= atual->minu){
		DS1307_GetData(data); 		
		//PREENCHER LEITURA DA HORA NO MES ATUAL
		if(atual->mes != data[1]) {	
            if(atual->end_proxmes.word + 4467 < ANO1) { //limite da área
				atual->mes = data[1];
				atual->end_diaria.word = atual->end_proxmes.word + 3;				
				atual->end_proxmes.word = atual->end_proxmes.word + 4467;
			} else {				
				EEPROM_MudaSemestre(data[1], data[2]);
				atual->mes = data[1];
				atual->end_diaria.word = MES1 + 3;				
				atual->end_proxmes.word = MES2;				
			}		
						
			leitura_hora.end.word = atual->end_diaria.word;
			if(atual->ano == data[2]) {	
                //ATUALIZA ESTRUTURA COM VALORES SALVOS			
				leitura_media = EEPROM_PegaMedia(atual->end_media.word);				
				if(leitura_media.acu_lo.word + *leitura_acumulada > 0xFFFF)								
					leitura_media.acu_hi.word++;				
				leitura_media.acu_lo.word = leitura_media.acu_lo.word + *leitura_acumulada;
				leitura_media.cnt_ms++;							
				EEPROM_SalvaMedia(leitura_media);
			} else {
				//PREPARA ESTRUTURA COM VALOR INICIAL
				leitura_media.end.word = atual->end_proxano.word;
				leitura_media.acu_hi.word = 0x0000;
				leitura_media.acu_lo.word = *leitura_acumulada;
				leitura_media.cnt_ms.word = 0x0001;
				leitura_media.cnt_ag.word = 0x0000;				
				EEPROM_SalvaMedia(leitura_media);
				
				atual->ano = data[2];
				if(atual->end_media.word + 9 <= ANOA) { //limite da área
					atual->end_media.word = atual->end_media.word + 10;
					atual->end_proxano.word = atual->end_proxano.word + 18;	
				} else {
					atual->end_media.word = ANO1 + 1;
					atual->end_proxano.word = ANO2;
				}								
			}
		}
		
		leitura_hora.end.word = atual->end_diaria.word;				
		leitura_hora.datetime[0] = data[0]; //DIA
		leitura_hora.datetime[1] = time[0]; //HOR
		leitura_hora.datetime[2] = time[1]; //MIN
		leitura_hora.datetime[3] = time[2]; //SEG
		leitura_hora.med.word = *leitura_acumulada;
		atual->end_diaria.word = atual->end_diaria.word + 6;
		atual->hora = time[2];
		atual->minu = time[1];	
		*leitura_acumulada = 0;		
		EEPROM_SalvaLeitura(leitura_hora);									
	}    
} 

//VARIÁVEIS EXTERNAS
//   TIMER0   -> flag ; tempo
//   GPIO PD1 -> pulsos ; leituras 
//FUNÇÃO A SER POSICIONADA NO LOOP PRINCIPAL
void Scan(REFTEMPO* atual, uint8_t* flag, uint8_t* tempo, uint16_t* pulsos, uint16_t* leituras) {
	if(!GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)) {												 // IF ENABLED PB2
		if(flag == 0x00) {											  							 //START READ CONDITION - F0X00
			OpenValve();												  						 
			*flag = 0x01;
			*tempo = 0;
			*pulsos = 0;
			//USAR VARIÁVEL flag PARA HABILITAR O TIMER0 if(flag == 0x01)	TimerEnable(TIMER0_BASE, TIMER_A);
		} else if (*tempo == 8 && *flag == 0x01) {                                              // MIDDLE OF COUNT / ONLY ONE TX WITH FLAG = 1
			*leituras += (*pulsos/16);
			CheckToSave(leituras, atual);
			//->BREAKPOINT: VIEW VARS<-//
			//Bluetooth_EnviaLeituraUnica(pulsos);  
			/*****************************************
			//todo: importar esta lógica para a função acima
			//FREQUENCY = ui16Pulso / 16;
			sprintf(CPulso,"%d",(pulsos*2));
			int ind = 0;
			for(ind = 0; ind < 5; ind++)
				UARTCharPut(UART0_BASE,CPulso[ind]);
			UARTCharPut(UART0_BASE,'\r');
			UARTCharPut(UART0_BASE,'\n');
			******************************************/
			*flag = 0x02;
		} else if (*tempo >= 16) {			
			*flag = 0x04;
			//USAR VARIÁVEL flag PARA DESABILITAR O TIMER0 E REINICIAR O FLAG PARA NOVAS OPERAÇÕES 
			//if(flag == 0x04) { TimerDisable(TIMER0_BASE, TIMER_A); flag = 0x00; SysCtlDelay(83333); }
		}
	} else {																					  //IF DISABLED PB2
		CloseValve();
		//ATUALIZAR CONTADOR DE FALTA DE ÁGUA
		EEPROM_Incrementa(atual->end_media);
		*tempo = 0;
		*pulsos = 0;
		*flag = 0x08;
		//USAR VARIÁVEL flag PARA DESABILITAR O TIMER0 E REINICIAR O FLAG PARA NOVAS OPERAÇÕES 
		//if(flag == 0x04) { TimerDisable(TIMER0_BASE, TIMER_A); flag = 0x00; SysCtlDelay(83333); }
	}
} 
	         