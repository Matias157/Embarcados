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

#include "fast_eddie.h"

void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);


//***************** Variáveis globais  ***************************************
uint32_t movimento_eddie;
uint32_t x_anterior_eddie, x_atual_eddie;
uint32_t y_anterior_eddie, y_atual_eddie;
uint32_t posicao_anterior_chefao, posicao_atual_chefao;
bool volta_chefao = false;
bool som_pulo = false;
bool som_subir_escada = false;
bool som_fim_game = false;
bool som_morte = false;
bool som_item = false;
bool som_proxima_fase = false;
bool reinicia_eddie = false;

bool pos_eddie_direita = false;
uint32_t andar_atual_eddie;
uint32_t posicaoEscadas[2][4];
uint32_t pontos= 0;
uint32_t quantidade_pontos = 0;
uint16_t frame_eddie = 0; //controla as animacaoes do eddie - 0 parado, 1 e 2 andar, 3 e 4 pular
uint16_t frame_chefao = 0; //controla as animacoes do chefao - 0 e 1 andar
uint16_t frame_inimigo = 0; //controla as animacoes do inimigo - 0 e 1 andar
uint16_t frame_item = 0; //controla as animacoes do item - 0 amarelo, 1 vermelho, 2 azul
uint16_t frame_escada = 0; //controla as animacoes da escada - 0 nada, 1 e 2 eddie subindo ou descendo
bool pulo = 0; // se for 1 pula
bool subida_ou_descida; // subida = 1, descida = 0
bool flag_fim_de_jogo = false;
uint32_t vidas = 3;
bool start = false;
bool chefinho = false;

//Criacao das threads
/*
***  O CMSIS veio configurado com um limite de 8 threads que podem ser executadas em paralelo,
***  eu mudei essa configuracao para 20 threads. Se precisar de mais eh so abrir o arquivo RTX_Conf_CM.c
***  (dentro da aba CMSIS) e aumentar o numero do #define OS_TASKCNT 
*/
void startGame(void const *argument);
void interacaoJoystick(void const *argument);
void interacaoBotao(void const *argument);
void movimentaEddie(void const *argument);
void movimentaInimigo(void const *argument);
void movimentaChefao(void const *argument);
void movimentaItem(void const *argument);
void interacaoPersonagens(void const *argument);
void painelPontuacao(void const *argument);
void painelVida(void const *argument);

//Varivel que determina ID das threads
osThreadId startGame_ID;
osThreadId interacaoJoystick_ID; 
osThreadId interacaoBotao_ID; 
osThreadId movimentaEddie_ID;
osThreadId movimentaInimigo0_ID;
osThreadId movimentaInimigo1_ID;
osThreadId movimentaInimigo2_ID;
osThreadId movimentaInimigo3_ID;
osThreadId movimentaChefao_ID;
osThreadId movimentaItem0_ID;
osThreadId movimentaItem1_ID;
osThreadId interacaoPersonagens_ID;
osThreadId painelPontuacao_ID;
osThreadId painelVida_ID;

//Definicao das threads
osThreadDef (startGame, osPriorityNormal, 1, 0);     // thread object
osThreadDef (interacaoJoystick, osPriorityNormal, 1, 0);     // thread object
osThreadDef (interacaoBotao, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaEddie, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaInimigo, osPriorityNormal, 4, 0);     // thread object
osThreadDef (movimentaChefao, osPriorityNormal, 1, 0);     // thread object
osThreadDef (movimentaItem, osPriorityNormal, 2, 0);     // thread object
osThreadDef (interacaoPersonagens, osPriorityNormal, 1, 0);     // thread object
osThreadDef (painelPontuacao, osPriorityNormal, 1, 0);     // thread object
osThreadDef (painelVida, osPriorityNormal, 1, 0);     // thread object

