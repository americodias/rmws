/*   This file is prepared for Doxygen automatic documentation generation   */
/*! \file ********************************************************************
 *
 * \brief
 *      TCP/IP connection handler definitions
 *  
 * $Id: connection.h 3 2010-01-10 17:43:47Z adias $
 *
 ****************************************************************************/
#ifndef __SEC_CONNECTION_H
#define __SEC_CONNECTION_H

int sec_connection_open(int port);
int sec_connection_listen();
int sec_connection_close();
int sec_connection_write(char *buff, int num_bytes);
int sec_connection_read(char *buff, int num_bytes);
int sec_connection_get_client_name(void *str, int num_bytes);

#endif //__SEC_CONNECTION_H
