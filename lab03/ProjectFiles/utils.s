; utils.s
; Desenvolvido para a placa EK-TM4C1294XL
; Código que apresenta algumas funcionalidades:
; - Altera a frequência do barramento usando o PLL
; - Configura o Systick para utilizar delays precisos

; Editado por Guilherme Peron
; 15/03/2018
; Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu

; -------------------------------------------------------------------------------------------------------------------------
; PLL
; -------------------------------------------------------------------------------------------------------------------------
; A frequência do barramento será 80MHz
; Representa o divisor para inicializar o PLL para a frequência desejada
; Frequência do barramento é 480MHz/(PSYSDIV+1) = 480MHz/(5+1) = 80 MHz
PSYSDIV                       EQU 5

SYSCTL_RIS_R                  EQU 0x400FE050
SYSCTL_RIS_MOSCPUPRIS         EQU 0x00000100  ; MOSC Power Up Raw Interrupt Status
SYSCTL_MOSCCTL_R              EQU 0x400FE07C
SYSCTL_MOSCCTL_PWRDN          EQU 0x00000008  ; Power Down
SYSCTL_MOSCCTL_NOXTAL         EQU 0x00000004  ; No Crystal Connected
SYSCTL_RSCLKCFG_R             EQU 0x400FE0B0
SYSCTL_RSCLKCFG_MEMTIMU       EQU 0x80000000  ; Memory Timing Register Update
SYSCTL_RSCLKCFG_NEWFREQ       EQU 0x40000000  ; New PLLFREQ Accept
SYSCTL_RSCLKCFG_USEPLL        EQU 0x10000000  ; Use PLL
SYSCTL_RSCLKCFG_PLLSRC_M      EQU 0x0F000000  ; PLL Source
SYSCTL_RSCLKCFG_PLLSRC_MOSC   EQU 0x03000000  ; MOSC is the PLL input clock source
SYSCTL_RSCLKCFG_OSCSRC_M      EQU 0x00F00000  ; Oscillator Source
SYSCTL_RSCLKCFG_OSCSRC_MOSC   EQU 0x00300000  ; MOSC is oscillator source
SYSCTL_RSCLKCFG_PSYSDIV_M     EQU 0x000003FF  ; PLL System Clock Divisor
SYSCTL_MEMTIM0_R              EQU 0x400FE0C0
SYSCTL_DSCLKCFG_R             EQU 0x400FE144
SYSCTL_DSCLKCFG_DSOSCSRC_M    EQU 0x00F00000  ; Deep Sleep Oscillator Source
SYSCTL_DSCLKCFG_DSOSCSRC_MOSC EQU 0x00300000  ; MOSC
SYSCTL_PLLFREQ0_R             EQU 0x400FE160
SYSCTL_PLLFREQ0_PLLPWR        EQU 0x00800000  ; PLL Power
SYSCTL_PLLFREQ0_MFRAC_M       EQU 0x000FFC00  ; PLL M Fractional Value
SYSCTL_PLLFREQ0_MINT_M        EQU 0x000003FF  ; PLL M Integer Value
SYSCTL_PLLFREQ0_MFRAC_S       EQU 10
SYSCTL_PLLFREQ0_MINT_S        EQU 0
SYSCTL_PLLFREQ1_R             EQU 0x400FE164
SYSCTL_PLLFREQ1_Q_M           EQU 0x00001F00  ; PLL Q Value
SYSCTL_PLLFREQ1_N_M           EQU 0x0000001F  ; PLL N Value
SYSCTL_PLLFREQ1_Q_S           EQU 8
SYSCTL_PLLFREQ1_N_S           EQU 0
SYSCTL_PLLSTAT_R              EQU 0x400FE168
SYSCTL_PLLSTAT_LOCK           EQU 0x00000001  ; PLL Lock

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
; -------------------------------------------------------------------------------------------------------------------------
; SYSTICK
; -------------------------------------------------------------------------------------------------------------------------
NVIC_ST_CTRL_R        EQU 0xE000E010
NVIC_ST_RELOAD_R      EQU 0xE000E014
NVIC_ST_CURRENT_R     EQU 0xE000E018
; -------------------------------------------------------------------------------------------------------------------------	
        EXPORT  SysTick_Init
		EXPORT  SysTick_Wait1ms
		EXPORT  SysTick_Wait1us
