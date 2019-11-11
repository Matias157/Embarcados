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

/****************** Dados da UART ***********************/
typedef struct{
	char tipo_onda;
	int frequencia;
	double amplitude;
} dados_UART;

/*************** Cria fila de mensagens ******************/
// Fila de mensagens do display
osMessageQId(fila_amplitude_display_ID);
osMessageQDef(fila_amplitude_display, 5, uint16_t);

// Fila de mensagens do PWM
osMessageQId(fila_amplitude_PWM_ID);
osMessageQDef(fila_amplitude_PWM, 5, uint16_t);

// Fila de mensagens do Handler da UART
osMessageQId(fila_handler_UART_ID);
osMessageQDef(fila_handler_UART, 5, char);

// Fila de mensagens dos dados da onda
osMessageQId(fila_amplitude_onda_ID);
osMessageQDef(fila_amplitude_onda, 5, double);

osMessageQId(fila_frequencia_onda_ID);
osMessageQDef(fila_frequencia_onda, 5, int);

osMessageQId(fila_tipo_onda_ID);
osMessageQDef(fila_tipo_onda, 5, char);

/****************** Criacao das threads *******************/
void geradorOnda(void const *argument);
void PWM(void const *argument);
void mostraDisplay(void const *argument);
void tratamentoHandler(void const *argument);

/*********************** IDs threads *********************/
osThreadId geradorOnda_ID; 
osThreadId PWM_ID; 
osThreadId mostraDisplay_ID; 
osThreadId tratamentoHandler_ID; 

/****************** Definicao das threads ****************/
osThreadDef (geradorOnda, osPriorityNormal, 1, 0);     // thread object
osThreadDef (PWM, osPriorityNormal, 1, 0);     // thread object
osThreadDef (mostraDisplay, osPriorityNormal, 1, 0);     // thread object
osThreadDef (tratamentoHandler, osPriorityHigh, 1, 0);     // thread object

/************************* Mutex *************************/
osMutexDef(mutex_display);	
osMutexId(mutex_display_ID);

int Init_Thread (void) {
	geradorOnda_ID = osThreadCreate (osThread(geradorOnda), NULL);
	if (!geradorOnda_ID) return(-1);
	
	PWM_ID = osThreadCreate (osThread(PWM), NULL);
	if (!PWM_ID) return(-1);
	
	mostraDisplay_ID = osThreadCreate (osThread(mostraDisplay), NULL);
	if (!mostraDisplay_ID) return(-1);
	
	tratamentoHandler_ID = osThreadCreate (osThread(tratamentoHandler), NULL);
	if (!tratamentoHandler_ID) return(-1);
	
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
	initPWM();
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
	
	osMessagePut(fila_handler_UART_ID, caracter, 0);
	osSignalSet(tratamentoHandler_ID,0x01);
}

/*----------------------------------------------------------------------------
 *    Threads
 *---------------------------------------------------------------------------*/

