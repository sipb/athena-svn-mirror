/* copyright (C) 2001 Sun Microsystems, Inc. and Dan Mueth*/

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <scrollkeeper.h>
#include <errno.h>
#include <string.h>
#include <libintl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

/*
 * sk_warning:
 *               This is a general purpose function for sending error and 
 *               warning messages to STDOUT and/or a log file.
 *
 *   verbose:    0 or 1 indicating whether the output goes only to the log
 *               file (0) or to both STDOUT and the log file (1).
 *
 *   funct_name: A string which is only printed to the log file and not to
 *               STDOUT.  This is useful for command-line routines where it
 *               doesn't make sense to put the name of the routine in the 
 *               warning/error message.  For internal functions, we typically
 *               leave this empty and place the function name in paranthesis
 *               inside the 'format' string.
 */

void sk_warning(int verbose, char *funct_name, char *format, ...)
{
    va_list ap;
    struct stat buf;
    FILE *fid;
    time_t current_time;
    struct tm *tm;
    char datestamp[512];

    /*
     * Write to STDOUT first, since we return if there is a problem
     *  accessing the log file.
     */
    va_start(ap, format);
    if (verbose)
        vfprintf(stderr, format, ap);
    va_end(ap);

    /*
     * Open log file, and rotate if necessary.
     */
    if (stat(SCROLLKEEPERLOGFILE, &buf) == -1) {
    	if (errno == ENOENT) {
	    fid = fopen(SCROLLKEEPERLOGFILE, "w");
	    if (fid == NULL) {
                printf("Cannot create log file: %s : %s\n", SCROLLKEEPERLOGFILE, strerror(errno));
	        return;
            }
	}
	else {
            printf("Error accessing log file: %s : %s\n", SCROLLKEEPERLOGFILE, strerror(errno));
	    return;
	}
    }
    else {
        if (buf.st_size < (1<<24)) {
	    fid = fopen(SCROLLKEEPERLOGFILE, "a");
	    if (fid == NULL) {
                printf("Cannot write to log file: %s : %s\n", SCROLLKEEPERLOGFILE, strerror(errno));
	        return;
            }
	}
	else {	/* Rotate log file */
	    rename(SCROLLKEEPERLOGFILE, SCROLLKEEPERLOGFILE_ROT);
	    fid = fopen(SCROLLKEEPERLOGFILE, "w");
	    if (fid == NULL) {
                printf("Cannot create log file: %s : %s\n", SCROLLKEEPERLOGFILE, strerror(errno));
	        return;
            }
	}
    }
    
    /* Write message to log file */
    time (&current_time);
    tm = localtime (&current_time);
    strftime (datestamp, sizeof (datestamp), "%b %d %X", tm);
    va_start(ap, format);
    fprintf(fid,"%s %s :", datestamp, funct_name);
    vfprintf(fid, format, ap);
    va_end(ap);
    fclose(fid);
}

