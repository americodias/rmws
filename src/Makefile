# Makefile
#
# $Id$

REV=$(shell svn info --revision HEAD | grep 'Revision: ' | sed 's/[^0-9]*\([0-9]\+\).*/\1/')

CC := avr32-linux-gcc
CFLAGS := -pipe -O2 -g -Wall -pthread -fno-strict-aliasing -D_REENTRANT -D_REV=$(REV)

DEPS = core51.h gpio.h uart.h parser.h timestamp.h log.h data_conn.h command_conn.h defs.h
OBJ = main.o core51.o gpio.o uart.o parser.o timestamp.o log.o data_conn.o command_conn.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

core51: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
.PHONY: clean
clean:
	$(RM) *.o *~ core51