void tratamentoHandler(void const *argument){
	
	#if GANTT == 1 
		int cycles_init;
		char cycles_init_char [30];
		int cycles_end;
		char cycles_end_char [30];
	#endif
	
	
	osEvent event;
	char dado;
	char tipo_onda = '1';
	double amplitude = 3.3;
	int frequencia = 50;

	fila_handler_UART_ID = osMessageCreate(osMessageQ(fila_handler_UART), NULL);
	
	while(1){
	
		#if GANTT == 1
			osMutexWait(mu_ID,osWaitForever);
			cycles_end = global_cycles++;
			intToString(cycles_end,cycles_end_char,30,10,0);
			UARTprintString("t: a,");
			UARTprintString(cycles_init_char);
			UARTprintString(",");
			UARTprintString(cycles_end_char);
			UARTprintString("\r\n");
			osMutexRelease(mu_ID);
		#endif
		
		osDelay(50);
		
		osSignalWait(0x01,osWaitForever);
		osSignalClear(tratamentoHandler_ID,0x01);
		
		/* espera mensagem vinda do handler */
		event = osMessageGet(fila_handler_UART_ID, osWaitForever); // espera dados vindo do handler da UART
		
		#if GANTT == 1 
				cycles_init = global_cycles++;
				intToString(cycles_init,cycles_init_char,30,10,0);
		#endif
		
		if(event.status == osEventMessage){
			dado = (char)event.value.v;
		
			/* atualiza tipo de onda */
			if(dado >= '1' && dado <= '5'){		
				tipo_onda = dado;
				osMessagePut(fila_tipo_onda_ID, (char)(tipo_onda), 0);
			}
			
			/* atualiza frequencia */
			else if(dado == 'A' && (frequencia + 5) <= 200){
				frequencia += 5;
				osMessagePut(fila_frequencia_onda_ID, (int)(frequencia), 0);
				osSignalSet(geradorOnda_ID,0x01);
			}
			else if(dado == 'S' && (frequencia - 5) >= 1){
				frequencia -= 5;
				osMessagePut(fila_frequencia_onda_ID, (int)(frequencia), 0);
				osSignalSet(geradorOnda_ID,0x01);
			}
			else if(dado == 'a' && (frequencia + 1) <= 200){
				frequencia += 1;
				osMessagePut(fila_frequencia_onda_ID, (int)(frequencia), 0);
				osSignalSet(geradorOnda_ID,0x01);
			}
			else if(dado == 's' && (frequencia - 1) >= 1){
				frequencia -= 1;
				osMessagePut(fila_frequencia_onda_ID, (int)(frequencia), 0);
				osSignalSet(geradorOnda_ID,0x01);
			}
			
			/* atualiza amplitude */
			else if(dado == 'J' && (amplitude + 1) <= 3.3){
				amplitude += 1;
				osMessagePut(fila_amplitude_onda_ID, (double)(amplitude*10), 0);
				osSignalSet(geradorOnda_ID,0x01);
			}
			else if(dado == 'K' && (amplitude - 1) >= 0){
				amplitude -= 1;
				osMessagePut(fila_amplitude_onda_ID, (double)(amplitude*10), 0);
				osSignalSet(geradorOnda_ID,0x01);
			}
			else if(dado == 'j' && (amplitude + 0.1) <= 3.3){
				amplitude += 0.1;
				osMessagePut(fila_amplitude_onda_ID, (double)(amplitude*10), 0);
				osSignalSet(geradorOnda_ID,0x01);
			}
			else if(dado == 'k' && (amplitude - 0.1) >= 0){
				amplitude -= 0.1;
				osMessagePut(fila_amplitude_onda_ID, (double)(amplitude*10), 0);
				osSignalSet(geradorOnda_ID,0x01);
			}
		}
	}
}

