/*============================================================================
 *                    Exemplos de utiliza��o do Kit
 *              EK-TM4C1294XL + Educational BooterPack MKII 
 *---------------------------------------------------------------------------*
 *                    Prof. Andr� Schneider de Oliveira
 *            Universidade Tecnol�gica Federal do Paran� (UTFPR)
 *===========================================================================
 * Autores das bibliotecas:
 * 		Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
 * 		Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
 * 		Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
 * 		Mariana Carri�o        - <mcarriao@alunos.utfpr.edu.br>
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
#define DEADLINE_A 151000		
#define DEADLINE_B 20000
#define DEADLINE_C 50000
#define DEADLINE_D 3000
#define DEADLINE_E 70000
#define DEADLINE_F 100000
#define PERIODICIDADE_A 8
#define PERIODICIDADE_B 2
#define PERIODICIDADE_C 5
#define PERIODICIDADE_D 1
#define PERIODICIDADE_E 6
#define PERIODICIDADE_F 10
#define UM_SEGUNDO 700000000
#define TOTAL_THREADS 6
#define TIMER_TICK 20

//To print on the screen
tContext sContext;

/****************** Enums ******************/
typedef enum estado{
	ready,
	running,
	waiting
} estado;

typedef enum prioridade{
	realtime = -100,
	high = -30,
	normal = 0,
	low = 30
} prioridade;

/****************** Struct para as tarefas ******************/
typedef struct tarefa_display{
	osThreadId id;
	int deadline;
	int periodicidade;
	int prioridade;
	int tempo_relaxamento;
	estado estado;
	double percentual_exec;
	int atraso;
	struct tarefa_display *proxima_tarefa;
}tarefa_display;

typedef struct info_tarefas{
	int duracao;
	int sequencia;
	int max_sequencia;
	double resultado;
	bool fault;
} info_tarefas;

/****************** Enums ******************/
osMailQId mail_tarefa_ID;
osMailQDef (mail_tarefa, 5, info_tarefas*);

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
osThreadDef (escalonador, osPriorityRealtime, 1, 0);     // thread object

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


void printDisplay(char *s, int linha, int coluna){
	osMutexWait(mutex_display_ID,osWaitForever);
	GrContextForegroundSet(&sContext, ClrWhite);
	GrStringDraw(&sContext, s, -1, (sContext.psFont->ui8MaxWidth)*coluna, (sContext.psFont->ui8Height+2)*linha, true);
	osMutexRelease(mutex_display_ID);
}

int fatorial(int a){
	int i;
	int result = 1;
	
	for(i = 1; i <= a; i++){
		result *= i;
	}
	
	return(result);
}


