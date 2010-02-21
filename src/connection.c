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
#include "connection.h"

int                 sockfd,
                    newsockfd,
                    portno;
struct sockaddr_in  serv_addr, 
                    cli_addr;

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

int connection_open(int port)
{
    int bind_result;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        return sockfd;
        
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = port;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    bind_result = bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
    
    if (bind_result < 0) 
        return bind_result;
    
    return 0;
}

int connection_listen() 
{
    size_t clilen;

    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
    
    if(newsockfd < 0)
        return newsockfd;
        
    return 0;
}

int connection_close() 
{
    shutdown(sockfd,2);
    close(sockfd);
    return 0;
}

int connection_write(char *buff, int num_bytes) 
{
	int result;
	printf("--> %s\n", buff);
	fflush(stdout);
    result = write(newsockfd, buff, num_bytes);
	return result;
}

int connection_read(char *buff, int num_bytes) 
{
	int result;
	bzero(buff,num_bytes+1);
    result = read(newsockfd, buff, num_bytes);
	strtoupper(buff);
	printf("<-- %s\n", buff);
	fflush(stdout);
	return result;
}

int connection_get_client_name(void *str, int num_bytes) 
{
    size_t clilen;
    int result;
    struct hostent *host;
    
	bzero(str,num_bytes+1);
    clilen = sizeof(cli_addr);
    
    result = getpeername(newsockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
    
    if (result < 0)
        return result;
    else {
        host = gethostbyaddr((char *) &cli_addr.sin_addr, sizeof cli_addr.sin_addr, AF_INET);
        
        if (host == NULL)
            return -1;
    }
    
    strncpy(str, host->h_name, num_bytes);
    
    return 0;
}
