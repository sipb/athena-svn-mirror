/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: initialize.h,v 1.1.1.3 1999-10-27 20:10:13 mwhitson Exp $
 ***************************************************************************/



#ifndef _INITIALIZE_H
#define _INITIALIZE_H

/* PROTOTYPES */

void Initialize( int argc, char *argv[], char *envp[] ); 
void Setup_configuration( void );
char *Get_user_information( void );
 
#endif
