/*============================================================================
 *                    Exemplos de utilização do Kit
 *              EK-TM4C1294XL + Educational BooterPack MKII 
 *---------------------------------------------------------------------------*
 *                    Prof. André Schneider de Oliveira
 *            Universidade Tecnológica Federal do Paraná (UTFPR)
 *===========================================================================
 * Autores das bibliotecas:
 * 		Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
 * 		Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
 * 		Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
 * 		Mariana Carrião        - <mcarriao@alunos.utfpr.edu.br>
 *===========================================================================*/
#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"

/*----------------------------------------------------------------------------
 * include libraries from drivers
 *----------------------------------------------------------------------------*/

#include "cfaf128x128x16.h"
#include "mensagens.h"

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7
#define osFeature_SysTick 1

//Functions in assembly
void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);

//To print on the screen
tContext sContext;

//Variaveis compartilhadas
bool flagT1 = false;
bool flagT2 = false;
bool flagT3 = false;
bool flagT4 = false;
bool flagT5 = false;
int count = 2;
int chave;
int primo_ant;
int prox_primo;
char string1[60];
char string2[60];
char string3[60];
bool chave_correta1 = false;
bool chave_correta2 = false;
bool chave_correta3 = false;
bool mensagemcorreta1 = false;
bool mensagemcorreta2 = false;
bool mensagemcorreta3 = false;

//Criacao das threads
void gerarPrimos(void const *argument);
void decodifica(void const *argument);
void testaPenultimo(void const *argument);
void testaUltimo(void const *argument);
void mostraDisplay(void const *argument);

//Varivel que determina ID das threads
osThreadId gerarPrimos_ID; 
osThreadId decodifica_ID; 
osThreadId testaPenultimo_ID; 
osThreadId testaUltimo_ID; 
osThreadId mostraDisplay_ID;  

//Definicao das threads
osThreadDef (gerarPrimos, osPriorityNormal, 1, 0);     // thread object
osThreadDef (decodifica, osPriorityNormal, 1, 0);     // thread object
osThreadDef (testaPenultimo, osPriorityNormal, 1, 0);     // thread object
osThreadDef (testaUltimo, osPriorityNormal, 1, 0);     // thread object
osThreadDef (mostraDisplay, osPriorityNormal, 1, 0);     // thread object

int Init_Thread (void) {
	gerarPrimos_ID = osThreadCreate (osThread(gerarPrimos), NULL);
	if (!gerarPrimos_ID) return(-1);
	
	decodifica_ID = osThreadCreate (osThread(decodifica), NULL);
	if (!decodifica_ID) return(-1);
	
	testaPenultimo_ID = osThreadCreate (osThread(testaPenultimo), NULL);
	if (!testaPenultimo_ID) return(-1);
	
	testaUltimo_ID = osThreadCreate (osThread(testaUltimo), NULL);
	if (!testaUltimo_ID) return(-1);
	
	testaPenultimo_ID = osThreadCreate (osThread(mostraDisplay), NULL); 
	if (!mostraDisplay_ID) return(-1);
	
  return(0);
}

/*----------------------------------------------------------------------------
 *  Transforming int to string
 *---------------------------------------------------------------------------*/
static void intToString(int64_t value, char * pBuf, uint32_t len, uint32_t base, uint8_t zeros){
	static const char* pAscii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	bool n = false;
	int pos = 0, d = 0;
	int64_t tmpValue = value;

	// the buffer must not be null and at least have a length of 2 to handle one
	// digit and null-terminator
	if (pBuf == NULL || len < 2)
			return;

	// a valid base cannot be less than 2 or larger than 36
	// a base value of 2 means binary representation. A value of 1 would mean only zeros
	// a base larger than 36 can only be used if a larger alphabet were used.
	if (base < 2 || base > 36)
			return;

	if (zeros > len)
		return;
	
	// negative value
	if (value < 0)
	{
			tmpValue = -tmpValue;
			value    = -value;
			pBuf[pos++] = '-';
			n = true;
	}

	// calculate the required length of the buffer
	do {
			pos++;
			tmpValue /= base;
	} while(tmpValue > 0);


	if (pos > len)
			// the len parameter is invalid.
			return;

	if(zeros > pos){
		pBuf[zeros] = '\0';
		do{
			pBuf[d++ + (n ? 1 : 0)] = pAscii[0]; 
		}
		while(zeros > d + pos);
	}
	else
		pBuf[pos] = '\0';

	pos += d;
	do {
			pBuf[--pos] = pAscii[value % base];
			value /= base;
	} while(value > 0);
}

static void floatToString(float value, char *pBuf, uint32_t len, uint32_t base, uint8_t zeros, uint8_t precision){
	static const char* pAscii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint8_t start = 0xFF;
	if(len < 2)
		return;
	
	if (base < 2 || base > 36)
		return;
	
	if(zeros + precision + 1 > len)
		return;
	
	intToString((int64_t) value, pBuf, len, base, zeros);
	while(pBuf[++start] != '\0' && start < len); 

	if(start + precision + 1 > len)
		return;
	
	pBuf[start+precision+1] = '\0';
	
	if(value < 0)
		value = -value;
	pBuf[start++] = '.';
	while(precision-- > 0){
		value -= (uint32_t) value;
		value *= (float) base;
		pBuf[start++] = pAscii[(uint32_t) value];
	}
}

