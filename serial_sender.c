#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
	if(argc<3){
		printf("usage: %s <filename> <baudrate>", argv[0]);
		return 0;
	}

	char* filename = argv[1];
	int baudrate = atoi(argv[2]);
	
	printf("Opening serial device [%s] ... ", filename);
	int fd = serial_open(filename);
	if(fd <= 0){
		printf("Error\n");
		return 0;
	}else{
		printf("Success\n");
	}
	
	printf("Setting baudrate [%d] ...", baudrate);
	int attribs = serial_set_interface_attribs(fd, baudrate, 0);
	if(attribs){
		printf("Error\n");
		return 0;
	}else{
		printf("Success\n");
	}

	serial_set_blocking(fd, 1);

	char command[7];
	int pos_x, pos_y;

	while(1){
		printf("\nPlease write new camera position, using the format x:y in degrees from 0 to 180 (e.g. 90:180): ");
		while(1){
			scanf("%s", command);
			if(sscanf(command, "%d:%d", &pos_x, &pos_y)){
				if((pos_x <= 180 && pos_x >= 0) && (pos_y <= 180 && pos_y >= 0)){
					break;
				}
			}
			printf("Some error please follow the format! Retry: ");
		}

		printf("Thanks! Trying to send: %d and %d\n", pos_x, pos_y);
	}
}
