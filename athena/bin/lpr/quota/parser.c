/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/parser.c,v 1.2 1990-04-25 11:47:54 epeisach Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/parser.c,v $ */
/* $Author: epeisach $ */
/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "logger.h"
#include <logger_ncs.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/param.h> 
#include <errno.h>
#include <sys/file.h>
#include <stdio.h>
#include <syslog.h>

extern int errno;

int 
logger_parse_quota(buf, out)
char *buf;
log_entity *out;
{
    

    /* Inital values are set here */
    Time t = -1;
    String_num name=0, inst=0, realm = 0, serv = 0;
    String_num name1=0, inst1=0, realm1 = 0;
    Function func = 999;
    Amount amt = 0;
    Time subt = -1;
    Num npages = 0, medcost = 0;
    String_num where = 0;

    /* funcsel is the current state. States so far:
       0 = Any function
       1 = Must be an offset type
       2 = must be a charge
     */
    int funcsel = 0;
    
    char *pos = NULL; /* Current position on the line */
    char cur[128];	/* Current inbuf */
    char *curpos;

    if(buf == NULL) return -1; /* Don't have anything to parse */

    pos = buf;

 loop:
    /* Remove initial spaces */
    while (*pos && *pos == ' ') pos++;

    curpos = cur;
    while(*pos && *pos != ' ') {
	/* Filter out newlines */
	if (*pos != '\n') *curpos++ = *pos;
	pos++;
    }

    *curpos = '\0';

    /* We are either at the end of a line or at a space. */
    /* Anyways, we know what the argument should look like. */
    if((curpos == cur) && !*pos) goto done;

    if(cur[1] != '=' || cur[2] == '\0') goto error;

    switch(cur[0]) {
    case 't':
	if(sscanf(cur + 2, "%d", &t) != 1) goto error;
	break;

    case 'n':
	if((name = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'i':
	if((inst = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'r':
	if((realm = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 's':
	if((serv = logger_add_string(cur + 2)) == 0) goto error;
	break;

#define CKSTATE(n) if(funcsel && funcsel != (n)) goto error; \
	           funcsel = (n);

    case 'N':
	CKSTATE(1);
	if((name1 = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'I':
	CKSTATE(1);
	if((inst1 = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'R':
	CKSTATE(1);
	if((realm1 = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'a':
	CKSTATE(1);
	if(sscanf(cur + 2, "%d", &amt) != 1) goto error;
	break;

    case 'p':
	CKSTATE(2);
	if(sscanf(cur + 2, "%d", &npages) != 1) goto error;
	break;

    case 'q':
	CKSTATE(2);
	if(sscanf(cur + 2, "%d", &subt) != 1) goto error;
	break;

    case 'c':
	CKSTATE(2);
	if(sscanf(cur + 2, "%d", &medcost) != 1) goto error;
	break;

    case 'w':
	CKSTATE(2);
	if((where = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'f':
	if(!strncasecmp(cur +2 , "add", 3)) {
	    func = LO_ADD; 
	    CKSTATE(1);
	    break;
	} else if(!strncasecmp(cur +2 , "charge", 6)) {
	    func = LO_CHARGE; 
	    CKSTATE(2);
	    break;
	} else if(!strncasecmp(cur +2 , "subtract", 7)) {
	    func = LO_SUBTRACT; 
	    CKSTATE(1);
	    break;
	} else if(!strncasecmp(cur +2 , "set", 3)) {
	    func = LO_SET; 
	    CKSTATE(1);
	    break;
	} else if(!strncasecmp(cur +2 , "delete", 6)) {
	    func = LO_DELETEUSER; 
	    CKSTATE(1);
	    break;
	} else if(!strncasecmp(cur +2 , "allow", 5)) {
	    func = LO_ALLOW; 
	    CKSTATE(1);
	    break;
	} else if(!strncasecmp(cur +2 , "disallow", 7)) {
	    func = LO_DISALLOW; 
	    CKSTATE(1);
	    break;
	} else if(!strncasecmp(cur +2 , "adjust", 6)) {
	    func = LO_ADJUST; 
	    CKSTATE(1);
	    break;
	}
	goto error;

    default:
	goto error;
    }

    /* Continue along... */
    goto loop;

 done:
    if((funcsel == 0) || func == 999) goto error;

    out->time = t;
    out->user.name = name;
    out->user.instance = inst;
    out->user.realm = realm;
    out->service = serv;
    out->func = func;

    switch (func) {
    case LO_ADD:
    case LO_SUBTRACT:
    case LO_SET:
    case LO_ADJUST:
    case LO_DELETEUSER:
    case LO_ALLOW:
    case LO_DISALLOW:
	out->trans.offset.amt = amt;
	out->trans.offset.name = name1;
	out->trans.offset.inst = inst1;
	out->trans.offset.realm = realm1;
	break;
    case LO_CHARGE:
	out->trans.charge.subtime = subt;
	out->trans.charge.npages = npages;
	out->trans.charge.med_cost = medcost;
	out->trans.charge.where = where;
	break;
    default:
	syslog(LOG_ERR, "Inconsistancy in parser func = %d\n", func);
	return -1;
    }

    /* We are done so return */
    
    return 0;

 error:
    
    syslog(LOG_ERR, "Could not parse line: %s, keyword: %s\n", buf, cur);
    return -1;
}


int logger_cvt_line(line, buf)
log_entity *line;
char *buf;
{

    char *ret;
    char buf1[128];
    buf[0]='\0';

#define ADDSTRING(f,n) if((n) != 0) { \
		       if(!(ret = logger_num_to_string(n))) return -1; \
		       sprintf(buf1,"%s=%s ", f, ret); strcat(buf, buf1); \
		       }

#define ADDNUM(f,n) if((n) != 0) {sprintf(buf1,"%s=%d ",f,n); strcat(buf, buf1);}

#define ADDTIME(f,n) if ((n) != -1) {sprintf(buf1,"%s=%d ",f,n); strcat(buf, buf1);}

    ADDTIME("t",line->time);
    ADDSTRING("n", line->user.name);
    ADDSTRING("i", line->user.instance);
    ADDSTRING("r", line->user.realm);
    ADDSTRING("s", line->service);
    switch(line->func) {
    case LO_ADD:
	strcat(buf, "f=add ");
	break;
    case LO_CHARGE:
	strcat(buf, "f=charge ");
	break;
    case LO_SET:
	strcat(buf, "f=set ");
	break;
    case LO_SUBTRACT:
	strcat(buf, "f=subtract ");
	break;
    case LO_DELETEUSER:
	strcat(buf, "f=delete ");
	break;
    case LO_ALLOW:
	strcat(buf, "f=allow ");
	break;
    case LO_DISALLOW:
	strcat(buf, "f=disallow ");
	break;
    case LO_ADJUST:
	strcat(buf, "f=adjust ");
	break;
    default: 
	return -1;
    }

    switch(line->func) {
    case LO_SET:
    case LO_ADD:
    case LO_SUBTRACT:
    case LO_DELETEUSER:
    case LO_ALLOW:
    case LO_DISALLOW:
    case LO_ADJUST:
	ADDNUM("a", line->trans.offset.amt);
	ADDSTRING("N", line->trans.offset.name);
	ADDSTRING("I", line->trans.offset.inst);
	ADDSTRING("R", line->trans.offset.realm);
	break;
    case LO_CHARGE:
	ADDTIME("q", line->trans.charge.subtime);
	ADDNUM("p", line->trans.charge.npages);
	ADDNUM("c", line->trans.charge.med_cost);
	ADDSTRING("p", line->trans.charge.where);
	break;
    }

    return 0;

}
    
