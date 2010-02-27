/*   This file is prepared for Doxygen automatic documentation generation     */
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "gpio.h"

// This just inserts a specific value into a given file
int set_file(char *path, char *fn, char *value, int len) {
	char *afn = malloc(strlen(path)+strlen(fn)+2); // '/' and '\0'
	int i, fh;

	sprintf(afn,"%s/%s",path,fn);
	fh = open(afn,O_WRONLY);
	if(fh<0) return fh;
	i = write(fh,value,len);
	close(fh);
	return i;
}

//
// This will set up the given port, using the avr32-specific gpio_dev-interface
//
// TODO check wether the port is really enabled!
//
#define PORTDIR "/config/gpio/"

int setup_port(char *label, char port, int mask, int flags) {
	char * fn  = malloc(strlen(PORTDIR)+strlen(label) + 1);
	char * buf = malloc(12);
	int i;

	strncpy(fn,PORTDIR,strlen(PORTDIR));
	strncpy(fn+strlen(PORTDIR),label,strlen(label)+1);
	// First create a setup-dir in the configfs
	/*fprintf(stderr,"Creating dir %s\n",fn);*/
	i = mkdir(fn,0700);
	if(i<0) return i;

	memset(buf,0,12);
	// Now set the port
	buf[0] =((port>='a')?(port-'a'):
		((port>='A')?(port-'A'):
		((port>='0')?(port-'0'):port))) + '0';
	i = set_file(fn,"gpio_id",buf,1);
	if(i<0) return i;
	// Set the correct mask
	buf[0] = '0'; buf[1] = 'x';
	for(i=7;i>=0;i--) {
		buf[i+2] = mask & 0x0F;
		buf[i+2] += (buf[i+2]<0x0A)?'0':('A'-0x0A);
		mask>>=4;
	}
	buf[10] = '\n';
	i = set_file(fn,"pin_mask",buf,12);
	if(i<0) return i;
	if((flags == O_WRONLY) || (flags == O_RDWR)) {
		i = set_file(fn,"oe_mask",buf,12);
		if(i<0) return i;
	}
	strncpy(buf,"1\n",3);
	i = set_file(fn,"enabled",buf,3);
	free(fn);
	free(buf);
	return i;
}

#define MAXBUF 100
#define DEVSTR "/dev/gpio0"
// This creates the corresponding devicefile and returns an handle to it.
int open_dev_file(int flags) {
	FILE *table;
	char *buf = malloc(MAXBUF); // Just one line of output
	char *name;
	int major,minor, i;

	/*
	 * The line read is eg:
	 *
	 * 0         1         2         3         4
	 * 0123456789012345678901234567890123456789012
	 * created gpio0 (port3/0x0000ffff) as (254:0)
	 *
	 * what we need is the name, the major and the minor-number
	 */
	table = popen("dmesg | grep \"created gpio\" | tail -n1","r");
	if(fgets(buf,MAXBUF,table) == NULL) return -1;
	
	name = malloc(strlen(DEVSTR)+1);
	strncpy(name,DEVSTR,strlen(DEVSTR)+1);
	strncpy(name+5,buf+8,5);

	sscanf(buf+36,"(%d:%d)",&major,&minor);
	mknod(name,S_IFCHR|0666,(major<<8)|minor);
	free(buf);
	i = open(name,flags);
	free(name);
	return i;
}


int init_gpio(char *label, char port, int mask, int flags) {
	int i;
	i = setup_port(label, port, mask, flags);
	if(i<0) return i;
	return open_dev_file(flags);
}

int destroy_gpio(char *label, int fh) {
	int i;
	char *fn = malloc(strlen(PORTDIR)+strlen(label)+1);

	strncpy(fn,PORTDIR,strlen(PORTDIR));
	strncpy(fn+strlen(PORTDIR),label,strlen(label)+1);
	// Close the filehandle
	close(fh);
	// Disable the port
	i = set_file(fn,"enabled","0\n",3);
	return rmdir(fn);
}
