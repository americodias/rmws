# Make file
#
# $Id: Makefile 4 2010-01-10 22:10:13Z adias $

CC := avr32-linux-gcc
CFLAGS := -pipe -O2 -g -Wall -pthread -fno-strict-aliasing

DEPS = core51.h gpio.h uart.h connection.h sec_connection.h parser.h
OBJ = main.o core51.o gpio.o uart.o connection.o sec_connection.o parser.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

core51: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
.PHONY: clean
clean:
	$(RM) *.o *~ core51
