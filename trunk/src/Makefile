# Makefile
#
# $Id$

CC := avr32-linux-gcc
CFLAGS := -pipe -O2 -g -Wall -pthread -fno-strict-aliasing -D_REENTRANT

DEPS = core51.h gpio.h uart.h parser.h timestamp.h
OBJ = main.o core51.o gpio.o uart.o parser.o timestamp.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

core51: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
.PHONY: clean
clean:
	$(RM) *.o *~ core51
