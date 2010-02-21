/*   This file is prepared for Doxygen automatic documentation generation   */
/*! \file ********************************************************************
 *
 * \brief
 *      TCP/IP connection handler definitions
 *  
 * $Id: connection.h 3 2010-01-10 17:43:47Z adias $
 *
 ****************************************************************************/
#ifndef __CONNECTION_H
#define __CONNECTION_H

    
int connection_open(int port);
int connection_listen();
int connection_close();
int connection_write(char *buff, int num_bytes);
int connection_read(char *buff, int num_bytes);
int connection_get_client_name(void *str, int num_bytes);

#endif //__CONNECTION_H
