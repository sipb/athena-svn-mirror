
/* lmtpstats.c -- automatically generated from lmtpstats.snmp by snmpgen */



#ifdef USING_SNMPGEN



/* We disable this code for now since it doesn't actually work and wastes

 * resources.  At some point in time, we'll make it work again as it would 

 * be useful to gather aggregate statistics on what commands are being used 

 * so we can better tune the server.  This change closes bug #1191. 

 * New bug 1267 opened to re-enable the feature.

 */



#ifdef HAVE_UNISTD_H

#include <unistd.h>

#endif

#include <stdio.h>

#include <stdlib.h>

#include <errno.h>

#include <string.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <sys/un.h>

#include <fcntl.h>

#include <stdarg.h>



#include "lmtpstats.h"



extern int close(int);



int variable_value[2+1];

int variable_tmpvalue[2+1];



int varvalue(lmtpstats_variable_t var)

{

    if (variable_tmpvalue[var]!=-1)

        return variable_tmpvalue[var];



    return variable_value[var];

}



const char *snmp_getdescription(lmtpstats_t evt)

{

    switch (evt) {

        case SIEVE_REDIRECT: return "sieve redirects";
        case SIEVE_DISCARD: return "sieve discards";
        case SERVER_UPTIME: return "Amount of time server has been running";
        case mtaSuccessfulConvertedMessages: return "Messages converted because of 8bit foo";
        case mtaReceivedMessages: return "Messages we've received";
        case mtaTransmittedMessages: return "Messages stored to disk";
        case SIEVE_VACATION_REPLIED: return "vacation messages sent";
        case SIEVE_NOTIFY: return "sieve notifications sent";
        case SIEVE_FILEINTO: return "sieve fileintos";
        case mtaReceivedRecipients: return "Recipients accepted";
        case mtaTransmittedVolume: return "Kbytes stored to disk";
        case SIEVE_VACATION_TOTAL: return "vacation messages considered";
        case SIEVE_REJECT: return "sieve rejects";
        case AUTHENTICATION_YES: return "Successful authentication of given mechanism";
        case AUTHENTICATION_NO: return "Failed authentication of given mechanism";
        case TOTAL_CONNECTIONS: return "Count of the total number of connections since the beginning of time";
        case SIEVE_KEEP: return "sieve messages kept";
        case mtaReceivedVolume: return "Kbytes received";
        case SERVER_NAME_VERSION: return "Name and version string for server";
        case SIEVE_MESSAGES_PROCESSED: return "Number of messages processed by Sieve scripts";
        case ACTIVE_CONNECTIONS: return "Count of the active number of connections";

    }

    return NULL;

}



const char *snmp_getoid(const char *name __attribute__((unused)),

			lmtpstats_t evt, char *buf, int buflen)

{

    switch (evt) {

        case SIEVE_REDIRECT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.1"); return buf;
        case SIEVE_DISCARD: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.2"); return buf;
        case SERVER_UPTIME: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.2.3.1"); return buf;
        case mtaSuccessfulConvertedMessages: snprintf(buf,buflen,"1.3.6.1.2.1.28.1.%d.10",varvalue(VARIABLE_MTA)); return buf;
        case mtaReceivedMessages: snprintf(buf,buflen,"1.3.6.1.2.1.28.1.%d.1",varvalue(VARIABLE_MTA)); return buf;
        case mtaTransmittedMessages: snprintf(buf,buflen,"1.3.6.1.2.1.28.1.%d.3",varvalue(VARIABLE_MTA)); return buf;
        case SIEVE_VACATION_REPLIED: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.7"); return buf;
        case SIEVE_NOTIFY: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.6"); return buf;
        case SIEVE_FILEINTO: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.4"); return buf;
        case mtaReceivedRecipients: snprintf(buf,buflen,"1.3.6.1.2.1.28.1.%d.7",varvalue(VARIABLE_MTA)); return buf;
        case mtaTransmittedVolume: snprintf(buf,buflen,"1.3.6.1.2.1.28.1.%d.6",varvalue(VARIABLE_MTA)); return buf;
        case SIEVE_VACATION_TOTAL: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.8"); return buf;
        case SIEVE_REJECT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.3"); return buf;
        case AUTHENTICATION_YES: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.2.2.%d.0",varvalue(VARIABLE_AUTH)); return buf;
        case AUTHENTICATION_NO: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.2.2.%d.1",varvalue(VARIABLE_AUTH)); return buf;
        case TOTAL_CONNECTIONS: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.2.1.1"); return buf;
        case SIEVE_KEEP: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.5"); return buf;
        case mtaReceivedVolume: snprintf(buf,buflen,"1.3.6.1.2.1.28.1.%d.4",varvalue(VARIABLE_MTA)); return buf;
        case SERVER_NAME_VERSION: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.2.3.0"); return buf;
        case SIEVE_MESSAGES_PROCESSED: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.3.4.0"); return buf;
        case ACTIVE_CONNECTIONS: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.2.1.2"); return buf;

    }

    return NULL;

}



#define SOCK_PATH "/tmp/.snmp_door"



static int mysock = -1;

static struct sockaddr_un remote;

