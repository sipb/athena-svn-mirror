/********************************************************************/
/*                                                                  */
/*                          M A I N                                 */
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



#include <stdio.h>
#include <signal.h>
#ifdef vms
#  include <types.h>
#  include <stat.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include "fcntl.h"
#endif
#include "ast.h"
#include "nidl.h"
#include "utils.h"
#include "backend.h"

extern void exit __PROTOTYPE((int));
void main __PROTOTYPE((int argc, char **argv));
    
void main(argc, argv)
    int  argc ;
    char **argv ;
{                                
    char *source_file ;
    binding_t *ast_p ;

        /*  Initialize internal NIDL tables */
    NIDL_init() ;
        /*  Parse the supplied arguments    */

    source_file = NIDL_parse_args(argc-1, &argv[1]) ;
    
        /*  Parse the specified NIDL file   */
    ast_p = NIDL_parse(source_file) ;
        /*  Initialize the backend          */

    BACKEND_init() ;
    
        /*  Generate the header files       */
    NIDL_gen_hdrs(ast_p) ;

    /*  Generate the stubs              */
    NIDL_gen_stubs(ast_p) ;
    exit (pgm_$ok);
    /*lint -unreachable*/
}

