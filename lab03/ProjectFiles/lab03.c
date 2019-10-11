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
#include "buttons.h"
#include "buzzer.h"
#include "joy.h"
#include "Eddie.h"
#include "Chefao.h"
#include "Escada.h"
#include "Inimigo.h"
#include "Item.h"
#include "Background.h"
#include "Numeros.h"

#define ALTURA_ANDAR			       20
#define ESPESSURA_CHAO		       2
#define ESPACO_PONTUACAO	       13
#define QUANT_ANDARES			       5
#define LARGURA_TELA			       128
#define ALTURA_EDDIE			       14
#define LARGURA_EDDIE		 	       8
#define X_INICIAL_EDDIE		       64
#define Y_INICIAL_EDDIE		       0
#define ANDAR_INICIAL_EDDIE		   4
#define ALTURA_ESCADA			       20
#define LARGURA_ESCADA		       17
#define ALTURA_CHEFAO		         14
#define LARGURA_CHEFAO	     	   8
#define POSICAO_INICIAL_CHEFAO   70
#define ALTURA_INIMIGO		       5
#define LARGURA_INIMIGO		       8
#define POSICAO_INICIAL_INIMIGO0 30
#define ANDAR_INIMIGO0           2
#define POSICAO_INICIAL_INIMIGO1 100
#define ANDAR_INIMIGO1           3
#define POSICAO_INICIAL_INIMIGO2 60
#define ANDAR_INIMIGO2           4
#define POSICAO_INICIAL_INIMIGO3 10
#define ANDAR_INIMIGO3           5
#define ALTURA_ITEM				       4
#define LARGURA_ITEM			       8
#define POSICAO_INICIAL_ITEM0    20
#define ANDAR_INICIAL_ITEM0      2
#define POSICAO_INICIAL_ITEM1    80
#define ANDAR_INICIAL_ITEM1      4
#define TOTAL_ESCADAS		         4
#define ALTURA_NUMEROS           8
#define LARGURA_NUMEROS          14

//To print on the screen
tContext sContext;

void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);

//Criacao das threads
/*
***  O CMSIS veio configurado com um limite de 8 threads que podem ser executadas em paralelo,
***  eu mudei essa configuracao para 20 threads. Se precisar de mais eh so abrir o arquivo RTX_Conf_CM.c
***  (dentro da aba CMSIS) e aumentar o numero do #define OS_TASKCNT 
*/
void gameStart(void const *argument);
void leituraJoystick(void const *argument);
void leituraBotao(void const *argument);
void movimentaEddie(void const *argument);
void movimentaInimigo0(void const *argument);
void movimentaInimigo1(void const *argument);
void movimentaInimigo2(void const *argument);
void movimentaInimigo3(void const *argument);
void movimentaChefao(void const *argument);
void movimentaItem0(void const *argument);
void movimentaItem1(void const *argument);
void somPulo(void const *argument);
void somSubirEscada(void const *argument);
void somFimGame(void const *argument);

//Varivel que determina ID das threads
osThreadId gameStart_ID;
osThreadId leituraJoystick_ID; 
osThreadId leituraBotao_ID; 
osThreadId movimentaEddie_ID;
osThreadId movimentaInimigo0_ID;
osThreadId movimentaInimigo1_ID;
osThreadId movimentaInimigo2_ID;
osThreadId movimentaInimigo3_ID;
osThreadId movimentaChefao_ID;
osThreadId movimentaItem0_ID;
osThreadId movimentaItem1_ID;
osThreadId somPulo_ID;
osThreadId somSubirEscada_ID;
osThreadId somFimGame_ID;

//Definicao das threads
osThreadDef (gameStart, osPriorityNormal, 1, 0);     // thread object
osThreadDef (leituraJoystick, osPriorityNormal, 1, 0);     // thread object
osThreadDef (leituraBotao, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaEddie, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaInimigo0, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaInimigo1, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaInimigo2, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaInimigo3, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaChefao, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaItem0, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaItem1, osPriorityNormal, 1, 0);     // thread object
osThreadDef (somPulo, osPriorityNormal, 1, 0);     // thread object
osThreadDef (somSubirEscada, osPriorityNormal, 1, 0);     // thread object
osThreadDef (somFimGame, osPriorityNormal, 1, 0);     // thread object

