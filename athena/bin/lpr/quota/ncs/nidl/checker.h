/********************************************************************/
/*                                                                  */
/*                      C H E C K E R                               */
/*                                                                  */
/*             Semantic checker for IDL compilers                   */
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



#ifndef checker__incl
#define checker__incl
    /*  Initialize the semantic checker */

void CHECKER_init(
#ifdef __STDC__
    void
#endif
    ) ;

    /*  Perform semantic checking on an interface   */

void CHECKER_check_interface(
#ifdef __STDC__
    binding_t *binding_node_p,
    char **import_dirs, 
    char **def_strings
#endif
    ) ;

#endif