;------------SysTick_Init------------
; Configura o sistema para utilizar o SysTick para delays
; Entrada: Nenhum
; Saída: Nenhum
; Modifica: R0, R1
SysTick_Init
	LDR R1, =NVIC_ST_CTRL_R			; R1 = &NVIC_ST_CTRL_R (ponteiro)
	MOV R0, #0 						; desabilita Systick durante a configuração
	STR R0, [R1]					; escreve no endereço de memória do periférico
	LDR R1, =NVIC_ST_RELOAD_R 		; R1 = &NVIC_ST_RELOAD_R (pointeiro)
	LDR R0, =0x00FFFFFF; 			; valor máximo de recarga 2^24 ticks
	STR R0, [R1] 					; escreve no endereço de memória do periférico o NVIC_ST_RELOAD_M
	LDR R1, =NVIC_ST_CURRENT_R 		; R1 = &NVIC_ST_CURRENT_R (ponteiro)
	MOV R0, #0 						; qualquer escrita no endereço NVIC_ST_CURRENT_R o limpa
	STR R0, [R1] 					; limpa o contador
	LDR R1, =NVIC_ST_CTRL_R 		; habilita o SysTick com o clock do core
	MOV R0, #0x05					; ENABLE | CLK_SRC
	STR R0, [R1] 					; Seta os bits de ENABLE e CLK_SRC na memória
	BX LR
	
;------------SysTick_Wait------------
; Atraso de tempo utilizando processador ocupado
; Entrada: R0 -> parâmetro de delay em unidades do clock do core (12.5ns)
; Saída: Nenhum
; Modifica: R0, R1 e R3
SysTick_Wait
	LDR R1, =NVIC_ST_RELOAD_R 			; R1 = &NVIC_ST_RELOAD_RSUB R0 (ponteiro)
	SUB R0, #1                          
	STR R0, [R1] 						; delay-1, número de contagens para esperar
	LDR R1, =NVIC_ST_CTRL_R 			; R1 = &NVIC_ST_CTRL_R
SysTick_Wait_loop
	LDR R3, [R1] 						; R3 = &NVIC_ST_CTRL_R (ponteiro)
	ANDS R3, R3, #0x00010000 			; O bit COUNT está setado? (Bit 16)
	BEQ SysTick_Wait_loop               ; Se sim permanece no loop
	BX LR                               ; Se não, retorna

;------------SysTick_Wait1ms------------
; tempo de atraso usando processador ocupado. Assume um clock de 80 MHz
; Entrada: R0 --> Número de vezes para contar 1ms.
; Saída: Não tem
; Modifica: R0
DELAY1MS EQU 80000 ; número de ciclos de clock para contar 1ms (assumindo 80 MHz)
	               ; 80000 x 12,5 ns = 1 ms

SysTick_Wait1ms
	PUSH {R4, LR} 						; salva o valor atual de R4 e Link Register
	MOVS R4, R0 						; R4 = R0  numEsperasRestantes com atualização dos flags
	BEQ SysTick_Wait1ms_done 			; Se o numEsperasRestantes == 0, vai para o fim
SysTick_Wait1ms_loop					
	LDR R0, =DELAY1MS 					; R0 = DELAY1MS (número de ticks para contar 1ms)
	BL SysTick_Wait 					; chama a rotina para esperar por 1ms
	SUBS R4, R4, #1 					; R4 = R4 - 1; numEsperasRestantes--
	BHI SysTick_Wait1ms_loop 			; se (numEsperasRestantes > 0), espera mais 1ms
SysTick_Wait1ms_done
	POP {R4, PC}                        ;return

;------------SysTick_Wait1us------------
; tempo de atraso usando processador ocupado. Assume um clock de 80 MHz
; Entrada: R0 --> Número de vezes para contar 1us.
; Saída: Não tem
; Modifica: R0
DELAY1US EQU 80 ; número de ciclos de clock para contar 1us (assumindo
				; 80 MHz) 80 x 12,5 ns = 1 us
				
SysTick_Wait1us
	PUSH {R4, LR} 						; salva o valor atual de R4 e Link Register
	MOVS R4, R0 						; R4 = R0 numEsperasRestantes com atualização dos flags
	BEQ SysTick_Wait1us_done 			; Se o numEsperasRestantes == 0, vai para o fim
SysTick_Wait1us_loop
	LDR R0, =DELAY1US 					; R0 = DELAY1US (número de ticks para contar 1us)
	BL SysTick_Wait 					; chama a rotina para esperar por 1us
	SUBS R4, R4, #1 					; R4 = R4 - 1; numEsperasRestantes--
	BHI SysTick_Wait1us_loop 			; se (numEsperasRestantes > 0), espera mais 1us
SysTick_Wait1us_done
	POP {R4, PC} 						;return

; -------------------------------------------------------------------------------------------------------------------------
; Fim do Arquivo
; -------------------------------------------------------------------------------------------------------------------------
    ALIGN                        ;Garante que o fim da seção está alinhada 
    END                          ;Fim do arquivo