int Init_Thread (void) {
	startGame_ID = osThreadCreate (osThread(startGame), NULL);
	if (!startGame_ID) return(-1);
	
	interacaoJoystick_ID = osThreadCreate (osThread(interacaoJoystick), NULL);
	if (!interacaoJoystick_ID) return(-1);
	
	interacaoBotao_ID = osThreadCreate (osThread(interacaoBotao), NULL);
	if (!interacaoBotao_ID) return(-1);
	
	movimentaEddie_ID = osThreadCreate (osThread(movimentaEddie), NULL); 
	if (!movimentaEddie_ID) return(-1);
	
	movimentaInimigo0_ID = osThreadCreate (osThread(movimentaInimigo), (void*)ANDAR_INIMIGO0); 
	if (!movimentaInimigo0_ID) return(-1);
	
	movimentaInimigo1_ID = osThreadCreate (osThread(movimentaInimigo), (void*)ANDAR_INIMIGO1); 
	if (!movimentaInimigo1_ID) return(-1);
	
	movimentaInimigo2_ID = osThreadCreate (osThread(movimentaInimigo), (void*)ANDAR_INIMIGO2); 
	if (!movimentaInimigo2_ID) return(-1);
	
	movimentaInimigo3_ID = osThreadCreate (osThread(movimentaInimigo), (void*)ANDAR_INIMIGO3); 
	if (!movimentaInimigo3_ID) return(-1);
	
	movimentaChefao_ID = osThreadCreate (osThread(movimentaChefao), NULL); 
	if (!movimentaChefao_ID) return(-1);
	
	movimentaItem0_ID = osThreadCreate (osThread(movimentaItem), (void*)3); 
	if (!movimentaItem0_ID) return(-1);

	movimentaItem1_ID = osThreadCreate (osThread(movimentaItem), (void*)2); 
	if (!movimentaItem1_ID) return(-1);
	
	interacaoPersonagens_ID = osThreadCreate (osThread(interacaoPersonagens), NULL); 
	if (!interacaoPersonagens_ID) return(-1);
	
	painelPontuacao_ID = osThreadCreate (osThread(painelPontuacao), NULL); 
	if (!painelPontuacao_ID) return(-1);
	
	painelVida_ID = osThreadCreate (osThread(painelVida), NULL); 
	if (!painelVida_ID) return(-1);
	
  return(0);
}



/*----------------------------------------------------------------------------
 *    Initializations
 *---------------------------------------------------------------------------*/

void init_all(){
	cfaf128x128x16Init();
	joy_init();
	buzzer_init(); 
	button_init();
	init_mutex();
}

void interacaoPersonagens(void const *argument){
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		
		if(som_item == true){
			somItem();
			som_item = false;
			som_pulo = false;
		}
		else if(som_pulo == true){
			somPulo();
			som_pulo = false;
		}
		else if(som_subir_escada == true){
			somSubirEscada();
			som_subir_escada = false;
		}
		else if(som_fim_game == true){
			somFimGame();
			som_fim_game = false;
		}
		else if(som_morte == true){
			somMorte();
			som_morte = false;
		}
		else if(som_proxima_fase == true){
			somProximaFase();
			som_proxima_fase = false;
		}
	}
}

void interacaoJoystick(void const *argument){
	// início seção crítica
	
	while(1){
		int x, y;
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		if(pulo == 0){
			x = joy_read_x();
			y = joy_read_y();
			x = x*200/0xFFF-100; 
			y = y*200/0xFFF-100;
			if (x<-50 && y>-50 && y<50){
				movimento_eddie = 1;						//esquerda
				 
				if(frame_eddie == 0){           //se estiver parado muda a animacao
					frame_eddie = 1;
				}
		 
			}
			else if (x>50 && y>-50 && y<50){
				movimento_eddie = 2;						//direita
				 
				if(frame_eddie == 0){           //se estiver parado muda a animacao
					frame_eddie = 1;
				}
		 
			}
			else if (y<-50 && x>-50 && x<50){
				movimento_eddie = 3;						//baixo
				 
				if(frame_eddie == 0){           //se estiver parado muda a animacao
					frame_eddie = 1;
				}
		 
			}
			else if (y>50 && x>-50 && x<50){
				movimento_eddie = 4;						//cima
				 
				if(frame_eddie == 0){           //se estiver parado muda a animacao
					frame_eddie = 1;
				}
		 
			}
			else{
				movimento_eddie = 0;
			}
		}
	osSignalSet(movimentaEddie_ID,0x01);
	osSignalSet(movimentaInimigo0_ID,0x01);
	osSignalSet(movimentaInimigo1_ID,0x01);
	osSignalSet(movimentaInimigo2_ID,0x01);
	osSignalSet(movimentaInimigo3_ID,0x01);
	osSignalSet(movimentaChefao_ID,0x01);
	osSignalSet(movimentaItem0_ID,0x01);
	osSignalSet(movimentaItem1_ID,0x01);
	}
	// fim seção crítica
}