void geradorOnda(void const *argument){
	
	#if GANTT == 1 
		int cycles_init;
		char cycles_init_char [30];
		int cycles_end;
		char cycles_end_char [30];
	#endif
	
	osEvent event1, event2, event3;
	char dado;
	int frequencia = 50;
	char tipo_onda = '1';
	double amplitude = 3.3;
	double tempo = 0.0;
	double pi = 3.1415926;
	double amplitude_msg;
	
	fila_tipo_onda_ID = osMessageCreate(osMessageQ(fila_tipo_onda), NULL);
	fila_amplitude_onda_ID = osMessageCreate(osMessageQ(fila_amplitude_onda), NULL);
	fila_frequencia_onda_ID = osMessageCreate(osMessageQ(fila_frequencia_onda), NULL);
	
	printInt(frequencia,1, 12);
	printString("Hz",1,15);
	printDouble(amplitude,0, 11);
	printString("V",0,14);
	
	while(1){
		
		#if GANTT == 1
			osMutexWait(mu_ID,osWaitForever);
			cycles_end = global_cycles++;
			intToString(cycles_end,cycles_end_char,30,10,0);
			UARTprintString("g: a,");
			UARTprintString(cycles_init_char);
			UARTprintString(",");
			UARTprintString(cycles_end_char);
			UARTprintString("\r\n");
			osMutexRelease(mu_ID);
		#endif
		
		osSignalWait(0x01,0);
		osSignalClear(geradorOnda_ID,0x01);
		
		/* recebe mensagens vindas da tratamentoUART mas não fica esperando */
		event1 = osMessageGet(fila_amplitude_onda_ID, 0); // espera dados vindo do tratamentoUART
		event2 = osMessageGet(fila_frequencia_onda_ID, 0); // espera dados vindo do tratamentoUART
		event3 = osMessageGet(fila_tipo_onda_ID, 0); // espera dados vindo do tratamentoUART
		
		#if GANTT == 1 
				cycles_init = global_cycles++;
				intToString(cycles_init,cycles_init_char,30,10,0);
		#endif
		
		/* recebe mensagens e atualiza display */
		if(event1.status == osEventMessage){
			amplitude = (double)event1.value.v/10.0;
			printString("   ",0,14);
			printDouble(amplitude,0, 11);
			printString("V",0,14);
		}
		if(event2.status == osEventMessage){
			frequencia = (int)event2.value.v;
			printString("   ",1,12);
			printInt(frequencia,1, 12);
			printString("Hz",1,15);
		}
		if(event3.status == osEventMessage){
			tipo_onda = (char)event3.value.v;
		}
			
		/* Calcula a porcentagem do ciclo */
		if(tipo_onda == '1'){								// seno
			if(frequencia*tempo*3.8 >= 1 ){
				tempo = 0;
			}
			else{
				tempo += 2.3*0.0000125;
			}
		}
		else if(tipo_onda == '3'){					// dente de serra
			if(frequencia*tempo >= 1 ){
				tempo = 0;
			}
			else{
				tempo += 4*0.000014;
			}
		}
		else if(tipo_onda == '4'){					// triangular
			if(frequencia*tempo >= 1 ){
				tempo = 0;
			}
			else{
				tempo += 3.5*0.0000165;
			}
		}
		else if(tipo_onda == '5'){					// trapezoidal
			if(frequencia*tempo >= 1 ){
				tempo = 0;
			}
			else{
				tempo += 3.8*0.0000167;
			}
		}
		else if(frequencia*tempo >= 1 ){		// quadrada
			tempo = 0;
		}
		else{
			tempo += 4.3*0.0000125;
		}
		
		/* Calcula a amplitude de acordo com a frequência e tipo de onda */
		if(tipo_onda == '1'){								// seno
			amplitude_msg = (amplitude*sin(tempo*2.0*pi*frequencia*3.8) + amplitude);
		}
		else if(tipo_onda == '2'){					// quadrada
			if(frequencia*tempo < 0.5){
				amplitude_msg = amplitude*2.0;
			}
			else{
				amplitude_msg = 0;
			}
		}
		else if(tipo_onda == '3'){					// dente de serra
			amplitude_msg = 2*amplitude*frequencia*tempo;
		}
		else if(tipo_onda == '4'){					//triangular
			if(frequencia*tempo < 0.5){
				amplitude_msg = 4*amplitude*frequencia*tempo;
			}
			else{
				amplitude_msg = 4*amplitude*(1-frequencia*tempo);
			}
		}
		else if(tipo_onda == '5'){					// trapezoidal
			if(frequencia*tempo <= 0.25){
				amplitude_msg = 8*amplitude*frequencia*tempo;
			}
			else if(frequencia*tempo <= 0.5){
				amplitude_msg = 2*amplitude;
			}
			else if(frequencia*tempo <= 0.75){
				amplitude_msg = 8*amplitude*(0.75-frequencia*tempo);
			}
			else if(frequencia*tempo <= 1){
				amplitude_msg = 0;
			}
		}

		/* envia mensagems para as threads mostraDisplay e PWM */
		osMessagePut(fila_amplitude_PWM_ID, (uint32_t)(7*1370*amplitude_msg), 0);
		osMessagePut(fila_amplitude_display_ID, (uint32_t)(120-amplitude_msg*11), 0);
		
		osSignalSet(mostraDisplay_ID,0x01);
		osSignalSet(PWM_ID,0x01);
		
	}
}

