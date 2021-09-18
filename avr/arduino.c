#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD 19600
#define MYUBRR (F_CPU/16/BAUD-1)
#define RX_SIZE 9
#define TX_SIZE 9 

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
volatile uint8_t error = 0;

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
	/*Set up Timer1 in 16-bit*/
		/*OCR1A is x, OCR1B is y*/
	TCCR1A= (1<<COM1A1) | (1<<COM1B1);
  	TCCR1B= (1<<WGM13) | (1<<CS11);
	TCNT1 = 0;
	ICR1 = 20000;
	TCCR1B |= 2;
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

uint8_t CRC8( uint8_t *addr, uint8_t len){
	uint8_t crc = 0;
	for( int i = 0; i < len; i++ ){
		uint8_t inbyte = addr[i];
		crc = crc ^ inbyte;
		for(int j = 0; j < 8; j++){
			if(crc & 0x01){
				crc = (crc >> 1) ^ 0x8C;
        		}else{
				crc >>= 1;
			}
		}
	}
	return crc;
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

void sendError(void){
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
	*tx_write++ = 200;
	*tx_write++ = 200;
	*tx_write++ = 0;

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

		uint8_t buffer[3];
		buffer[0] = *rx_read++;
		buffer[1] = *rx_read++;
		buffer[2] = *rx_read++;
		messages -= 1;

		if(CRC8(buffer, 2) == buffer[2]){
			pos_x = buffer[0];
			pos_y = buffer[1];
			checksum = buffer[2];
		}else{
			sendError();
			error = 1;			
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
	/*Inizialise pin 12 and pin 11 as output port*/
	DDRB |= 96;
	/*Set Timer*/
	setTimer();
	/*Inizialise the rx_buffer*/
	rxInit();
	/*Inizialise the tx_buffer*/
	txInit();
	sei();
	/*Start working*/
	while(1){

		/*If there are messages in the rx_buffer*/
		if(messages > 0){

			getCommand();
			if(error == 0){
				sendPosition();
			}
			error = 0;

			OCR1A = 500+11*pos_x;
			OCR1B = 500+11*pos_y;

		}

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

		tx_read++;

		if(tx_read >= tx_end){
			tx_read = tx_buffer;
		}

		positions -= 1;

	}

	/*Leaving the ISR*/
	UCSR0B &= ~_BV(5);

}