void interacaoBotao(void const *argument){
	// início seção crítica
	while(1){
		bool s1_press;
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		s1_press = button_read_s1();
		
		if (s1_press){
			start = true;
			if(pulo == 0){ // verificar se isso será necessário com a implementação do mutex
				pulo = 1;		
				subida_ou_descida = 1;
				som_pulo = true;
			}
		}
	}
	// fim seção crítica
}

void movimentaEddie(void const *argument){
	int sobe_escada = 0, i;
	// início seção crítica
	while(1){
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		
		if(reinicia_eddie){
			apagaEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
			andar_atual_eddie = 5;
			x_atual_eddie = X_INICIAL_EDDIE;
			reinicia_eddie = false;
		}
		
		if(pulo == 1 && subida_ou_descida == 1){
			if(som_pulo == true){
				osSignalSet(interacaoPersonagens_ID,0x01);
			}
			if(y_atual_eddie < 6){
				y_atual_eddie = y_anterior_eddie + 1;
				 
				frame_eddie = 3; //animacao de subir
		 
			}
			else{
				y_atual_eddie = y_anterior_eddie - 1;
				subida_ou_descida = 0;
				 
				frame_eddie = 4; //animacao de descer
		 
			}
		}
		else if(pulo == 1 && subida_ou_descida == 0){
			if(y_atual_eddie > 0){
				y_atual_eddie = y_anterior_eddie - 1;
				 
				frame_eddie = 3; //animacao de subir
		 
			}
			else{
				pulo = 0;
				y_atual_eddie = 0;
				 
				frame_eddie = 1; //quando alcanca o chao muda a animacao
		 
			}
		}
		else{
			y_atual_eddie = 0;
		}
		if(movimento_eddie == 2 && x_atual_eddie < 120){
			x_atual_eddie = x_anterior_eddie + 1;
			 
		}
		else if(movimento_eddie == 1 && x_atual_eddie > 0){
			x_atual_eddie = x_anterior_eddie -1;
			 
			//apagaEddie(andar_atual_eddie, x_anterior_eddie, y_anterior_eddie); //se estiver parado apaga a animacao de parado
			 
		}
		else if(movimento_eddie == 4){
			if((posicaoEscadas[1][andar_atual_eddie - 2] <= x_atual_eddie) &&
			(posicaoEscadas[1][andar_atual_eddie - 2] + LARGURA_ESCADA >= x_atual_eddie + LARGURA_EDDIE)){
				sobe_escada = 0;
				while(sobe_escada < 3){
					sobe_escada++;
					y_atual_eddie = y_atual_eddie + 6;
					imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
					apagaEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie - 6);
					som_subir_escada = true;
					osSignalSet(interacaoPersonagens_ID,0x01);
					for(i = 0; i < 1000000; i++);
				}
				imprimeBackground();
				y_atual_eddie = 0;
				andar_atual_eddie = andar_atual_eddie - 1;
			}
			else{
				frame_eddie = 0;
			}
		}
		else if(movimento_eddie == 3){
			if((posicaoEscadas[1][andar_atual_eddie - 1] <= x_atual_eddie) &&
			(posicaoEscadas[1][andar_atual_eddie - 1] + LARGURA_ESCADA >= x_atual_eddie + LARGURA_EDDIE)){
				sobe_escada = 0;
				while(sobe_escada < 3){
					sobe_escada++;
					y_atual_eddie = y_atual_eddie - 6;
					imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
					apagaEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie + 6);
					som_subir_escada = true;
					osSignalSet(interacaoPersonagens_ID,0x01);
					for(i = 0; i < 1000000; i++);
				}
				imprimeBackground();
				y_atual_eddie = 0;
				andar_atual_eddie = andar_atual_eddie + 1;
			}
			else{
				frame_eddie = 0;
			}
		}
		else{
			if(pulo == 0){
				 
				frame_eddie = 0; //imprime o eddie parado
				 
			}
		}
	
		if(andar_atual_eddie != 1)
			imprimeEscada(posicaoEscadas[0][andar_atual_eddie - 2], posicaoEscadas[1][andar_atual_eddie - 2]);
		imprimeEddie(andar_atual_eddie, x_atual_eddie, y_atual_eddie);
		
		x_anterior_eddie = x_atual_eddie;
		y_anterior_eddie = y_atual_eddie;
		
		osSignalSet(movimentaEddie_ID,0x01);
		osSignalSet(movimentaInimigo0_ID,0x01);
		osSignalSet(movimentaInimigo1_ID,0x01);
		osSignalSet(movimentaInimigo2_ID,0x01);
		osSignalSet(movimentaInimigo3_ID,0x01);
		osSignalSet(movimentaChefao_ID,0x01);
		osSignalSet(movimentaItem0_ID,0x01);
		osSignalSet(movimentaItem1_ID,0x01);
		
		while(!start){
			osSignalSet(interacaoBotao_ID,0x01);
		}
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
			posicao_atual_chefao -= 1;
		else
			posicao_atual_chefao += 1;
		
		if(quantidade_pontos >= 90){
			chefinho = true;
			apagaChefao(posicao_atual_chefao);
		}
		else{
			chefinho = false;
		}
		
		imprimeChefao(posicao_atual_chefao, chefinho);		
		posicao_anterior_chefao = posicao_atual_chefao;
		
		if(chefinho){
			if((1 == andar_atual_eddie) && (posicao_atual_chefao + 3 >= x_atual_eddie) && 
				(posicao_atual_chefao - 3 + LARGURA_CHEFAO <= x_atual_eddie + LARGURA_EDDIE) && (y_atual_eddie == 0)){
					reinicia_eddie = true;
					posicao_anterior_chefao = rand()%110;
					vidas--;
					if(vidas == 0){
						start = false;
						vidas = 3;
						pontos = 0;
						quantidade_pontos = 0;
						som_fim_game = true;
						osSignalSet(painelPontuacao_ID,0x01);
						osSignalSet(interacaoPersonagens_ID,0x01);
					}
					else{
				som_morte = true;
				osSignalSet(interacaoPersonagens_ID,0x01);
				}
					osSignalSet(painelVida_ID,0x01);
					
			}
			else if((1 == andar_atual_eddie) && (posicao_atual_chefao + 3 >= x_atual_eddie) && 
			(posicao_atual_chefao - 3 + LARGURA_ITEM <= x_atual_eddie + LARGURA_EDDIE) && (y_atual_eddie >= 1)){
				apagaChefao(posicao_atual_chefao);
				reinicia_eddie = true;
				posicao_anterior_chefao = rand()%110;
				start = false;
				vidas = 3;
				pontos+=310;
				quantidade_pontos = 10;
				som_proxima_fase = true;
				osSignalSet(interacaoPersonagens_ID,0x01);
				osSignalSet(painelPontuacao_ID,0x01);
			}
		}
		else{
			if((1 == andar_atual_eddie) && (posicao_atual_chefao + 3 >= x_atual_eddie) && 
			(posicao_atual_chefao - 3 + LARGURA_CHEFAO <= x_atual_eddie + LARGURA_EDDIE)){
				reinicia_eddie = true;
				apagaChefao(posicao_atual_chefao);
				posicao_atual_chefao = rand()%110;
				vidas--;
				if(vidas == -1){
					start = false;
					vidas = 3;
					pontos = 0;
					quantidade_pontos = 0;
					som_fim_game = true;
					osSignalSet(painelPontuacao_ID,0x01);
					osSignalSet(interacaoPersonagens_ID,0x01);
				}
				else{
				som_morte = true;
				osSignalSet(interacaoPersonagens_ID,0x01);
				}
				osSignalSet(painelVida_ID,0x01);
			}
			else{	
				if((1 == andar_atual_eddie) && (posicao_atual_chefao + 3 >= x_atual_eddie) && 
					(posicao_atual_chefao - 3 + LARGURA_ITEM <= x_atual_eddie + LARGURA_EDDIE) && (y_atual_eddie >= 4)){
					start = false;
				}
				
			}
		}
		
		while(!start){
			osSignalSet(interacaoBotao_ID,0x01);
		}
	}
}