void escalonador(void const *argument){
	tarefa_display *prontas[7];
	tarefa_display *espera[7];
	tarefa_display tarefa_A, tarefa_B, tarefa_C, tarefa_D, tarefa_E, tarefa_F;
	tarefa_display *tarefa_running=NULL;
	tarefa_display *prox_tarefa=NULL;
	int i,j;
	char str_s[50];
	// evento
	osEvent evt;
	// cria tarefa atual
	info_tarefas *info_tarefa_atual = NULL;
	// inicializa contador de um segundo
	int conta_um_segundo = 0;
	// inicializa tempo de relaxamento
	tarefa_A.periodicidade = 8;
	tarefa_B.periodicidade = 2;
	tarefa_C.periodicidade = 5;
	tarefa_D.periodicidade = 1;
	tarefa_E.periodicidade = 6;
	tarefa_F.periodicidade = 10;
	// inicializa prioridades
	tarefa_A.prioridade = low;
	tarefa_B.prioridade = normal;
	tarefa_C.prioridade = high;
	tarefa_D.prioridade = normal;
	tarefa_E.prioridade = high;
	tarefa_F.prioridade = realtime;
	// inicializa tempo de relaxamento
	tarefa_A.tempo_relaxamento = 0;
	tarefa_B.tempo_relaxamento = 0;
	tarefa_C.tempo_relaxamento = 0;
	tarefa_D.tempo_relaxamento = 0;
	tarefa_E.tempo_relaxamento = 0;
	tarefa_F.tempo_relaxamento = 0;
	// inicializa estado
	tarefa_A.estado = ready;
	tarefa_B.estado = ready;
	tarefa_C.estado = ready;
	tarefa_D.estado = ready;
	tarefa_E.estado = ready;
	tarefa_F.estado = ready;
	// inicializa percentual de duracao
	tarefa_A.percentual_exec = 0.0;
	tarefa_B.percentual_exec = 0.0;
	tarefa_C.percentual_exec = 0.0;
	tarefa_D.percentual_exec = 0.0;
	tarefa_E.percentual_exec = 0.0;
	tarefa_F.percentual_exec = 0.0;
	// inicializa atraso
	tarefa_A.atraso = 0;
	tarefa_B.atraso = 0;
	tarefa_C.atraso = 0;
	tarefa_D.atraso = 0;
	tarefa_E.atraso = 0;
	tarefa_F.atraso = 0;
	// inicializa proxima tarefa
	tarefa_A.proxima_tarefa = NULL;
	tarefa_B.proxima_tarefa = NULL;
	tarefa_C.proxima_tarefa = NULL;
	tarefa_D.proxima_tarefa = NULL;
	tarefa_E.proxima_tarefa = NULL;
	tarefa_F.proxima_tarefa = NULL;
	// inicializa proxima tarefa
	tarefa_A.deadline = DEADLINE_A;
	tarefa_B.deadline = DEADLINE_B;
	tarefa_C.deadline = DEADLINE_C;
	tarefa_D.deadline = DEADLINE_D;
	tarefa_E.deadline = DEADLINE_E;
	tarefa_F.deadline = DEADLINE_F;
	// inicializa id tarefa
	tarefa_A.id = thread_A_ID;
	tarefa_B.id = thread_B_ID;
	tarefa_C.id = thread_C_ID;
	tarefa_D.id = thread_D_ID;
	tarefa_E.id = thread_E_ID;
	tarefa_F.id = thread_F_ID;
	// inicializa fila de prontas
	prontas[0] = &tarefa_A;
	prontas[1] = &tarefa_B;
	prontas[2] = &tarefa_C;
	prontas[3] = &tarefa_D;
	prontas[4] = &tarefa_E;
	prontas[5] = &tarefa_F;
	prontas[6] = NULL;
	// criacao do Mail
	mail_tarefa_ID = osMailCreate(osMailQ(mail_tarefa), NULL);
	osSignalWait(0x3f,osWaitForever);
	while(1){
		
		//coloca o escalonador para dormir
		osSignalWait(0x01,TIMER_TICK);
		if(tarefa_running)osThreadSetPriority(tarefa_running->id,osPriorityNormal);
		evt = osMailGet(mail_tarefa_ID,0);
		if(evt.status == osEventMail){
			info_tarefa_atual = (info_tarefas*)evt.value.p;
			
			if(tarefa_running->id == thread_A_ID){
					printDisplay("Tarefa A",0,8);
				}
				else if(tarefa_running->id == thread_B_ID){
					printDisplay("Tarefa B",0,8);
				}
				else if(tarefa_running->id == thread_C_ID){
					printDisplay("Tarefa C",0,8);
				}
				else if(tarefa_running->id == thread_D_ID){
					printDisplay("Tarefa D",0,8);
				}
				else if(tarefa_running->id == thread_E_ID){
					printDisplay("Tarefa E",0,8);
				}
				else if(tarefa_running->id == thread_F_ID){
					printDisplay("Tarefa F",0,8);
				}
				
				// calcula tempo atraso
				tarefa_running->atraso = (info_tarefa_atual->duracao) - tarefa_running->deadline;
				intToString(tarefa_running->atraso,str_s,50,10,3);
				printDisplay(str_s,5,8);
				
			if(info_tarefa_atual->fault == true){
				if(tarefa_running->id == thread_F_ID){
					//PARA TUDO
					printDisplay("   MASTER FAULT   ",9,0);
					printDisplay("Calc.atual: ",10,0);
				floatToString(info_tarefa_atual->resultado,str_s,50,10,3,5);
				printDisplay(str_s,10,12);
				printDisplay("tempo: ",11,0);
				intToString(info_tarefa_atual->duracao,str_s,50,10,3);
				printDisplay(str_s,11,7);
				printDisplay("sequencia: ",12,0);
				intToString(info_tarefa_atual->sequencia,str_s,50,10,3);
				printDisplay(str_s,12,11);
				
					while(1);
				}
				// atualiza prioridade
				tarefa_running->prioridade--;
				printDisplay("calc.atual: ",10,0);
				floatToString(info_tarefa_atual->resultado,str_s,50,10,3,5);
				printDisplay(str_s,10,12);
				printDisplay("tempo: ",11,0);
				intToString(info_tarefa_atual->duracao,str_s,50,10,3);
				printDisplay(str_s,11,7);
				printDisplay("sequencia: ",12,0);
				intToString(info_tarefa_atual->sequencia,str_s,50,10,3);
				printDisplay(str_s,12,11);
				printDisplay("   SECONDARY FAULT   ",9,0);
				
				
			}
			else if(info_tarefa_atual->duracao < tarefa_running->deadline/2.0){
					if(tarefa_running->id == thread_F_ID){
						printDisplay("   MASTER FAULT   ",9,0);
						printDisplay("calc.atual: ",10,0);
				floatToString(info_tarefa_atual->resultado,str_s,50,10,3,5);
				printDisplay(str_s,10,12);
				printDisplay("tempo: ",11,0);
				intToString(info_tarefa_atual->duracao,str_s,50,10,3);
				printDisplay(str_s,11,7);
				printDisplay("sequencia: ",12,0);
				intToString(info_tarefa_atual->sequencia,str_s,50,10,3);
				printDisplay(str_s,12,11);
						while(1);
					}
					// atualiza prioridade
					tarefa_running->prioridade++;
					printDisplay("   SECONDARY FAULT   ",9,0);
					printDisplay("calc.atual: ",10,0);
				floatToString(info_tarefa_atual->resultado,str_s,50,10,3,5);
				printDisplay(str_s,10,12);
				printDisplay("tempo: ",11,0);
				intToString(info_tarefa_atual->duracao,str_s,50,10,3);
				printDisplay(str_s,11,7);
				printDisplay("sequencia: ",12,0);
				intToString(info_tarefa_atual->sequencia,str_s,50,10,3);
				printDisplay(str_s,12,11);
					
			}
			
			// seta estado e ajeita a fila
			if(tarefa_running->periodicidade == 0){
				if(tarefa_running->id == thread_A_ID){
					tarefa_running->periodicidade = PERIODICIDADE_A;
				}
				else if(tarefa_running->id == thread_B_ID){
					tarefa_running->periodicidade = PERIODICIDADE_B;
				}
				else if(tarefa_running->id == thread_C_ID){
					tarefa_running->periodicidade = PERIODICIDADE_C;
				}
				else if(tarefa_running->id == thread_D_ID){
					tarefa_running->periodicidade = PERIODICIDADE_D;
				}
				else if(tarefa_running->id == thread_E_ID){
					tarefa_running->periodicidade = PERIODICIDADE_E;
				}
				else if(tarefa_running->id == thread_F_ID){
					tarefa_running->periodicidade = PERIODICIDADE_F;
				}
				tarefa_running->estado = waiting;
				for(i = 0; i < TOTAL_THREADS && espera[i] != NULL; i++);
				espera[i] = tarefa_running;
				espera[i]->estado = waiting;
			}
			else{
				//coloca na lista de prontas
				for(i = 0; i < TOTAL_THREADS && prontas[i] != NULL; i++);
				if (tarefa_running) prontas[i] = tarefa_running;
				if (prontas[i]) prontas[i]->estado = ready;
			}
		
		}
		else{
			for(i = 0; i < TOTAL_THREADS && prontas[i] != NULL; i++);
			if (tarefa_running)prontas[i] = tarefa_running;
			if (prontas[i]) prontas[i]->estado = ready;
		}
		if(tarefa_running){
			intToString(tarefa_running->prioridade,str_s,50,10,3);
			printDisplay(str_s,1,12);
			// atualiza periodicidade
			tarefa_running->periodicidade--;
			// calcula tempo de relaxamento
			tarefa_running->tempo_relaxamento = tarefa_running->deadline - (info_tarefa_atual->duracao);
			intToString(tarefa_running->tempo_relaxamento,str_s,50,10,3);
			printDisplay(str_s,2,12);
			// calcula percentual de execucao (baseado na sequencia)
			tarefa_running->percentual_exec = (double)info_tarefa_atual->sequencia/(double)info_tarefa_atual->max_sequencia;
			floatToString(tarefa_running->percentual_exec,str_s,50,10,3,2);
			printDisplay(str_s,4,14);
			printDisplay("Ready  ",3,8);
		}
		
		// varrer lista de espera depois de um segundo e coloc�-las na lista de prontas
		if((osKernelSysTick() - conta_um_segundo) > UM_SEGUNDO){
			conta_um_segundo = osKernelSysTick();
			for(i = 0; i < TOTAL_THREADS; i++){
				if(espera[i] != NULL){
					for(j = 0; j < TOTAL_THREADS && prontas[j] != NULL; j++);
					prontas[j] = espera[i];
					if (prontas[j]) prontas[j]->estado = ready;
				}
			}
		}
		
		// varrer lista de prontas, ver qual tem maior prioridade
		for(i = 0; i < TOTAL_THREADS && prontas[i] == NULL; i++); // acha um que n�o � null
		tarefa_running = prontas[i];
		j=i;
		for(i = 0; i < TOTAL_THREADS; i++){
			if(prontas[i] != NULL ){
				if(prontas[i]->prioridade == tarefa_running->prioridade){
					if(prontas[i]->periodicidade > tarefa_running->periodicidade){
						tarefa_running = prontas[i];
						j=i;
					}
				}
				else if(prontas[i]->prioridade < tarefa_running->prioridade){
					tarefa_running = prontas[i];
					j=i;
				}
			}
		}
		prontas[j] = NULL;
		
		for(i = 0; i < TOTAL_THREADS && prontas[i] == NULL; i++); // acha um que n�o � null
		prox_tarefa = prontas[i];
		for(i = 0; i < TOTAL_THREADS; i++){
			if(prontas[i] != NULL ){
				if(prontas[i]->prioridade == prox_tarefa->prioridade){
					if(prontas[i]->periodicidade > prox_tarefa->periodicidade){
						prox_tarefa = prontas[i];
					}
				}
				else if(prontas[i]->prioridade < prox_tarefa->prioridade){
					prox_tarefa = prontas[i];
				}
			}
		}
		
		if(prox_tarefa->id == thread_A_ID){
			printDisplay("Tarefa A",6,9);
		}
		else if(prox_tarefa->id == thread_B_ID){
			printDisplay("Tarefa B",6,9);
		}
		else if(prox_tarefa->id == thread_C_ID){
			printDisplay("Tarefa C",6,9);
		}
		else if(prox_tarefa->id == thread_D_ID){
			printDisplay("Tarefa D",6,9);
		}
		else if(prox_tarefa->id == thread_E_ID){
			printDisplay("Tarefa E",6,9);
		}
		else if(prox_tarefa->id == thread_F_ID){
			printDisplay("Tarefa F",6,9);
		}
	
		printDisplay("Running ",3,8);
		
		printDisplay("Tarefa: ",0,0);
		printDisplay("Prioridade: ",1,0);
		printDisplay("Temp.relax: ",2,0);
		printDisplay("Estado: ",3,0);
		printDisplay("Percent.exec: ",4,0);
		printDisplay("Atraso: ",5,0);
		printDisplay("Proximo: ",6,0);
		printDisplay("Faltas: ",7,0);
		// aumenta a prioridade da um signal para a que foi escolhida
		if(info_tarefa_atual)osMailFree(mail_tarefa_ID,info_tarefa_atual);
		osThreadSetPriority(tarefa_running->id,osPriorityHigh);
		osSignalSet((tarefa_running->id),0x01);
	}
}

