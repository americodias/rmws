/*! \file **********************************************************************
 *
 * \brief
 *      Log file functions
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
#include <stdarg.h>
#include <pthread.h>

#include "defs.h"
#include "timestamp.h"

pthread_mutex_t     log_file_mutex;

void log_init(void)
{
    pthread_mutex_init(&log_file_mutex, NULL);
}
                    
void log_write(char *fmt, ... )
{
    va_list         list;
    char            *p, *r;
    int             e;
    FILE *fp;
    pthread_mutex_lock(&log_file_mutex);
    
    fp = fopen(LOG_FILE_NAME, "a");	

	if(fp != NULL) {
        timestamp(fp);
        fprintf(fp, " ");
        
        /* prepare list for va_arg */
        va_start( list, fmt );
 
        for ( p = fmt ; *p ; ++p )
        {
            /* check if we should later look for *
            * i (integer) or s (string) */
            if ( *p != '%' )
            {
                /* not a string or integer print *
                * the character to stdout */
                putc( *p, fp );
            } else {
                /* character was % so check the *
                * letter after it and see if it's *
                * one of s or i */
                switch ( *++p )
                {
                    /* string */
                    case 's':
                    {
                        /* set r as the next char *
                        * in list (string) */
                        r = va_arg( list, char * );
 
                        /* print results to stdout */
                        fprintf(fp, "%s", r);
                        continue;
                    }
 
                    /* integer */
                    case 'i':
                    {
                        /* set e as the next char *
                        * in list (integer) */
                        e = va_arg( list, int );
 
                        /* print results to stdout */
                        fprintf(fp, "%i", e);
                        continue;
                    }
 
                    default: putc( *p, fp );
                }
            }
        }
        va_end( list );
        fprintf(fp, "\n");
        fflush( fp );
        fclose(fp);
    }
    
    pthread_mutex_unlock(&log_file_mutex);
    
    return;
}