int Init_Thread (void) {
	gameStart_ID = osThreadCreate (osThread(gameStart), NULL);
	if (!gameStart_ID) return(-1);
	
	leituraJoystick_ID = osThreadCreate (osThread(leituraJoystick), NULL);
	if (!leituraJoystick_ID) return(-1);
	
	leituraBotao_ID = osThreadCreate (osThread(leituraBotao), NULL);
	if (!leituraBotao_ID) return(-1);
	
	movimentaEddie_ID = osThreadCreate (osThread(movimentaEddie), NULL); 
	if (!movimentaEddie_ID) return(-1);
	
	movimentaInimigo0_ID = osThreadCreate (osThread(movimentaInimigo0), NULL); 
	if (!movimentaInimigo0_ID) return(-1);
	
	movimentaInimigo1_ID = osThreadCreate (osThread(movimentaInimigo1), NULL); 
	if (!movimentaInimigo1_ID) return(-1);
	
	movimentaInimigo2_ID = osThreadCreate (osThread(movimentaInimigo2), NULL); 
	if (!movimentaInimigo2_ID) return(-1);
	
	movimentaInimigo3_ID = osThreadCreate (osThread(movimentaInimigo3), NULL); 
	if (!movimentaInimigo3_ID) return(-1);
	
	movimentaChefao_ID = osThreadCreate (osThread(movimentaChefao), NULL); 
	if (!movimentaChefao_ID) return(-1);
	
	movimentaItem0_ID = osThreadCreate (osThread(movimentaItem0), NULL); 
	if (!movimentaItem0_ID) return(-1);

	movimentaItem1_ID = osThreadCreate (osThread(movimentaItem1), NULL); 
	if (!movimentaItem1_ID) return(-1);
	
	somPulo_ID = osThreadCreate (osThread(somPulo), NULL); 
	if (!somPulo_ID) return(-1);
	
	somSubirEscada_ID = osThreadCreate (osThread(somSubirEscada), NULL); 
	if (!somSubirEscada_ID) return(-1);
	
	somFimGame_ID = osThreadCreate (osThread(somFimGame), NULL); 
	if (!somFimGame_ID) return(-1);
	
  return(0);
}

//mutexes
osMutexDef(mutex_display);	
osMutexId(mutex_display_ID);
osMutexDef(mutex_frames);	
osMutexId(mutex_frames_ID);

//***************** Variáveis globais  ***************************************
uint32_t movimento_eddie;
uint32_t x_anterior_eddie, x_atual_eddie;
uint32_t y_anterior_eddie, y_atual_eddie;
uint32_t posicao_anterior_chefao, posicao_atual_chefao;
bool volta_chefao = false;
uint32_t posicao_anterior_inimigo0, posicao_atual_inimigo0;
bool volta_inimigo0 = false;
uint32_t posicao_anterior_inimigo1, posicao_atual_inimigo1;
bool volta_inimigo1 = true;
uint32_t posicao_anterior_inimigo2, posicao_atual_inimigo2;
bool volta_inimigo2 = false;
uint32_t posicao_anterior_inimigo3, posicao_atual_inimigo3;
bool volta_inimigo3 = true;
uint32_t posicao_anterior_item0, posicao_atual_item0;
bool volta_item0 = false;
uint32_t posicao_anterior_item1, posicao_atual_item1;
bool volta_item1 = false;
bool pos_eddie_direita = false;
uint32_t andar_atual_eddie;
uint32_t andar_atual_item0;
uint32_t andar_atual_item1;
uint32_t posicaoEscadas[2][4];
uint16_t pontos;
uint16_t frame_eddie = 0; //controla as animacaoes do eddie - 0 parado, 1 e 2 andar, 3 e 4 pular
uint16_t frame_chefao = 0; //controla as animacoes do chefao - 0 e 1 andar
uint16_t frame_inimigo = 0; //controla as animacoes do inimigo - 0 e 1 andar
uint16_t frame_item = 0; //controla as animacoes do item - 0 amarelo, 1 vermelho, 2 azul
uint16_t frame_escada = 0; //controla as animacoes da escada - 0 nada, 1 e 2 eddie subindo ou descendo
bool pulo = 0; // se for 1 pula
bool subida_ou_descida; // subida = 1, descida = 0
bool flag_fim_de_jogo = false;

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
	joy_init();
	buzzer_init(); 
	button_init();
}

void draw_pixel(uint16_t x, uint16_t y){
	GrPixelDraw(&sContext, x, y);
}

void draw_circle(uint16_t x, uint16_t y){
	GrCircleDraw(&sContext, 
		(sContext.psFont->ui8MaxWidth)*x + (sContext.psFont->ui8MaxWidth)/2, 
		(sContext.psFont->ui8Height+2)*y + sContext.psFont->ui8Height/2 - 1, 
		(sContext.psFont->ui8MaxWidth)/2);
}

void fill_circle(uint16_t x, uint16_t y){
	GrCircleFill(&sContext, 
		(sContext.psFont->ui8MaxWidth)*x + sContext.psFont->ui8MaxWidth/2, 
		(sContext.psFont->ui8Height+2)*y + sContext.psFont->ui8Height/2 - 1, 
		(sContext.psFont->ui8MaxWidth)/2-1);
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

uint32_t saturate(uint8_t r, uint8_t g, uint8_t b){
	uint8_t *max = &r, 
					*mid = &g, 
					*min = &b,
					*aux, 
					div, num;
	if (*mid > *max){ aux = max; max = mid; mid = aux; }
	if (*min > *mid){ aux = mid; mid = min; min = aux; }
	if (*mid > *max){	aux = max; max = mid; mid = aux; }
	if(*max != *min){
		div = *max-*min;
		num = *mid-*min;
		*max = 0xFF;
		*min = 0x00;
		*mid = (uint8_t) num*0xFF/div;
	}
	return 	(((uint32_t) r) << 16) | 
					(((uint32_t) g) <<  8) | 
					( (uint32_t) b       );
}

void somPulo(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(somPulo_ID,0x01); //limpa o signal
		buzzer_vol_set(5000);
		buzzer_per_set(1703);
		buzzer_write(true);
		osDelay(654);
		buzzer_write(false);
		buzzer_per_set(2200);
		buzzer_write(true);
		osDelay(655);
		buzzer_write(false);
	}
}