void thread_A(void const *argument){
	
	int i;
	double x;
	uint64_t tick_init = 0;
	uint64_t tick_end = 0;
	uint64_t duracao = 0;
	char tick_init_char [32];
	char tick_end_char [32];
	info_tarefas* tarefaA_info;
	
	osSignalSet(escalonador_ID,0x01);
	
	while(1){
		osSignalWait(0x01,osWaitForever);
		tarefaA_info = (info_tarefas*)osMailCAlloc(mail_tarefa_ID,osWaitForever);
		x = 0.0;
		tick_init = osKernelSysTick();
		tarefaA_info->fault = false;
		for(i = 0; i <= 256; i++){
			x += (double)i + (i + 2);
			tick_end = osKernelSysTick();
			duracao = tick_end - tick_init;
			tarefaA_info->duracao = duracao;
			tarefaA_info->sequencia = i;
			tarefaA_info->resultado = x;
			if(duracao > DEADLINE_A){
				tarefaA_info->fault = true;
			}
		}
		tarefaA_info->max_sequencia = 256;
		osMailPut(mail_tarefa_ID,(info_tarefas*)tarefaA_info);
	}
}

void thread_B(void const *argument){
	
	int i;
	double dois_a_n;
	double x;
	uint64_t tick_init = 0;
	uint64_t tick_end = 0;
	uint64_t duracao = 0;
	char tick_init_char [32];
	char tick_end_char [32];
	info_tarefas *tarefaB_info;
	
	osSignalSet(escalonador_ID,0x02);
	while(1){
		
		osSignalWait(0x01,osWaitForever);
		tarefaB_info = (info_tarefas*)osMailCAlloc(mail_tarefa_ID,osWaitForever);
		dois_a_n = 1.0;
		x = 0.0;
		tick_init = osKernelSysTick();
		tarefaB_info->fault = false;
		for(i = 1; i <= 16; i++){
			dois_a_n = dois_a_n * 2;
			x += dois_a_n/fatorial(i);
			tick_end = osKernelSysTick();
			duracao = tick_end - tick_init;
			tarefaB_info->duracao = duracao;
			tarefaB_info->sequencia = i;
			tarefaB_info->resultado = x;
			if(duracao > DEADLINE_B){
				tarefaB_info->fault = true;
			}
		}
//		tick_end = osKernelSysTick();
//		tick_end = tick_end - tick_init;
//		intToString(tick_end,tick_end_char,32,10,0);
//		UARTprintString("tick_end: ");
//		UARTprintString(tick_end_char);
//		UARTprintString("\r\n");
		tarefaB_info->max_sequencia = 16;
		osMailPut(mail_tarefa_ID,(info_tarefas*)tarefaB_info);
	}
}

