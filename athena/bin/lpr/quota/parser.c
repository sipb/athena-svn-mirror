/* $Id: parser.c,v 1.7 1999-01-22 23:11:05 ghudson Exp $ */
/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "logger.h"
#include <logger_ncs.h>
#include <strings.h>
#include <sys/types.h>
#if defined(ultrix) && defined(NULL)
#undef NULL
#endif
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
    String_num name1=0, inst1=0, realm1 = 0, grp = 0;
    Function func = 999;
    Amount amt = 0;
    Time subt = -1;
    Num npages = 0, medcost = 0;
    String_num where = 0;
    int is_group = 0;

    /* funcsel is the current state. States so far:
       0 = Any function
       1 = Must be an offset type
     */
    int funcsel = 0;
    
    char *pos = NULL; /* Current position on the line */
    char cur[128];	/* Current inbuf */
    char grp_str[128];
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

#define CK_OFFSET   1
#define CK_CHARGE   2
#define CK_GROUP    4
#define CK_ANY      (CK_OFFSET | CK_CHARGE | CK_GROUP)

#define CKSTATE(n) if(funcsel && (funcsel & (n)) == 0) goto error; \
	           funcsel = (funcsel | (n));


    case 'g':
	CKSTATE(CK_GROUP);
	grp_str[0] = ':';
	strncpy(&grp_str[1], (cur + 2), 120);
	grp_str[127] = '\0';
	is_group++;
	if ((grp = logger_add_string(grp_str)) == 0) goto error;
	break;

    case 'N':
	CKSTATE(CK_ANY);
	if((name1 = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'I':
	CKSTATE(CK_ANY);
	if((inst1 = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'R':
	CKSTATE(CK_ANY);
	if((realm1 = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'a':
	CKSTATE(CK_OFFSET | CK_GROUP);
	if(sscanf(cur + 2, "%d", &amt) != 1) goto error;
	break;

    case 'p':
	CKSTATE(CK_CHARGE | CK_GROUP);
	if(sscanf(cur + 2, "%d", &npages) != 1) goto error;
	break;

    case 'q':
	CKSTATE(CK_CHARGE | CK_GROUP);
	if(sscanf(cur + 2, "%d", &subt) != 1) goto error;
	break;

    case 'c':
	CKSTATE(CK_CHARGE | CK_GROUP);
	if(sscanf(cur + 2, "%d", &medcost) != 1) goto error;
	break;

    case 'w':
	CKSTATE(CK_CHARGE | CK_GROUP);
	if((where = logger_add_string(cur + 2)) == 0) goto error;
	break;

    case 'f':
	/* Watch out for order, eg. 'add_admin' before 'add' etc. */
	if(!strncasecmp(cur +2 , "add_admin", 9)) {
	    func = LO_ADD_ADMIN; 
	    CKSTATE(CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "delete_admin", 12)) {
	    func = LO_DELETE_ADMIN; 
	    CKSTATE(CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "add_user", 8)) {
	    func = LO_ADD_USER; 
	    CKSTATE(CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "delete_user", 11)) {
	    func = LO_DELETE_USER; 
	    CKSTATE(CK_GROUP);
	    break;
	}if(!strncasecmp(cur +2 , "add", 3)) {
	    func = LO_ADD; 
	    CKSTATE(CK_OFFSET);
	    break;
	} else if(!strncasecmp(cur +2 , "charge", 6)) {
	    func = LO_CHARGE; 
	    CKSTATE(CK_CHARGE | CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "subtract", 7)) {
	    func = LO_SUBTRACT; 
	    CKSTATE(CK_OFFSET | CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "set", 3)) {
	    func = LO_SET; 
	    CKSTATE(CK_OFFSET | CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "delete", 6)) {
	    func = LO_DELETEUSER; 
	    CKSTATE(CK_OFFSET | CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "allow", 5)) {
	    func = LO_ALLOW; 
	    CKSTATE(CK_OFFSET | CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "disallow", 8)) {
	    func = LO_DISALLOW; 
	    CKSTATE(CK_OFFSET | CK_GROUP);
	    break;
	} else if(!strncasecmp(cur +2 , "adjust", 6)) {
	    func = LO_ADJUST; 
	    CKSTATE(CK_OFFSET | CK_GROUP);
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

    if (!is_group) {
	out->user.name = name;
	out->user.instance = inst;
	out->user.realm = realm;
    } else {
	out->user.name = grp;
	out->user.instance = grp;
	out->user.realm = grp;
    }

    out->time = t;
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
	out->trans.charge.name = name1;
	out->trans.charge.inst = inst1;
	out->trans.charge.realm = realm1;	
	break;
    case LO_ADD_ADMIN:
    case LO_DELETE_ADMIN:
    case LO_ADD_USER:
    case LO_DELETE_USER:
	out->trans.group.uname  = name;
	out->trans.group.uinst  = inst;
	out->trans.group.urealm = realm;
	out->trans.group.aname  = name1;
	out->trans.group.ainst  = inst1;
	out->trans.group.arealm = realm1;
	break;
    default:
	syslog(LOG_ERR, "Inconsistancy in parser func = %d\n", func);
	return -1;
    }

    /* We are done so return */
    return 0;

 error:
    
    syslog(LOG_ERR, "Could not parse line (%d): %s, keyword: %s\n",
	   funcsel, buf, cur);
    return -1;
}


int logger_cvt_line(line, buf)
log_entity *line;
char *buf;
{

    char *ret, *tmp;
    char buf1[128];

    buf[0]='\0';

#define ADDSTRING(f,n) if((n) != 0) { \
                         if(!(ret = logger_num_to_string(n))) return -1; \
                         (void) sprintf(buf1,"%s=%s ", f, ret); (void) strcat(buf, buf1); \
                         }

#define ADDNUM(f,n) if((n) != 0) {(void) sprintf(buf1,"%s=%d ",f,n); (void) strcat(buf, buf1);}

#define ADDTIME(f,n) if ((n) != -1) {(void) sprintf(buf1,"%s=%d ",f,n); (void) strcat(buf, buf1);}

    if((line->user.name == 0) && (line->user.instance == 0) && 
       (line->user.realm == 0)) return 0;

    ADDTIME("t",line->time);

    tmp = logger_num_to_string(line->user.name);
    if (!tmp) return -1;

    if (tmp[0] != ':') {
	ADDSTRING("n", line->user.name);
	ADDSTRING("i", line->user.instance);
	ADDSTRING("r", line->user.realm);
    } else {
	(void) sprintf(buf1,"g=%s ", &tmp[1]); 
	(void) strcat(buf, buf1);
    }
    ADDSTRING("s", line->service);
    switch(line->func) {
    case LO_ADD:
	(void) strcat(buf, "f=add ");
	break;
    case LO_CHARGE:
	(void) strcat(buf, "f=charge ");
	break;
    case LO_SET:
	(void) strcat(buf, "f=set ");
	break;
    case LO_SUBTRACT:
	(void) strcat(buf, "f=subtract ");
	break;
    case LO_DELETEUSER:
	(void) strcat(buf, "f=delete ");
	break;
    case LO_ALLOW:
	(void) strcat(buf, "f=allow ");
	break;
    case LO_DISALLOW:
	(void) strcat(buf, "f=disallow ");
	break;
    case LO_ADJUST:
	(void) strcat(buf, "f=adjust ");
	break;
    case LO_ADD_ADMIN:
	(void) strcat(buf, "f=add_admin ");
	break;
    case LO_DELETE_ADMIN:
	(void) strcat(buf, "f=delete_admin ");
	break;
    case LO_ADD_USER:
	(void) strcat(buf, "f=add_user ");
	break;
    case LO_DELETE_USER:
	(void) strcat(buf, "f=delete_user ");
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
	ADDSTRING("w", line->trans.charge.where);
	ADDSTRING("N", line->trans.charge.name);
	ADDSTRING("I", line->trans.charge.inst);
	ADDSTRING("R", line->trans.charge.realm);
	break;
    case LO_ADD_ADMIN:
    case LO_DELETE_ADMIN:
    case LO_ADD_USER:
    case LO_DELETE_USER:
	ADDSTRING("n", line->trans.group.uname);
	ADDSTRING("i", line->trans.group.uinst);
	ADDSTRING("r", line->trans.group.urealm);
	ADDSTRING("N", line->trans.group.aname);
	ADDSTRING("I", line->trans.group.ainst);
	ADDSTRING("R", line->trans.group.arealm);
	break;
    }

    return 0;

}
