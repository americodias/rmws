#ifndef __CORE51_H
#define __CORE41_H

int core51_init(unsigned long int baudrate);
int core51_destroy();
int core51_sendhex(char *filename);
int core51_run(void);
int core51_abort(void);
int srkey(unsigned char state, unsigned char ch);
int srreset(unsigned char state);
int srint(unsigned char state);

int uart_read();
char *get_uart_buffer();

#endif
