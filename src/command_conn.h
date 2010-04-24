/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      Command connection functions definitions
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
#ifndef __COMMAND_CONN_H
#define __COMMAND_CONN_H

void command_conn_init(void);
void *command_conn(void *arg);
void *command_conn_busy(void *arg);
void *command_conn_accept(void *arg);
void command_conn_set_status(int status);
int command_conn_get_status(void);
#endif
