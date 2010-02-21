/*
 * Just an interface to easy open a GPIO-Port
 *
 * After opening standard I/O-Calls can be used.
 *
 * (C) 2008 by Benjamin Tietz <benjamin.tietz@in-circuit.de>
 */
#ifndef __GPIO_H
#define __GPIO_H

#include <sys/types.h>

/*
 * This will open an GPIO-Port.
 *
 * label is some descriptive word, used to identify this port
 * port is the port to open, either 'A'-'D' or 0-3
 * mask is a bitmask for the pins on this port to use.
 * 	each bit configs one pin, if set, the pin is used,
 * 	if not it is ignored.
 * flags are O_RDONLY, O_WRONLY or O_RDWR for ro, wo or rw access of the pins.
 *
 * This will return a filehandle-number or something smaller 0 on an Error
 */

int init_gpio(char *label, char port, int mask, int flags);

/* 
 * After opening the Port you simply can read and write to the filehandle.
 *
 * See "man 2 read" and "man 2 write". It will always use up to 4 bytes (32bit)
 * at a time and read/set them according to the mask you set above.
 */

/*
 * when the port isn't used any longer, it should be disabled again, so other
 * processes can use this port again.
 *
 * label must be the same label as given by init, fh is the filehandle used
 * to communicate with the GPIOs
 */
int destroy_gpio(char *label, int fh);

#endif
