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

#ifdef vms
#  include <types.h>
#  include <stat.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include <stdio.h>
#include <string.h>

#include "idl_base.h"
#include "errors.h"
#include "files.h"
#include "nametbl.h"
#include "sysdep.h"

char    message_buf[388];

extern stat __PROTOTYPE((char *path, struct stat *info));
    

/*
 *
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
 *
 */

FileType file_type (pathname)
char   *pathname;
{
        struct stat fileInfo;

        if (stat (pathname, &fileInfo) == -1) {
                sprintf (message_buf, "Name not found: %s", pathname);
                error (message_buf);
        }

        switch (fileInfo.st_mode & S_IFMT) {
        case S_IFDIR: 
                return FileDirectory;

        case S_IFREG: 
                return FileFile;
        }

        return FileSpecial;
}


/*
 *
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
 *
 */
void parse_path_name (pathname, directory, leafname, extension)
STRTAB_str_t pathname;
char   *directory,
*leafname,
*extension;
{
#if defined(HASDIRTREE)
        char   *pn;
        int     pnLen,
        leafLen;
        int     i,
        j;
        int     leafStart,
        extStart;
        int     dirEnd,
        leafEnd;
        boolean slashSeen, dotSeen;

        /* 
          *  Scan backwards looking for a BRANCHCHAR
          *  IF not found, then no directory was specified.
          *
  */

        STRTAB_str_to_string (pathname, &pn);
        pnLen = strlen (pn);
        slashSeen = false;
        dirEnd = -1;
        leafStart = 0;
        dotSeen = false;
        if (directory)
                directory[0] = '\0';
        if (leafname)
                leafname[0] = '\0';
        if (extension)
                extension[0] = '\0';

        for (i = pnLen - 1; i >= 0; i--)
                if (pn[i] == BRANCHCHAR) {
                        leafStart = i + 1;
                        dirEnd = i > 0 ? i : 1;
                        slashSeen = true;
                        break;
                }

        if (directory) {
                if (slashSeen) {
                        strncpy (directory, pn, dirEnd);
                        directory[dirEnd] = '\0';
                }
                else
                        directory[0] = '\0';
        }

        /* 
         *  Start scanning from the BRANCHCHAR for a '.' to find the leafname.
         * 
         */

        extStart = pnLen;
    leafEnd = pnLen;
        for (j = pnLen; j > leafStart; --j)
                if (pn[j] == '.') {
                        leafEnd = j - 1;
                        extStart = j + 1;
                        dotSeen = true;
                        break;
                };

        if (leafEnd >= dirEnd + 1) {
                leafLen = dotSeen ? leafEnd - leafStart + 1 : leafEnd - leafStart;
                if (leafname) {
                        strncpy (leafname, &pn[leafStart], leafLen);
                        leafname[leafLen] = '\0';
                }
                if (!dotSeen) {
                        if (extension)
                                extension[0] = '\0';
                        return;
                }
                else {
                        if (extension)
                                strcpy (extension, &pn[extStart]);
                };
        }
#else
    error("Function not implemented for non-unix systems. File: %s, line: %d", __FILE__, __LINE__);
#endif
}

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

boolean contains_ev_ref (pathName)
STRTAB_str_t pathName;
{
        char   *pn;
        int     i;

        STRTAB_str_to_string (pathName, &pn);
        for (i = 0; i < strlen (pn) - 1; i++)
                if (pn[i] == '$' && pn[i + 1] == '(')
                        return true;
        return false;
}

/*  
 *
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
 *
 */

void FormPathName (dirPath, leafname, extension, resultantPath)
char   *dirPath,
*leafname,
*extension;
char   *resultantPath;
{     
#ifdef HASDIRTREE
        strcpy (resultantPath, dirPath);
#ifndef VMS
        strcat (resultantPath, BRANCHSTRING);
#endif
        strcat (resultantPath, leafname);
    if (extension == NULL)
        return ;
        strcat (resultantPath, ".");
        strcat (resultantPath, extension);
#else
    error("Function not implemented for non-unix systems. File: %s, line: %d", __FILE__, __LINE__);
#endif 
}

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
 *
 */

FILE * OpenOrFail (pathname, openFlags)
char   *pathname;
char   *openFlags;
{
        FILE * fd;

        /* 
          * Try to open the specified path.
          */

        if ((fd = fopen (pathname, openFlags)) == 0) {
                /* Open failed.  issue message and bomb out */

                sprintf (message_buf, "Unable to open %s for %s access\n", pathname, openFlags);
                error (message_buf);
        }

        return fd;
}

/*--------------------------------------------------------------------*/

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

boolean lookup(file_name, idir_list, stat_buf, new_path)
STRTAB_str_t    file_name ;
char    **idir_list ;
struct stat *stat_buf ;
char    *new_path ;
{
#ifdef HASDIRTREE
    int    i ;
    char   *fn ;

    STRTAB_str_to_string(file_name, &fn) ;

    /* if no list of dirs then use current dir only and 
       treat absolute pathnames absolutely */
    if ((idir_list[0] == NULL) || (fn[0] == BRANCHCHAR)) {
        if (stat(fn, stat_buf) != -1) {
            strcpy(new_path, fn) ;
            return true ;
        }
        else
            return false ;
    }

    /* lookup other pathnames in the dirs in the idir_list */
    STRTAB_str_to_string(file_name, &fn) ;
    for (i = 0; idir_list[i]; i++) {
        FormPathName(idir_list[i], fn, (char *)NULL, new_path);
        if (stat(new_path, stat_buf) != -1)
            return true;
    }
    return false ;
#else
    error("Function not implemented for non-unix systems. File: %s, line: %d", __FILE__, __LINE__);
#endif

}