void thread_C(void const *argument){
	
	int i;
	double x;
	uint64_t tick_init = 0;
	uint64_t tick_end = 0;
	uint64_t duracao = 0;
	char tick_init_char [32];
	char tick_end_char [32];
	info_tarefas *tarefaC_info;
	
	osSignalSet(escalonador_ID,0x04);
	while(1){
		
		osSignalWait(0x01,osWaitForever);
		tarefaC_info = (info_tarefas*)osMailCAlloc(mail_tarefa_ID,osWaitForever);
		x = 0.0;
		tick_init = osKernelSysTick();
		tarefaC_info->fault = false;
		for(i = 1; i <= 72; i++){
			x += (i + 1) / (double)i;
			tick_end = osKernelSysTick();
			duracao = tick_end - tick_init;
			tarefaC_info->duracao = duracao;
			tarefaC_info->sequencia = i;
			tarefaC_info->resultado = x;
			if(duracao > DEADLINE_C){
				tarefaC_info->fault = true;
			}
		}
//		tick_end = osKernelSysTick();
//		tick_end = tick_end - tick_init;
//		intToString(tick_end,tick_end_char,32,10,0);
//		UARTprintString("tick_end: ");
//		UARTprintString(tick_end_char);
//		UARTprintString("\r\n");
		tarefaC_info->max_sequencia = 72;
		osMailPut(mail_tarefa_ID,(info_tarefas*)tarefaC_info);
	}
}

