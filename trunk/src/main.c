#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "uart.h"
#include "gpio.h"
#include "connection.h"
#include "sec_connection.h"
#include "parser.h"
#include "core51.h"

#define DEFAULT_BAUDRATE    57600UL
#define DEFAULT_PORT        5000
#define BUFF_LENGHT		    512
#define HOST_LENGHT		    256

/* The signal handler function */
void handler( int signal ) {
    psignal( signal, "\nSignal");
    connection_close();
    core51_destroy();
    exit(EXIT_SUCCESS);
}

void error(char *msg)
{
    perror(msg);
    connection_close();
    core51_destroy();
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    unsigned long int baudrate = DEFAULT_BAUDRATE;
    unsigned int port = DEFAULT_PORT;
    char buffer[BUFF_LENGHT+1];
    //char client_name[HOST_LENGHT+1];
    int c;
    
    /* Registering the handler, catching SIGINT signals */
    signal( SIGINT, handler );
    
    opterr = 0;
    
    while ((c = getopt (argc, argv, "b:p:h")) != -1) {
        switch (c)
        {
        case 'b':
            baudrate = strtoul(optarg, NULL, 0);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case '?':
            if (optopt == 'b' || optopt == 'p')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
            fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
            exit(EXIT_FAILURE);
        case 'h':
            printf( "Usage: %s -b BAUDRATE\n", argv[0]);
            exit(EXIT_SUCCESS);
        default:
            abort ();
        }
    }
    
    c = core51_init(baudrate);
    if(c < 0)
        error("Fatal error on initialization");

    c = connection_open(port);
    
    if(c < 0)
        error("ERROR opening connection");

    c = sec_connection_open(port+1);

    if(c < 0)
        error("ERROR opening connection");
        
    while(1) {
        c = connection_listen();
        if(c < 0)
            error("ERROR listening connection");
        c = sec_connection_listen();
        if(c < 0)
            error("ERROR listening connection");
        /*
        c = connection_get_client_name(&client_name, HOST_LENGHT);
        if(c < 0)
            error("ERROR getting client name");        
        
        printf("New connection from '%s'.\n", client_name);
        */
                 
        while(1) {
            c = connection_read(buffer,BUFF_LENGHT);
            if (c < 0)
                error("ERROR reading from socket");

			c = command_parser(buffer);
			
			if(c == 1)
				break;
			else if(c < 0)
				error("Error processing command");
			
			if(uart_read() > 0) {
			    sec_connection_write(get_uart_buffer(), strlen(get_uart_buffer()));
			}
        }
    }

    connection_close();
    core51_destroy();
    return EXIT_SUCCESS; 

}

