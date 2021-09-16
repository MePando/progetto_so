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
volatile uint8_t *tx_write, *tx_read, *tx_end;
volatile uint8_t positions = 0;
 
volatile uint8_t pos_x = 90;
volatile uint8_t pos_y = 90;
volatile uint8_t checksum;

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

void txInit(void){
	/*We inizialise the buffer*/
	tx_write = tx_read = tx_buffer;
	tx_end = &tx_buffer[TX_SIZE];
}

uint8_t crc(uint8_t x, uint8_t y, uint8_t checksum){
	return 1;
}

void print(uint8_t c){
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

void sendPosition(void){

	/*If the write pointer points to the same cell of read pointer
	  it seems that reading is to slow... we forgot about reading the next positions
	  to align the reading with the writing*/
	if(tx_write == tx_read && positions != 0){
		for(uint8_t i = 0; i < TX_SIZE/3; i++){
			tx_read--;
			if(tx_read <= tx_buffer){
				tx_read = tx_end;
			}
		}
		positions = 1;
	}

	/*Fill the tx_buffer*/
	*tx_write++ = pos_x;
	*tx_write++ = pos_y;
	*tx_write++ = checksum;

	positions += 1;

	if (tx_write >= tx_end){
		tx_write = tx_buffer;
	}

	/*A response is ready to be send!*/
	UCSR0B |= _BV(5);

}

void getCommand(void){

	/*Only if they are different, because in case they are the same it means that there are not
	  any new messages to read: the previous command is still valid. Messages>0 in the main should
	  make this check useless, but we do it anyway to be sure*/
	if( rx_read != rx_write){

		uint8_t x = *rx_read++;
		uint8_t y = *rx_read++;
		uint8_t csum = *rx_read++;
		messages -= 1;

		if(crc(x, y, csum)){
			pos_x = x;
			pos_y = y;
			checksum = csum;
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
	/*Inizialise the tx_buffer*/
	txInit();
	sei();
	/*Start working*/
	while(1){

		/*While there are messages in the rx_buffer*/
		while(messages > 0){

			getCommand();
			sendPosition();

		}
		/*Continuosly sending the posistions to the servos*/

	}

}

ISR(USART0_RX_vect){

	/*If the write pointer points to the same cell of read pointer
	  it seems that reading is to slow... we forgot about reading the next messages
	  to align the reading with the writing*/
	if(rx_write == rx_read && messages != 0){
		for(uint8_t i = 0; i < RX_SIZE/3; i++){
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
	if(rx_counter == RX_SIZE/3){
		rx_counter = 0;
		messages += 1;
	}

	if (rx_write >= rx_end){
		rx_write = rx_buffer;
	}

}

ISR(USART0_UDRE_vect) {

	/*Only if they are different, because in case they are the same it means that there are not
	  any new positions to send*/
	if(tx_read != tx_write){

		for(uint8_t i = 0; i < TX_SIZE/3-1; i++){
			UDR0 = *tx_read++;
		}

		if(tx_read >= tx_end){
			tx_read = tx_buffer;
		}

		positions -= 1;

	}

	/*Leaving the ISR*/
	UCSR0B &= ~_BV(5);

}