void thread_D(void const *argument){
	
	int i = 1;
	double x;
	uint64_t tick_init = 0;
	uint64_t tick_end = 0;
	uint64_t duracao = 0;
	char tick_init_char [32];
	char tick_end_char [32];
	info_tarefas *tarefaD_info;
	
	osSignalSet(escalonador_ID,0x08);
	while(1){
		
		osSignalWait(0x01,osWaitForever);
		tarefaD_info = (info_tarefas*)osMailCAlloc(mail_tarefa_ID,osWaitForever);
		x = 0.0;
		tick_init = osKernelSysTick();
		tarefaD_info->fault = false;
		x = 1.0 + (5.0/(fatorial(3))) + (5.0/(fatorial(5))) + (5.0/(fatorial(7))) + (5.0/(fatorial(9)));
		tick_end = osKernelSysTick();
		duracao = tick_end - tick_init;
		tarefaD_info->duracao = duracao;
		tarefaD_info->sequencia = i;
		tarefaD_info->resultado = x;
		if(duracao > DEADLINE_D){
			tarefaD_info->fault = true;
		}
//		intToString(duracao,tick_end_char,32,10,0);
//		UARTprintString("tick_end: ");
//		UARTprintString(tick_end_char);
//		UARTprintString("\r\n");
//		tarefaD_info->max_sequencia = 1;
		osMailPut(mail_tarefa_ID,(info_tarefas*)tarefaD_info);
	}
}

