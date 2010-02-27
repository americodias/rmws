/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      General definitions
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
#ifndef __DEFS_H
#define __DEFS_H

#ifdef _DEBUG
#undef _DEBUG
#define _DEBUG              1       // Change to 0 to disable debug messages
#endif

#define DEFAULT_BAUDRATE    57600UL // Core51 serial port baudrate
#define DEFAULT_PORT        5000    // Default connection port for command
                                    // connection
#define BUFF_LENGHT		    512     
#define HOST_LENGHT		    256
#define MAX_CONNECTIONS     100     // Maximum number of connections
#define BACKLOG 10	                // How many pending connections queue 
                                    // will hold
#define SOCKET_BUFFER_SIZE	2048

#define HEX_FILE_NAME       "/tmp/core51.hex"       // Core51 HEX file
#define LOG_FILE_NAME       "/var/log/core51.log"   // Log file

#endif //__DEFS_H