void movimentaInimigo(void const *argument){
	uint32_t andar_inimigo = (int) argument;
	uint32_t posicao_anterior_inimigo = rand()%110;
	uint32_t posicao_atual_inimigo = posicao_anterior_inimigo;
	bool volta_inimigo = false;
	while(1){
		if(!start){
			posicao_anterior_inimigo = rand()%110;
			posicao_atual_inimigo = posicao_anterior_inimigo;
		}
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
		if(posicao_atual_inimigo >= 120) //alcancou o maximo da tela
			volta_inimigo = true; //muda flag
		if(posicao_atual_inimigo <= 0) //alcancou o minimo da tela
			volta_inimigo = false; //muda flag
		if(volta_inimigo) //se estiver indo para a esquerda
			posicao_atual_inimigo -= 1;
		else
			posicao_atual_inimigo += 1;
		
		 
		//apagaInimigo(andar_inimigo, posicao_anterior_inimigo);
		imprimeEscada(posicaoEscadas[0][andar_inimigo - 2], posicaoEscadas[1][andar_inimigo - 2]);
		imprimeInimigo(andar_inimigo, posicao_atual_inimigo);
		 
		posicao_anterior_inimigo = posicao_atual_inimigo;
		
		if((andar_inimigo == andar_atual_eddie) && (posicao_atual_inimigo + 3 >= x_atual_eddie) && 
			(posicao_atual_inimigo - 3 + LARGURA_INIMIGO <= x_atual_eddie + LARGURA_EDDIE) && (y_atual_eddie == 0)){
				
				reinicia_eddie = true;
				apagaInimigo(andar_inimigo, posicao_atual_inimigo);
				
				posicao_atual_inimigo = rand()%110;
				vidas--;
				if(vidas == -1){
					start = false;
					vidas = 3;
					pontos = 0;
					quantidade_pontos = 0;
					som_fim_game = true;
					osSignalSet(painelPontuacao_ID,0x01);
					osSignalSet(interacaoPersonagens_ID,0x01);
				}
				else{
				som_morte = true;
				osSignalSet(interacaoPersonagens_ID,0x01);
				}
				osSignalSet(painelVida_ID,0x01);
		}
			while(!start){
			osSignalSet(interacaoBotao_ID,0x01);
		}
	}
}

