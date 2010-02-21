/*   This file is prepared for Doxygen automatic documentation generation   */
/*! \file ********************************************************************
 *
 * \brief
 *      TCP/IP connection handler
 * 
 * $Id: connection.c 4 2010-01-10 22:10:13Z adias $
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <unistd.h>
#include "sec_connection.h"

int                 sec_sockfd,
                    sec_newsockfd,
                    sec_portno;
struct sockaddr_in  sec_serv_addr, 
                    sec_cli_addr;

extern void strtoupper(char *str);

int sec_connection_open(int port)
{
    int bind_result;
    
    sec_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sec_sockfd < 0) 
        return sec_sockfd;
        
    bzero((char *) &sec_serv_addr, sizeof(sec_serv_addr));
    sec_portno = port;
    sec_serv_addr.sin_family = AF_INET;
    sec_serv_addr.sin_addr.s_addr = INADDR_ANY;
    sec_serv_addr.sin_port = htons(sec_portno);
    
    bind_result = bind(sec_sockfd, (struct sockaddr *) &sec_serv_addr,sizeof(sec_serv_addr));
    
    if (bind_result < 0) 
        return bind_result;
    
    return 0;
}

int sec_connection_listen() 
{
    size_t clilen;

    listen(sec_sockfd,5);
    clilen = sizeof(sec_cli_addr);
    sec_newsockfd = accept(sec_sockfd, (struct sockaddr *) &sec_cli_addr, (socklen_t *)&clilen);
    
    if(sec_newsockfd < 0)
        return sec_newsockfd;
        
    return 0;
}

int sec_connection_close() 
{
    shutdown(sec_sockfd,2);
    close(sec_sockfd);
    return 0;
}

int sec_connection_write(char *buff, int num_bytes) 
{
	int result;
    result = write(sec_newsockfd, buff, num_bytes);
	return result;
}

int sec_connection_read(char *buff, int num_bytes) 
{
	int result;
	bzero(buff,num_bytes+1);
    result = read(sec_newsockfd, buff, num_bytes);
	strtoupper(buff);
	printf("<-- %s\n", buff);
	fflush(stdout);
	return result;
}

int sec_connection_get_client_name(void *str, int num_bytes) 
{
    size_t clilen;
    int result;
    struct hostent *host;
    
	bzero(str,num_bytes+1);
    clilen = sizeof(sec_cli_addr);
    
    result = getpeername(sec_newsockfd, (struct sockaddr *) &sec_cli_addr, (socklen_t *)&clilen);
    
    if (result < 0)
        return result;
    else {
        host = gethostbyaddr((char *) &sec_cli_addr.sin_addr, sizeof sec_cli_addr.sin_addr, AF_INET);
        
        if (host == NULL)
            return -1;
    }
    
    strncpy(str, host->h_name, num_bytes);
    
    return 0;
}
