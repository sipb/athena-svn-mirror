
/* pushstats.c -- automatically generated from pushstats.snmp by snmpgen */



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



#include "index.h"

#include "pushstats.h"



extern int close(int);



int variable_value[1+1];

int variable_tmpvalue[1+1];



int varvalue(pushstats_variable_t var)

{

    if (variable_tmpvalue[var]!=-1)

        return variable_tmpvalue[var];



    return variable_value[var];

}



const char *snmp_getdescription(pushstats_t evt)

{

    switch (evt) {

        case RENAME_COUNT: return "Number of rename";
        case EXAMINE_COUNT: return "Number of examine";
        case NOOP_COUNT: return "Number of noop";
        case LOGOUT_COUNT: return "Number of logout";
        case SETACL_COUNT: return "Number of setacl";
        case SETQUOTA_COUNT: return "Number of setquota";
        case GETANNOTATION_COUNT: return "Number of getannotation";
        case IDLE_COUNT: return "Number of idle";
        case SORT_COUNT: return "Number of sort";
        case GETUIDS_COUNT: return "Number of getuids";
        case EXPUNGE_COUNT: return "Number of expunge";
        case CHECK_COUNT: return "Number of check";
        case AUTHENTICATION_NO: return "Failed authentication of given mechanism";
        case SELECT_COUNT: return "Number of select";
        case GETQUOTAROOT_COUNT: return "Number of getquotaroot";
        case UNSELECT_COUNT: return "Number of unselect";
        case STARTTLS_COUNT: return "Number of starttls";
        case SERVER_NAME_VERSION: return "Name and version string for server";
        case THREAD_COUNT: return "Number of thread";
        case DELETE_COUNT: return "Number of delete";
        case COPY_COUNT: return "Number of copy";
        case STORE_COUNT: return "Number of store";
        case SERVER_UPTIME: return "Amount of time server has been running";
        case GETQUOTA_COUNT: return "Number of getquota";
        case FIND_COUNT: return "Number of find";
        case LSUB_COUNT: return "Number of lsub";
        case APPEND_COUNT: return "Number of append";
        case FETCH_COUNT: return "Number of fetch";
        case SEARCH_COUNT: return "Number of search";
        case AUTHENTICATE_COUNT: return "Number of authenticate";
        case BBOARD_COUNT: return "Number of bboard";
        case CLOSE_COUNT: return "Number of close";
        case PARTIAL_COUNT: return "Number of partial";
        case ID_COUNT: return "Number of id";
        case SETANNOTATION_COUNT: return "Number of setannotation";
        case NAMESPACE_COUNT: return "Number of namespace";
        case SUBSCRIBE_COUNT: return "Number of subscribe";
        case LOGIN_COUNT: return "Number of login";
        case AUTHENTICATION_YES: return "Successful authentication of given mechanism";
        case DELETEACL_COUNT: return "Number of deleteacl";
        case TOTAL_CONNECTIONS: return "Count of the total number of connections since the beginning of time";
        case CREATE_COUNT: return "Number of create";
        case GETACL_COUNT: return "Number of getacl";
        case CAPABILITY_COUNT: return "Number of capability";
        case LIST_COUNT: return "Number of list";
        case UNSUBSCRIBE_COUNT: return "Number of unsubscribe";
        case STATUS_COUNT: return "Number of status";
        case ACTIVE_CONNECTIONS: return "Count of the active number of connections";
        case LISTRIGHTS_COUNT: return "Number of listrights";
        case MYRIGHTS_COUNT: return "Number of myrights";

    }

    return NULL;

}



const char *snmp_getoid(const char *name __attribute__((unused)),

			pushstats_t evt, char *buf, int buflen)

