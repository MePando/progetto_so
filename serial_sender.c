#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
		buffer[2] = CRC8(buffer, 2);
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