void somSubirEscada(void const *argument){
	while(1){
		buzzer_vol_set(5000);
		buzzer_per_set(2551);
		buzzer_write(true);
		osDelay(600);
		buzzer_write(false);
		osDelay(600);
		buzzer_per_set(2551);
		buzzer_write(true);
		osDelay(600);
		buzzer_write(false);
		osDelay(600);
		buzzer_write(true);
		osDelay(600);
		buzzer_write(false);
		osDelay(600);
		buzzer_per_set(2551);
		buzzer_write(true);
		osDelay(600);
		buzzer_write(false);
	}
}

void somFimGame(void const *argument){
	while(1){
		buzzer_vol_set(5000);
		buzzer_per_set(2000);
		buzzer_write(true);
		osDelay(1570);
		buzzer_write(false);
		buzzer_per_set(1923);
		buzzer_write(true);
		osDelay(1570);
		buzzer_write(false);
		buzzer_per_set(1851);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		buzzer_per_set(1785);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		buzzer_per_set(1724);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		buzzer_per_set(1666);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		buzzer_per_set(1612);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		buzzer_per_set(0);
		buzzer_write(true);
		osDelay(700);
		buzzer_write(false);
	}
}

void imprimeBackground(){
	uint32_t color = 0xF34EF1; //cor roxa
	uint32_t i, j, pixel, n;
	pixel = Background_start;
	for(n = 1; n <= QUANT_ANDARES; n++){		//imprime o chao de todos os andares
		for(j = 0; j < ESPESSURA_CHAO; j++){ 
			for(i = 0; i < LARGURA_TELA; i++){
				GrContextForegroundSet(&sContext, color);
				//deixa um espaço para pontuacao no topo da tela (ESPACO_PONTUACAO)
				draw_pixel(i, j+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*n);
			}
		}
	}
}

