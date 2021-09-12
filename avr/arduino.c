#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD 19600
#define MYUBRR (F_CPU/16/BAUD-1)
#define MAX_SIZE 3

volatile uint8_t rx_flag = 0;
volatile uint8_t data;

void uart_init(void){

	/*Set baud rate*/
	UBRR0H = (uint8_t)(MYUBRR>>8);
	UBRR0L = (uint8_t)MYUBRR;
	
	/*8-bit data*/
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);
	/*Enable     Rx            Tx      Rx-interrupts     Tx-interrupt*/
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0) | (1<<TXCIE0);

}

void putChar(uint8_t c){
	/*Looping on status bit*/
	while(!(UCSR0A & (1<<UDRE0)));
	/*Start trasmission*/
	UDR0 = c;
}

void putString(uint8_t* buf){
	while(*buf){
		//putChar(*buf);
	}
}

uint8_t getChar(void){
	return data;
}

int main(void){
	cli();
	/*Inizialise the uart*/
	uart_init();
	/*Inizialise ports*/
	//port_init();
	sei();
	/*Start working*/
	while(1){
		if(rx_flag == 1){
			rx_flag = 0;
			uint8_t c = getChar();
			putChar(c);
		}
	}

}

ISR(USART0_RX_vect){
	data = UDR0;
	rx_flag = 1;
}

ISR(USART0_UDRE_vect) {
	
}

