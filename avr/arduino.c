#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD 19600
#define MYUBRR (F_CPU/16/BAUD-1)
#define RX_SIZE 9
#define TX_SIZE 9
#define TCCRA_MASK (1<<WGM10)|(1<<COM1C0)|(1<<COM1C1)
#define TCCRB_MASK ((1<<WGM12)|(1<<CS10))   

volatile uint8_t rx_buffer[RX_SIZE];
volatile uint8_t *rx_write, *rx_read, *rx_end;
volatile uint8_t messages = 0;
volatile uint8_t rx_counter = 0;
volatile uint8_t tx_buffer[RX_SIZE];
volatile uint8_t tx_counter = 0; 
volatile uint8_t pos_x = 90;
volatile uint8_t pos_y = 90;
const uint8_t mask_x = (1<<4); /* mask for pin 10 */
const uint8_t mask_y = (1<<5); /* mask for pin 11 */

void uartInit(void){

	/*8-bit data*/
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);

	/*Enable     Rx            Tx      Rx-interrupts     Tx-interrupt*/
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0) | (1<<TXCIE0);

	/*Set baud rate*/
	UBRR0H = (uint8_t)(MYUBRR>>8);
	UBRR0L = (uint8_t)MYUBRR;
	
}

void setTimer(void){
	/*Set up Timer1 in 16-bit Fast PWM mode (mode 14)*/
	TCCR1A=TCCRA_MASK;
  	TCCR1B=TCCRB_MASK;
	ICR1 = 20000;
}

void rxInit(void){
	/*We inizialise the buffer*/
	rx_write = rx_read = rx_buffer;
	rx_end = &rx_buffer[RX_SIZE];
}

uint8_t crc(uint8_t x, uint8_t y, uint8_t checksum){
	return 1;
}

void putChar(uint8_t c){

	if(tx_buffer[TX_SIZE-1] == 0)
		{
			tx_buffer[tx_counter]=c;

			if(tx_counter < TX_SIZE){
				tx_counter++;
			}
		}
		else
		{
			for(uint8_t i = 0 ; i < TX_SIZE-2; i++){
				tx_buffer[i] = tx_buffer[i+1];
			}
			tx_buffer[TX_SIZE-1] = c;
		}

		if(tx_counter>0){
			UCSR0B |= _BV(5);
		}

}

void putString(uint8_t* buf){
	while(*buf){
		//putChar(*buf);
	}
}

void getCommand(void){

	/*Only if they are different because, in case they are the same it means that there are not
	  any new messages to read: the previous command is still valid. Messages>0 in the main should
	  make this check useless, but we do it anyway to be sure*/
	if( rx_read != rx_write){

		uint8_t x = *rx_read++;
		uint8_t y = *rx_read++;
		uint8_t checksum = *rx_read++;
		messages -= 1;

		if(crc(x, y, checksum)){
			pos_x = x;
			pos_y = y;
		}

		if(rx_read >= rx_end){
			rx_read = rx_buffer;
		}
	}

}

int main(void){

	cli();
	/*Inizialise the uart*/
	uartInit();
	/*Inizialise the port*/
	DDRB |= mask_x;
	/*Set Timer*/
	setTimer();
	/*Inizialise the rx_buffer*/
	rxInit();
	sei();
	/*Start working*/
	while(1){

		/*While there are messages in the rx_buffer*/
		while(messages > 0){

			getCommand();

		}
		/*Continuosly sending the posistions to the servos*/

	}

}

ISR(USART0_RX_vect){

	/*If the write pointer points to the same cell of read pointer
	  it seems that reading is to slow... we forgot about reading the next messages
	  to align the reading with the writing*/
	if(rx_write == rx_read && messages != 0){
		for(uint8_t i = 0; i < 3; i++){
			rx_read--;
			if(rx_read <= rx_buffer){
				rx_read = rx_end;
			}
		}
		messages = 1;
	}

	*rx_write++ = UDR0;
	rx_counter += 1;


	/*An entire message is arrived!*/
	if(rx_counter == 3){
		rx_counter = 0;
		messages += 1;
	}

	if (rx_write >= rx_end){
		rx_write = rx_buffer;
	}

}

ISR(USART0_UDRE_vect) {

	uint8_t temp;

	if( tx_counter > 0){

		temp = tx_buffer[0];

		for(uint8_t i = 0; i < TX_SIZE-2; i++){
			tx_buffer[i] = tx_buffer[i+1];
		}

		tx_buffer[TX_SIZE-1] = 0;
		tx_counter--;

		UDR0 = temp;

	}

	if(tx_counter == 0){
		UCSR0B &= ~_BV(5);
	}

}

