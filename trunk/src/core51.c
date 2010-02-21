#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "uart.h"
#include "gpio.h"
#include "connection.h"
#include "sec_connection.h"
#include "parser.h"

#define RST         0x01
#define INT         0x02
#define KEY4        0x04
#define KEY3        0x08
#define KEY2        0x10
#define KEY1        0x20

#define MAX_KEYS    4

#define GPIO_LABEL  "core51"

static int uart, gpio;
char *gpio_label = GPIO_LABEL;
unsigned char buf[4] = {0x00, 0x00, 0x00, 0x00};
unsigned char keys[4] = {KEY1, KEY2, KEY3, KEY4};

char uart_buffer[2048];


int init_uart(unsigned long int baudrate) {
    
    struct icnova_usart us_set = {
        .baudrate 	= baudrate,
		.databits 	= 8,
		.stopbits 	= 1,
		.parity		= US_PAR_NONE,
		.flow_control	= US_CTL_NONE,
	};
	
	uart = init_usart(1,&us_set); // Open and set the correct parameters.
	
	if(uart < 0)
	    return uart;

	return fcntl(uart, F_SETFL, FNDELAY);
}

int uart_read() {
    memset(uart_buffer, 0x00, 2048);
    return read(uart, uart_buffer, 2048);
}

char *get_uart_buffer() {
    return uart_buffer;
}

int core51_init(unsigned long int baudrate) {
    int result;
    unsigned long output_enable = 0x00000000UL;
    
    buf[0] = 0x00; buf[1] = 0x00; buf[2] = 0x00; buf[3] = 0x00;
    
    result = init_uart(baudrate);
    
    if(result < 0) {
        printf("Uart!\n");
		return uart;
	}
    
    output_enable |= RST + INT + KEY1 + KEY2 + KEY3 + KEY4;

    gpio = init_gpio(gpio_label,'D',output_enable,O_WRONLY);
    
    if(gpio < 0) {
        printf("GPIO!\n");
		return gpio;
	}
	
    // RESET = 1, INT = NOT_PRESSED, KEY = NOT_PRESSED
    buf[3] = 0x00;
    buf[3] |= RST;
    buf[3] |= INT + KEY4;
    buf[3] ^= 0x07;
    
    result = write(gpio,buf,4);
	if(result<0) {
		return result;
	}
	
	// 500mS RESET
    usleep(500000);
	
	// RESET = 0, INT = NOT_PRESSED, KEY = NOT_PRESSED
	buf[3] = 0x00;
	buf[3] &= ~RST;
    buf[3] |= INT + KEY4;
    buf[3] ^= 0x07;
    
    result = write(gpio,buf,4);
	if(result<0) {
		return result;
	}
	
	return 0;
}

int core51_destroy() {
    if(uart > 0)
        uart = close(uart);        
         
    gpio = destroy_gpio(gpio_label, gpio);
    
    return gpio;
}

int core51_sendhex(char *filename) {
    int result;

    char strbuf[256];

    FILE *fp;
    int n;

    buf[0] = 0x00; buf[1] = 0x00; buf[2] = 0x00; buf[3] = 0x00;
    
    // RESET = 1, INT = PRESSED, KEY = PRESSED
    buf[3] = 0x00;
    buf[3] |= RST;
    buf[3] ^= 0x07;
    
    result = write(gpio,buf,4);
	if(result<0) {
		return result;
	}
	
    usleep(500000);     // 500mS reset delay
    
    // RESET = 0, INT = PRESSED, KEY = PRESSED
    buf[3] = 0x00;
    buf[3] ^= 0x07;
    
    result = write(gpio,buf,4);
	if(result<0) {
		return result;
	}
	
	sleep(1);    // 1s delay
	
	// RESET = 0, INT = NOT_PRESSED, KEY = NOT_PRESSED
	buf[3] = 0x00;
    buf[3] |= (INT + KEY4);
    buf[3] ^= 0x07;
	
    result = write(gpio,buf,4);
	if(result<0) {
		return result;
	}
	
    fp = fopen(filename, "r");
		
    if(fp == NULL)
        return -1;

    while((n = fread ( strbuf, sizeof(unsigned char), 255, fp)) > 0) {
        write(uart, strbuf, n);
        //sec_connection_write(strbuf, n);
    }
    
    return n;
}

int core51_run(void) {
    char ch = 0x20;
    return write(uart, &ch, 1);
}

int core51_abort(void) {
    char ch = 0x1B;
    return write(uart, &ch, 1);
}

int srkey(unsigned char state, unsigned char ch) {
    
    // Numerial check (1-9)
    if(ch < 0x31 || ch > 0x39)
        return -1;
    
    ch -= 0x30;
    
    if(ch > MAX_KEYS)
        return -1;
    
    if(state) {
        buf[3] &= ~keys[ch-1];
    }
    else {
        buf[3] |= keys[ch-1];
    }
    
    return write(gpio,buf,4);
}

int srreset(unsigned char state) {
    
    if(state) {
        buf[3] &= ~RST;
    }
    else {
        buf[3] |= RST;
    }
    
    return write(gpio,buf,4);
}

int srint(unsigned char state) {
    
    if(state) {
        buf[3] &= ~INT;
    }
    else {
        buf[3] |= INT;
    }
    
    return write(gpio,buf,4);
}

