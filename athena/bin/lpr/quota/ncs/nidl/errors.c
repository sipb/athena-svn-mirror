/********************************************************************/
/*                                                                  */
/*          IDC Error logging and reporting routines.               */
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
#include <string.h>
#include "idl_base.h"
#include "errors.h"
#include "utils.h"
#include "nametbl.h"
#include "sysdep.h"

#define MAX_LINE_LEN    256 
#define yyerfp          stderr

#ifndef __PROTOTYPE
#ifdef __STDC__
#define __PROTOTYPE(x) x
#else
#define __PROTOTYPE(x) ()
#endif
#endif


/*--------------------------------------------------------------------*/

typedef struct error_log_rec_t
{
    int     line_no;
    char    *message ;
    char    *format_string ;
    union
    {
        struct 
        {
            struct  error_log_rec_t *left ;
            struct  error_log_rec_t *right ;
        } asBinTree ;
        struct 
        {
            struct  error_log_rec_t *next ;
        } asList ;
    } links ;
    struct  error_log_rec_t *first_this_line ;
    struct  error_log_rec_t *last_this_line ;
} error_log_rec_t ;

/*--------------------------------------------------------------------*/

#define MAX_WARNINGS    5

static  int              warnings = 0 ;
static  error_log_rec_t  *errors = NULL;
extern  int              yylineno ;

static  int              last_error_line = 0 ;
static  char             *current_file   = NULL ;
        int              error_count     = 0 ;
extern  boolean          no_warnings ;

extern void exit __PROTOTYPE((int));
extern void sysdep_cleanup_temp __PROTOTYPE((void));    

/*--------------------------------------------------------------------*/

/*
 *  Function:  Returns current input position for yyparse.
 *
 *  Inputs:     
 *
 *  Outputs:
 *
 *  Globals: yytext, yyleng, yylineno
 *
 *  Functional value:
 *
 *  Notes:  This was adpated from the book 
 *              "Compiler Construction under Unix"
 *          by A. T. Schreiner & H. G. Friedman
 *
 */

extern char yytext[] ;              /* current token                */
extern int  yyleng;                 /* ...and length                */
extern int  yylineno;               /* current input line no.       */
extern FILE *yyin ;

static char *source = NULL; /* current input file name      */

#ifdef __STDC__
void yywhere(void)
#else
void yywhere()
#endif    
{
    boolean colon = false ;

    if (source && *source && strcmp(source,"\"\""))
    {
        char    *cp = source ;
        int     len = strlen(source) ;

        if (*cp == '"')
            ++cp, len -=2 ;
        if (strncmp(cp,"./",2) == 0)
            cp += 2, len -= 2;
        fprintf(yyerfp, "file %.*s", len, cp);
        colon = true ;
    }

    if (yylineno > 0) 
    {
        if (colon)
            fputs(", ",yyerfp) ;
        if (! feof(yyin)) 
            fprintf(yyerfp, 
                   "line %d of file %s ",
                    yylineno - (*yytext == '\n' || ! *yytext), current_file) ;
        else
            fprintf(yyerfp, 
                   "<eof> at line %d of file %s ",
                    yylineno - (*yytext == '\n' || ! *yytext), current_file) ;

        colon = true ;
    }

    if (*yytext)
    {
        int i ;

        for (i = 0; i < 20; ++i)
            if (!yytext[i] || yytext[i] == '\n')
                break ;
        if (i)
        {
            if (colon)
                putc(' ', yyerfp);
            fprintf(yyerfp, "near \"%.*s\"", i, yytext) ;
            colon = true ;
        }
    }
        
    if (colon)
        fputs(": ", yyerfp) ;
}


/*--------------------------------------------------------------------*/

/*
 *  Function:  Called by yypaser when a parse error is encountered.
 *
 *  Inputs:    message -  error message to display
 *             token    - expected token
 *
 *  Outputs:
 *
 *  Globals:    yynerrs ;
 *
 *  Functional value:
 *
 *  Notes:  This was adpated from the book 
 *              "Compiler Construction under Unix"
 *          by A. T. Schreiner & H. G. Friedman
 *
 */

