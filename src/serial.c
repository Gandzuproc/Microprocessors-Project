#include <stm32f031x6.h>

void initSerial()
{
	/* On the nucleo board, TX is on PA2 while RX is on PA15 */
	RCC->AHBENR |= (1 << 17); // enable GPIOA
	RCC->APB2ENR |= (1 << 14); // enable USART1
	pinMode(GPIOA,2,2); // enable alternate function on PA2
	pinMode(GPIOA,15,2); // enable alternate function on PA15
	// AF1 = USART1 TX on PA2
	GPIOA->AFR[0] &= 0xfffff0ff;
	GPIOA->AFR[0] |= (1 << 8);
	// AF1 = USART1 RX on PA15
	GPIOA->AFR[1] &= 0x0fffffff;
	GPIOA->AFR[1] |= (1 << 28);
	// De-assert reset of USART1 
	RCC->APB2RSTR &= ~(1u << 14);
	
	USART1->CR1 = 0; // disable before configuration
	USART1->CR3 |= (1 << 12); // disable overrun detection
	USART1->BRR = 48000000/9600; // assuming 48MHz clock and 9600 bps data rate
	USART1->CR1 |= (1 << 2) + (1 << 3); // enable Transmistter and receiver
	USART1->CR1 |= 1; // enable the UART

}
void eputchar(char c)
{
	while( (USART1->ISR & (1 << 6)) == 0); // wait for any ongoing transmission to finish
	USART1->ICR=0xffffffff; // clear any error that may be on the port
	USART1->TDR = c; // write the character to the Transmit Data Register
}
char egetchar()
{
	while( (USART1->ISR & (1 << 5)) == 0); // wait for a character
	return (char)USART1->RDR; // return the character that is waiting in the Receive Data Register
}
void eputs(char *String)
{
	while(*String) // keep printing until a NULL is found
	{
		eputchar(*String);
		String++;
	}
}
void printDecimal(int32_t Value)
{
	char DecimalString[12]; // a 32 bit value range from -2 billion to +2 billion approx
												// That's 10 digits
												// plus a null character, plus a sign
	DecimalString[11] = 0; // terminate the string;
	if (Value < 0)
	{
		DecimalString[0]='-';
		Value = -Value;
	}
	else
	{
		DecimalString[0]='+';
	}
	int index = 10;
	while(index > 0)
	{
		DecimalString[index]=(Value % 10) + '0';
		Value = Value / 10;
		index--;
	}
	eputs(DecimalString);
}
