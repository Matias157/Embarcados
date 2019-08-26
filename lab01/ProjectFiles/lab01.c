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
#include "joy.h"
#include "airplane.h"
#include "buffalo.h"

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7
#define ALTURAIMG  64
#define LARGURAIMG 96
#define CENTROTELA 63

tContext sContext;
unsigned long pixel, pixel_init;
uint8_t tela[128*128];
	
void Redimensiona_imagem(unsigned char *imagem,int dima);
void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);

void init_all(){
	
	cfaf128x128x16Init();
	button_init();
	
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
	GrContextBackgroundSet(&sContext, ClrWhite);
}

void init_sidelong_menu(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

void filtro_por_media(){
	int i, j, x, y;
	uint8_t valor_pixel;
	for(i = 0; i < 128; i += 2){
//		if((i + 1) < 128)
//			i_mais_um = i + 1;
//		else
//			i_mais_um = i;
		for(j = 0; j < 128; j += 2){
//			if((j + 1) < 128)
//				j_mais_um = j + 1;
//			else
//				j_mais_um = j;
			if((tela[i*128 + j] + tela[i*128 + j + 1] +
					tela[(i + 1)*128 + j] + tela[(i + 1)*128 + j + 1]) >= 1)
				valor_pixel = 1;
			else
				valor_pixel = 0;
			
			for(x = 0; x < 2; x++){
				for(y = 0; y < 2; y++){
					tela[(i + x)*128 + y + j] = valor_pixel;
				}
			}
		}
	}
}

int main (void) {	
	int dimensao = 1;
	uint8_t x;
	uint8_t y;
	bool s1_press, s2_press, s1_alterna = true, s2_alterna = true;
	unsigned char *image = (unsigned char*)(AirplaneImage);
		pixel_init = AirplaneImage_start;
	//Initializing all peripherals
	init_all();
	//Sidelong menu creation
	init_sidelong_menu();
	SysTick_Init();
	
	while(1)
	{
		for(dimensao = 3; dimensao < 14 ; dimensao++){
			s1_press = button_read_s1();
			s2_press = button_read_s2();
			
			if(s1_press){
				if(s1_alterna){
					pixel_init = AirplaneImage_start;
					image = (unsigned char*)AirplaneImage;
					s1_alterna = false;
				}
				else{
					pixel_init = BuffaloImage_start;
					image = (unsigned char*)BuffaloImage;
					s1_alterna = true;
				}
			}
			
			if(s2_press){
				if(s2_alterna){
					s2_alterna = false;
				}
				else{
					s2_alterna = true;
				}
			}

			pixel = pixel_init;
			for(x = 0; x < 128; x++){		// zera o vetor da tela
				for(y = 0; y < 128; y++){
					tela[x*128+y] = 0;
				}
			}
			
			Redimensiona_imagem(image,dimensao);
			if(dimensao <= 3) filtro_por_media();
			
			// imprime só a imagem
			for(x = 0; x < 64*4/dimensao; x++){
				for(y = 0; y < 96*4/dimensao; y++){
					if(s2_alterna){
						if(tela[128*x + y])
							draw_pixel(y, x);
					}
					else{
						if(!tela[128*x + y])
							draw_pixel(y, x);
					}
				}
			}
			
			/*for(i = 0; i < ALTURAIMG; i++){
				for(j = 0; j < LARGURAIMG; j++){
					if(s2_alterna){
						// mudar para if (image_redimensionada[pixel] = 1)
						if(image[pixel] <= 0xF0 && image[pixel + 1] <= 0xF0 && image[pixel + 2] <= 0xF0){
							draw_pixel(j, i,dimensao);
						}
						pixel += 3;
					}
					else{
						if(!(image[pixel] <= 0xF0 && image[pixel + 1] <= 0xF0 && image[pixel + 2] <= 0xF0)){
							draw_pixel(j, i,dimensao);
						}
						pixel += 3;
					}
				}
			} */
			SysTick_Wait1ms(500);
			clear();
		}
	}
}
