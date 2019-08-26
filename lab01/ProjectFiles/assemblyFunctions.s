        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
		EXPORT  Redimensiona_imagem
		IMPORT  tela
			
Redimensiona_imagem	
	push {r8,r7,r6,r5,r4,r3,r2,r1,r0}
	ldr r5,=tela
	mov r6,r0 ; imagem
	add r6,#0x3a ; inicio da imagem no vetor
	mov r7,r1 ; dimensao
	
	mov r4,#0 ; i = 0 (percorre a imagem)
volta
	mov r3,#0 ; j = 0 (percorre a imagem)
continua
	mov r8,#4
	mul r2,r4,r8 ; x = 4*i/dimensao (percorre a tela)
	mul r1,r3,r8 ; y = 4*j/dimensao (percorre a tela)
	udiv r2,r2,r7
	udiv r1,r1,r7

; posicao_vetor_tela = (x_redimensionado*128 + y)
	mov r8,#128
	mul r0,r2,r8 
	add r0,r0,r1

; obtem o endereco do vetor "tela" e adiciona o valor da posicao_vetor_tela
	ldr r5,=tela
	add r5,r5,r0

; carrega em r8 o valor pixel da imagem a ser analisado
; r6 percore o vetor da imagem de 3 em 3
	ldrb r8,[r6],#3
	cmp r8,0xf0		;verifica se eh um pixel preto ou branco
	bge ehbranco

	mov r8,#0		; se for preto eh representado como 0
	b ehpreto
ehbranco
	mov r8,#1		; se for branco eh representado como 1
ehpreto
	strb r8,[r5]	; coloca 1 ou 0 (branco ou preto) no vetor tela
	
	add r3,#1		; adiciona 1 em i que percorre a imagem (passa para o pixel seguinte)
	cmp r3,#96		; se for menor que 96 continua, se nao adiciona 1 em j (percorre a proxima linha)
	bne continua
	add r4,#1		; adiciona 1 em j que percorre a imagem (passa para a linha seguinte)
	cmp r4,#64		; se for menor que 64 percorre a próxima linha, se nao encerra a rotina
	bne volta
	pop {r8,r7,r6,r5,r4,r3,r2,r1,r0}
	bx lr
	
