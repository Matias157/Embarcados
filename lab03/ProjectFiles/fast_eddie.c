#include "fast_eddie.h"
osMutexDef(mutex_display);	
osMutexId(mutex_display_ID);

//To print on the screen
tContext sContext;
void init_mutex(){
	mutex_display_ID = osMutexCreate(osMutex(mutex_display));

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
	osMutexWait(mutex_display_ID,osWaitForever);
	
	GrContextForegroundSet(&sContext, ClrBlack);
	GrContextBackgroundSet(&sContext, ClrWhite);
	
	GrRectFill(&sContext, &tela);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
	osMutexRelease(mutex_display_ID);
}

void imprimeBackground(){
	uint32_t color = 0xF34EF1; //cor roxa
	uint32_t i, j, pixel, n;
	pixel = Background_start;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	for(n = 1; n <= QUANT_ANDARES; n++){		//imprime o chao de todos os andares
		for(j = 0; j < ESPESSURA_CHAO; j++){ 
			for(i = 0; i < LARGURA_TELA; i++){
				GrContextForegroundSet(&sContext, color);
				//deixa um espaço para pontuacao no topo da tela (ESPACO_PONTUACAO)
				draw_pixel(i, j+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*n);
			}
		}
	}
	osMutexRelease(mutex_display_ID);
}

