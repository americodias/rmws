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
#include "parser.h"
#include "core51.h"

#define DEFAULT_BAUDRATE    57600UL
#define DEFAULT_PORT        5000
#define BUFF_LENGHT		    512
#define HOST_LENGHT		    256
#define MAX_CONNECTIONS     100

#define BACKLOG 10	 // how many pending connections queue will hold

#define SOCKET_BUFFER_SIZE	2048
int data_connection = 0,
	command_connection;
	

struct args {
    unsigned long int baudrate;
	int port;
	int socket_fd;
	int slave;
};

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

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
	fflush(stdout);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* The signal handler function */
void handler( int signal ) {
    psignal( signal, "\nSignal");
    core51_destroy();
    exit(EXIT_SUCCESS);
}

void *command_conn(void *arg) {
	int *socket = arg;
	int result;
	char socket_buffer[SOCKET_BUFFER_SIZE];
	
	while(1) {
		bzero(socket_buffer, SOCKET_BUFFER_SIZE);
	    result = read(*socket, socket_buffer, SOCKET_BUFFER_SIZE);
		if(result < 0) {
			perror("data socket read");
			break;
		}
		strtoupper(socket_buffer);
		printf("<-- %s\n", socket_buffer); fflush(stdout);
		
		result = command_parser(socket_buffer, socket);
		
		if(result == 1)
			break;
		else if(result < 0) {
			perror("command parser");
			break;
		}
	}
	
	command_connection = 0;
	close(*socket);  // parent doesn't need this
	return NULL;
		
}

void *command_conn_busy(void *arg) {
	int *new_fd = arg;
	
	if (send(*new_fd, "BUSY", 4, 0) == -1)
		perror("send");
		
	close(*new_fd);  // parent doesn't need this
	
	return NULL;
		
}

void *data_conn(void *arg) {
	int *socket = arg;
	
	while(1) {
		if(uart_read() > 0) {
		    if(write(*socket, get_uart_buffer(), strlen(get_uart_buffer())) < 0 )
				break;
		}
		usleep(250000UL);
	}
	
	data_connection = 0;
	close(*socket);  // parent doesn't need this
	return NULL;
		
}

void *data_conn_busy(void *arg) {
	int *new_fd = arg;
	
	if (send(*new_fd, "BUSY", 4, 0) == -1)
		perror("send");
		
	close(*new_fd);  // parent doesn't need this
	
	return NULL;
		
}

void *accept_cmd_conn(void *arg) {
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

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for command connections...\n");

    while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got command connection from %s\n", s);

		if(command_connection == 0) {
			pthread_create(&command_conn_tpid, NULL, command_conn, &new_fd);
			command_connection = 1;
		}
		else
			pthread_create(&command_conn_tpid, NULL, command_conn_busy, &new_fd);
	}
	
    return NULL;
}

void *accept_data_conn(void *arg) {
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

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for command connections...\n");

    while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got command connection from %s\n", s);

		if(data_connection == 0) {
			pthread_create(&data_conn_tpid, NULL, data_conn, &new_fd);
			data_connection = 1;
		}
		else
			pthread_create(&data_conn_tpid, NULL, data_conn_busy, &new_fd);
	}
	
    return NULL;
}

int main(int argc, char *argv[]) {
    int opt;
    pthread_t accept_cmd_conn_tpid, accept_data_conn_tpid;
    struct args arglist;
    
    arglist.baudrate = DEFAULT_BAUDRATE;
    arglist.port = DEFAULT_PORT;
    
    /* Registering the handler, catching SIGINT signals */
    //signal( SIGINT, handler );
    
    opterr = 0;
    
    while ((opt = getopt (argc, argv, "b:p:h")) != -1) {
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
    
    core51_destroy();
    
    if( core51_init(arglist.baudrate) < 0) {
        perror("core51 initialization");
		return EXIT_FAILURE;
	}

    pthread_create(&accept_cmd_conn_tpid, NULL, accept_cmd_conn, &arglist);
    pthread_create(&accept_data_conn_tpid, NULL, accept_data_conn, &arglist);
    
    pthread_join(accept_cmd_conn_tpid,NULL);
    pthread_join(accept_data_conn_tpid,NULL);
    
    core51_destroy();
    
    return EXIT_SUCCESS;
}

