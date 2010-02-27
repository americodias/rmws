/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      Command connection functions
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
#include "core51.h"
#include "command_conn.h"
#include "data_conn.h"

int                 command_connection;

pthread_mutex_t     command_connection_mutex;

void command_conn_sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
	fflush(stdout);
}

// get sockaddr, IPv4 or IPv6:
void *command_conn_get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void strtoupper(char *str) 
{
	while(*str) {
		if(*str != '\n' && *str != '\r')
			*str = toupper(*str);
		else
			*str = 0x00;
			
		str++;
	}
	return;
}

void command_conn_init(void) {
    command_connection = 0;
    pthread_mutex_init(&command_connection_mutex, NULL);
    return;
}

void *command_conn(void *arg) {
	int *socket = arg;
	int result;
	char socket_buffer[SOCKET_BUFFER_SIZE];

    fd_set rfds;
    struct timeval tv;

    command_conn_set_status(1);
	log_write("Command connection started on socket %i", *socket);
	
	if (send(*socket, CMD_READY, strlen(CMD_READY), 0) < 0)
		log_write("Connection error on command socket %i", *socket);
#ifdef _DEBUG
    else
		printf("--> %s\n", CMD_READY); fflush(stdout);
#endif
    
	while(1) {
		bzero(socket_buffer, SOCKET_BUFFER_SIZE);

        FD_ZERO(&rfds);
        FD_SET(*socket, &rfds);

        tv.tv_sec = 30;
        tv.tv_usec = 0;
    
	    result = select((int)(*socket+1), &rfds, NULL, NULL, &tv);

        if (result < 0) {
            perror("select");
            break;
        }
        else if (result == 0){
            log_write("Connection timeout on command socket %i", *socket);
            break;
        }

	    result = read(*socket, socket_buffer, SOCKET_BUFFER_SIZE);
		if(result < 0) {
		    log_write("Command connection error");
			break;
		}
		
		strtoupper(socket_buffer);
#ifdef _DEBUG		
		printf("<-- %s\n", socket_buffer); fflush(stdout);
#endif
		result = command_parser(socket_buffer, socket);
		
		if(result == 1)
			break;
		else if(result < 0) {
			log_write("Connection error on command socket %i", *socket);
			break;
		}
	}
	
	command_conn_set_status(0);
	data_conn_set_status(0);

	close(*socket);
	
	log_write("Command connection closed on socket %i", *socket);
	
	return NULL;
		
}

void *command_conn_busy(void *arg) {
	int *socket = arg;
	
	if (send(*socket, CMD_BUSY, strlen(CMD_BUSY), 0) < 0)
		log_write("Command connection error on socket %i", *socket);
    else {
        log_write("Command connection busy on socket %i", *socket);
#ifdef _DEBUG
		printf("--> %s\n", CMD_BUSY); fflush(stdout);
#endif
    }

	close(*socket);
	
	return NULL;
		
}

void *command_conn_accept(void *arg) {
	pthread_t command_conn_tpid;
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

    sprintf(PORT, "%d", arglist->port);
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
	    log_write("Command connection erro: %s", gai_strerror(rv));
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			log_write("Error creating command connection socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			log_write("Error changing connection socket options");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			log_write("Error binding command connection socket");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		log_write("Fatal error creating command connection socket");
		return (void *)2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		log_write("Error listening command connection socket");
		exit(1);
	}

	sa.sa_handler = command_conn_sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

    log_write("Accepting command connection");

    while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			command_conn_get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
			
		log_write("New command connection from %s", s);	
        
		if(command_conn_get_status() == 0) {
			pthread_create(&command_conn_tpid, NULL, command_conn, &new_fd);
		}
		else
			pthread_create(&command_conn_tpid, NULL, command_conn_busy, &new_fd);
	}
	
    return NULL;
}

void command_conn_set_status(int status)
{
    pthread_mutex_lock(&command_connection_mutex);
    command_connection = status;
    pthread_mutex_unlock(&command_connection_mutex);
}

int command_conn_get_status(void)
{
    int result;
    
    pthread_mutex_lock(&command_connection_mutex);
    result = command_connection;
    pthread_mutex_unlock(&command_connection_mutex);
    
    return result;
}