/*----------------------------------------------------------------------------
 *    Initializations
 *---------------------------------------------------------------------------*/

void init_all(){
	cfaf128x128x16Init();	
}

void draw_pixel(uint16_t x, uint16_t y){
	GrPixelDraw(&sContext, x, y);
}

void clear(void){
	tRectangle tela;
	tela.i16XMin = 0;
	tela.i16YMin = 0;
	tela.i16XMax = 127;
	tela.i16YMax = 127;
	
	GrContextForegroundSet(&sContext, ClrBlack);
	GrContextBackgroundSet(&sContext, ClrWhite);
	
	GrRectFill(&sContext, &tela);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

void init_sidelong_menu(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

/*----------------------------------------------------------------------------
 *    Threads
 *---------------------------------------------------------------------------*/

void gerarPrimos(void const *argument){ 
	while(1){
		if(flagT1){
			int i = 3, c;
			primo_ant = 2;
			chave = 3;
			prox_primo = 5;
			while(1){
				for (c = 2; c <= i - 1; c++){
					if (i%c == 0)
						break;
				}
				if (c == i){
					count++;
					if(count > 4){
						primo_ant = chave;
						chave = prox_primo;
						prox_primo = i;
						flagT1 = false;
						flagT3 = true;
						flagT4 = true;
						osThreadYield();
					}
				}
				i++;
			}
		}
		else{
			osThreadYield();
		}
	}
}

void decodifica(void const *argument){
	while(1){
		if(flagT2){
			int i, j = 0;
			int16_t aux;
			int16_t letraux;
			char letra;
			if(true){
				chave = chave;
			}
			if(!mensagemcorreta1){
				for(i = 0; i < 60; i+=2){
					aux = Mensagem1[i];
					aux += Mensagem1[i+1]*16*16;
					if(!((i/2)%2)){
						letraux = aux/(chave - primo_ant);
						letra = (char)letraux;
					}
					else{
						letraux = aux/(chave - prox_primo);
						letra = (char)letraux;
					}
					string1[j] = letra;
					j++;
				}
			}
			if(chave_correta1)
				mensagemcorreta1 = true;
			j = 0;
			aux = 0x00;
			if(!mensagemcorreta2){
				for(i = 0; i < 60; i+=2){
					aux = Mensagem2[i];
					aux += Mensagem2[i+1]*16*16;
					if(!((i/2)%2)){
						letraux = aux/(chave - primo_ant);
						letra = (char)letraux;
					}
					else{
						letraux = aux/(chave - prox_primo);
						letra = (char)letraux;
					}
					string2[j] = letra;
					j++;
				}
			}
			if(chave_correta2)
				mensagemcorreta2 = true;
			j = 0;
			aux = 0x00;
			if(!mensagemcorreta3){
				for(i = 0; i < 60; i+=2){
					aux = Mensagem3[i];
					aux += Mensagem3[i+1]*16*16;
					if(!((i/2)%2)){
						letraux = aux/(chave - primo_ant);
						letra = (char)letraux;
					}
					else{
						letraux = aux/(chave - prox_primo);
						letra = (char)letraux;
					}
					string3[j] = letra;
					j++;
				}
			}
			if(chave_correta3)
				mensagemcorreta3 = true;
			flagT2 = false;
			flagT5 = true;
			osKernelSysTickMicroSec(1000);
			osThreadYield();
		}
		else{
			osThreadYield();
		}
	}
}

void testaPenultimo(void const *argument){
	while(1){	
		if(flagT3){
			uint32_t aux1 = 0x00;
			uint32_t aux2 = 0x00;
			uint32_t aux3 = 0x00;
			aux1 += Mensagem1[60];
			aux1 += Mensagem1[61]*16*16;
			aux1 += Mensagem1[62]*16*16*16*16;
			aux1 += Mensagem1[63]*16*16*16*16*16*16;
			if(aux1 == (chave*prox_primo)){
				chave_correta1 = true;
				mensagemcorreta1 = true;
			}
			
			aux2 += Mensagem2[60];
			aux2 += Mensagem2[61]*16*16;
			aux2 += Mensagem2[62]*16*16*16*16;
			aux2 += Mensagem2[63]*16*16*16*16*16*16;
			if(aux2 == (chave*prox_primo))
				chave_correta2 = true;
			
			aux3 += Mensagem3[60];
			aux3 += Mensagem3[61]*16*16;
			aux3 += Mensagem3[62]*16*16*16*16;
			aux3 += Mensagem3[63]*16*16*16*16*16*16;
			if(aux3 == (chave*prox_primo))
				chave_correta3 = true;
			
			flagT3 = false;
			if(!(chave_correta1 || chave_correta2 || chave_correta3))
				flagT1 = true;
			//flagT2 = true;
			osThreadYield();
		}
		else{
			osThreadYield();
		}
	}
}

void testaUltimo(void const *argument){
	while(1){
		if(flagT4){
			uint32_t aux1 = 0x00;
			uint32_t aux2 = 0x00;
			uint32_t aux3 = 0x00;
			aux1 += Mensagem1[64];
			aux1 += Mensagem1[65]*16*16;
			aux1 += Mensagem1[66]*16*16*16*16;
			aux1 += Mensagem1[67]*16*16*16*16*16*16;	
			if(aux1 == (chave*primo_ant)){
				chave_correta1 = true;
				mensagemcorreta1 = true;
			}
			
			aux2 += Mensagem2[64];
			aux2 += Mensagem2[65]*16*16;
			aux2 += Mensagem2[66]*16*16*16*16;
			aux2 += Mensagem2[67]*16*16*16*16*16*16;	
			if(aux2 == (chave*primo_ant))
				chave_correta2 = true;
			
			aux3 += Mensagem3[64];
			aux3 += Mensagem3[65]*16*16;
			aux3 += Mensagem3[66]*16*16*16*16;
			aux3 += Mensagem3[67]*16*16*16*16*16*16;	
			if(aux3 == (chave*primo_ant))
				chave_correta3 = true;
		
			flagT4 = false;
			if(!(chave_correta1 || chave_correta2 || chave_correta3))
				flagT1 = true;
			flagT2 = true;
			osThreadYield();
		}
		else{
			osThreadYield();
		}
	}
}

void mostraDisplay(void const *argument){
	int i;
	char aux11[21];
	char aux12[21];
	char aux21[21];
	char aux22[21];
	char aux31[21];
	char aux32[21];
	while(1){
		if(flagT5){
			if(chave == 761 || chave == 13399 || chave == 48341){
				char buffchave[10];
				char bufftime[10];
				intToString(chave, buffchave, 10, 10, 3);
				intToString(osKernelSysTick()/100000, bufftime, 10, 10, 3);
				clear();
				GrStringDraw(&sContext, "Chave: ", -1, 0, (sContext.psFont->ui8Height+2)*0, true);
				GrStringDraw(&sContext, buffchave, -1, (sContext.psFont->ui8MaxWidth)*7, (sContext.psFont->ui8Height+2)*0, true);
				GrStringDraw(&sContext, "Tempo total: ", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
				GrStringDraw(&sContext, bufftime, -1, (sContext.psFont->ui8MaxWidth)*13, (sContext.psFont->ui8Height+2)*12, true);
				if(chave_correta1 && !chave_correta2 && !chave_correta3){
					GrStringDraw(&sContext,"Chave correta para", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
					GrStringDraw(&sContext,"mensagem 1!", -1, 0, (sContext.psFont->ui8Height+2)*2, true);
					chave_correta1 = false;
				}
				else if(chave_correta2 && !chave_correta3){
					GrStringDraw(&sContext,"Chave correta para", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
					GrStringDraw(&sContext,"mensagem 2!", -1, 0, (sContext.psFont->ui8Height+2)*2, true);
					chave_correta2 = false;
				}
				else if(chave_correta3){
					GrStringDraw(&sContext,"Chave correta para", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
					GrStringDraw(&sContext,"mensagem 3!", -1, 0, (sContext.psFont->ui8Height+2)*2, true);
					chave_correta3 = false;
				}
				else{
					GrStringDraw(&sContext,"Chave incorreta para", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
					GrStringDraw(&sContext,"todas as mensagens", -1, 0, (sContext.psFont->ui8Height+2)*2, true);
				}
				
				for(i = 0; i < 21; i++){
					aux11[i] = string1[i];
					aux21[i] = string2[i];
					aux31[i] = string3[i];
					aux12[i] = string1[i + 21];
					aux22[i] = string2[i + 21];
					aux32[i] = string3[i + 21];
				}
				GrStringDraw(&sContext,aux11, -1, 0, (sContext.psFont->ui8Height+2)*4, true);
				GrStringDraw(&sContext,aux12, -1, 0, (sContext.psFont->ui8Height+2)*5, true);
				GrStringDraw(&sContext,aux21, -1, 0, (sContext.psFont->ui8Height+2)*7, true);
				GrStringDraw(&sContext,aux22, -1, 0, (sContext.psFont->ui8Height+2)*8, true);
				GrStringDraw(&sContext,aux31, -1, 0, (sContext.psFont->ui8Height+2)*10, true);
				GrStringDraw(&sContext,aux32, -1, 0, (sContext.psFont->ui8Height+2)*11, true);
			}
			if(chave >= 48341){
				osThreadTerminate(gerarPrimos_ID);
				osThreadTerminate(decodifica_ID); 
				osThreadTerminate(testaPenultimo_ID); 
				osThreadTerminate(testaUltimo_ID); 
				osThreadTerminate(mostraDisplay_ID); 
			}
			flagT5 = false;
			osThreadYield();
		}
		else{
			osThreadYield();
		}
	}
} 

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void) {
	char buff[10];
	
	osKernelInitialize();
	
	//init_all();
	//init_sidelong_menu();
	
	flagT1 = true;
	
	if(Init_Thread()==0)
		return 0;
	
	osKernelStart(); 
	osDelay(osWaitForever);
}