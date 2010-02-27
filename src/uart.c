/*! \file **********************************************************************
 *
 * \brief
 *      Command parser
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
#include "uart.h"
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define SETSPEED(x) case x: cfsetospeed(termios,B##x); cfsetispeed(termios,B##x); break;
int setup_usart(int fh, struct icnova_usart *config) {
	struct termios *termios = malloc(sizeof(struct termios));
	int i;

	if(config == NULL) return 0;
	tcgetattr(fh,termios);
	//cfsetispeed(termios,B0);
	switch(config->baudrate) {
#ifdef B230400
		SETSPEED(230400);
#endif
		SETSPEED(115200);
		SETSPEED(57600);
		SETSPEED(38400);
		SETSPEED(19200);
		SETSPEED(9600);
		SETSPEED(4800);
		SETSPEED(2400);
		SETSPEED(1800);
		SETSPEED(1200);
		SETSPEED(600);
		SETSPEED(300);
		SETSPEED(200);
		SETSPEED(150);
		SETSPEED(134);
		SETSPEED(110);
		SETSPEED(75);
		SETSPEED(50);
		SETSPEED(0);
	}
	switch(config->flow_control) {
		case US_CTL_NONE:
			termios->c_iflag &= ~(IXON|IXOFF);
			break;
		case US_CTL_SW:
			termios->c_iflag |= IXON|IXOFF;
			break;
		default:
		    break;
	}
	termios->c_cflag &= ~(PARENB|PARODD|CRTSCTS);
	termios->c_cflag |= (CLOCAL | CREAD | CS8);
    termios->c_lflag &= ~(ICANON | ECHO | ISIG);
    termios->c_oflag &= ~OPOST;


	switch(config->parity) {
		case US_PAR_ODD:
			termios->c_cflag |= PARODD;
			break;
		case US_PAR_EVEN:
			termios->c_cflag |= PARENB;
			break;
		default:
		    break;
	};
	if(config->stopbits == 2) {
		termios->c_cflag |= CSTOPB;
	} else {
		termios->c_cflag &= ~CSTOPB;
	}
	termios->c_cflag &= ~(CS5|CS6|CS7|CS8);
	switch(config->databits) {
		case 5:
			termios->c_cflag |= CS5; break;
		case 6:
			termios->c_cflag |= CS6; break;
		case 7:
			termios->c_cflag |= CS7; break;
		default:
			termios->c_cflag |= CS8; break;
	}
	i = tcsetattr(fh,TCSAFLUSH,termios);
	free(termios);
	return i;
}

#define PATH "/dev/ttyS0"

int init_usart(int port, struct icnova_usart *config) {
	char *fn = malloc(strlen(PATH));
	int ret;

	strncpy(fn,PATH,strlen(PATH));
	// Changing filename to the correct port
	fn[strlen(PATH)-1]+=port;
	ret = open(fn,O_RDWR | O_NOCTTY | O_NDELAY);
	free(fn);
	if(ret<0) return ret;
	setup_usart(ret,config);
	return ret;
}
