CC=gcc
CCOPTS=-std=gnu99
BINS= serial_example

.phony: clean all

all: $(BINS)

serial_example:	serial_example.c serial_linux.c 
	$(CC) -$(CCOPTS) -o $@ $^  

clean:
	rm -rf *~ $(BINS) *.pgm