#ifdef __STDC__
void yyerror (char *message)
#else
void yyerror(message)
        char    *message;
#endif
{
    extern int yynerrs ;
    static int list = 0 ;

    if (message || !list)
    {
        fprintf(yyerfp, "[error %d] ", yynerrs+1) ;
        yywhere() ;
        if (message)
        {
            fputs(message, yyerfp) ;
            putc('\n', yyerfp) ;
            return ;
        }

        fputs("syntax error\n", yyerfp) ;
        return ;
    }
    putc('\n', yyerfp) ;
    list = 0 ;
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
error_log_rec_t *alloc_log_rec(line_no, format_string, message)
        int     line_no;
        char    *message ;
        char    *format_string ;
#else
error_log_rec_t *alloc_log_rec(int line_no,  char *format_string, char *message)
#endif
{
        error_log_rec_t *log_rec_p ;

    log_rec_p = (error_log_rec_t *) alloc(sizeof(error_log_rec_t)) ;

    log_rec_p->line_no  = line_no  ;
    log_rec_p->message  = message  ;
    log_rec_p->format_string = format_string ;

    log_rec_p->first_this_line = 0 ;
    log_rec_p->last_this_line  = 0 ;

    log_rec_p->links.asBinTree.left  = 0 ;
    log_rec_p->links.asBinTree.right = 0 ;

    log_rec_p->links.asList.next     = 0 ; 

    return  log_rec_p ;
}

/*--------------------------------------------------------------------*/

/*
 *  Function:   Adds an error message to the end of a list of
 *              errors queued for a particular line.
 *
 *  Inputs:     log_rec_p - a pointer to the header error log
 *              rec.
 *
 *              message   - the error message.
 *
 *  Outputs:
 *
 *  Functional value:
 *
 */

#ifndef __STDC__
void queue_error(log_rec_p, format_string, message)
        struct error_log_rec_t *log_rec_p;
        char            *message ;
        char            *format_string ;
#else
void queue_error(struct error_log_rec_t * log_rec_p, char *format_string, char *message)
#endif
{     
        error_log_rec_t    *new_log_rec_p ;
    new_log_rec_p = alloc_log_rec(log_rec_p->line_no, format_string, message) ;

    if (log_rec_p->first_this_line == NULL)
    {
        log_rec_p->first_this_line = (struct error_log_rec_t *) new_log_rec_p ;
        log_rec_p->last_this_line  = (struct error_log_rec_t *) new_log_rec_p ;
    
        return ;
    }

    log_rec_p->last_this_line->links.asList.next = (struct error_log_rec_t *) new_log_rec_p ;
    log_rec_p->last_this_line                    = (struct error_log_rec_t *) new_log_rec_p ;
}


/*--------------------------------------------------------------------*/
                        
/*
 *  Function:   Adds an error log to the sorted binary tree of
 *              error messages.
 *
 *  Inputs:     log_rec_p - pointer to current root of tree.
 *              lineno    - line number on which error occurred.
 *              message   - the error message.
 *
 *  Outputs:
 *
 *  Functional value:
 *
 */
       
#ifndef __STDC__                
void add_error_log_rec(log_rec_p, lineno, format_string, message) 
        error_log_rec_t     *log_rec_p ;
        int                 lineno;
        char                *message ;
        char                *format_string ;
#else
void add_error_log_rec(error_log_rec_t *log_rec_p, int lineno, char *format_string, char *message) 
#endif
{
    if (log_rec_p->line_no < lineno)
    {
        if (log_rec_p->links.asBinTree.right != NULL) 
            add_error_log_rec(log_rec_p->links.asBinTree.right, lineno, format_string, message)  ;
        else
            log_rec_p->links.asBinTree.right = alloc_log_rec(lineno, format_string, message) ;
        return ;
    }

    if (log_rec_p->line_no > lineno)
    {
        if (log_rec_p->links.asBinTree.left != NULL) 
            add_error_log_rec(log_rec_p->links.asBinTree.left, lineno, format_string, message)  ;
        else
            log_rec_p->links.asBinTree.left = alloc_log_rec(lineno, format_string, message) ;
        return ;
    }

    if (log_rec_p->line_no == lineno)
        queue_error(log_rec_p, format_string, message) ;
}


/*--------------------------------------------------------------------*/

/*
 *  Function: Accumulates an error message for later printout.
 *            All accumulated errors are printed by log_print.
 *            Errors are kept sorted by line number.
 *
 *  Inputs:   lineno  - the line number of the error.
 *            message - the error message.
 *
 *  Outputs:  
 *
 *  Functional Value:
 *
 */

#ifdef __STDC__
void log_error(int lineno, char *format_string, char *message)
#else
void log_error(lineno, format_string, message)
    int     lineno;
    char    *message ;
    char    *format_string ;
#endif
{
    ++ error_count ;

    if (errors == NULL)
        errors = alloc_log_rec(lineno, format_string,message) ;
    else
        add_error_log_rec(errors, lineno, format_string, message);

}

/*--------------------------------------------------------------------*/

/*
 *  Function: Accumulates a warning message for later printout.
 *            All accumulated errors are printed by log_print.
 *            Errors are kept sorted by line number.
 *
 *  Inputs:   lineno  - the line number of the error.
 *            message - the error message.
 *
 *  Outputs:  
 *
 *  Functional Value:
 *
 */

#ifdef __STDC__
void log_warning(int lineno, char *format_string, char *message)
#else
void log_warning(lineno, format_string, message)
    int     lineno;
    char    *message ;
    char    *format_string ;
#endif
{   
    if (no_warnings)
        return ;
    ++warnings ;
    if (errors == NULL)
        errors = alloc_log_rec(lineno, format_string,message) ;
    else
        add_error_log_rec(errors, lineno, format_string, message);
}

/*--------------------------------------------------------------------*/

/*
 *  Function:   Reads the line specified by lineno .
 *
 *  Inputs:     source - the file descriptor for the source file.
 *
 *              lineno - the number of the line in error.
 *
 *  Outputs:    source_line - the source line is returned through here
 *
 *  Function value:
 *
 *  Globals:    last_error_line
 *
 */

#ifndef __STDC__
void seek_for_line(source_file, line_no, source_line)
    FILE    *source_file;
    int     line_no ;
    char    *source_line ;
#else
void seek_for_line(FILE *source_file, int line_no, char *source_line)
#endif
{
    int lines_to_skip ;
    int i ;

    lines_to_skip = line_no - last_error_line ;
                                                 
    for (i=0; i<lines_to_skip; i++)
        (void) fgets(source_line, MAX_LINE_LEN, source_file) ;
    last_error_line = line_no ;
}


/*--------------------------------------------------------------------*/

/*
 *  Function:   Prints out a source line and accumulated errors
 *              for that line.
 *
 *  Inputs:     log_rec_ptr - a pointer to the header log rec
 *              for the line.
 *
 *  Outputs:
 *
 *  Functional value:
 *
 */

#ifndef __STDC__
void print_errors_for_line(fd, source, log_rec_ptr)
        FILE            *fd ;
        error_log_rec_t *log_rec_ptr ;
        char            *source ;
#else
void print_errors_for_line(FILE *fd, char *source, error_log_rec_t *log_rec_ptr)
#endif
{

        char            source_line[MAX_LINE_LEN] ;
        error_log_rec_t *erp ;

    seek_for_line(fd, log_rec_ptr->line_no, source_line) ;
        
    fprintf(yyerfp, "\nfile %s: ( %d ) %s**** ", source, log_rec_ptr->line_no, source_line) ;
    fprintf(yyerfp, log_rec_ptr->format_string, log_rec_ptr->message) ;

    for(erp=(error_log_rec_t *) log_rec_ptr->first_this_line;erp;erp=erp->links.asList.next) {
        fprintf(yyerfp, "**** ") ;
        fprintf(yyerfp, erp->format_string, erp->message) ;
    }
}


/*--------------------------------------------------------------------*/

/*
 *  Function:   Recursively prints all accumulated error messages.
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Functional value:
 *
 */
#ifdef __STDC__
void print_error_messages(FILE *fd, char *source, error_log_rec_t *log_rec_ptr)
#else
void print_error_messages(fd, source, log_rec_ptr)
        FILE            *fd ;
        char            *source ;
        error_log_rec_t *log_rec_ptr ;
#endif
{
    if (log_rec_ptr->links.asBinTree.left != NULL)
        print_error_messages(fd, source, log_rec_ptr->links.asBinTree.left) ;

    print_errors_for_line(fd, source, log_rec_ptr) ;

    if (log_rec_ptr->links.asBinTree.right != NULL)
        print_error_messages(fd, source, log_rec_ptr->links.asBinTree.right) ;
}

/*--------------------------------------------------------------------*/

/*
 *  Function:   Prints all accumulated error messages.
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Functional value:
 *
 */

#ifdef __STDC__
boolean print_errors(STRTAB_str_t source)
#else
boolean print_errors(source)
    STRTAB_str_t    source ;
#endif
{
       
    FILE    *fd ;
    char    *fn ;

    if (errors)
    {                
                                 
        STRTAB_str_to_string(source, &fn) ;
        fd = fopen(fn,"r") ;
        print_error_messages(fd, fn, errors) ;
        errors          = NULL ;
        last_error_line = 0 ;
        return true ;
    }

    return false ;
}

/*--------------------------------------------------------------------*/

/*
 *  Function:  Prints the specifed error message and terminates the program
 *
 *  Inputs:    message - the message to be printed.
 *
 *  Outputs:  
 *
 *  Globals:  yylineno
 *
 *  Functional value:
 *
 *  Notes:      This call terminates the calling program with a 
 *              status of -1.
 *
 */

#ifdef __STDC__
void error(char *message)
#else
void error(message)
char *message;
#endif
{    
    if (current_file)
        fprintf(yyerfp,"File %s: %s\n", current_file, message) ;
    else
        fprintf(yyerfp,"%s\n", message) ;

#ifndef HASPOPEN
    sysdep_cleanup_temp();
#endif
    exit(-1) ;
}

/*--------------------------------------------------------------------*/

/*
 *  Function:  Prints the specifed error message. Terminates if the
 *             error count excees the threshold.
 *
 *  Inputs:    message - the message to be printed.
 *
 *  Outputs:  
 *
 *  Globals:  yylineno
 *
 *  Functional value:
 *
 *  Notes:      This call terminates the calling program with a 
 *              status of -1.
 *
 */

#ifdef __STDC__
void warning(char *message)
#else
void warning(message)    
    char    *message ;
#endif
{
    fprintf(yyerfp,"error: %s on line %d of file %s\n", message, yylineno, current_file) ;
    if (++warnings > MAX_WARNINGS)
        exit(-2) ;
}

/*--------------------------------------------------------------------*/

/*
 *  Function:   Records the name of the file being processed.  This name
 *              will be prepended onto error messages.
 *
 *  Inputs:     name - a pointer to the file name.
 *
 *  Outputs:
 *
 *  Functional value:
 */

#ifdef __STDC__
void set_name_for_errors(char *file_name)
#else
void set_name_for_errors(file_name)
        char    *file_name ;
#endif
{
    current_file = file_name ;
}

/*--------------------------------------------------------------------*/

/*
 *  Function:   Returns the name of the file being processed. 
 *             
 *
 *  Inputs:    
 *
 *  Outputs:   name - the file name is copied through this.
 *
 *  Functional value:
 */

#ifdef __STDC__
void    inq_name_for_errors(char *name)
#else
void    inq_name_for_errors(name)
        char    *name ;
#endif
{
    if (current_file)
        strcpy(name, current_file) ;
    else
        *name = '\0' ;
}
