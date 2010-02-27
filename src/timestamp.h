/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      Timestamp functions definitions
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
#ifndef __TIMESTAMP_H
#define __TIMESTAMP_H

#define TIME_SIZE 40

double cpu_time(void);
void timestamp(FILE *fd);
char *timestring(void);

#endif //__TIMESTAMP_H