void thread_E(void const *argument){
	
	int i;
	double x;
	uint64_t tick_init = 0;
	uint64_t tick_end = 0;
	uint64_t duracao = 0;
	char tick_init_char [32];
	char tick_end_char [32];
	info_tarefas *tarefaE_info;
	
	osSignalSet(escalonador_ID,0x10);
	while(1){
		
		osSignalWait(0x01,osWaitForever);
		tarefaE_info = (info_tarefas*)osMailCAlloc(mail_tarefa_ID,osWaitForever);
		x = 0.0;
		tick_init = osKernelSysTick();
		tarefaE_info->fault = false;
		for(i = 1; i <= 100; i++){
			x += i * PI * PI;
			tick_end = osKernelSysTick();
			duracao = tick_end - tick_init;
			tarefaE_info->sequencia = i;
			tarefaE_info->resultado = x;
			tarefaE_info->duracao = duracao;
			if(duracao > DEADLINE_E){
				tarefaE_info->fault = true;
			}
		}
//		intToString(duracao,tick_end_char,32,10,0);
//		UARTprintString("tick_end: ");
//		UARTprintString(tick_end_char);
//		UARTprintString("\r\n");
		tarefaE_info->max_sequencia = 100;
		osMailPut(mail_tarefa_ID,(info_tarefas*)tarefaE_info);
	}
}

void thread_F(void const *argument){
	
	int i;
	double dois_a_n;
	double x;
	uint64_t tick_init = 0;
	uint64_t tick_end = 0;
	uint64_t duracao = 0;
	char tick_init_char [32];
	char tick_end_char [32];
	info_tarefas *tarefaF_info;

	osSignalSet(escalonador_ID,0x20);
	while(1){
		
		osSignalWait(0x01,osWaitForever);
		tarefaF_info = (info_tarefas*)osMailCAlloc(mail_tarefa_ID,osWaitForever);
		dois_a_n = 1.0;
		x = 0.0;
		tick_init = osKernelSysTick();
		tarefaF_info->fault = false;
		for(i = 1; i <= 128; i++){
			dois_a_n = dois_a_n * 2.0;
			x += i * i * i / dois_a_n;
			tick_end = osKernelSysTick();
			duracao = tick_end - tick_init;
			tarefaF_info->duracao = duracao;
			tarefaF_info->sequencia = i;
			tarefaF_info->resultado = x;
			if(duracao > DEADLINE_F){
				tarefaF_info->fault = true;
			}
		}
//		intToString(duracao,tick_end_char,32,10,0);
//		UARTprintString("tick_end: ");
//		UARTprintString(tick_end_char);
//		UARTprintString("\r\n");
		tarefaF_info->max_sequencia = 128;
		osMailPut(mail_tarefa_ID,(info_tarefas*)tarefaF_info);
	}
}

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void) {
	init_all();
	osKernelInitialize();
	
	if(Init_Thread()!= 0)
		return 0;
		
	osKernelStart(); 
	osDelay(osWaitForever);
}