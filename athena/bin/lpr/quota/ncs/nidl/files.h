/********************************************************************/
/*                                                                  */
/*                      IDL Translator                              */
/*                                                                  */
/*                  File manipulation routines                      */
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

#ifndef files_incl
#define files_incl

#ifndef S_IFREG
#ifdef vms
#  include <types.h>
#  include <stat.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#endif
#endif

#include "nametbl.h"

typedef enum {FileDirectory, FileFile, FileSpecial} FileType ;

#define DirectorySpecified  0x0001
#define FileSpecified       0x0002
#define ExtensionSpecified  0x0004

#ifndef __PROTOTYPE
#ifdef __STDC__
#define __PROTOTYPE(x) x
#else
#define __PROTOTYPE(x) ()
#endif
#endif

/*
 *  Function:  Returns whether a pathname is a directory, a file, or something 
 *             else
 *
 *  Inputs:    pathname  - the pathname of interest.
 *
 *  Outputs:   
 *
 *  Globals:  
 *
 *  Functional value: one of FileDirectory, FileFile, FileSpecial
 */


FileType file_type(
#ifdef __STDC__
 char *pathname
#endif
);


/*
 *  Function:  Parses a specified pathanme into individual components.
 *
 *  Inputs:    pathname  - the pathname to parse.
 *
 *  Outputs:   directory - the directory portion or nil
 *
 *             leafname -  the leaf portion of the pathname or nil.
 *
 *             extension - the extension found or nil.
 *
 *  Globals:  
 *
 *  Functional value:
 */

void parse_path_name(
#ifdef __STDC__
        STRTAB_str_t pathname,
        char         *directory,
        char         *leafname,
        char         *extension   
#endif
) ;
 

/*  
 *  Function:  Forms a pathname from the specified components
 *
 *  Inputs:    dirPath  -  the directory portion of the pathname.
 *
 *             leafname -  the leaf portion of the pathname.
 *
 *             extension - the extension to tack on.
 *
 *  Outputs:   resultantPath - the newly minted pathname.
 *
 *  Globals:  
 *
 *  Functional value:
 */

void FormPathName(
#ifdef __STDC__
        char    *dirPath,
        char    *leafname, 
        char    *extension,
        char    *resultantPath  
#endif
);


/*
 *  Function:  Opens the specified file.  terminates program if not
 *             possible.
 *
 *  Inputs:    pathname - the name of the file to open.
 *
 *             openFlag - the unix mode in which to open it.
 *
 *  Outputs:  
 *
 *  Globals:  
 *
 *  Functional value: a descriptor for the specified file.
 */

FILE * OpenOrFail(                          
#ifdef __STDC__
        char    *pathname,
        char    *openFlags
#endif
) ;

/*
 *  Function:  Scans a pathname to see if it contains an 
 *             environment variable reference.
 *
 *  Inputs:     pathName - The path name to scan.
 *
 *  Outputs:
 *
 *  Functional value: Boolean
 *                      true:  The pathname contains an EV reference.
 *                      false: The pathname does not contain an EV ref.
 *
 */

boolean contains_ev_ref __PROTOTYPE((STRTAB_str_t pathName));

    
/*
 *  Function:   Looks for the specified file first in the
 *              working directory, and then in the list
 *              of specified directories.
 *
 *  Inputs:     file_name - the pathname of the file to be opened.
 *  
 *              dir_list - a list of directory pathnames
 *
 *  Outputs:    stat_buf : stat buffer of the file
 *
 *  Functional value - boolean: File was found.
 *
 */

boolean lookup __PROTOTYPE((STRTAB_str_t file_name, char **idir_list, struct stat *stat_buf, char *new_path));


#endif /* files_incl */