// andar mais alto é o 1, o mais baixo é o 5
// posicao é o tamanho da largura da tela menos a largura do eddie (0 a 120)
void imprimeEddie(uint32_t andar, uint32_t posicao_x, uint32_t posicao_y){
	
	uint32_t color = 0x00, i, j, pixel;
	pixel = Eddie_start;
	osMutexWait(mutex_display_ID,osWaitForever);
	GrContextForegroundSet(&sContext, ClrBlack);
	GrLineDrawV(&sContext,posicao_x -1, -posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1-posicao_y+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	GrLineDrawV(&sContext,posicao_x -2, -posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1-posicao_y+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	GrLineDrawV(&sContext,posicao_x + LARGURA_EDDIE, -posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1-posicao_y+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	GrLineDrawV(&sContext,posicao_x + LARGURA_EDDIE+1, -posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1-posicao_y+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	
	if(pulo == 1 && subida_ou_descida == 1 && posicao_y > 1){
		//GrLineDrawH(&sContext,posicao_x-1,posicao_x+LARGURA_EDDIE+1,-2+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		GrLineDrawH(&sContext,posicao_x-1,posicao_x+LARGURA_EDDIE+1,-1+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		}
	else if(pulo == 1 && subida_ou_descida == 0){
		GrLineDrawH(&sContext,posicao_x-4,3+posicao_x+LARGURA_EDDIE,-1-y_atual_eddie-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);	
	}
	for(j = 0; j < ALTURA_EDDIE; j++){ //altura
		for(i = 0; i < LARGURA_EDDIE; i++){ //largura
			color = 0x00;
			color += Eddie[(pixel+(frame_eddie*ALTURA_EDDIE*LARGURA_EDDIE*3))+2]; //imprime um eddie diferente dependendo do frame
			color += Eddie[(pixel+(frame_eddie*ALTURA_EDDIE*LARGURA_EDDIE*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Eddie[(pixel+(frame_eddie*ALTURA_EDDIE*LARGURA_EDDIE*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color);
			}
			else
				GrContextForegroundSet(&sContext, ClrBlack);
			pixel += 3;
			if(x_anterior_eddie < x_atual_eddie){ //se estava andando para direita
					draw_pixel((-i)+posicao_x+LARGURA_EDDIE-1, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime espelhado
					pos_eddie_direita =  true; //flag para guardar a ultima posicao
				}
				else if(x_anterior_eddie > x_atual_eddie){ //se estava andando para a esquerda
					draw_pixel(i+posicao_x, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime normal
					pos_eddie_direita = false; //flag para guardar a ultima posicao
				}
				else
					if(pos_eddie_direita){ //se a ultima posicao era virado para direita
						draw_pixel((-i)+posicao_x+LARGURA_EDDIE-1, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime espelhado
					}
					else{
						draw_pixel(i+posicao_x, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime normal
					}
		}
	}	
	osMutexRelease(mutex_display_ID);
}

// andar mais alto é o 1, o mais baixo é o 5
// posicao é o tamanho da largura da tela menos a largura do eddie (0 a 120)
void apagaEddie(uint32_t andar, uint32_t posicao_x, uint32_t posicao_y){
	uint32_t color = 0xFF, i, j, pixel;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	
	for(j = 0; j < ALTURA_EDDIE; j++){ //altura
		for(i = 0; i < LARGURA_EDDIE+1; i++){ //largura+1 pq na hora de virar tava deixando uns rastros
			GrContextForegroundSet(&sContext, ClrBlack);
			if(x_anterior_eddie < x_atual_eddie){ //se estava andando para direita
				draw_pixel((-i)+posicao_x+LARGURA_EDDIE, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime espelhado
				pos_eddie_direita =  true; //flag para guardar a ultima posicao
			}
			else if(x_anterior_eddie > x_atual_eddie){ //se estava andando para a esquerda
				draw_pixel(i+posicao_x, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime normal
				pos_eddie_direita = false; //flag para guardar a ultima posicao
			}
			else
				if(pos_eddie_direita){ //se a ultima posicao era virado para direita
					draw_pixel((-i)+posicao_x+LARGURA_EDDIE, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime espelhado
				}
				else{
					draw_pixel(i+posicao_x, j-posicao_y-ALTURA_EDDIE+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //imprime normal
				}
		}
	}
	osMutexRelease(mutex_display_ID);
}

void apagaEscada(uint32_t andar, uint32_t posicao){
	uint32_t color = 0xE2697A, i, j, pixel;
	pixel = Escada_start;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	for(j = 0; j < ALTURA_ESCADA; j++){ //altura
		for(i = 0; i < LARGURA_ESCADA; i++){ //largura
			GrContextForegroundSet(&sContext, 0x00000000); // imprime cor AZUL
			draw_pixel(i+posicao, j-ALTURA_ESCADA+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		}
	}
	osMutexRelease(mutex_display_ID);
}

void imprimeEscada(uint32_t andar, uint32_t posicao){
	uint32_t color, i, j, pixel;
	pixel = Escada_start;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	for(j = 0; j < ALTURA_ESCADA; j++){ //altura
		for(i = 0; i < LARGURA_ESCADA; i++){ //largura
			color = 0x00;
			color += Escada[(pixel+(frame_escada*ALTURA_ESCADA*LARGURA_ESCADA*3))+2];
			color += Escada[(pixel+(frame_escada*ALTURA_ESCADA*LARGURA_ESCADA*3))+1]*16*16;
			color += Escada[(pixel+(frame_escada*ALTURA_ESCADA*LARGURA_ESCADA*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color); // imprime cor AZUL
				draw_pixel(i+posicao, j-ALTURA_ESCADA+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
			}
			pixel += 3;
		}
	}
	osMutexRelease(mutex_display_ID);
}

void imprimeEscadas(){
	int i;
	
	for(i = 0; i < TOTAL_ESCADAS; i++){
		imprimeEscada(posicaoEscadas[0][i], posicaoEscadas[1][i]);
	}
}

void apagaChefao(uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel, andar= 1;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	for(j = 0; j < ALTURA_CHEFAO; j++){ //altura
		for(i = 0; i < LARGURA_CHEFAO; i++){ //largura
			GrContextForegroundSet(&sContext, 0x000000); //apaga os pixels colocando preto
			draw_pixel(i+posicao, j-ALTURA_CHEFAO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		}
	}
	osMutexRelease(mutex_display_ID);
}

void imprimeChefao(uint32_t posicao, bool chefinho){
	uint32_t color = 0x00, i, j, pixel, andar= 1;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	if(!chefinho){
	pixel = Chefao_start;		
	
	GrContextForegroundSet(&sContext, ClrBlack);
	GrLineDrawV(&sContext,posicao -1, 0-ALTURA_CHEFAO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	GrLineDrawV(&sContext,posicao + LARGURA_CHEFAO, 0-ALTURA_CHEFAO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	
	for(j = 0; j < ALTURA_CHEFAO; j++){ //altura
		for(i = 0; i < LARGURA_CHEFAO; i++){ //largura
			color = 0x00;
			color += Chefao[(pixel+(frame_chefao*ALTURA_CHEFAO*LARGURA_CHEFAO*3))+2];
			color += Chefao[(pixel+(frame_chefao*ALTURA_CHEFAO*LARGURA_CHEFAO*3))+1]*16*16;
			color += Chefao[(pixel+(frame_chefao*ALTURA_CHEFAO*LARGURA_CHEFAO*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color > 0x100){ //não imprime os pixels azuis para dar efeito de fundo transparente
				if (j<3)
					GrContextForegroundSet(&sContext, ClrOrange);
				else 
					GrContextForegroundSet(&sContext, ClrWhite);
			}
			else
				GrContextForegroundSet(&sContext, ClrBlack);
			draw_pixel(i+posicao, j-ALTURA_CHEFAO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
			
			pixel += 3;
		}
	}
}
	else{
		pixel = Inimigo_start;	
		GrContextForegroundSet(&sContext, ClrBlack);
	
	GrLineDrawV(&sContext,posicao -1, 0-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	GrLineDrawV(&sContext,posicao + LARGURA_INIMIGO, 0-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	
	for(j = 0; j < ALTURA_INIMIGO; j++){ //altura
		for(i = 0; i < LARGURA_INIMIGO; i++){ //largura
			color = 0x00;
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))+2];
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))+1]*16*16;
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color>0xff){ //não imprime os pixels azuis para dar efeito de fundo transparente
				if(j<3)
					GrContextForegroundSet(&sContext, ClrOrange);
				else 
					GrContextForegroundSet(&sContext, ClrWhite);
			}
			else if (j == 2)
				GrContextForegroundSet(&sContext, ClrOrange);

			else
				GrContextForegroundSet(&sContext, ClrBlack);
			
			draw_pixel(i+posicao, j-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
			
			pixel += 3;
		}
	}
	
	pixel = Chave_start;
	GrContextForegroundSet(&sContext, ClrBlack);
	
	GrLineDrawV(&sContext,posicao -1, -ALTURA_ITEM-7+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1-7+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	GrLineDrawV(&sContext,posicao + LARGURA_ITEM, -ALTURA_ITEM-7+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1-7+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	
	for(j = 0; j < ALTURA_ITEM; j++){ //altura
		for(i = 0; i < LARGURA_ITEM; i++){ //largura
			color = 0x00;
			color += Chave[(pixel+(ALTURA_ITEM*LARGURA_ITEM*3))+2];
			color += Chave[(pixel+(ALTURA_ITEM*LARGURA_ITEM*3))+1]*16*16;
			color += Chave[(pixel+(ALTURA_ITEM*LARGURA_ITEM*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, ClrYellow);
			}
			else{
				GrContextForegroundSet(&sContext, ClrBlack);
			}
			draw_pixel(i+posicao, j-ALTURA_ITEM-7+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //não sei porque do 16, mas assim que dá certo
			pixel += 3;
		}
	}
	
	}
	
	osMutexRelease(mutex_display_ID);
}

void apagaInimigo(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	for(j = 0; j < ALTURA_INIMIGO; j++){ //altura
		for(i = 0; i < LARGURA_INIMIGO; i++){ //largura
			GrContextForegroundSet(&sContext, 0x000000);
			draw_pixel(i+posicao, j-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
		}
	}
	osMutexRelease(mutex_display_ID);
}

void imprimeInimigo(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	pixel = Inimigo_start;
	osMutexWait(mutex_display_ID,osWaitForever);
	GrContextForegroundSet(&sContext, ClrBlack);
	
	GrLineDrawV(&sContext,posicao -1, 0-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	GrLineDrawV(&sContext,posicao + LARGURA_INIMIGO, 0-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	
	for(j = 0; j < ALTURA_INIMIGO; j++){ //altura
		for(i = 0; i < LARGURA_INIMIGO; i++){ //largura
			color = 0x00;
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))+2];
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))+1]*16*16;
			color += Inimigo[(pixel+(frame_inimigo*ALTURA_INIMIGO*LARGURA_INIMIGO*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color>0xff){ //não imprime os pixels azuis para dar efeito de fundo transparente
				if(j<3)
					GrContextForegroundSet(&sContext, ClrBlue);
				else 
					GrContextForegroundSet(&sContext, ClrWhite);
			}
			else if (j == 2)
				GrContextForegroundSet(&sContext, ClrBlue);

			else
				GrContextForegroundSet(&sContext, ClrBlack);
			
			draw_pixel(i+posicao, j-ALTURA_INIMIGO+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
			
			pixel += 3;
		}
	}
	osMutexRelease(mutex_display_ID);
}

void apagaItem(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	for(j = 0; j < ALTURA_ITEM; j++){ //altura
		for(i = 0; i < LARGURA_ITEM; i++){ //largura
			GrContextForegroundSet(&sContext, 0x00000000);
			draw_pixel(i+posicao, j-ALTURA_ITEM-16+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //não sei porque do 16, mas assim que dá certo
		}
	}
	osMutexRelease(mutex_display_ID);
}

void imprimeItem(uint32_t andar, uint32_t posicao){
	uint32_t color = 0x00, i, j, pixel;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	pixel = Item_start;
	
	GrContextForegroundSet(&sContext, ClrBlack);
	
	GrLineDrawV(&sContext,posicao -1, -ALTURA_ITEM-16+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1-16+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	GrLineDrawV(&sContext,posicao + LARGURA_ITEM, -ALTURA_ITEM-16+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar, -1-16+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar);
	
	
	for(j = 0; j < ALTURA_ITEM; j++){ //altura
		for(i = 0; i < LARGURA_ITEM; i++){ //largura
			color = 0x00;
			color += Item[(pixel+(frame_item*ALTURA_ITEM*LARGURA_ITEM*3))+2];
			color += Item[(pixel+(frame_item*ALTURA_ITEM*LARGURA_ITEM*3))+1]*16*16;
			color += Item[(pixel+(frame_item*ALTURA_ITEM*LARGURA_ITEM*3))]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			if(color != 0x00000000 && color != 0x000000FF){ //não imprime os pixels azuis para dar efeito de fundo transparente
				GrContextForegroundSet(&sContext, color);
			}
			else{
				GrContextForegroundSet(&sContext, ClrBlack);
			}
			draw_pixel(i+posicao, j-ALTURA_ITEM-16+ESPACO_PONTUACAO+(ALTURA_ANDAR+ESPESSURA_CHAO)*andar); //não sei porque do 16, mas assim que dá certo
			pixel += 3;
		}
	}
	osMutexRelease(mutex_display_ID);
}

void inicializaPontuacao(){
	char pbufx[10];
	
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

void imprimePontuacao(uint32_t pontos){
	uint32_t color = 0xFF, i, j, pixel;
	uint16_t pos_primeiro_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+((-2)*(LARGURA_NUMEROS+1));
	uint16_t pos_segundo_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+((-1)*(LARGURA_NUMEROS+1));
	uint16_t pos_terceiro_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(0*(LARGURA_NUMEROS+1));
	uint16_t pos_quarto_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(1*(LARGURA_NUMEROS+1));
	uint16_t pos_quinto_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(2*(LARGURA_NUMEROS+1));
	uint16_t primeiro_digito = pontos/10000;
	uint16_t segundo_digito = (pontos - (primeiro_digito*10000))/1000;
	uint16_t terceiro_digito = (pontos - (primeiro_digito*10000) - (segundo_digito*1000))/100;
	uint16_t quarto_digito = (pontos - (primeiro_digito*10000) - (segundo_digito*1000) - (terceiro_digito*100))/10;
	uint16_t quinto_digito = (pontos - (primeiro_digito*10000) - (segundo_digito*1000) - (terceiro_digito*100) - (quarto_digito*10));
	
	//imprime o primeiro digito
	pixel = Numeros_start;
	osMutexWait(mutex_display_ID,osWaitForever);
	
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(primeiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(primeiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[pixel+(primeiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3)]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){  
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_primeiro_digito, j);
			}
			pixel += 3;
		}
	}
	
	//imprime o segundo digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(segundo_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(segundo_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[(pixel+(segundo_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_segundo_digito, j);
			}
			pixel += 3;
		}
	}
	
	//imprime o terceiro digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(terceiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(terceiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[(pixel+(terceiro_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_terceiro_digito, j);
			}
			pixel += 3;
		}
	}
	
	//imprime o quarto digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(quarto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(quarto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[(pixel+(quarto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_quarto_digito, j);
			}
			pixel += 3;
		}
	}
	
	//imprime o quinto digito
	pixel = Numeros_start;
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			color = 0x00;
			color += Numeros[(pixel+(quinto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+2];
			color += Numeros[(pixel+(quinto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))+1]*16*16; //desloca 8 bits para esquerda
			color += Numeros[(pixel+(quinto_digito*ALTURA_NUMEROS*LARGURA_NUMEROS*3))]*16*16*16*16; //desloca 16 bits para esquerda
			color += 0x00*16*16*16*16*16*16;
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			if(color != 0x00000000 && color != 0x000000FF){ 
				GrContextForegroundSet(&sContext, color); // para apagar os pixeis coloridos
				draw_pixel(i+pos_quinto_digito, j);
			}
			pixel += 3;
		}
	}
	osMutexRelease(mutex_display_ID);
}

void apagaPontuacao(){
	uint16_t i, j, k;
	uint16_t pos_primeiro_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+((-2)*(LARGURA_NUMEROS+1));
	uint16_t pos_segundo_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+((-1)*(LARGURA_NUMEROS+1));
	uint16_t pos_terceiro_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(0*(LARGURA_NUMEROS+1));
	uint16_t pos_quarto_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(1*(LARGURA_NUMEROS+1));
	uint16_t pos_quinto_digito = ((LARGURA_TELA/2)-(LARGURA_NUMEROS/2))+(2*(LARGURA_NUMEROS+1));
	osMutexWait(mutex_display_ID,osWaitForever);
	
	for(j = 0; j < ALTURA_NUMEROS; j++){ //altura
		for(i = 0; i < LARGURA_NUMEROS; i++){ //largura
			//não imprime os pixels pretos ou azuis para dar efeito de fundo transparente
			GrContextForegroundSet(&sContext, 0x00000000); // para apagar os pixeis coloridos
			draw_pixel(i+pos_primeiro_digito, j);
			draw_pixel(i+pos_segundo_digito, j);
			draw_pixel(i+pos_terceiro_digito, j);
			draw_pixel(i+pos_quarto_digito, j);
			draw_pixel(i+pos_quinto_digito, j);
		}
	}
	osMutexRelease(mutex_display_ID);
}

void somPulo(){
			buzzer_vol_set(5000);
			buzzer_per_set(1703);
			buzzer_write(true);
			osDelay(654);
			buzzer_write(false);
			buzzer_per_set(2200);
			buzzer_write(true);
			osDelay(655);
			buzzer_write(false);
}

void somSubirEscada(){
		buzzer_vol_set(5000);
		buzzer_per_set(2551);
		buzzer_write(true);
		osDelay(600);
		buzzer_write(false);
		osDelay(600);
		buzzer_per_set(2551);
		buzzer_write(true);
		osDelay(600);
		buzzer_write(false);
		osDelay(600);
		buzzer_write(true);
		osDelay(600);
		buzzer_write(false);
		osDelay(600);
		buzzer_per_set(2551);
		buzzer_write(true);
		osDelay(600);
		buzzer_write(false);
}

void somFimGame(){
		buzzer_vol_set(5000);
		
		buzzer_per_set(0);
		buzzer_write(true);
		osDelay(700);
		buzzer_write(false);
		
		buzzer_per_set(1612);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1666);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1724);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1785);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1851);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1923);
		buzzer_write(true);
		osDelay(1570);
		buzzer_write(false);
		
		buzzer_per_set(2000);
		buzzer_write(true);
		osDelay(1570);
		buzzer_write(false);
		
		
}

void somItem(){
		buzzer_vol_set(5000);
	
	buzzer_per_set(1000);
			buzzer_write(true);
			osDelay(500);
			buzzer_write(false);
			
		buzzer_per_set(1500);
			buzzer_write(true);
			osDelay(700);
			buzzer_write(false);
	
	buzzer_per_set(800);
			buzzer_write(true);
			osDelay(1000);
			buzzer_write(false);
	
}

void somMorte(){
	buzzer_vol_set(5000);
	
	buzzer_per_set(1851);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1923);
		buzzer_write(true);
		osDelay(1570);
		buzzer_write(false);
		
		buzzer_per_set(2000);
		buzzer_write(true);
		osDelay(1570);
		buzzer_write(false);
}

void somProximaFase(){
		buzzer_vol_set(5000);
		
		buzzer_per_set(2000);
		buzzer_write(true);
		osDelay(1570);
		buzzer_write(false);
		
		buzzer_per_set(1923);
		buzzer_write(true);
		osDelay(1570);
		buzzer_write(false);
		
		buzzer_per_set(1851);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1785);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1724);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1666);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		buzzer_per_set(1612);
		buzzer_write(true);
		osDelay(770);
		buzzer_write(false);
		
		
		
		buzzer_per_set(0);
		buzzer_write(true);
		osDelay(700);
		buzzer_write(false);
}

void imprimeVidas(uint32_t vidas){
	osMutexWait(mutex_display_ID,osWaitForever);
	GrContextForegroundSet(&sContext,vidas>=1?ClrGreen:ClrBlack);
	GrLineDrawH(&sContext,48,56,11);
	GrLineDrawH(&sContext,48,56,12);
	GrContextForegroundSet(&sContext,vidas>=2?ClrGreen:ClrBlack);
	GrLineDrawH(&sContext,60,68,11);
	GrLineDrawH(&sContext,60,68,12);
	GrContextForegroundSet(&sContext,vidas>=3?ClrGreen:ClrBlack);
	GrLineDrawH(&sContext,72,80,11);
	GrLineDrawH(&sContext,72,80,12);
	osMutexRelease(mutex_display_ID);
}