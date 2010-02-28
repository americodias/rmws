/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      Main file
 * 
 * \author
 *      Américo Dias <americo.dias@fe.up.pt>
 *
 * $Revision$
 * $HeadURL$
 * $Date$
 * $Author$
 * $Id$
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "defs.h"
#include "log.h"
#include "core51.h"
#include "connection.h"


int main(int argc, char *argv[]) {
    pthread_t command_conn_accept_tpid, data_conn_accept_tpid;
	unsigned long int baudrate;
	int port, opt;
    
    baudrate = DEFAULT_BAUDRATE;
    port = DEFAULT_PORT;
       
    opterr = 0;
    
    while ((opt = getopt (argc, argv, "b:hp:v")) != -1) {
        switch (opt)
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
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            exit(EXIT_FAILURE);
        case 'h':
            printf( "Usage: %s -b BAUDRATE -p PORT\n\n", argv[0]);
            
            printf( "  Options:\n");
            printf( "    -b  : Serial port baudrate (%ld)\n", DEFAULT_BAUDRATE);
            printf( "    -h  : Show this help\n");
            printf( "    -p  : Connection port (%d)\n", DEFAULT_PORT);
            printf( "    -v  : Show version information\n\n");
            exit(EXIT_SUCCESS);
        case 'v':
            printf("\nRemote Microcontroller Workbench Server\n");
            printf("(c) 2010 Américo Dias <americo.dias@fe.up.pt>\n\n");
            printf("Version %s (Revision %d)\n\n", VERSION, _REV);
            exit(EXIT_SUCCESS);
        default:
            abort ();
        }
    }
    
	log_init();
    log_write("Initializing server");
    connection_init(port);
    core51_destroy();
    
    if( core51_init(baudrate) < 0) {
        log_write("Fatal error: hardware initialization");
		return EXIT_FAILURE;
	}

    pthread_create(&command_conn_accept_tpid, NULL, connection_accept, (int *)(_COMMAND));
    pthread_create(&data_conn_accept_tpid, NULL, connection_accept, (int *)(_DATA));
    
    pthread_join(command_conn_accept_tpid,NULL);
    pthread_join(data_conn_accept_tpid,NULL);
    
    core51_destroy();
    
    log_write("Server exited");
    
    return EXIT_SUCCESS;
}

