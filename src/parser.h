/*   This file is prepared for Doxygen automatic documentation generation   */
/*! \file ********************************************************************
 *
 * \brief
 *      Command parser definitions
 * 
 * $Id: parser.h 4 2010-01-10 22:10:13Z adias $
 *
 ****************************************************************************/
#ifndef __PARSER_H
#define __PARSER_H

#define CMD_EXIT 			"EXIT"
#define CMD_PING 			"PING"
#define CMD_PONG 			"PONG"
#define CMD_PROGRAM			"PROG"
#define CMD_END_PROGRAM		"ENDP"
#define CMD_ACKNOWLEDGE		"ACKN"
#define CMD_NOT_ACKNOWLEDGE "NACK"
#define CMD_PROGRAM_HEX     "PHEX"
#define CMD_RUN_HEX 		"RHEX"
#define CMD_ABORT_HEX       "AHEX"
#define CMD_SET_KEY         "SKEY"
#define CMD_SET_RESET       "SRST"
#define CMD_SET_INTERRUPT   "SINT"
#define CMD_CLEAR_KEY       "CKEY"
#define CMD_CLEAR_RESET     "CRST"
#define CMD_CLEAR_INTERRUPT "CINT"

#define TEXT_BUF_LEN	1024
#define FILE_NAME_LEN	512

#define FILE_NAME		"core51.hex"
int command_parser(char *command);


#endif //__PARSER_H

