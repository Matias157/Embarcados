#include "UART.h"

void UART_Send(char c){//Envio UM caracter
	while((UART0->FR & (1<<5)) != 0);
	UART0->DR = c;	//Escreve na saida o conteudo de c
}

void UART_printstring(char* string){//Envia uma String
	while(*string){//enquanto houver elementos na string
		UART_Send(*(string++));//Envia caracteres para a saida
	}
}

/*Leitura de elementos*/
char UART_Receive(void){//leitura de caractere
	char c;
	// verifica se esta receber
	while((UART0->FR & (1<<4)) != 0);
	//Recebe caracteres para a saida
	c = UART0->DR;
	return c;
}

// Inicializao UART
void UART_Enable(void){
		uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                              SYSCTL_OSC_MAIN |
                                              SYSCTL_USE_PLL |
                                              SYSCTL_CFG_VCO_480),
                                              120000000); // PLL em 120MHz
		
		static unsigned long g_ulBase = 0;
		static const unsigned long g_ulUARTBase[3] ={UART0_BASE, UART1_BASE, UART2_BASE};
		static const unsigned long g_ulUARTPeriph[3] ={SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2};
    
		// Habilita o periferico da UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    // Inicia as configuracoes para o port A
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // Delay de inicializao
    SysCtlDelay(3);
    // Configura PA0 como RX
    // Configura PA1 como Tx
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    // Delay de inicializao
    SysCtlDelay(3);
		GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    // Config UART
    //UARTStdioConfig(0, 115200, ui32SysClock);
		
		//
    // Select the base address of the UART.
    //
    g_ulBase = g_ulUARTBase[0];
		
    //
    // Enable the UART peripheral for use.
    //
    MAP_SysCtlPeripheralEnable(g_ulUARTPeriph[0]);

    //
    // Configure the UART for 115200, n, 8, 1
    //
    MAP_UARTConfigSetExpClk(g_ulBase, ui32SysClock, 115200,
                            (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_WLEN_8));
		
		//
    // Enable the UART operation.
    //
    MAP_UARTEnable(g_ulBase);
		
		// Config de interrupo
		UART0 ->	CTL &= ~(1<<0);							//Disable a UART, para fazer as configuracoes 
		UART0 ->	LCRH = (0x3<<5);						//palavra de 8 bits sem paridade 
		UART0 ->	IFLS |=(1<<4);							//Uso default
		UART0 ->	IM |=(1<<4);								//Habilita interrupcoes 
		UART0 ->	CTL = (1<<0)|(1<<8)|(1<<9);	//Habilita a UART com Rx&Tx
		NVIC -> ISER[0] |= (1<<5);						//Habilita interrupcoes de UART0 no vetor de interrupcoes
		__NVIC_SetPriority(UART0_IRQn,1<<5);
		
    // Configuracoes do UART
		// Delay de inicializao
    SysCtlDelay(30);

    // Envio de mensagem para a UART
    UART_printstring("\033[2J");
    UART_printMenu();
}

// Menu 1 - UART
void UART_printMenu(){
	UART_printstring("FORMA DE ONDA:\n\r");
	UART_printstring("	1 -> Senoidal\n\r");
	UART_printstring("	2 -> Quadrada\n\r");
	UART_printstring("	3 -> Dente de serra\n\r");
	UART_printstring("	4 -> Triangular\n\r");
	UART_printstring("	5 -> Trapezoidal\n\r");
	UART_printstring("CONFIGURACAO DO SINAL:\n\r");
	UART_printstring("	A -> +5Hz | a -> +1Hz\n\r");
	UART_printstring("	S -> -5Hz | s -> -1Hz\n\r");
	UART_printstring("	J -> +1V  | j -> +0.1V\n\r");
	UART_printstring("	K -> -1V  | k -> -0.1V\n\r");
}
