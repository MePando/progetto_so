#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  if (argc<3) {
    printf("usage: %s <filename> <baudrate>", argv[0]);
    return 0;
  }
  char* filename=argv[1];
  int baudrate=atoi(argv[2]), pos_x = 0, pos_y = 0;
  char command[8];

  printf( "opening serial device [%s] ... ", filename);
  int fd=serial_open(filename);
  if (fd<=0) {
    printf ("Error\n");
    return 0;
  } else {
    printf ("Success\n");
  }
  printf( "setting baudrate [%d] ... ", baudrate);

  int attribs=serial_set_interface_attribs(fd, baudrate, 0);
  if (attribs) {
    printf("Error\n");
    return 0;
  }

  serial_set_blocking(fd, 1);

  printf("Please write new camera position, using the format x:y in degrees from 0 to 180 (e.g. 90:180): ");
  while (1){
  	scanf("%s", command);
        if(sscanf(command, "%d:%d", &pos_x, &pos_y)){

		if((pos_x<=180 && pos_x>=0) && (pos_y<=180 && pos_y>=0)){
			break;
		}
	}
	printf("\nSome error please follow the format! Retry: ");
  }
  
  printf("Thanks! Trying to send: %s\n", command);
	
  int n_write = write(fd, command, 8);

  const int bsize=10;
  char buf[bsize];
  while (1) {
    int n_read=read(fd, buf, bsize);
    for (int i=0; i<n_read; ++i) {
      printf("%c", buf[i]);
    }
  }
}
