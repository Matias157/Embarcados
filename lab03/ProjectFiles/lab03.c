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


#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7

//To print on the screen
tContext sContext;

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

void init_sidelong_menu(){
	uint8_t i;
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
	
	//Escreve menu lateral:
	GrStringDraw(&sContext,"Exemplo EK-TM4C1294XL", -1, 0, (sContext.psFont->ui8Height+2)*0, true);
	GrStringDraw(&sContext,"---------------------", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
	GrStringDraw(&sContext,"RGB", -1, 0, (sContext.psFont->ui8Height+2)*2, true);
	GrStringDraw(&sContext,"ACC", -1, 0, (sContext.psFont->ui8Height+2)*3, true);
	GrStringDraw(&sContext,"TMP", -1, 0, (sContext.psFont->ui8Height+2)*4, true);
	GrStringDraw(&sContext,"OPT", -1, 0, (sContext.psFont->ui8Height+2)*5, true);
	GrStringDraw(&sContext,"MIC", -1, 0, (sContext.psFont->ui8Height+2)*6, true);
	GrStringDraw(&sContext,"JOY", -1, 0, (sContext.psFont->ui8Height+2)*7, true);
	GrStringDraw(&sContext,"BUT", -1, 0, (sContext.psFont->ui8Height+2)*8, true);
	GrStringDraw(&sContext,"NOVO ICONE", -1, 0, (sContext.psFont->ui8Height+2)*9, true);

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

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void) {

  char pbufx[10], pbufy[10], pbufz[10];
	bool center;
	float temp,lux;
	float mic;
	bool s1_press, s2_press;
	uint8_t  	r, g, b;
	uint32_t color = 0x00, i, j, pixel;
	uint16_t x, y, z, angle=0;
	
	//Initializing all peripherals
	init_all();
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);

  while(1){
///*  Joystick		*/
//			x = joy_read_x();
//			y = joy_read_y();
//			center = joy_read_center();

	pixel = Background_start;
	for(j = 0; j < 128; j++){ //altura
		for(i = 0; i < 128; i++){ //largura
			color = 0x00;
			color += Background[pixel+2];
			color += Background[pixel+1]*16*16;
			color += Background[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i, j);
			}
			pixel += 3;
		}
	}

	pixel = Eddie_start;
	for(j = 0; j < 14; j++){ //altura
		for(i = 0; i < 8; i++){ //largura
			color = 0x00;
			color += Eddie[pixel+2];
			color += Eddie[pixel+1]*16*16;
			color += Eddie[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+64, j+19+22+22+22+22+2);
			}
			pixel += 3;
		}
	}
	
	pixel = Escada_start;
	for(j = 0; j < 20; j++){ //altura
		for(i = 0; i < 17; i++){ //largura
			color = 0x00;
			color += Escada[pixel+2];
			color += Escada[pixel+1]*16*16;
			color += Escada[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+40, j+15+22+22+22+22);
			}
			pixel += 3;
		}
	}
	
	pixel = Chefao_start;
	for(j = 0; j < 14; j++){ //altura
		for(i = 0; i < 8; i++){ //largura
			color = 0x00;
			color += Chefao[pixel+2];
			color += Chefao[pixel+1]*16*16;
			color += Chefao[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+64, j+19);
			}
			pixel += 3;
		}
	}
	
	pixel = Inimigo_start;
	for(j = 0; j < 5; j++){ //altura
		for(i = 0; i < 8; i++){ //largura
			color = 0x00;
			color += Inimigo[pixel+2];
			color += Inimigo[pixel+1]*16*16;
			color += Inimigo[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+20, j+29+22+22+22);
			}
			pixel += 3;
		}
	}

	pixel = Item_start;
	for(j = 0; j < 4; j++){ //altura
		for(i = 0; i < 8; i++){ //largura
			color = 0x00;
			color += Item[pixel+2];
			color += Item[pixel+1]*16*16;
			color += Item[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i+20, j+14+22+22);
			}
			pixel += 3;
		}
	}
	

	/*x = 0.0048*joy_read_x();
	y = -0.0029*joy_read_y() + 12;

	GrContextForegroundSet(&sContext, ClrTurquoise);
	GrContextBackgroundSet(&sContext, ClrBlack);

	draw_circle(x, y);
	fill_circle(x, y);
	
	
	for(i = 0; i < 1000000; i++){}
	
	GrContextForegroundSet(&sContext, ClrBlack);
	GrContextBackgroundSet(&sContext, ClrWhite);
	
	draw_circle(x, y);
	fill_circle(x, y);*/

//			intToString(x*200/0xFFF-100, pbufx, 10, 10, 4);
//			intToString(y*200/0xFFF-100, pbufy, 10, 10, 4);
			
//			if (center)
//				intToString(1, pbufz, 10, 10, 1);
//			else
//				intToString(0, pbufz, 10, 10, 1);
//				
//			GrContextBackgroundSet(&sContext, ClrBlack);
//			GrContextForegroundSet(&sContext, ClrWhite);
//			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*7, true);
//			GrStringDraw(&sContext,(char*)pbufy, -1,  (sContext.psFont->ui8MaxWidth)*11, (sContext.psFont->ui8Height+2)*7, true);
//			GrStringDraw(&sContext,(char*)pbufz, -1,  (sContext.psFont->ui8MaxWidth)*18, (sContext.psFont->ui8Height+2)*7, true);
//
///*	Botoes 	*/			
//			s1_press = button_read_s1();
//			s2_press = button_read_s2();

//			if (s1_press)
//				intToString(1, pbufx, 10, 10, 1);
//			else
//				intToString(0, pbufx, 10, 10, 1);

//			if (s2_press)
//				intToString(1, pbufy, 10, 10, 1);
//			else
//				intToString(0, pbufy, 10, 10, 1);

//			GrContextBackgroundSet(&sContext, ClrBlack);
//			GrContextForegroundSet(&sContext, ClrWhite);
//			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*8, true);
//			GrStringDraw(&sContext,(char*)pbufy, -1,  (sContext.psFont->ui8MaxWidth)*11, (sContext.psFont->ui8Height+2)*8, true);
			
	}	
}
