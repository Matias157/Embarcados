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

#define GANTT 0
#define PI 3.14159265358979323846

// if usado para gerar o Gantt
#if GANTT == 1 
int global_cycles = 0 ;					// ciclos usados para diminuir o tempo gerado no Gantt
#endif

//To print on the screen
tContext sContext;
#if GANTT == 1
osMutexDef(mu);	
osMutexId(mu_ID);
#endif

/****************** Criacao das threads *******************/
void thread_A(void const *argument);
void thread_B(void const *argument);
void thread_C(void const *argument);
void thread_D(void const *argument);
void thread_E(void const *argument);
void thread_F(void const *argument);
void escalonador(void const *argument);

/*********************** IDs threads *********************/
osThreadId thread_A_ID; 
osThreadId thread_B_ID; 
osThreadId thread_C_ID; 
osThreadId thread_D_ID;
osThreadId thread_E_ID;
osThreadId thread_F_ID;
osThreadId escalonador_ID;

/****************** Definicao das threads ****************/
osThreadDef (thread_A, osPriorityNormal, 1, 0);     // thread object
osThreadDef (thread_B, osPriorityNormal, 1, 0);     // thread object
osThreadDef (thread_C, osPriorityNormal, 1, 0);     // thread object
osThreadDef (thread_D, osPriorityNormal, 1, 0);     // thread object
osThreadDef (thread_E, osPriorityNormal, 1, 0);     // thread object
osThreadDef (thread_F, osPriorityNormal, 1, 0);     // thread object
osThreadDef (escalonador, osPriorityNormal, 1, 0);     // thread object

/************************* Mutex *************************/
osMutexDef(mutex_display);	
osMutexId(mutex_display_ID);