// andar mais alto é o 1, o mais baixo é o 5
// posicao é o tamanho da largura da tela menos a largura do eddie (0 a 120)
void imprimeEddie(uint32_t andar, uint32_t posicao_x, uint32_t posicao_y){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Eddie_start;
	for(j = 0; j < ALTURA_EDDIE; j++){ //altura
		for(i = 0; i < LARGURA_EDDIE; i++){ //largura
			color = 0x00;
			color += Eddie[(pixel+(frame_eddie*ALTURA_EDDIE*LARGURA_EDDIE*3))+2]; //imprime um eddie diferente dependendo do frame
			color += Eddie[(pixel+(frame_eddie*ALTURA_EDDIE*LARGURA_EDDIE*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Eddie[(pixel+(frame_eddie*ALTURA_EDDIE*LARGURA_EDDIE*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color);
				if(x_anterior_eddie < x_atual_eddie){ //se estava andando para direita
					draw_pixel((-i)+posicao_x+LARGURA_EDDIE, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime espelhado
					pos_eddie_direita =  true; //flag para guardar a ultima posicao
				}
				else if(x_anterior_eddie > x_atual_eddie){ //se estava andando para a esquerda
					draw_pixel(i+posicao_x, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime normal
					pos_eddie_direita = false; //flag para guardar a ultima posicao
				}
				else
					if(pos_eddie_direita){ //se a ultima posicao era virado para direita
						draw_pixel((-i)+posicao_x+LARGURA_EDDIE, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime espelhado
					}
					else{
						draw_pixel(i+posicao_x, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime normal
					}
			}
			pixel += 3;
		}
	}	
}

// andar mais alto é o 1, o mais baixo é o 5
// posicao é o tamanho da largura da tela menos a largura do eddie (0 a 120)
void apagaEddie(uint32_t andar, uint32_t posicao_x, uint32_t posicao_y){
	uint32_t color = 0xFF, i, j, pixel;
	for(j = 0; j < ALTURA_EDDIE; j++){ //altura
		for(i = 0; i < LARGURA_EDDIE+1; i++){ //largura+1 pq na hora de virar tava deixando uns rastros
			GrContextForegroundSet(&sContext, 0x000000);
			if(x_anterior_eddie < x_atual_eddie){ //se estava andando para direita
				draw_pixel((-i)+posicao_x+LARGURA_EDDIE, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime espelhado
				pos_eddie_direita =  true; //flag para guardar a ultima posicao
			}
			else if(x_anterior_eddie > x_atual_eddie){ //se estava andando para a esquerda
				draw_pixel(i+posicao_x, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime normal
				pos_eddie_direita = false; //flag para guardar a ultima posicao
			}
			else
				if(pos_eddie_direita){ //se a ultima posicao era virado para direita
					draw_pixel((-i)+posicao_x+LARGURA_EDDIE, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime espelhado
				}
				else{
					draw_pixel(i+posicao_x, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime normal
				}
		}
	}
}

void apagaEscada(uint32_t andar, uint32_t posicao){
	uint32_t color = 0xE2697A, i, j, pixel;
	pixel = Escada_start;
	for(j = 0; j < ALTURA_ESCADA; j++){ //altura
		for(i = 0; i < LARGURA_ESCADA; i++){ //largura
			GrContextForegroundSet(&sContext, 0x00000000); // imprime cor AZUL
			draw_pixel(i+posicao, j-ALTURA_ESCADA+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		}
	}
}

void imprimeEscada(uint32_t andar, uint32_t posicao){
	uint32_t color, i, j, pixel;
	pixel = Escada_start;
	for(j = 0; j < ALTURA_ESCADA; j++){ //altura
		for(i = 0; i < LARGURA_ESCADA; i++){ //largura
			color = 0x00;
			color += Escada[(pixel+(frame_escada*ALTURA_ESCADA*LARGURA_ESCADA*3))+2];
			color += Escada[(pixel+(frame_escada*ALTURA_ESCADA*LARGURA_ESCADA*3))+1]*16*16;
			color += Escada[(pixel+(frame_escada*ALTURA_ESCADA*LARGURA_ESCADA*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color); // imprime cor AZUL
				draw_pixel(i+posicao, j-ALTURA_ESCADA+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
			}
			pixel += 3;
		}
	}
}

void imprimeEscadas(){
	int i;
	for(i = 0; i < TOTAL_ESCADAS; i++){
		imprimeEscada(posicaoEscadas[0][i], posicaoEscadas[1][i]);
	}
}

void apagaChefao(uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel, andar= 1;
	for(j = 0; j < ALTURA_CHEFAO; j++){ //altura
		for(i = 0; i < LARGURA_CHEFAO; i++){ //largura
			GrContextForegroundSet(&sContext, 0x000000); //apaga os pixels colocando preto
			draw_pixel(i+posicao, j-ALTURA_CHEFAO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		}
	}
}

void imprimeChefao(uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel, andar= 1;
	pixel = Chefao_start;
	for(j = 0; j < ALTURA_CHEFAO; j++){ //altura
		for(i = 0; i < LARGURA_CHEFAO; i++){ //largura
			color = 0x00;
			color += Chefao[(pixel+(frame_chefao*ALTURA_CHEFAO*LARGURA_CHEFAO*3))+2];
			color += Chefao[(pixel+(frame_chefao*ALTURA_CHEFAO*LARGURA_CHEFAO*3))+1]*16*16;
			color += Chefao[(pixel+(frame_chefao*ALTURA_CHEFAO*LARGURA_CHEFAO*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+posicao, j-ALTURA_CHEFAO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
			}
			pixel += 3;
		}
	}
}

void apagaInimigo(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	for(j = 0; j < ALTURA_INIMIGO; j++){ //altura
		for(i = 0; i < LARGURA_INIMIGO; i++){ //largura
			GrContextForegroundSet(&sContext, 0x000000);
			draw_pixel(i+posicao, j-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		}
	}
}

void imprimeInimigo(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Inimigo_start;
	for(j = 0; j < ALTURA_INIMIGO; j++){ //altura
		for(i = 0; i < LARGURA_INIMIGO; i++){ //largura
			color = 0x00;
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))+2];
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))+1]*16*16;
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+posicao, j-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
			}
			pixel += 3;
		}
	}
}

void apagaItem(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	for(j = 0; j < ALTURA_ITEM; j++){ //altura
		for(i = 0; i < LARGURA_ITEM; i++){ //largura
			GrContextForegroundSet(&sContext, 0x00000000);
			draw_pixel(i+posicao, j-ALTURA_ITEM-16+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //não sei porque do 16, mas assim que dá certo
		}
	}
}

void imprimeItem(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Item_start;
	for(j = 0; j < ALTURA_ITEM; j++){ //altura
		for(i = 0; i < LARGURA_ITEM; i++){ //largura
			color = 0x00;
			color += Item[(pixel+(frame_item*ALTURA_ITEM*LARGURA_ITEM*3))+2];
			color += Item[(pixel+(frame_item*ALTURA_ITEM*LARGURA_ITEM*3))+1]*16*16;
			color += Item[(pixel+(frame_item*ALTURA_ITEM*LARGURA_ITEM*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+posicao, j-ALTURA_ITEM-16+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //não sei porque do 16, mas assim que dá certo
			}
			pixel += 3;
		}
	}
}

void leituraJoystick(void const *argument){
	// início seção crítica
	while(1){
		int x, y;
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(leituraJoystick_ID,0x01); //limpa o signal
		if(pulo == 0){
			x = joy_read_x();
			y = joy_read_y();
			x = x*200/0xFFF-100; 
			y = y*200/0xFFF-100;
			if (x<-50 && y>-50 && y<50){
				movimento_eddie = 1;						//esquerda
				osMutexWait(mutex_frames_ID,osWaitForever);
				if(frame_eddie == 0){           //se estiver parado muda a animacao
					frame_eddie = 1;
				}
				osMutexRelease(mutex_frames_ID);
			}
			else if (x>50 && y>-50 && y<50){
				movimento_eddie = 2;						//direita
				osMutexWait(mutex_frames_ID,osWaitForever);
				if(frame_eddie == 0){           //se estiver parado muda a animacao
					frame_eddie = 1;
				}
				osMutexRelease(mutex_frames_ID);
			}
			else if (y<-50 && x>-50 && x<50){
				movimento_eddie = 3;						//baixo
				osMutexWait(mutex_frames_ID,osWaitForever);
				if(frame_eddie == 0){           //se estiver parado muda a animacao
					frame_eddie = 1;
				}
				osMutexRelease(mutex_frames_ID);
			}
			else if (y>50 && x>-50 && x<50){
				movimento_eddie = 4;						//cima
				osMutexWait(mutex_frames_ID,osWaitForever);
				if(frame_eddie == 0){           //se estiver parado muda a animacao
					frame_eddie = 1;
				}
				osMutexRelease(mutex_frames_ID);
			}
			else{
				movimento_eddie = 0;
			}
		}
	}
	// fim seção crítica
}

void leituraBotao(void const *argument){
	// início seção crítica
	while(1){
		bool s1_press;
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(leituraBotao_ID,0x01); //limpa o signal	
		s1_press = button_read_s1();
		
		if (s1_press && pulo == 0){ // verificar se isso será necessário com a implementação do mutex
			pulo = 1;		
			subida_ou_descida = 1;
			osSignalSet(somPulo_ID,0x01);
		}
	}
	// fim seção crítica
}

void movimentaEddie(void const *argument){
	// início seção crítica
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(movimentaEddie_ID,0x01); //limpa o signal
		if(pulo == 1 && subida_ou_descida == 1){
			if(y_atual_eddie < 6){
				y_atual_eddie = y_anterior_eddie + 2;
				osMutexWait(mutex_frames_ID,osWaitForever);
				frame_eddie = 3; //animacao de subir
				osMutexRelease(mutex_frames_ID);
			}
			else{
				y_atual_eddie = y_anterior_eddie - 2;
				subida_ou_descida = 0;
				osMutexWait(mutex_frames_ID,osWaitForever);
				frame_eddie = 4; //animacao de descer
				osMutexRelease(mutex_frames_ID);
			}
		}
		else if(pulo == 1 && subida_ou_descida == 0){
			if(y_atual_eddie > 0){
				y_atual_eddie = y_anterior_eddie - 2;
				osMutexWait(mutex_frames_ID,osWaitForever);
				frame_eddie = 3; //animacao de subir
				osMutexRelease(mutex_frames_ID);
			}
			else{
				pulo = 0;
				y_atual_eddie = 0;
				osMutexWait(mutex_frames_ID,osWaitForever);
				frame_eddie = 1; //quando alcanca o chao muda a animacao
				osMutexRelease(mutex_frames_ID);
			}
		}
		else{
			y_atual_eddie = 0;
		}
		
		if(movimento_eddie == 3){
			osMutexWait(mutex_display_ID,osWaitForever);
			apagaEddie(andar_atual_eddie, x_anterior_eddie, y_anterior_eddie);
			imprimeEscada(posicaoEscadas[0][andar_atual_eddie - 2], posicaoEscadas[1][andar_atual_eddie - 2]);
			imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
			osMutexRelease(mutex_display_ID);
			// para cima se houver escada, senão só mexe as perninhas
		}
		else if(movimento_eddie == 4){
			osMutexWait(mutex_display_ID,osWaitForever);
			apagaEddie(andar_atual_eddie, x_anterior_eddie, y_anterior_eddie); //se estiver parado apaga a animacao de parado
			imprimeEscada(posicaoEscadas[0][andar_atual_eddie - 2], posicaoEscadas[1][andar_atual_eddie - 2]);
			imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
			osMutexRelease(mutex_display_ID);
			// para baixo se houver escada, senão só mexe as perninhas
		}
		else if(movimento_eddie == 2 && x_atual_eddie < 120){
			x_atual_eddie = x_anterior_eddie + 4.5;
			osMutexWait(mutex_display_ID,osWaitForever);
			apagaEddie(andar_atual_eddie, x_anterior_eddie, y_anterior_eddie); //se estiver parado apaga a animacao de parado
			imprimeEscada(posicaoEscadas[0][andar_atual_eddie - 2], posicaoEscadas[1][andar_atual_eddie - 2]);
			imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
			osMutexRelease(mutex_display_ID);
		}
		else if(movimento_eddie == 1 && x_atual_eddie > 0){
			x_atual_eddie = x_anterior_eddie -4.5;
			osMutexWait(mutex_display_ID,osWaitForever);
			apagaEddie(andar_atual_eddie, x_anterior_eddie, y_anterior_eddie); //se estiver parado apaga a animacao de parado
			imprimeEscada(posicaoEscadas[0][andar_atual_eddie - 2], posicaoEscadas[1][andar_atual_eddie - 2]);
			imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
			osMutexRelease(mutex_display_ID);
		}
		else{
			if(pulo == 0){
				osMutexWait(mutex_frames_ID,osWaitForever);
				frame_eddie = 0; //imprime o eddie parado
				osMutexRelease(mutex_frames_ID);
				osMutexWait(mutex_display_ID,osWaitForever);
				apagaEddie(andar_atual_eddie, x_anterior_eddie, y_anterior_eddie);
				imprimeEscada(posicaoEscadas[0][andar_atual_eddie - 2], posicaoEscadas[1][andar_atual_eddie - 2]);
				imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
				osMutexRelease(mutex_display_ID);
			}
			else{
				osMutexWait(mutex_display_ID,osWaitForever);
				apagaEddie(andar_atual_eddie, x_anterior_eddie, y_anterior_eddie); //apaga a animacao contraria a qual sera impressa
				imprimeEscada(posicaoEscadas[0][andar_atual_eddie - 2], posicaoEscadas[1][andar_atual_eddie - 2]);
				imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
				osMutexRelease(mutex_display_ID);
			}
		}
		x_anterior_eddie = x_atual_eddie;
		y_anterior_eddie = y_atual_eddie;
	}
	// fim seção crítica
}

void movimentaChefao(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(movimentaChefao_ID,0x01); //limpa o signal
		if(posicao_atual_chefao >= 120) //alcancou o maximo da tela
			volta_chefao = true; //muda flag
		if(posicao_atual_chefao <= 0) //alcancou o minimo da tela
			volta_chefao = false; //muda flag
		if(volta_chefao) //se estiver indo para a esquerda
			posicao_atual_chefao -= 5;
		else
			posicao_atual_chefao += 5;
		
		osMutexWait(mutex_display_ID,osWaitForever);
		apagaChefao(posicao_anterior_chefao);
		imprimeChefao(posicao_atual_chefao);
		osMutexRelease(mutex_display_ID);
		posicao_anterior_chefao = posicao_atual_chefao;
	}
}

void movimentaInimigo0(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(movimentaInimigo0_ID,0x01); //limpa o signal
		if(posicao_atual_inimigo0 >= 120) //alcancou o maximo da tela
			volta_inimigo0 = true; //muda flag
		if(posicao_atual_inimigo0 <= 0) //alcancou o minimo da tela
			volta_inimigo0 = false; //muda flag
		if(volta_inimigo0) //se estiver indo para a esquerda
			posicao_atual_inimigo0 -= 5;
		else
			posicao_atual_inimigo0 += 5;
		
		osMutexWait(mutex_display_ID,osWaitForever);
		apagaInimigo(ANDAR_INIMIGO0, posicao_anterior_inimigo0);
		imprimeEscada(posicaoEscadas[0][ANDAR_INIMIGO0 - 2], posicaoEscadas[1][2 - 2]);
		imprimeInimigo(ANDAR_INIMIGO0, posicao_atual_inimigo0);
		osMutexRelease(mutex_display_ID);
		posicao_anterior_inimigo0 = posicao_atual_inimigo0;
	}
}

void movimentaInimigo1(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(movimentaInimigo1_ID,0x01); //limpa o signal
		if(posicao_atual_inimigo1 >= 120) //alcancou o maximo da tela
			volta_inimigo1 = true; //muda flag
		if(posicao_atual_inimigo1 <= 0) //alcancou o minimo da tela
			volta_inimigo1 = false; //muda flag
		if(volta_inimigo1) //se estiver indo para esquerda
			posicao_atual_inimigo1 -= 5;
		else
			posicao_atual_inimigo1 += 5;
		
		osMutexWait(mutex_display_ID,osWaitForever);
		apagaInimigo(ANDAR_INIMIGO1, posicao_anterior_inimigo1);
		imprimeEscada(posicaoEscadas[0][ANDAR_INIMIGO1 - 2], posicaoEscadas[1][3 - 2]);
		imprimeInimigo(ANDAR_INIMIGO1, posicao_atual_inimigo1);
		osMutexRelease(mutex_display_ID);
		posicao_anterior_inimigo1 = posicao_atual_inimigo1;
	}
}

void movimentaInimigo2(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(movimentaInimigo2_ID,0x01); //limpa o signal
		if(posicao_atual_inimigo2 >= 120) //alcancou o maximo da tela
			volta_inimigo2 = true; //muda flag
		if(posicao_atual_inimigo2 <= 0) //alcancou o minimo da tela
			volta_inimigo2 = false; //muda flag
		if(volta_inimigo2) //se estiver indo para esquerda
			posicao_atual_inimigo2 -= 5;
		else
			posicao_atual_inimigo2 += 5;
		
		osMutexWait(mutex_display_ID,osWaitForever);
		apagaInimigo(ANDAR_INIMIGO2, posicao_anterior_inimigo2);
		imprimeEscada(posicaoEscadas[0][ANDAR_INIMIGO2 - 2], posicaoEscadas[1][4 - 2]);
		imprimeInimigo(ANDAR_INIMIGO2, posicao_atual_inimigo2);
		osMutexRelease(mutex_display_ID);
		posicao_anterior_inimigo2 = posicao_atual_inimigo2;
	}
}

void movimentaInimigo3(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(movimentaInimigo3_ID,0x01); //limpa o signal
		if(posicao_atual_inimigo3 >= 120) //alcancou o maximo da tela
			volta_inimigo3 = true; //muda flag
		if(posicao_atual_inimigo3 <= 0) //alcancou o minimo da tela
			volta_inimigo3 = false; //muda tela
		if(volta_inimigo3) //se estiver indo para esquerda
			posicao_atual_inimigo3 -= 5;
		else
			posicao_atual_inimigo3 += 5;
		
		osMutexWait(mutex_display_ID,osWaitForever);
		apagaInimigo(ANDAR_INIMIGO3, posicao_anterior_inimigo3);
		imprimeEscada(posicaoEscadas[0][ANDAR_INIMIGO3 - 2], posicaoEscadas[1][5 - 2]);
		imprimeInimigo(ANDAR_INIMIGO3, posicao_atual_inimigo3);
		osMutexRelease(mutex_display_ID);
		posicao_anterior_inimigo3 = posicao_atual_inimigo3;
	}
}

void movimentaItem0(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(movimentaItem0_ID,0x01); //limpa o signal
		if(posicao_atual_item0 >= 120) //alcancou o maximo da tela
			posicao_atual_item0 = 0; //vai pata o outro lado da tela
		posicao_atual_item0 += 5; //continua indo para direira
		
		osMutexWait(mutex_display_ID,osWaitForever);
		apagaItem(andar_atual_item0, posicao_anterior_item0);
		imprimeEscada(posicaoEscadas[0][andar_atual_item0 - 2], posicaoEscadas[1][2 - 2]);
		imprimeItem(andar_atual_item0, posicao_atual_item0);
		osMutexRelease(mutex_display_ID);
		posicao_anterior_item0 = posicao_atual_item0;
	}
}

void movimentaItem1(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		osSignalClear(movimentaItem1_ID,0x01); //limpa o signal
		if(posicao_atual_item1 <= 0) //alcancou o minimo da tela
			posicao_atual_item1 = 120; //vai para o outro lado da tela
		posicao_atual_item1 -= 5; //continu aindo para a esquerda
		
		osMutexWait(mutex_display_ID,osWaitForever);
		apagaItem(andar_atual_item1, posicao_anterior_item1);
		imprimeEscada(posicaoEscadas[0][andar_atual_item1 - 2], posicaoEscadas[1][4 - 2]);
		imprimeItem(andar_atual_item1, posicao_atual_item1);
		osMutexRelease(mutex_display_ID);
		posicao_anterior_item1 = posicao_atual_item1;
	}
}

void inicializaPontuacao(){
	char pbufx[10];
	
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
	
	intToString(pontos, pbufx, 10, 10, 3);
}

void imprimePontuacao(){
	uint32_t color = 0xFF, i, j, pixel;
	uint16_t pos_primeiro_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+((-2)*(LARGURA_NUMEROS+1));
	uint16_t pos_segundo_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+((-1)*(LARGURA_NUMEROS+1));
	uint16_t pos_terceiro_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(0*(LARGURA_NUMEROS+1));
	uint16_t pos_quarto_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(1*(LARGURA_NUMEROS+1));
	uint16_t pos_quinto_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(2*(LARGURA_NUMEROS+1));
	uint16_t primeiro_digito = pontos/10000;
	uint16_t segundo_digito = (pontos - (primeiro_digito*10000))/1000;
	uint16_t terceiro_digito = (pontos - (primeiro_digito*10000) - (segundo_digito*1000))/100;
	uint16_t quarto_digito = (pontos - (primeiro_digito*10000) - (segundo_digito*1000) - (terceiro_digito*100))/10;
	uint16_t quinto_digito = (pontos - (primeiro_digito*10000) - (segundo_digito*1000) - (terceiro_digito*100) - (quarto_digito*10));
	
	//imprime o primeiro digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(primeiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(primeiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[pixel+(primeiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3)]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){  
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_primeiro_digito, j);
			}
			pixel += 3;
		}
	}
	
	//imprime o segundo digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(segundo_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(segundo_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[(pixel+(segundo_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_segundo_digito, j);
			}
			pixel += 3;
		}
	}
	
	//imprime o terceiro digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(terceiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(terceiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[(pixel+(terceiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_terceiro_digito, j);
			}
			pixel += 3;
		}
	}
	
	//imprime o quarto digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(quarto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(quarto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[(pixel+(quarto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_quarto_digito, j);
			}
			pixel += 3;
		}
	}
	
	//imprime o quinto digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(quinto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(quinto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[(pixel+(quinto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_quinto_digito, j);
			}
			pixel += 3;
		}
	}
}

