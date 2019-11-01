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
#include <math.h>
#include "grlib/grlib.h"

/*----------------------------------------------------------------------------
 * include libraries from drivers
 *----------------------------------------------------------------------------*/

#include "cfaf128x128x16.h"
#include "UART/UART.h"
#include "PWM/PWM.h"

//To print on the screen
tContext sContext;

//Variaveis compartilhadas


//Criacao das threads
void geradorOnda(void const *argument);
void PWM(void const *argument);
void UART(void const *argument);
void mostraDisplay(void const *argument);

//Varivel que determina ID das threads
osThreadId geradorOnda_ID; 
osThreadId PWM_ID; 
osThreadId UART_ID; 
osThreadId mostraDisplay_ID;   

//Definicao das threads
osThreadDef (geradorOnda, osPriorityNormal, 1, 0);     // thread object
osThreadDef (PWM, osPriorityNormal, 1, 0);     // thread object
osThreadDef (UART, osPriorityNormal, 1, 0);     // thread object
osThreadDef (mostraDisplay, osPriorityNormal, 1, 0);     // thread object

int Init_Thread (void) {
	geradorOnda_ID = osThreadCreate (osThread(geradorOnda), NULL);
	if (!geradorOnda_ID) return(-1);
	
	PWM_ID = osThreadCreate (osThread(PWM), NULL);
	if (!PWM_ID) return(-1);
	
	UART_ID = osThreadCreate (osThread(UART), NULL);
	if (!UART_ID) return(-1);
	
	mostraDisplay_ID = osThreadCreate (osThread(mostraDisplay), NULL);
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
	PWM_function_init();
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

void geradorOnda(void const *argument){
}
	
void UART(void const *argument){
}

void mostraDisplay(void const *argument){
}

void PWM(void const *argument){
	uint16_t amp = 28;
	uint8_t data = 64;
	double pi = 3.1415926;
	float tempo = 0;
	float tempo_inc = 0;
	uint16_t val;
	uint32_t cycles;
	char tempo_char[5];
	uint32_t clock = osKernelSysTickFrequency;
	PWM_per_set(200);
	PWM_enable(true);
	while(true){
		tempo_inc = (float)osKernelSysTick();
		tempo_inc -= (float) cycles;
		tempo_inc = (float)tempo_inc/clock;
		tempo += tempo_inc;	
		cycles = osKernelSysTick();	
		data = amp*sin(tempo*2.0*pi*(29/7))+85;
		//data = amp*sin(tempo*2.0*pi*29);
		//if(data <= 0)
		//		data = amp + 85  ;
		//else
		//if(data > 0)
		//		data = 85 - amp ;
		val = 1020*abs(120-data);
		PWM_amplitude_set(val);
	}
}

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void) {	
	osKernelInitialize();
	
	init_all();
	
	if(Init_Thread()==0)
		return 0;
	
	osKernelStart(); 
	osDelay(osWaitForever);
}