{

    switch (evt) {

        case RENAME_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.30"); return buf;
        case EXAMINE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.11"); return buf;
        case NOOP_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.27"); return buf;
        case LOGOUT_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.22"); return buf;
        case SETACL_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.36"); return buf;
        case SETQUOTA_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.38"); return buf;
        case GETANNOTATION_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.15"); return buf;
        case IDLE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.20"); return buf;
        case SORT_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.40"); return buf;
        case GETUIDS_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.18"); return buf;
        case EXPUNGE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.10"); return buf;
        case CHECK_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.4"); return buf;
        case AUTHENTICATION_NO: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.2.%d.1",varvalue(VARIABLE_AUTH)); return buf;
        case SELECT_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.33"); return buf;
        case GETQUOTAROOT_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.17"); return buf;
        case UNSELECT_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.43"); return buf;
        case STARTTLS_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.31"); return buf;
        case SERVER_NAME_VERSION: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.3.0"); return buf;
        case THREAD_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.41"); return buf;
        case DELETE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.8"); return buf;
        case COPY_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.5"); return buf;
        case STORE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.32"); return buf;
        case SERVER_UPTIME: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.3.1"); return buf;
        case GETQUOTA_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.16"); return buf;
        case FIND_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.13"); return buf;
        case LSUB_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.24"); return buf;
        case APPEND_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.1"); return buf;
        case FETCH_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.12"); return buf;
        case SEARCH_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.34"); return buf;
        case AUTHENTICATE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.0"); return buf;
        case BBOARD_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.2"); return buf;
        case CLOSE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.7"); return buf;
        case PARTIAL_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.29"); return buf;
        case ID_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.19"); return buf;
        case SETANNOTATION_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.37"); return buf;
        case NAMESPACE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.28"); return buf;
        case SUBSCRIBE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.35"); return buf;
        case LOGIN_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.21"); return buf;
        case AUTHENTICATION_YES: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.2.%d.0",varvalue(VARIABLE_AUTH)); return buf;
        case DELETEACL_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.9"); return buf;
        case TOTAL_CONNECTIONS: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.1.1"); return buf;
        case CREATE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.6"); return buf;
        case GETACL_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.14"); return buf;
        case CAPABILITY_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.3"); return buf;
        case LIST_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.23"); return buf;
        case UNSUBSCRIBE_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.42"); return buf;
        case STATUS_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.39"); return buf;
        case ACTIVE_CONNECTIONS: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.1.2"); return buf;
        case LISTRIGHTS_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.25"); return buf;
        case MYRIGHTS_COUNT: snprintf(buf,buflen,"1.3.6.1.4.1.3.2.2.3.1.4.26"); return buf;

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



    for (lup=0;lup < 1+1; lup++)

        variable_tmpvalue[lup] = -1;



    remote.sun_family = AF_UNIX;

    strlcpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path));

    sockaddr_len = strlen(remote.sun_path) + sizeof(remote.sun_family);



    /* put us in non-blocking mode */

    fdflags = fcntl(s, F_GETFD, 0);

    if (fdflags != -1) fdflags = fcntl(s, F_SETFL, O_NONBLOCK | fdflags);

    if (fdflags == -1) { close(s); return -1; }



    mysock = s;

    snmp_send("R 1.3.6.1.4.1.3.2.2.3.1.1\n");
    snmp_send("R 1.3.6.1.4.1.3.2.2.3.1.2\n");
    snmp_send("R 1.3.6.1.4.1.3.2.2.3.1.3\n");
    snmp_send("R 1.3.6.1.4.1.3.2.2.3.1.4\n");

    return 0;

}



int snmp_close(void)

{

    if (mysock > -1)

	close(mysock);



    return 0;

}



int snmp_increment_args(pushstats_t cmd, int incr, ...)

{

    char tosend[256]; /* xxx UDP max size??? */

    char buf[256];



      va_list ap; /* varargs thing */

      pushstats_variable_t vval;

      int ival;



      if (mysock == -1) return 1;



      va_start(ap, incr);



      do {

          vval = va_arg(ap, pushstats_variable_t); /* get the next arg */



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

          vval = va_arg(ap, pushstats_variable_t); /* get the next arg */



          if (vval!=VARIABLE_LISTEND)

          {

              ival = va_arg(ap, int); /* get the next arg */

              variable_tmpvalue[vval] = -1;              

          }



      } while ( vval != VARIABLE_LISTEND);



      va_end(ap);



    return 0;

}



int snmp_increment(pushstats_t cmd, int incr)

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



int snmp_set(pushstats_t cmd, int value)

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



int snmp_set_str(pushstats_t cmd, char *value)

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



int snmp_set_time(pushstats_t cmd, time_t t)

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

int snmp_set_oid(pushstats_t cmd, char *str)

{

   return snmp_set_str(cmd,str);

}



void snmp_setvariable(pushstats_variable_t name, int value)

{

    variable_value[name] = value;

}



#endif





