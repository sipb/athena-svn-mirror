/********************************************************************/
/*                                                                  */
/*                          N I D L . H                             */
/*                                                                  */
/*                  Mainline for IDL compilers.                     */
/*                                                                  */
/********************************************************************/

/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 *
 * Apollo Computer Inc. reserves all rights, title and interest with respect 
 * to copying, modification or the distribution of such software programs and
 * associated documentation, except those rights specifically granted by Apollo
 * in a Product Software Program License, Source Code License or Commercial
 * License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
 * Licensee.  Without such license agreements, such software programs may not
 * be used, copied, modified or distributed in source or object code form.
 * Further, the copyright notice must appear on the media, the supporting
 * documentation and packaging as set forth in such agreements.  Such License
 * Agreements do not grant any rights to use Apollo Computer's name or trademarks
 * in advertising or publicity, with respect to the distribution of the software
 * programs without the specific prior written permission of Apollo.  Trademark 
 * agreements may be obtained in a separate Trademark License Agreement.
 * ========================================================================== 
 */



#include "ast.h"
     
#ifndef __PROTOTYPE
#ifdef __STDC__
#define __PROTOTYPE(x) x
#else
#define __PROTOTYPE(x) ()
#endif
#endif


    /********************************/
    /*  Initialize NIDL frontend    */
    /********************************/

void NIDL_init __PROTOTYPE((void)) ;

    /****************************************/              
    /*  Parse arguments to NIDL compiler    */
    /****************************************/              

char *NIDL_parse_args(
#ifdef __STDC__
    int   argc,
    char  **argv
#endif
    ) ;

    /****************************/
    /*  Parse a NIDL file       */
    /****************************/

binding_t *NIDL_parse(
#ifdef __STDC__
    char *source_file
#endif
    ) ;

    /****************************/
    /*  Generate header files   */
    /****************************/

void NIDL_gen_hdrs(
#ifdef  __STDC__
    binding_t *ast_p 
#endif
    ) ;

    /****************************/
    /*  Generate stubs          */
    /****************************/

void  NIDL_gen_stubs(
#ifdef  __STDC__
    binding_t *ast_p 
#endif
    ) ;