void movimentaItem(void const *argument){
	float posicao_anterior_item = rand()%110;
	float posicao_atual_item = posicao_anterior_item;
	uint32_t andar_atual_item = rand()%4+2;
	float sentido = 0.5*(rand()%2?1:-1);
	while(1){
		if(!start){
			posicao_anterior_item = rand()%110;
			posicao_atual_item = posicao_anterior_item;
			andar_atual_item = rand()%4+2;
			sentido = 0.5*(rand()%2?1:-1);
		}
		osSignalWait(0x01,osWaitForever); //espera o signal para continuar
	
		posicao_atual_item += sentido; //continua indo para direira

		if(posicao_atual_item > 120){ //alcancou o maximo da tela
			apagaItem(andar_atual_item, posicao_anterior_item);
			posicao_atual_item = 1; //vai pata o outro lado da tela
		}
		if (posicao_atual_item <= 0){
			posicao_atual_item = 120;
			apagaItem(andar_atual_item, posicao_anterior_item);
		}
		
		 
	
		imprimeEscada(posicaoEscadas[0][andar_atual_item - 2], posicaoEscadas[1][andar_atual_item - 2]);
		imprimeItem(andar_atual_item, posicao_atual_item);
		
		if((andar_atual_item == andar_atual_eddie) && (posicao_atual_item + 3 >= x_atual_eddie) && 
			(posicao_atual_item - 3 + LARGURA_ITEM <= x_atual_eddie + LARGURA_EDDIE) && (y_atual_eddie >= 4)){
				apagaItem(andar_atual_item, posicao_atual_item);
				andar_atual_item = rand()%4+2;
				posicao_anterior_item = rand()%110;
				som_item = true;
				osSignalSet(interacaoPersonagens_ID,0x01);
				osSignalSet(painelPontuacao_ID,0x01);
		}
		
		posicao_anterior_item = posicao_atual_item;
		while(!start){
			osSignalSet(interacaoBotao_ID,0x01);
		}
	}
}