void apagaPontuacao(){
	uint16_t i, j, k;
	uint16_t pos_primeiro_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+((-2)*(LARGURA_NUMEROS+1));
	uint16_t pos_segundo_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+((-1)*(LARGURA_NUMEROS+1));
	uint16_t pos_terceiro_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(0*(LARGURA_NUMEROS+1));
	uint16_t pos_quarto_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(1*(LARGURA_NUMEROS+1));
	uint16_t pos_quinto_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(2*(LARGURA_NUMEROS+1));
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			GrContextForegroundSet(&sContext, 0x00000000); // para apagar os pixeis coloridos
			draw_pixel(i+pos_primeiro_digito, j);
			draw_pixel(i+pos_segundo_digito, j);
			draw_pixel(i+pos_terceiro_digito, j);
			draw_pixel(i+pos_quarto_digito, j);
			draw_pixel(i+pos_quinto_digito, j);
		}
	}
}

void timer_frames(void const *argument){
	//faz o chaveamento dos frames de animacao
	frame_chefao = !frame_chefao;
	frame_inimigo = !frame_inimigo;
	if(frame_eddie == 2)
		frame_eddie = 1;
	else if(frame_eddie == 1)
		frame_eddie = 2;
	else if(frame_eddie == 3)
		frame_eddie = 4;
	else if(frame_eddie == 4)
		frame_eddie = 3;
	if(frame_item == 0)
		frame_item = 1;
	else if(frame_item == 1)
		frame_item = 2;
	else if(frame_item == 2)
		frame_item = 0;
	//envia signal para ativar a thread gameStart
	osSignalSet(gameStart_ID,0x01); 
}

