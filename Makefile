CC=gcc
CCOPTS=-std=gnu99
BINS= serial_sender

.phony: clean all

all: $(BINS)

serial_sender:	serial_sender.c serial_linux.c 
	$(CC) -$(CCOPTS) -o $@ $^  

clean:
	rm -rf *~ $(BINS) *.pgm

