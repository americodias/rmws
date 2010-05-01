/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      Data connection functions
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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sys/un.h>
#include <netdb.h>
#include <sys/select.h>
#include "defs.h"
#include "log.h"
#include "parser.h"
#include "command_conn.h"
#include "data_conn.h"
#include "core51.h"

int                 data_connection;
pthread_mutex_t     data_connection_mutex;
    

void data_conn_sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
	fflush(stdout);
}
       
// get sockaddr, IPv4 or IPv6:
void *data_conn_get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
       
void data_conn_init(void) {
    data_connection = 0;
    pthread_mutex_init(&data_connection_mutex, NULL);
    return;
}

void *data_conn(void *arg) {
	int *socket = arg;
	
	data_conn_set_status(1);
	
	if (send(*socket, CMD_READY, strlen(CMD_READY), 0) < 0)
		log_write("Data connection error");
	
	while(1) {
	    if(data_conn_get_status() == 0)
	        break;
		if(uart_read() > 0) {
		    if(write(*socket, get_uart_buffer(), strlen(get_uart_buffer())) < 0 ) {
		        log_write("Data connection error");
				break;
			}
		}
		usleep(SERIAL_PORT_DELAY);
	}
	
	data_conn_set_status(0);
    
	close(*socket);
	return NULL;
		
}

void *data_conn_busy(void *arg) {
	int *socket = arg;
	
	if (send(*socket, CMD_BUSY, strlen(CMD_BUSY), 0) < 0)
		log_write("Command connection error");
		
	close(*socket);
	
	return NULL;
		
}

void *data_conn_accept(void *arg) {
	pthread_t data_conn_tpid;
    struct args *arglist = arg;
    char PORT[10];
	
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

    sprintf(PORT, "%d", arglist->port+1);
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return (void *)2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = data_conn_sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for data connections...\n");

    while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			data_conn_get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got data connection from %s\n", s);
        
		if(data_conn_get_status() == 0) {
			pthread_create(&data_conn_tpid, NULL, data_conn, &new_fd);
		}
		else
			pthread_create(&data_conn_tpid, NULL, data_conn_busy, &new_fd);
	}
	
    return NULL;
}

void data_conn_set_status(int status)
{
    pthread_mutex_lock(&data_connection_mutex);
    data_connection = status;
    pthread_mutex_unlock(&data_connection_mutex);
}

int data_conn_get_status(void)
{
    int result;
    
    pthread_mutex_lock(&data_connection_mutex);
    result = data_connection;
    pthread_mutex_unlock(&data_connection_mutex);
    
    return result;
}