void
check_ptr (void *p, char *name)
{
    if (p == NULL)
    {
        fprintf (stderr, _("%s: out of memory: %s\n"), name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}

static void reconcile_skout_prefs(char outputprefs, int stdout_threshold, int log_threshold, int *do_stdout_message, int *do_log_message)
{
  int stdout_value, log_value;

  /*
   * Up to this point, the various flags have not been checked for self-consistancy.
   * For example, quiet and debug may both be turned on.
   *
   * Order of priority is:
   *    DEBUG
   *    QUIET
   *    VERBOSE
   *    DEFAULT
   *
   * We assign a final value according to:
   *    DEBUG           =       SKOUT_DEBUG     =       4
   *    VERBOSE         =       SKOUT_VERBOSE   =       3
   *    DEFAULT         =       SKOUT_DEFAULT   =       2
   *    QUIET           =       SKOUT_QUIET     =       1
   *
   * If the threshold value is less than this number, then the message
   * event will occur.
   */

  stdout_value=SKOUT_DEFAULT;
  if (outputprefs & SKOUT_STD_VERBOSE) stdout_value=SKOUT_VERBOSE;
  if (outputprefs & SKOUT_STD_QUIET) stdout_value=SKOUT_QUIET;
  if (outputprefs & SKOUT_STD_DEBUG) stdout_value=SKOUT_DEBUG;

  log_value=SKOUT_DEFAULT;
  if (outputprefs & SKOUT_LOG_VERBOSE) log_value=SKOUT_VERBOSE;
  if (outputprefs & SKOUT_LOG_QUIET) log_value=SKOUT_QUIET;
  if (outputprefs & SKOUT_LOG_DEBUG) log_value=SKOUT_DEBUG;

  if (stdout_value >= stdout_threshold) *do_stdout_message=1;
  if (log_value >= log_threshold) *do_log_message=1;
}

/*
 * sk_message:
 *               This is a general purpose function for sending error and 
 *               warning messages to STDOUT and/or a log file.
 *
 *   verbose:    0 or 1 indicating whether the output goes only to the log
 *               file (0) or to both STDOUT and the log file (1).
 *
 *   funct_name: A string which is only printed to the log file and not to
 *               STDOUT.  This is useful for command-line routines where it
 *               doesn't make sense to put the name of the routine in the 
 *               warning/error message.  For internal functions, we typically
 *               leave this empty and place the function name in paranthesis
 *               inside the 'format' string.
 */

extern void sk_message(char outputprefs, int stdout_threshold, int log_threshold, char *funct_name, char *format, ...)
{
    va_list ap;
    struct stat buf;
    FILE *fid;
    time_t current_time;
    struct tm *tm;
    char datestamp[512];
    int do_stdout_message=0, do_log_message=0;

    reconcile_skout_prefs(outputprefs, stdout_threshold, log_threshold, &do_stdout_message, &do_log_message);

    /*
     * Write to STDOUT if do_log_message is set.
     */
    if (do_stdout_message) {
        /*
         * (We write to STDOUT first, since we return if there is a problem
         *  accessing the log file.)
         */
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
    }

    /*
     * Write to log file if do_log_message is set.
     */
    if (do_log_message) {
        /*
         * Open log file, and rotate if necessary.
         */
        if (stat(SCROLLKEEPERLOGFILE, &buf) == -1) {
            if (errno == ENOENT) {
                fid = fopen(SCROLLKEEPERLOGFILE, "w");
                if (fid == NULL) {
                    printf("Cannot create log file: %s : %s\n", SCROLLKEEPERLOGFILE, strerror(errno));
	            return;
                }
	    }
	    else {
                printf("Error accessing log file: %s : %s\n", SCROLLKEEPERLOGFILE, strerror(errno));
	        return;
	    }
        }
        else {
            if (buf.st_size < (1<<24)) {
	        fid = fopen(SCROLLKEEPERLOGFILE, "a");
	        if (fid == NULL) {
                    printf("Cannot write to log file: %s : %s\n", SCROLLKEEPERLOGFILE, strerror(errno));
	            return;
                }
	    }
	    else {	/* Rotate log file */
	        rename(SCROLLKEEPERLOGFILE, SCROLLKEEPERLOGFILE_ROT);
	        fid = fopen(SCROLLKEEPERLOGFILE, "w");
	        if (fid == NULL) {
                    printf("Cannot create log file: %s : %s\n", SCROLLKEEPERLOGFILE, strerror(errno));
	            return;
                }
	    }
        }
    
        /* Write message to log file */
        time (&current_time);
        tm = localtime (&current_time);
        strftime (datestamp, sizeof (datestamp), "%b %d %X", tm);
        va_start(ap, format);
        fprintf(fid,"%s %s: ", datestamp, funct_name);
        vfprintf(fid, format, ap);
        va_end(ap);
        fclose(fid);
    }
}


/*
 * Grab the warning and error messages from libxml validating the OMF file
 * against the DTD and pass it to sk_message().
 */
void sk_dtd_validation_message(char *outputprefs, char *format, ...)
{
    va_list ap;
    char message[4096];

    va_start(ap, format);
    vsprintf(message, format, ap);
    va_end(ap);

    sk_message(*outputprefs, SKOUT_VERBOSE, SKOUT_DEBUG, "(install)",_("OMF validation error: %s"), message);
}
