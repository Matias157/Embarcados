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
#include "Chave.h"
#include <stdlib.h>

#define ALTURA_ANDAR			       20
#define ESPESSURA_CHAO		       2
#define ESPACO_PONTUACAO	       13
#define QUANT_ANDARES			       5
#define LARGURA_TELA			       128
#define ALTURA_EDDIE			       14
#define LARGURA_EDDIE		 	       8
#define X_INICIAL_EDDIE		       64
#define Y_INICIAL_EDDIE		       0
#define ANDAR_INICIAL_EDDIE		   5
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
#define ANDAR_INICIAL_ITEM1      3
#define TOTAL_ESCADAS		         4
#define ALTURA_NUMEROS           8
#define LARGURA_NUMEROS          14




//***************** Variáveis globais  ***************************************
extern uint32_t movimento_eddie;
extern uint32_t x_anterior_eddie, x_atual_eddie;
extern uint32_t y_anterior_eddie, y_atual_eddie;
extern uint32_t posicao_anterior_chefao, posicao_atual_chefao;
extern bool volta_chefao;
extern uint32_t posicao_anterior_inimigo0, posicao_atual_inimigo0;
extern bool volta_inimigo0;
extern uint32_t posicao_anterior_inimigo1, posicao_atual_inimigo1;
extern bool volta_inimigo1;
extern uint32_t posicao_anterior_inimigo2, posicao_atual_inimigo2;
extern bool volta_inimigo2;
extern uint32_t posicao_anterior_inimigo3, posicao_atual_inimigo3;
extern bool volta_inimigo3;
extern uint32_t posicao_anterior_item0, posicao_atual_item0;
extern bool volta_item0;
extern uint32_t posicao_anterior_item1, posicao_atual_item1;
extern bool volta_item1;
extern bool pos_eddie_direita;
extern uint32_t andar_atual_eddie;
extern uint32_t andar_atual_item0;
extern uint32_t andar_atual_item1;
extern uint32_t posicaoEscadas[2][4];
extern uint16_t frame_eddie; //controla as animacaoes do eddie - 0 parado, 1 e 2 andar, 3 e 4 pular
extern uint16_t frame_chefao; //controla as animacoes do chefao - 0 e 1 andar
extern uint16_t frame_inimigo; //controla as animacoes do inimigo - 0 e 1 andar
extern uint16_t frame_item; //controla as animacoes do item - 0 amarelo, 1 vermelho, 2 azul
extern uint16_t frame_escada; //controla as animacoes da escada - 0 nada, 1 e 2 eddie subindo ou descendo
extern uint16_t frame_chave; //controla as animacoes da escada - 0 nada, 1 e 2 eddie subindo ou descendo
extern bool pulo; // se for 1 pula
extern bool subida_ou_descida; // subida = 1, descida = 0
extern bool flag_fim_de_jogo;

void draw_pixel(uint16_t x, uint16_t y);
	
void clear(void);

void imprimeBackground();

// andar mais alto é o 1, o mais baixo é o 5
// posicao é o tamanho da largura da tela menos a largura do eddie (0 a 120)
void imprimeEddie(uint32_t andar, uint32_t posicao_x, uint32_t posicao_y);

// andar mais alto é o 1, o mais baixo é o 5
// posicao é o tamanho da largura da tela menos a largura do eddie (0 a 120)
void apagaEddie(uint32_t andar, uint32_t posicao_x, uint32_t posicao_y);

void apagaEscada(uint32_t andar, uint32_t posicao);

void imprimeEscada(uint32_t andar, uint32_t posicao);

void imprimeEscadas();

void apagaChefao(uint32_t posicao);

void imprimeChefao(uint32_t posicao, bool chefinho);

void apagaInimigo(uint32_t andar, uint32_t posicao);

void imprimeInimigo(uint32_t andar, uint32_t posicao);

void apagaItem(uint32_t andar, uint32_t posicao);

void imprimeItem(uint32_t andar, uint32_t posicao);

void inicializaPontuacao();

void imprimePontuacao();

void apagaPontuacao();

void somPulo();

void somSubirEscada();

void somFimGame();

void somMorte();

void somItem();

void somProximaFase();

void imprimeVidas(uint32_t vidas);
void init_mutex();