//timers
osTimerId timer_frames_ID;
osTimerDef(frames,timer_frames);

void gameStart(void const *argument){ //inicia o timer, imprime o background e as escadas e fica constantemente atualizando a pontuacao
	char pbufx[10], pbufy[10], pbufz[10];
	bool center;
	float temp,lux;
	float mic;
	bool s1_press, s2_press;
	uint32_t i, j, i_anterior = 0;
	
	//Initializing all peripherals
	init_all();
	mutex_display_ID = osMutexCreate(osMutex(mutex_display));
	mutex_frames_ID = osMutexCreate(osMutex(mutex_frames));
	timer_frames_ID = osTimerCreate(osTimer(frames),osTimerPeriodic,(void *)0); //cria o timer
	inicializaPontuacao();
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	osTimerStart(timer_frames_ID, 50); //inicia o timer
	while(1){		
		// Inicialização das variáveis globais
		movimento_eddie = 0;
		x_anterior_eddie = X_INICIAL_EDDIE;
		x_atual_eddie = X_INICIAL_EDDIE;
		andar_atual_eddie = ANDAR_INICIAL_EDDIE;
		posicao_atual_chefao = POSICAO_INICIAL_CHEFAO;
		posicao_atual_inimigo0 = POSICAO_INICIAL_INIMIGO0;
		posicao_atual_inimigo1 = POSICAO_INICIAL_INIMIGO1;
		posicao_atual_inimigo2 = POSICAO_INICIAL_INIMIGO2;
		posicao_atual_inimigo3 = POSICAO_INICIAL_INIMIGO3;
		posicao_atual_item0 = POSICAO_INICIAL_ITEM0;
		andar_atual_item0 = ANDAR_INICIAL_ITEM0;
		posicao_atual_item1 = POSICAO_INICIAL_ITEM1;
		andar_atual_item1 = ANDAR_INICIAL_ITEM1;
		pontos = 0;
		posicaoEscadas[0][3] = 5;
		posicaoEscadas[0][2] = 4;
		posicaoEscadas[0][1] = 3;
		posicaoEscadas[0][0] = 2;
		posicaoEscadas[1][3] = 30;
		posicaoEscadas[1][2] = 80;
		posicaoEscadas[1][1] = 60;
		posicaoEscadas[1][0] = 100;
		
		osMutexWait(mutex_display_ID,osWaitForever);
		clear();
		imprimeBackground();
		imprimeEscadas();
		osMutexRelease(mutex_display_ID);
		
		while(1){
			osSignalWait(0x01,osWaitForever); //espera o signal para continuar
			osSignalClear(gameStart_ID,0x01); //limpa o signal
			osMutexWait(mutex_display_ID,osWaitForever);
			apagaPontuacao();
			imprimePontuacao();
			osMutexRelease(mutex_display_ID);
			pontos++;
			if(flag_fim_de_jogo)
				break;
			//envia signal para ativar as threads
			osSignalSet(leituraJoystick_ID,0x01);
			osSignalSet(leituraBotao_ID,0x01);
			osSignalSet(movimentaEddie_ID,0x01);
			osSignalSet(movimentaInimigo0_ID,0x01);
			osSignalSet(movimentaInimigo1_ID,0x01);
			osSignalSet(movimentaInimigo2_ID,0x01);
			osSignalSet(movimentaInimigo3_ID,0x01);
			osSignalSet(movimentaChefao_ID,0x01);
			osSignalSet(movimentaItem0_ID,0x01);
			osSignalSet(movimentaItem1_ID,0x01);
		}
	}
}

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void){	
	if(Init_Thread()==0)
		return 0;
	
	osKernelStart(); 
	osDelay(osWaitForever);
}