void painelPontuacao(void const *argument){
	inicializaPontuacao();
	while(1){
		osSignalWait(0x01, osWaitForever);
		pontos+=quantidade_pontos;
		quantidade_pontos+=10;
		 
		apagaPontuacao();
		imprimePontuacao(pontos);
		imprimeVidas(vidas);
			
	}
}	
void painelVida(void const *argument){
	while(1){
		osSignalWait(0x01, osWaitForever);
		imprimeVidas(vidas);
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
	
	osSignalSet(interacaoJoystick_ID,0x01);
	osSignalSet(interacaoBotao_ID,0x01);
		
}

//timers
osTimerId timer_frames_ID;
osTimerDef(frames,timer_frames);

void startGame(void const *argument){ //inicia o timer, imprime o background e as escadas e fica constantemente atualizando a pontuacao
	char pbufx[10], pbufy[10], pbufz[10];
	bool center;
	float temp,lux;
	float mic;
	bool s1_press, s2_press;
	uint32_t i, j, i_anterior = 0;
	
	//Initializing all peripherals
	init_all();

	timer_frames_ID = osTimerCreate(osTimer(frames),osTimerPeriodic,(void *)0); //cria o timer
	
	osTimerStart(timer_frames_ID, 100); //inicia o timer
	while(1){		
		// Inicialização das variáveis globais
		movimento_eddie = 0;
		x_anterior_eddie = X_INICIAL_EDDIE;
		x_atual_eddie = X_INICIAL_EDDIE;
		andar_atual_eddie = ANDAR_INICIAL_EDDIE;
		posicao_atual_chefao = POSICAO_INICIAL_CHEFAO;
		posicaoEscadas[0][3] = 5;
		posicaoEscadas[0][2] = 4;
		posicaoEscadas[0][1] = 3;
		posicaoEscadas[0][0] = 2;
		posicaoEscadas[1][3] = 30;
		posicaoEscadas[1][2] = 80;
		posicaoEscadas[1][1] = 60;
		posicaoEscadas[1][0] = 100;
		
		 
		clear();
		
		imprimeBackground();
		imprimeEscadas();
		osSignalSet(painelPontuacao_ID, 0x01);
		 
		
		while(1){
			osSignalWait(0x01,osWaitForever); //espera o signal para continuar

			//envia signal para ativar as threads
			osSignalSet(interacaoJoystick_ID,0x01);
			osSignalSet(interacaoBotao_ID,0x01);
		}
	}
}

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void){	
	srand(4000);
	
	if(Init_Thread()==0)
		return 0;
	
	osKernelStart(); 
	osDelay(osWaitForever);
}