void mostraDisplay(void const *argument){
	
	#if GANTT == 1 
		int cycles_init;
		char cycles_init_char [30];
		int cycles_end;
		char cycles_end_char [30];
	#endif
	
	osEvent event;
	uint16_t amplitude= 0;
	uint16_t i = 0, y, amplitude_anterior = -1;
	
	fila_amplitude_display_ID = osMessageCreate(osMessageQ(fila_amplitude_display), NULL);
	
	while(1){
		
		for(i = 0; i < 128; i++){
			
		#if GANTT == 1
			osMutexWait(mu_ID,osWaitForever);
			cycles_end = global_cycles++;
			intToString(cycles_end,cycles_end_char,30,10,0);
			UARTprintString("d: a,");
			UARTprintString(cycles_init_char);
			UARTprintString(",");
			UARTprintString(cycles_end_char);
			UARTprintString("\r\n");
			osMutexRelease(mu_ID);
		#endif
			
			osSignalWait(0x01,osWaitForever);
			osSignalClear(mostraDisplay_ID,0x01);
			
			/* espera receber mensagem vinda da geradorOnda */
			event = osMessageGet(fila_amplitude_display_ID, osWaitForever); // espera dados vindo do handler da UART
		
		#if GANTT == 1 
				cycles_init = global_cycles++;
				intToString(cycles_init,cycles_init_char,30,10,0);
		#endif
			
			amplitude = (uint16_t)event.value.v;

			/* escreve no display */
			// INICIO DA SECAO CRITICA
			osMutexWait(mutex_display_ID,osWaitForever);
			GrContextForegroundSet(&sContext, ClrBlack);
			GrLineDrawV(&sContext,i,30,127);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrLineDrawV(&sContext,i,amplitude,amplitude_anterior);
			osMutexRelease(mutex_display_ID);
			// FIM DA SECAO CRITICA
			
			
			amplitude_anterior = amplitude;
		}
	}
}

void PWM(void const *argument){
	
	#if GANTT == 1 
		int cycles_init;
		char cycles_init_char [30];
		int cycles_end;
		char cycles_end_char [30];
	#endif
	
	osEvent event;
	uint16_t amplitude= 0;
	char buffer[10];
	
	PWM_per_set(200);
	PWM_enable(true);
	fila_amplitude_PWM_ID = osMessageCreate(osMessageQ(fila_amplitude_PWM), NULL);
	
	while(1){
		
		#if GANTT == 1
			osMutexWait(mu_ID,osWaitForever);
			cycles_end = global_cycles++;
			intToString(cycles_end,cycles_end_char,30,10,0);
			UARTprintString("P: a,");
			UARTprintString(cycles_init_char);
			UARTprintString(",");
			UARTprintString(cycles_end_char);
			UARTprintString("\r\n");
			osMutexRelease(mu_ID);
		#endif
		
		osSignalWait(0x01,osWaitForever);
		osSignalClear(PWM_ID,0x01);
		
		/* espera receber mensagem vinda da geradorOnda */
		event = osMessageGet(fila_amplitude_PWM_ID, osWaitForever); // espera dados vindo do handler da UART
		
		#if GANTT == 1 
				cycles_init = global_cycles++;
				intToString(cycles_init,cycles_init_char,30,10,0);
		#endif
		
		amplitude = (uint16_t)event.value.v;
		/* escreve no PWM */
		PWM_amplitude_set(amplitude);
		
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