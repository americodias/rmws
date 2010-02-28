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
 * $Date$
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
#include "connection.h"


int                 connection_status[2];
pthread_mutex_t     connection_mutex[2];
unsigned int		port[2];

void connection_sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
	fflush(stdout);
}

// get sockaddr, IPv4 or IPv6:
void *connection_get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void connection_strtoupper(char *str) 
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

void connection_set_status(connection_t type, int status)
{
    pthread_mutex_lock(&connection_mutex[type]);
    connection_status[type] = status;
    pthread_mutex_unlock(&connection_mutex[type]);
}

int connection_get_status(connection_t type)
{
    int result;
    
    pthread_mutex_lock(&connection_mutex[type]);
    result = connection_status[type];
    pthread_mutex_unlock(&connection_mutex[type]);
    
    return result;
}

void connection_init(int p) {
	port[_COMMAND] = p;
	port[_DATA] = p + 1;
    pthread_mutex_init(&connection_mutex[_COMMAND], NULL);
	pthread_mutex_init(&connection_mutex[_DATA], NULL);
	connection_set_status(_COMMAND, _READY);
	connection_set_status(_DATA, _READY);
    return;
}

void *connection_handler(void *arg) {
	connection *conn = arg;
	int result;
	char socket_buffer[SOCKET_BUFFER_SIZE];

    fd_set rfds;
    struct timeval tv;

    connection_set_status(conn->type, _BUSY);
	log_write("Command connection started on socket %i", *socket);
	
	if (send(conn->socket, CMD_READY, strlen(CMD_READY), 0) < 0)
		log_write("Connection error on command socket %i", conn->socket);
#ifdef _DEBUG
    else
		printf("--> %s\n", CMD_READY); fflush(stdout);
#endif
    
	if(conn->type == COMMAND) {
		while(1) {
			bzero(socket_buffer, SOCKET_BUFFER_SIZE);

	        FD_ZERO(&rfds);
	        FD_SET(conn->socket, &rfds);

	        tv.tv_sec = 30;
	        tv.tv_usec = 0;
    
		    result = select((int)(conn->socket+1), &rfds, NULL, NULL, &tv);

	        if (result < 0) {
	            perror("select");
	            break;
	        }
	        else if (result == 0){
	            log_write("Connection timeout on command socket %i", *socket);
	            break;
	        }

		    result = read(conn->socket, socket_buffer, SOCKET_BUFFER_SIZE);
			if(result < 0) {
			    log_write("Command connection error");
				break;
			}
		
			connection_strtoupper(socket_buffer);
	#ifdef _DEBUG		
			printf("<-- %s\n", socket_buffer); fflush(stdout);
	#endif
			result = command_parser(socket_buffer, &conn->socket);
		
			if(result == 1)
				break;
			else if(result < 0) {
				log_write("Connection error on command socket %i", conn->socket);
				break;
			}
		}
	}
	else if (conn->type == DATA) {
		while(1) {
		    if(connection_get_status(_DATA) == _READY)
		        break;
			if(uart_read() > 0) {
			    if(write(conn->socket, get_uart_buffer(), 
						strlen(get_uart_buffer())) < 0 ) {
			        log_write("Connection error on data socket %i", 
						conn->socket);
					break;
				}
			}
			usleep(SERIAL_PORT_DELAY);
		}
	}
	
	connection_set_status(_COMMAND, _READY);
	connection_set_status(_DATA, _READY);

	close(conn->socket);
	
	
	log_write("Command connection closed on socket %i", conn->socket);
	free(conn);
	
	return NULL;
		
}

void *connection_busy(void *arg) {
	connection *conn = arg;
	
	if (send(conn->socket, CMD_BUSY, strlen(CMD_BUSY), 0) < 0)
		log_write("Command connection error on socket %i", *socket);
    else {
        log_write("Command connection busy on socket %i", *socket);
#ifdef _DEBUG
		printf("--> %s\n", CMD_BUSY); fflush(stdout);
#endif
    }

	close(conn->socket);
	
	free(conn);
	
	return NULL;
		
}

void *connection_accept(void *arg) {
    connection_t *type = arg;
	
    char PORT[10];
	
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	connection *conn;

    sprintf(PORT, "%d", port[*type]);

    
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

	sa.sa_handler = connection_sigchld_handler; // reap all dead processes
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

		conn = calloc(1, sizeof(connection));
		conn->type = *type;
		conn->socket = new_fd;
		
		inet_ntop(their_addr.ss_family,
			connection_get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		
		conn->client = calloc(strlen(s), sizeof(char));
		memcpy(conn->client, s, strlen(s));
		
		log_write("New command connection from %s", s);			
		
		if(connection_get_status(*type) == 0) {
			pthread_create(&conn->tpid, NULL, connection_handler, conn);
		}
		else
			pthread_create(&conn->tpid, NULL, connection_busy, conn);
	}
	
    return NULL;
}


