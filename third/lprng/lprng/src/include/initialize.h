/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: initialize.h,v 1.2 2001-03-07 01:20:08 ghudson Exp $
 ***************************************************************************/



#ifndef _INITIALIZE_H
#define _INITIALIZE_H

/* PROTOTYPES */

void Initialize( int argc, char *argv[], char *envp[] ); 
void Setup_configuration( void );
char *Get_user_information( void );
 
#endif
