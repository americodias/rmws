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
#ifndef __UART_H
#define __UART_H

/* 
 * with this struct the basic configuration of the serial line can be performed.
 *
 * For a more specific configuration, please see "man 3 tcsetattr".
 */
struct icnova_usart {
	unsigned long baudrate;
	char databits, stopbits;
	enum { US_PAR_NONE = 0, US_PAR_ODD, US_PAR_EVEN } parity;
	enum { US_CTL_NONE, US_CTL_SW } flow_control;
};

/*
 * This will open a serial line.
 *
 * Port is the number of the line to use.
 * the config-struct might be NULL, to use the default settings, otherwise
 * the given configuration will be set.
 *
 * This will return a filehandle which can be used with the default file
 * operation functions like read and write.
 */
int init_usart(int port,struct icnova_usart *config);

/*
 * This will set the basic values to a opened line
 *
 * handle is the file-handle returned by init_usart
 */
int setup_usart(int handle, struct icnova_usart *config);

#endif
