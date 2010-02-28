/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      Timestamp functions
 * 
 * \author
 *      Américo Dias <americo.dias@fe.up.pt>
 *
 * $Revision$
 * $Date$
 * $Id$
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

# include "timestamp.h"


double cpu_time(void)
{
    double value;

    value = (double) clock () / (double) CLOCKS_PER_SEC;
    return value;
}


void timestamp(FILE *fd)
{
    static char time_buffer[TIME_SIZE];
    const struct tm *tm;
    size_t len;
    time_t now;

    now = time ( NULL );
    tm = localtime ( &now );

    len = strftime ( time_buffer, TIME_SIZE, "[%d %B %Y %I:%M:%S %p]", tm );

    fprintf(fd, "%s", time_buffer);

    return;
}


char *timestring (void)
{
    const struct tm *tm;
    size_t len;
    time_t now;
    char *s;
    
    now = time ( NULL );
    tm = localtime ( &now );
    s = calloc(TIME_SIZE, sizeof(char));

    len = strftime ( s, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

    return s;
}
