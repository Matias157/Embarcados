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

#include "rgb.h"
#include "cfaf128x128x16.h"
#include "servo.h"
#include "temp.h"
#include "opt.h"
#include "buttons.h"
#include "buzzer.h"
#include "joy.h"
#include "mic.h"
#include "accel.h"
#include "led.h"
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

void init_all(){
	cfaf128x128x16Init();
	//joy_init();
	//button_init();
}

void draw_pixel(uint16_t x, uint16_t y){
	GrPixelDraw(&sContext, x, y);
}

void init_sidelong_menu(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

int main (void) {	
	//Initializing all peripherals
	init_all();
	//Sidelong menu creation
	init_sidelong_menu();
	
	while(1)
	{	
		unsigned long pixel = BuffaloImage_start;
		uint8_t i, i_centralizado;
		uint8_t j, j_centralizado;
		//centralizando
		i_centralizado = CENTROTELA - ALTURAIMG/2;
		j_centralizado = CENTROTELA - LARGURAIMG/2;
		
		for(i = i_centralizado; i < ALTURAIMG/2 + CENTROTELA; i++){
			for(j = j_centralizado; j < LARGURAIMG/2 + CENTROTELA; j++){
				if(BuffaloImage[pixel] <= 0xF0 && BuffaloImage[pixel + 1] <= 0xF0 && BuffaloImage[pixel + 2] <= 0xF0){
					draw_pixel(j, i);
				}
				pixel += 3;
			}
		}
	}
}
