#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CRC16 0x8005

uint16_t crc16(const uint8_t *data, uint16_t size){

	uint16_t out = 0;
	int bits_read = 0, bit_flag;

	/* Sanity check: */
	if(data == NULL){
		return 0;
	}

	while(size > 0){
		bit_flag = out >> 15;

	        /* Get next bit: */
	        out <<= 1;
	        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

	        /* Increment bit counter: */
	        bits_read++;
	        if(bits_read > 7){
	            bits_read = 0;
	            data++;
	            size--;
	        }

	        /* Cycle check: */
	        if(bit_flag){
	            out ^= CRC16;
		}
	
	}

	   // item b) "push out" the last 16 bits
	int i;
	for (i = 0; i < 16; ++i) {
		bit_flag = out >> 15;
		out <<= 1;
		if(bit_flag){
			out ^= CRC16;
		}
	}

	// item c) reverse the bits
	uint16_t crc = 0;
	i = 0x8000;
	int j = 0x0001;
	for (; i != 0; i >>=1, j <<= 1) {
		if (i & out){
			crc |= j;
		}
	}

	return crc;

}

int main(int argc, char** argv){
	if(argc<3){
		printf("usage: %s <filename> <baudrate>", argv[0]);
		return 0;
	}

	char* filename = argv[1];
	int baudrate = atoi(argv[2]);
	int n_write;
	int n_read;

	/*Opening the serial device*/
	printf("Opening serial device [%s] ... ", filename);
	int fd = serial_open(filename);
	if(fd <= 0){
		printf("Error\n");
		return 0;
	}else{
		printf("Success\n");
	}

	/*Setting Baudrate*/	
	printf("Setting baudrate [%d] ...", baudrate);
	int attribs = serial_set_interface_attribs(fd, baudrate, 0);
	if(attribs){
		printf("Error\n");
		return 0;
	}else{
		printf("Success\n");
	}

	serial_set_blocking(fd, 1);

	const int size = 7;
	char command[size]; /*At maximum 7, 3 for the 2 numbers and one for ":"*/
	int pos_x, pos_y;
	
	uint8_t bsize = 3;
	uint8_t buffer[bsize]; /*One for pos_x, one for pos_y and the last one for checksum*/
	uint8_t read_buffer[bsize];

	printf("\nPlease enter new camera position, using the format x:y in degrees from 0 to 180 (e.g. 90:180) or q to close the program.\n");
	while(1){
		printf("\nNew position or q to quit: ");
		while(1){
			scanf("%s", command);
			if(sscanf(command, "%d:%d", &pos_x, &pos_y)){
				if((pos_x <= 180 && pos_x >= 0) && (pos_y <= 180 && pos_y >= 0)){
					break;
				}
			}
			if(command[0] == 'q'){
				printf("\nProgram closed!");
				return 0;
			}
			printf("Some error please follow the format! Retry: ");
		}

		printf("Thanks! Trying to send: %d and %d\n", pos_x, pos_y);
		buffer[0] = pos_x;
		buffer[1] = pos_y;

		printf("Calculating the checksum...\n");
		buffer[2] = crc16(buffer, 2);
		printf("Checksum: %u\n", buffer[2]);

		printf("Sending...\t\t");
		n_write = write(fd, buffer, bsize);
		printf("[Sent]\n");

		printf("Waiting response...\t");
		n_read=read(fd, read_buffer, bsize);
		printf("[Arrived]\n");
		
		printf("Position set: ");
    		for (int i=0; i < n_read; ++i) {
   			printf("%u ", read_buffer[i]);
   		}

		printf("\n");
	}
}
