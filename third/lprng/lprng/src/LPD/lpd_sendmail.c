/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1997, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: lpd_sendmail.c
 * PURPOSE: send mail to users on job completion
 **************************************************************************/

static char *const _id =
"lpd_sendmail.c,v 3.7 1997/12/16 15:06:21 papowell Exp";
#include "lp.h"
#include "errorcodes.h"
#include "fileopen.h"
#include "pathname.h"
#include "pr_support.h"
#include "setup_filter.h"
#include "setstatus.h"
/**** ENDINCLUDE ****/

/*
 * sendmail --- tell people about job completion
 * 1. fork a sendmail process
 * 2. if successful, send the good news
 * 3. if unsuccessful, send the bad news
 */

void Sendmail_to_user( int status, struct control_file *cfp,
	struct printcap_entry *pc_used )
{
	FILE *mail = 0;
	int fd;
	struct stat statb;
	char *pname, *transport = NULL, *user;
	char tbuf[LINEBUFFER];
	/*
	 * check to see if the user really wanted
	 * "your file was printed ok" message
	 */
	DEBUG2("Sendmail: MAILNAME '%s' sendmail '%s'",
		cfp->MAILNAME, Sendmail );
	if( cfp->MAILNAME == 0
		|| cfp->MAILNAME[0] == 0
		|| cfp->MAILNAME[1] == 0
		|| ( hostport(cfp->MAILNAME) && !Extended_notification ) ){
		DEBUG2("Sendmail: no mail wanted");
		return;
	}

	if( hostport(cfp->MAILNAME) ){
		safestrncpy( tbuf, cfp->MAILNAME + 1 );
		transport = tbuf;
		user = strchr( transport, '%' );
		if( !user ){
			fatal( LOG_ERR, "Sendmail_to_user: name too long" );
		}
		*user++ = '\0';
	} else {
		user = cfp->MAILNAME + 1;
	}

	if( !transport ){
		if( !Sendmail || !*Sendmail ){
			DEBUG2("Sendmail: mail is turned off");
			return;
		}

		/* create the sendmail process */
		if( Make_filter( 'f', cfp, &Pr_fd_info, Sendmail, 1, 1, 1,
				pc_used, (void*)0, 0, 0, 0) ){
			fatal( LOG_ERR, "%s", cfp->error );
		}
	} else if( strcmp( transport, "zephyr" ) == 0 ){
		char zbuf[SMALLBUFFER];

		if( !Zwrite || !*Zwrite ){
			DEBUG2("Sendmail: zephyr is turned off");
			return;
		}

		/* create the zwrite process */
		if( plp_snprintf( zbuf, sizeof(zbuf),
				 "%s -n -q -d -l -s '%s' %s", Zwrite,
				 Printer, user ) >= SMALLBUFFER - 1 ){
			fatal( LOG_ERR, "buffer overflow sending zephyr" );
		}
		if( Make_filter( 'f', cfp, &Pr_fd_info, zbuf, 1, 1, 1,
				pc_used, (void*)0, 0, 0, 0) ){
			fatal( LOG_ERR, "%s", cfp->error );
		}

	} else {
		DEBUG2("Sendmail: unknown notification transport '%s'",
		       transport);
		return;
	}

	mail = fdopen( Pr_fd_info.input, "a+" );
	if( mail == 0 ){
		logerr_die( LOG_ERR, _("Sendmail: fdopen failed") );
	}

	if( !transport ){
		/* Print the mail header */
		(void) fprintf( mail, "To: %s\n", user );
		if( status != JSUCC && Mail_operator_on_error ){
			fprintf( mail, "CC: %s\n", Mail_operator_on_error );
		}
		(void) fprintf( mail, "From: %s@%s\n",
			       Mail_from ? Mail_from : Printer, FQDNHost );
		(void) fprintf( mail, "Subject: %s@%s job %s\n\n",
			       Printer, FQDNHost, cfp->transfername );
	}

	/* now do the message */
	(void) fprintf( mail, _("printer %s job %s"), Printer, cfp->transfername );
	if( cfp->JOBNAME ){
		(void) fprintf( mail, " (%s)", cfp->JOBNAME+1 );
	}
	switch( status) {
	case JSUCC:
		(void) fprintf( mail, _(" was successful.\n"));
		break;

	case JFAIL:
		(void) fprintf( mail, _(" failed, and retry count was exceeded.\n") );
		break;

	case JABORT:
		(void) fprintf( mail, _(" failed and could not be retried.\n") );
		break;

	default:
		(void) fprintf( mail, _(" died a horrible death.\n"));
		break;
	}

	if( !transport ){
		/*
		 * get the last status of the spooler
		 */
		pname = Add2_path( CDpathname, "status.", Printer );
		if( (fd = Checkread( pname, &statb ) ) >= 0 ){
			FILE *sfile;
			char msg[LINEBUFFER];

			sfile = fdopen( fd, "r" );
			if( sfile == 0 ){
				logerr_die( LOG_ERR,
					   _("Sendmail: fdopen failed") );
			}
			/*
			 * we read the file,writing each line out
			 */
			while( fgets( msg, sizeof(msg), sfile ) ){
				fprintf( mail, _("   Status: %s"), msg );
			}
			fclose( sfile );
		}
		close(fd);
	}

	(void) fflush( mail );
	(void) fclose( mail );
	/* give the mail a chance */
	Close_filter( 0, &Pr_fd_info, 0, "mailer" );
}
