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
#ifndef __CONNECTION_H
#define __CONNECTION_H

#include <pthread.h>
#define _BUSY		0
#define _READY		1
#define _COMMAND	0
#define _DATA		1

typedef enum
{
	BUSY = _BUSY,
	READY = _READY
} status_t;

typedef enum
{
	COMMAND = _COMMAND,
	DATA = _DATA
} connection_t;

typedef struct {
	connection_t type;
	int socket;
	char *client;
	pthread_t tpid;
} connection;

void connection_init(int p);
void *connection_handler(void *connection);
void *connection_busy(void *arg);
void *connection_accept(void *arg);
void connection_set_status(connection_t type, int status);
int connection_get_status(connection_t type);

#endif //__CONNECTION_H

