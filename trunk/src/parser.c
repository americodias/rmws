/*   This file is prepared for Doxygen automatic documentation generation     */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parser.h"
#include "core51.h"
#include "defs.h"

int send_ack(int *socket) {
    return write(*socket, CMD_ACKNOWLEDGE, strlen(CMD_ACKNOWLEDGE));
}

int send_nack(int *socket) {
	return write(*socket, CMD_NOT_ACKNOWLEDGE, strlen(CMD_NOT_ACKNOWLEDGE));
}

int command_test(char *string, char *command) {
    if(strlen(string) < strlen(command))
        return 0;

    return (strncmp(string, command, strlen(command)) == 0) ? 1 : 0;
}

int command_parser(char *string, int *socket) 
{
    static int program_status = 0;
    
	FILE *fp;
	int n;
	
	/*
	 * Command exit:
	 * Close the connection!
	 */
    if(command_test(string, CMD_EXIT)) {
		if((n=send_ack(socket)) < 0) return n;
		sleep(1);
        return 1;
    }
    /*
     * Command program:
     * Receive the hexadecimal file from the TCP/IP connection and sent it
     * to the FILE_NAME in the system temporary directory.
     * If the file already exist, it is rewritten.
     */
	else if(command_test(string, CMD_PROGRAM)) {
		fp = fopen(HEX_FILE_NAME, "w");
		
		if(fp == NULL)
			return -1;
		
		fclose(fp);
		
		program_status = 1;
		
		if((n=send_ack(socket)) < 0) return n;
	}
    /*
     * Command end program:
     * Receive the hexadecimal file from the TCP/IP connection and sent it
     * to the FILE_NAME in the system temporary directory.
     * If the file already exist, it is rewritten.
     */
	else if(command_test(string, CMD_END_PROGRAM)) {				
		program_status = 0;
		
		if((n=send_ack(socket)) < 0) return n;
	}
	/*
	 * Command ping:
	 * Just for testing... answer with a "PONG".
	 */
    else if(command_test(string, CMD_PING)) {
        n = write(*socket, CMD_PONG, strlen(CMD_PONG));
        if (n < 0)
			return n;
    }
    /*
     * Command program hex:
     * Send the file in the temporary directory to the CORE51 board via serial
     * port.
     */
    else if(command_test(string, CMD_PROGRAM_HEX)) {
		
        core51_sendhex(HEX_FILE_NAME);
        if((n=send_ack(socket)) < 0) return n;
    }
    /*
     * Command set key:
     * Set the specified key
     */
    else if(command_test(string, CMD_SET_KEY)) {
        if(strlen(string) > strlen(CMD_SET_KEY)) {
            if(srkey(1,string[strlen(CMD_SET_KEY)]) < 0) {
                if((n=send_nack(socket)) < 0) return n;
            }
            else {
                if((n=send_ack(socket)) < 0) return n;
            }
        }
        else {
            if((n=send_nack(socket)) < 0) return n;
        }
    }
    /*
     * Command clear key:
     * Clears the specified key
     */
    else if(command_test(string, CMD_CLEAR_KEY)) {
        if(strlen(string) > strlen(CMD_SET_KEY)) {
            if(srkey(0,string[strlen(CMD_SET_KEY)]) < 0) {
                if((n=send_nack(socket)) < 0) return n;
            }
            else {  
                if((n=send_ack(socket)) < 0) return n;
            }
        }
        else {
            if((n=send_nack(socket)) < 0) return n;
        }
    }
    /*
     * Command set reset:
     * Set the reset pin
     */
    else if(command_test(string, CMD_SET_RESET)) {
        srreset(1);
        if((n=send_ack(socket)) < 0) return n;
    }
    /*
     * Command clear reset:
     * Clears the reset pin
     */
    else if(command_test(string, CMD_CLEAR_RESET)) {
        srreset(0);
        if((n=send_ack(socket)) < 0) return n;
    }
    /*
     * Command set interrupt:
     * Set the interrupt pin
     */
    else if(command_test(string, CMD_SET_INTERRUPT)) {
        srint(1);
        if((n=send_ack(socket)) < 0) return n;
    }
    /*
     * Command clear interrupt:
     * Clears the interrupt pin
     */
    else if(command_test(string, CMD_CLEAR_INTERRUPT)) {
        srint(0);
        if((n=send_ack(socket)) < 0) return n;
    }
    /*
     * Command run:
     * Send the RUN command to the CORE51 bootloader
     */ 
    else if(command_test(string, CMD_RUN_HEX)) {
        core51_run();
        if((n=send_ack(socket)) < 0) return n;
    }
    /*
     * Command abort:
     * Send the ABORT command to the CORE51 bootloader
     */ 
    else if(command_test(string, CMD_ABORT_HEX)) {
        core51_abort();
        if((n=send_ack(socket)) < 0) return n;
    }
    /*
     * Program mode:
     * If the program mode is active, send the data to the program file
     */ 
    else if(program_status == 1) {
		
		fp = fopen(HEX_FILE_NAME, "a");
		
		if(fp == NULL)
			return -1;
			
		n = fwrite(string, sizeof(char), strlen(string), fp);
		fputc('\n', fp);
		
		fclose(fp);
		
		if(n < 0)
			return n;
			
		if((n=send_ack(socket)) < 0) return n;
			
    }
    /*
     * Else:
     * Command or data not understood. Send a NOT ACKOWLEDGE.
     */ 
	else {
		if((n=send_nack(socket)) < 0) return n;
	}

	return 0;
}

