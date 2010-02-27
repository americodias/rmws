/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      Data connection functions definitions
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
#ifndef __DATA_CONN_H
#define __DATA_CONN_H

void data_conn_init(void);
void *data_conn(void *arg);
void *data_conn_busy(void *arg);
void *data_conn_accept(void *arg);
void data_conn_set_status(int status);
int data_conn_get_status(void);

#endif //__DATA_CONN_H