int Init_Thread (void) {
	thread_A_ID = osThreadCreate (osThread(thread_A), NULL);
	if (!thread_A_ID) return(-1);
	
	thread_B_ID = osThreadCreate (osThread(thread_B), NULL);
	if (!thread_B_ID) return(-1);
	
	thread_C_ID = osThreadCreate (osThread(thread_C), NULL);
	if (!thread_C_ID) return(-1);
	
	thread_D_ID = osThreadCreate (osThread(thread_D), NULL);
	if (!thread_D_ID) return(-1);
	
	thread_E_ID = osThreadCreate (osThread(thread_E), NULL);
	if (!thread_E_ID) return(-1);
	
	thread_F_ID = osThreadCreate (osThread(thread_F), NULL);
	if (!thread_F_ID) return(-1);
	
	escalonador_ID = osThreadCreate (osThread(escalonador), NULL);
	if (!escalonador_ID) return(-1);
	
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

void init_mutex(){
	mutex_display_ID = osMutexCreate(osMutex(mutex_display));
}

void init_sidelong_menu(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
	
	GrStringDraw(&sContext,"Amplitude: ", -1, 0, (sContext.psFont->ui8Height+2)*0, true);
	GrStringDraw(&sContext,"Frequencia: ", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
}

void init_all(){
	cfaf128x128x16Init();	
	initUART();
	init_sidelong_menu();
	init_mutex();
}

/*----------------------------------------------------------------------------
 *    Display
 *---------------------------------------------------------------------------*/

void draw_pixel(uint16_t x, uint16_t y){
	// INICIO DA SECAO CRITICA
	osMutexWait(mutex_display_ID,osWaitForever);
	GrPixelDraw(&sContext, x, y);
	osMutexRelease(mutex_display_ID);
	// FIM DA SECAO CRITICA
}

void printInt(int valor, int lin, int col){
	char buffer[10];
	// INICIO DA SECAO CRITICA
	osMutexWait(mutex_display_ID,osWaitForever);
	intToString(valor,buffer,10,10,0);
	GrStringDraw(&sContext,(char*)buffer, -1, (sContext.psFont->ui8MaxWidth)*col,  (sContext.psFont->ui8Height+2)*lin, true);
	osMutexRelease(mutex_display_ID);
	// FIM DA SECAO CRITICA
}

void printDouble(double value, int lin, int col){
	char buffer[10];
	// INICIO DA SECAO CRITICA
	osMutexWait(mutex_display_ID,osWaitForever);
	floatToString(value,buffer,10,10,0,2);
	GrStringDraw(&sContext,(char*)buffer, -1, (sContext.psFont->ui8MaxWidth)*col,  (sContext.psFont->ui8Height+2)*lin, true);
	osMutexRelease(mutex_display_ID);
	// FIM DA SECAO CRITICA
}

void printString(char* string, int lin, int col){
	// INICIO DA SECAO CRITICA
	osMutexWait(mutex_display_ID,osWaitForever);
	GrStringDraw(&sContext,(char*)string, -1, (sContext.psFont->ui8MaxWidth)*col,  (sContext.psFont->ui8Height+2)*lin, true);
	osMutexRelease(mutex_display_ID);
	// FIM DA SECAO CRITICA
}

int fatorial(int a){
	int i;
	int result = 1;
	
	for(i = 1; i <= a; i++){
		result *= i;
	}
	
	return(result);
}

/*----------------------------------------------------------------------------
 *    Handlers
 *---------------------------------------------------------------------------*/

void UART0_Handler(void){
	// Variaveis para leitura de dados da UART
	char caracter;
	
	// Espera flag de interrupção ser 0
	while((UART0->FR & (0x10000)) != 0);
	UART0->FR &= (0x10000); // limpa flag de interrupção (coloca 1)
	
	// Coloca saida na variável
	caracter = UART0->DR;
	
	UART0->RIS |= (0x1000); // seta RIS
	
	//writeUART(caracter);
	//UARTprintString("\n\r");
}

/*----------------------------------------------------------------------------
 *    Threads
 *---------------------------------------------------------------------------*/

void thread_A(void const *argument){
	int i;
	double x;
	int tick_init;
	int tick_end;
	char tick_init_char [32];
	char tick_end_char [32];
	while(1){
		x = 0.0;
		tick_init = osKernelSysTick();
		for(i = 0; i <= 256; i++){
			x += (double)i + (i + 2);
		}
		tick_end = osKernelSysTick();
		tick_end = tick_end - tick_init;
		intToString(tick_end,tick_end_char,32,10,0);
		UARTprintString("tick_end: ");
		UARTprintString(tick_end_char);
		UARTprintString("\r\n");
	}
}

void thread_B(void const *argument){
	int i;
	double dois_a_n;
	double x;
	int tick_init;
	int tick_end;
	char tick_init_char [32];
	char tick_end_char [32];
	while(1){
		dois_a_n = 1.0;
		x = 0.0;
		tick_init = osKernelSysTick();
		for(i = 1; i <= 16; i++){
			dois_a_n = dois_a_n * 2;
			x += dois_a_n/fatorial(i);
		}
		tick_end = osKernelSysTick();
		tick_end = tick_end - tick_init;
		intToString(tick_end,tick_end_char,32,10,0);
		UARTprintString("tick_end: ");
		UARTprintString(tick_end_char);
		UARTprintString("\r\n");
	}
}

void thread_C(void const *argument){
	int i;
	double x;
	int tick_init;
	int tick_end;
	char tick_init_char [32];
	char tick_end_char [32];
	while(1){
		x = 0.0;
		tick_init = osKernelSysTick();
		for(i = 1; i <= 72; i++){
			x += (i + 1) / (double)i;
		}
		tick_end = osKernelSysTick();
		tick_end = tick_end - tick_init;
		intToString(tick_end,tick_end_char,32,10,0);
		UARTprintString("tick_end: ");
		UARTprintString(tick_end_char);
		UARTprintString("\r\n");
	}
}

void thread_D(void const *argument){
	int i;
	double x;
	int tick_init;
	int tick_end;
	char tick_init_char [32];
	char tick_end_char [32];
	while(1){
		x = 0.0;
		tick_init = osKernelSysTick();
		x = 1.0 + (5.0/(fatorial(3))) + (5.0/(fatorial(5))) + (5.0/(fatorial(7))) + (5.0/(fatorial(9)));
		tick_end = osKernelSysTick();
		tick_end = tick_end - tick_init;
		intToString(tick_end,tick_end_char,32,10,0);
		UARTprintString("tick_end: ");
		UARTprintString(tick_end_char);
		UARTprintString("\r\n");
	}
}

void thread_E(void const *argument){
	int i;
	double x;
	int tick_init;
	int tick_end;
	char tick_init_char [32];
	char tick_end_char [32];
	while(1){
		x = 0.0;
		tick_init = osKernelSysTick();
		for(i = 1; i <= 100; i++){
			x += i * PI * PI;
		}
		tick_end = osKernelSysTick();
		tick_end = tick_end - tick_init;
		intToString(tick_end,tick_end_char,32,10,0);
		UARTprintString("tick_end: ");
		UARTprintString(tick_end_char);
		UARTprintString("\r\n");
	}
}

void thread_F(void const *argument){
	int i;
	double dois_a_n;
	double x;
	int tick_init;
	int tick_end;
	char tick_init_char [32];
	char tick_end_char [32];
	while(1){
		dois_a_n = 1.0;
		x = 0.0;
		tick_init = osKernelSysTick();
		for(i = 1; i <= 128; i++){
			dois_a_n = dois_a_n * 2.0;
			x += i * i * i / dois_a_n;
		}
		tick_end = osKernelSysTick();
		tick_end = tick_end - tick_init;
		intToString(tick_end,tick_end_char,32,10,0);
		UARTprintString("tick_end: ");
		UARTprintString(tick_end_char);
		UARTprintString("\r\n");
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