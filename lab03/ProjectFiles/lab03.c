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
#include "mic.h"
#include "Eddie.h"
#include "Chefao.h"
#include "Escada.h"
#include "Inimigo.h"
#include "Item.h"
#include "Background.h"


#define ALTURA_ANDAR			20
#define ESPESSURA_CHAO		2
#define ESPACO_PONTUACAO	13
#define QUANT_ANDARES			5
#define LARGURA_TELA			128
#define ALTURA_EDDIE			14
#define LARGURA_EDDIE			8
#define X_INICIAL_EDDIE		64
#define ANDAR_INICIAL_EDDIE		5

//To print on the screen
tContext sContext;

void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);

//***************** Variáveis globais  ***************************************
uint32_t movimento_eddie;
uint32_t x_anterior_eddie, x_atual_eddie;
uint32_t andar_atual_eddie;

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
	mic_init();
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

void background(){
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
void imprimeEddie(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Eddie_start;
	for(j = 0; j < ALTURA_EDDIE; j++){ //altura
		for(i = 0; i < LARGURA_EDDIE; i++){ //largura
			color = 0x00;
			color += Eddie[pixel+2];
			color += Eddie[pixel+1]*16*16; //desloca 8 bits para esquerda
			color += Eddie[pixel]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+posicao, j-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
			}
			pixel += 3;
		}
	}
}

// andar mais alto é o 1, o mais baixo é o 5
// posicao é o tamanho da largura da tela menos a largura do eddie (0 a 120)
void apagaEddie(uint32_t andar, uint32_t posicao){
	uint32_t color = 0xFF, i, j, pixel;
	pixel = Eddie_start;
	for(j = 0; j < ALTURA_EDDIE; j++){ //altura
		for(i = 0; i < LARGURA_EDDIE; i++){ //largura
			color = 0x00;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			GrContextForegroundSet(&sContext, color);
			draw_pixel(i+posicao, j-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		}
	}
}

void escada(){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Escada_start;
	for(j = 0; j < 20; j++){ //altura
		for(i = 0; i < 17; i++){ //largura
			color = 0x00;
			color += Escada[pixel+2];
			color += Escada[pixel+1]*16*16;
			color += Escada[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+40, j+15+22+22+22+22);
			}
			pixel += 3;
		}
	}
}

void chefao(){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Chefao_start;
	for(j = 0; j < 14; j++){ //altura
		for(i = 0; i < 8; i++){ //largura
			color = 0x00;
			color += Chefao[pixel+2];
			color += Chefao[pixel+1]*16*16;
			color += Chefao[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+64, j+21);
			}
			pixel += 3;
		}
	}
}

void inimigo(){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Inimigo_start;
	for(j = 0; j < 5; j++){ //altura
		for(i = 0; i < 8; i++){ //largura
			color = 0x00;
			color += Inimigo[pixel+2];
			color += Inimigo[pixel+1]*16*16;
			color += Inimigo[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+20, j+30+22+22+22);
			}
			pixel += 3;
		}
	}
}

void item(){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Item_start;
	for(j = 0; j < 4; j++){ //altura
		for(i = 0; i < 8; i++){ //largura
			color = 0x00;
			color += Item[pixel+2];
			color += Item[pixel+1]*16*16;
			color += Item[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+20, j+16+22+22);
			}
			pixel += 3;
		}
	}
}

uint32_t leituraJoystick(){
	int x, y;
	x = joy_read_x();
	y = joy_read_y();
	x = x*200/0xFFF-100; 
	y = y*200/0xFFF-100;
	if (x<-50 && y>-50 && y<50){
		movimento_eddie = 1;						//esquerda
	}
	else if (x>50 && y>-50 && y<50){
		movimento_eddie = 2;						//direita
	}
	else if (y<-50 && x>-50 && x<50){
		movimento_eddie = 3;						//baixo
	}
	else if (y>50 && x>-50 && x<50){
		movimento_eddie = 4;						//cima
	}
	else{
		movimento_eddie = 0;
	}
}

void movimentaEddie(){
	if(movimento_eddie == 3){
		// para cima se houver escada, senão só mexe as perninhas
	}
	else if(movimento_eddie == 4){
		// para baixo se houver escada, senão só mexe as perninhas
	}
	else if(movimento_eddie == 2 && x_atual_eddie < 120){
		x_atual_eddie = x_anterior_eddie + 2;
	}
	else if(movimento_eddie == 1 && x_atual_eddie > 0){
		x_atual_eddie = x_anterior_eddie -2;
	}
	apagaEddie(andar_atual_eddie, x_anterior_eddie);
	imprimeEddie(andar_atual_eddie, x_atual_eddie);
	x_anterior_eddie = x_atual_eddie;
}
/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void) {

  char pbufx[10], pbufy[10], pbufz[10];
	bool center;
	float temp,lux;
	float mic;
	bool s1_press, s2_press;
	uint32_t i, i_anterior = 0;
	
	movimento_eddie = 0;
	x_anterior_eddie = X_INICIAL_EDDIE;
	x_atual_eddie = X_INICIAL_EDDIE;
	andar_atual_eddie = ANDAR_INICIAL_EDDIE;
	
	//Initializing all peripherals
	init_all();
	SysTick_Init();
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);

  while(1){
	leituraJoystick();
	escada();
	background();
	movimentaEddie();
	inimigo();
	chefao();
	item();
	SysTick_Wait1ms(50);
		
	}	
}
