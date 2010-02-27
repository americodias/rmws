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
#include "defs.h"
#include "log.h"
#include "core51.h"
#include "command_conn.h"
#include "data_conn.h"


int main(int argc, char *argv[]) {
    int opt;
    pthread_t command_conn_accept_tpid, data_conn_accept_tpid;
    struct args arglist;
    
    command_conn_init();
    data_conn_init();
    log_init();
    
    arglist.baudrate = DEFAULT_BAUDRATE;
    arglist.port = DEFAULT_PORT;
       
    opterr = 0;
    
    while ((opt = getopt (argc, argv, "b:hp:v")) != -1) {
        switch (opt)
        {
        case 'b':
            arglist.baudrate = strtoul(optarg, NULL, 0);
            break;
        case 'p':
            arglist.port = atoi(optarg);
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
    
    log_write("Initializing server");
    
    core51_destroy();
    
    if( core51_init(arglist.baudrate) < 0) {
        log_write("Fatal error: incorrect baudrate");
		return EXIT_FAILURE;
	}

    pthread_create(&command_conn_accept_tpid, NULL, command_conn_accept, &arglist);
    pthread_create(&data_conn_accept_tpid, NULL, data_conn_accept, &arglist);
    
    pthread_join(command_conn_accept_tpid,NULL);
    pthread_join(data_conn_accept_tpid,NULL);
    
    core51_destroy();
    
    log_write("Server exited");
    
    return EXIT_SUCCESS;
}