static int sockaddr_len = 0;



static void snmp_send(char *str)

{

    if (mysock == -1) return;



    if (sendto(mysock, str, strlen(str), 0, (struct sockaddr *) &remote, sockaddr_len) == -1) {

	return;

    }



    return;

}



int snmp_connect(void)

{

    int s;

    int fdflags;

    int lup;



    if ((s = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {

	return 1;

    }



    for (lup=0;lup < 2+1; lup++)

        variable_tmpvalue[lup] = -1;



    remote.sun_family = AF_UNIX;

    strcpy(remote.sun_path, SOCK_PATH);

    sockaddr_len = strlen(remote.sun_path) + sizeof(remote.sun_family);



    /* put us in non-blocking mode */

    fdflags = fcntl(s, F_GETFD, 0);

    if (fdflags != -1) fdflags = fcntl(s, F_SETFL, O_NONBLOCK | fdflags);

    if (fdflags == -1) { close(s); return -1; }



    mysock = s;

    snmp_send("R 1.3.6.1.4.1.3.2.2.3.2.1\n");
    snmp_send("R 1.3.6.1.4.1.3.2.2.3.2.2\n");
    snmp_send("R 1.3.6.1.4.1.3.2.2.3.2.3\n");
    snmp_send("R 1.3.6.1.2.1.28.1\n");
    snmp_send("R 1.3.6.1.4.1.3.2.2.3.3.4\n");

    return 0;

}



int snmp_close(void)

{

    if (mysock > -1)

	close(mysock);



    return 0;

}



int snmp_increment_args(lmtpstats_t cmd, int incr, ...)

{

    char tosend[256]; /* xxx UDP max size??? */

    char buf[256];



      va_list ap; /* varargs thing */

      lmtpstats_variable_t vval;

      int ival;



      if (mysock == -1) return 1;



      va_start(ap, incr);



      do {

          vval = va_arg(ap, lmtpstats_variable_t); /* get the next arg */



          if (vval!=VARIABLE_LISTEND)

          {

              ival = va_arg(ap, int); /* get the next arg */

              variable_tmpvalue[vval] = ival;              

          }



      } while ( vval != VARIABLE_LISTEND);



      va_end(ap);



    snprintf(tosend, sizeof(tosend),"C %s %d\n",snmp_getoid(NULL,cmd,buf,sizeof(buf)), incr);



    if (sendto(mysock, tosend, strlen(tosend), 0, (struct sockaddr *) &remote, sockaddr_len) == -1) {

	return 1;

    }



    /* set tmp variables back */

    va_start(ap, incr);



      do {

          vval = va_arg(ap, lmtpstats_variable_t); /* get the next arg */



          if (vval!=VARIABLE_LISTEND)

          {

              ival = va_arg(ap, int); /* get the next arg */

              variable_tmpvalue[vval] = -1;              

          }



      } while ( vval != VARIABLE_LISTEND);



      va_end(ap);



    return 0;

}



int snmp_increment(lmtpstats_t cmd, int incr)

{

    char tosend[256]; /* xxx UDP max size??? */

    char buf[256];



    if (mysock == -1) return 1;



    snprintf(tosend, sizeof(tosend),"C %s %d\n",snmp_getoid(NULL,cmd,buf,sizeof(buf)), incr);



    if (sendto(mysock, tosend, strlen(tosend), 0, (struct sockaddr *) &remote, sockaddr_len) == -1) {

	return 1;

    }



    return 0;

}



int snmp_set(lmtpstats_t cmd, int value)

{

    char tosend[256];

    char buf[256];



    if (mysock == -1) return 1;



    snprintf(tosend, sizeof(tosend),"I %s %d\n",snmp_getoid(NULL,cmd,buf,sizeof(buf)), value);



    if (sendto(mysock, tosend, strlen(tosend), 0, (struct sockaddr *) &remote, sockaddr_len) == -1) {

	return 1;

    }



    return 1;

}



int snmp_set_str(lmtpstats_t cmd, char *value)

{

    char tosend[256];

    char buf[256];



    if (mysock == -1) return 1;



    snprintf(tosend, sizeof(tosend),"S %s %s\n",snmp_getoid(NULL,cmd,buf,sizeof(buf)), value);



    if (sendto(mysock, tosend, strlen(tosend), 0, (struct sockaddr *) &remote, sockaddr_len) == -1) {

	return 1;

    }



    return 1;

}



int snmp_set_time(lmtpstats_t cmd, time_t t)

{

    char tosend[256];

    char buf[256];



    if (mysock == -1) return 1;



    snprintf(tosend, sizeof(tosend),"T %s %lu\n",snmp_getoid(NULL,cmd,buf,sizeof(buf)), t);



    if (sendto(mysock, tosend, strlen(tosend), 0, (struct sockaddr *) &remote, sockaddr_len) == -1) {

	return 1;

    }



    return 1;

}



/* should use SNMPDEFINE's as parameter */

int snmp_set_oid(lmtpstats_t cmd, char *str)

{

   return snmp_set_str(cmd,str);

}



void snmp_setvariable(lmtpstats_variable_t name, int value)

{

    variable_value[name] = value;

}



#endif





