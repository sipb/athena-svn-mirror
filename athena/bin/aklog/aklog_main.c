/* 
 * $Id: aklog_main.c,v 1.4 1990-07-11 17:30:37 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/aklog/aklog_main.c,v $
 * $Author: qjb $
 *
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Id: aklog_main.c,v 1.4 1990-07-11 17:30:37 qjb Exp $";
#endif lint || SABER

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <krb.h>

#include <afs/auth.h>
#include <afs/cellconfig.h>
#include <afs/vice.h>
#include <afs/venus.h>
#include <afs/ptserver.h>

#include <aklog.h>
#include "linked_list.h"

#define AFSKEY "afs"
#define AFSINST ""

#define AKLOG_SUCCESS 0
#define AKLOG_USAGE 1
#define AKLOG_SOMETHINGSWRONG 2
#define AKLOG_AFS 3
#define AKLOG_KERBEROS 4
#define AKLOG_TOKEN 5
#define AKLOG_BADPATH 6
#define AKLOG_MISC 7

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define DIR '/'			/* Character that divides directories */
#define DIRSTRING "/"		/* String form of above */
#define VOLMARKER ':'		/* Character separating cellname from mntpt */
#define VOLMARKERSTRING ":"	/* String form of above */

typedef struct {
    char cell[BUFSIZ];
    char realm[REALM_SZ];
} cellinfo_t;


char *malloc();
char *calloc();
extern int errno;
extern char *sys_errlist[];

static aklog_params params;	/* Various aklog functions */
static char msgbuf[BUFSIZ];	/* String for constructing error messages */
static char *progname = NULL;	/* Name of this program */
static int dflag = FALSE;	/* Give debugging information */
static int noauth = FALSE;	/* If true, don't try to get tokens */
static int zsubs = FALSE;	/* Are we keeping track of zephyr subs? */
static linked_list zsublist;	/* List of zephyr subscriptions */
static linked_list authedcells;	/* List of cells already logged to */


#ifdef __STDC__
static char *copy_cellinfo(cellinfo_t *cellinfo)
#else
static char *copy_cellinfo(cellinfo)
  cellinfo_t *cellinfo;
#endif /* __STDC__ */
{
    cellinfo_t *new_cellinfo;

    if (new_cellinfo = (cellinfo_t *)malloc(sizeof(cellinfo_t)))
	bcopy(cellinfo, new_cellinfo, sizeof(cellinfo_t));
    
    return ((char *)new_cellinfo);
}


#ifdef __STDC__
static char *copy_string(char *string)    
#else
static char *copy_string(string)
  char *string;
#endif /* __STDC__ */
{
    char *new_string;

    if (new_string = calloc(strlen(string) + 1, sizeof(char))) 
	(void) strcpy(new_string, string);

    return (new_string);
}


/* Figure out the local afs cell of the calling host.  Exit on error. */
#ifdef __STDC__
static void get_local_cell(char *cell, int size)
#else
static void get_local_cell(cell, size)
  char *cell;
  int size;
#endif /* __STDC__ */
{
    struct afsconf_dir *configdir;
    
    if (!(configdir = afsconf_Open(AFSCONF_CLIENTNAME))) {
	sprintf(msgbuf, 
		"%s: can't get afs configuration. Is this an afs client?\n",
		progname);
	params.pstderr(msgbuf);
	params.exitprog(AKLOG_AFS);
    }
    
    if (afsconf_GetLocalCell(configdir, cell, size)) {
	sprintf(msgbuf, "%s: can't find local cell.\n", progname);
	params.pstderr(msgbuf);
	sprintf(msgbuf, "You may want to check %s/ThisCell.\n", 
		AFSCONF_CLIENTNAME);
	params.pstderr(msgbuf);
	params.exitprog(AKLOG_AFS);
    }

    afsconf_Close(configdir);
}

/* 
 * Figure out a host that is a primary server for a given cell.  If
 * we cannot get any afs information at all, we will exit.  If we just
 * fail to find out a host for this cell, continue, flagging an error.
 */
#ifdef __STDC__
static int get_host_of_cell(char *cell, char *host)
#else
static int get_host_of_cell(cell, host)
  char *cell;
  char *host;
#endif /* __STDC__ */
{
    int status = AKLOG_SUCCESS;

    struct afsconf_dir *configdir;
    struct afsconf_cell cellinfo;

    bzero(&cellinfo, sizeof(cellinfo));

    if (!(configdir = afsconf_Open(AFSCONF_CLIENTNAME))) {
	sprintf(msgbuf, 
		"%s: can't get afs configuration. Is this an afs client?\n",
		progname);
	params.pstderr(msgbuf);
	params.exitprog(AKLOG_AFS);
    }

    if (afsconf_GetCellInfo(configdir, cell, NULL, &cellinfo)) {
	sprintf(msgbuf, "%s: Can't get information about cell %s.\n",
		progname, cell);
	params.pstderr(msgbuf);
	sprintf(msgbuf, "You may want to check %s/CellServDB\n",
		 AFSCONF_CLIENTNAME);
	params.pstderr(msgbuf);
	status = AKLOG_AFS;
    }

    afsconf_Close(configdir);

    if (status == AKLOG_SUCCESS) {
	/* We don't care which host we get */
	strcpy(host, cellinfo.hostName[0]);
    }

    return(status);
}    

/*
 * Figure out the kerberos realm of the cell in question.  If we 
 * cannot find a host for this cell, we won't be able to get tokens
 * anyway, so don't bother finding the realm.
 */ 
#ifdef __STDC__
static int get_realm_of_cell(char *cell, char *realm)
#else
static int get_realm_of_cell(cell, realm)
  char *cell;
  char *realm;
#endif /* __STDC__ */
{
    int status = AKLOG_SUCCESS;
    char host[MAXHOSTNAMELEN + 1];

    bzero(host, sizeof(host));

    /* If this fails, we'll try uppercasing the cell name */
    if ((status = get_host_of_cell(cell, host)) == AKLOG_SUCCESS)
	strcpy(realm, krb_realmofhost(host));
    
    return(status);
}

/* 
 * Log to a cell.  If the cell has already been logged to, return without
 * doing anything.  Otherwise, log to it and mark that it has been logged
 * to.
 */
#ifdef __STDC__
static int auth_to_cell(char *cell, char *realm)
#else
static int auth_to_cell(cell, realm)
  char *cell;
  char *realm;
#endif /* __STDC__ */
{
    int status = AKLOG_SUCCESS;

    char *cell_to_use;		/* Cell we are going to authenticate to */
    char local_cell[BUFSIZ];	/* Our local afs cell */

    char username[BUFSIZ]; /* To hold client username structure */
    long viceId;		/* AFS uid of user */

    char name[ANAME_SZ];	/* Name of afs key */
    char instance[INST_SZ];	/* Instance of afs key */
    char realm_of_cell[REALM_SZ]; /* Kerberos realm of cell */

    CREDENTIALS c;
    struct ktc_principal aserver;
    struct ktc_principal aclient;
    struct ktc_token atoken;
    
    char *calloc();

    bzero(local_cell, sizeof(local_cell));
    bzero(name, sizeof(name));
    bzero(instance, sizeof(instance));
    bzero(realm_of_cell, sizeof(realm_of_cell));

    if (cell && cell[0])
	cell_to_use = cell;
    else {
	get_local_cell(local_cell, sizeof(local_cell));
	cell_to_use = local_cell;
    }

    if (ll_string(&authedcells, ll_s_check, cell_to_use)) {
	if (dflag) {
	    sprintf(msgbuf, "Already authenticated to %s\n", cell_to_use);
	    params.pstdout(msgbuf);
	}
	return(AKLOG_SUCCESS);
    }

    if (!noauth) {
	if (dflag) {
	    sprintf(msgbuf, "Authenticating to cell %s.\n", cell_to_use);
	    params.pstdout(msgbuf);
	}
	
	if (realm && realm[0])
	    strcpy(realm_of_cell, realm);
	else {
	    if (status = get_realm_of_cell(cell_to_use, realm_of_cell))
		return(status);
	}
	
	/* We use the afs.<cellname> convention here... */
	strcpy(name, AFSKEY);
	strncpy(instance, cell_to_use, sizeof(instance));
	instance[sizeof(instance)-1] = NULL;
	
	if (dflag) {
	    sprintf(msgbuf, "Getting tickets: %s.%s@%s\n", name, instance, 
		    realm_of_cell);
	    params.pstdout(msgbuf);
	}
	
	/* 
	 * Extract the session key from the ticket file and hand-frob an
	 * afs style authenticator.
	 */
	status = params.get_cred(name, instance, realm_of_cell, &c);
	
	if (status != KSUCCESS) {
	    if (dflag) {
		sprintf(msgbuf, 
			"Kerberos error code returned by get_cred: %d\n",
			status);
		params.pstdout(msgbuf);
	    }
	    sprintf(msgbuf, "%s: Couldn't get AFS tickets (%s.%s@%s)",
		    progname, name, instance, realm_of_cell);
	    params.pstderr(msgbuf);
	    sprintf(msgbuf," for cell %s", cell_to_use);
	    params.pstderr(msgbuf);
	    sprintf(msgbuf,":\n%s\n", krb_err_txt[status]);
	    params.pstderr(msgbuf);
	    return(AKLOG_KERBEROS);
	}
	
	strncpy(aserver.name, AFSKEY, MAXKTCNAMELEN - 1);
	strncpy(aserver.instance, AFSINST, MAXKTCNAMELEN - 1);
	strncpy(aserver.cell, cell_to_use, MAXKTCREALMLEN - 1);

	strcpy (username, c.pname);
	if (c.pinst[0]) {
	    strcat (username, ".");
	    strcat (username, c.pinst);
	}

	if (dflag) {
	    sprintf(msgbuf, "About to resolve name %s to id\n", username);
	    params.pstdout(msgbuf);
	}

	if (pr_Initialize (0, AFSCONF_CLIENTNAME, aserver.cell) == 0)
	    status = pr_SNameToId (username, &viceId);
	
	if (dflag) {
	    if (status) 
		sprintf(msgbuf, "Error %d\n", status);
	    else
		sprintf(msgbuf, "Id %d\n", viceId);
	    params.pstdout(msgbuf);
	}

	/*
	 * Contrary to what you may think by looking at the code
	 * for tokens, this hack (AFS ID %d) will not work if you
	 * change %d to something else.
	 */
	if ((status == 0) && (viceId != ANONYMOUSID))
	    sprintf (username, "AFS ID %d", viceId);

	if (dflag) {
	    sprintf(msgbuf, "Set username to %s\n", username);
	    params.pstdout(msgbuf);
	}

	strncpy(aclient.name, username, MAXKTCNAMELEN - 1);
	strcpy(aclient.instance, "");
	strncpy(aclient.cell, c.realm, MAXKTCREALMLEN - 1);
	
	atoken.kvno = c.kvno;
	atoken.startTime = c.issue_date;
	/* ticket lifetime is in five-minutes blocks. */
	atoken.endTime = c.issue_date + (c.lifetime * 5 * 60);
	bcopy (c.session, &atoken.sessionKey, 8);
	atoken.ticketLen = c.ticket_st.length;
	bcopy (c.ticket_st.dat, atoken.ticket, atoken.ticketLen);
	
	if (dflag) {
	    sprintf(msgbuf, "Getting tokens.\n");
	    params.pstdout(msgbuf);
	}
	
	/* 
	 * Last argument is only used for non-afs services; not 
	 * needed for athena service model.
	 */
	
	if (ktc_SetToken(&aserver, &atoken, &aclient)) {
	    sprintf(msgbuf, "%s: unable to obtain tokens for cell %s.\n",
		    progname, cell_to_use);
	    params.pstderr(msgbuf);
	    status = AKLOG_TOKEN;
	}
    }
    else
	if (dflag) {
	    sprintf(msgbuf, "Noauth mode; not authenticating.\n");
	    params.pstdout(msgbuf);
	}
	
    /* Record that we have logged to this cell */
    (void)ll_string(&authedcells, ll_s_add, cell_to_use);

    /* Record this cell in the list of zephyr subscriptions */
    if (ll_string(&zsublist, ll_s_add, cell_to_use) == LL_FAILURE) {
	sprintf(msgbuf, 
		"%s: failure adding cell to zephyr subscriptions list.\n",
		progname);
	params.pstderr(msgbuf);
	params.exitprog(AKLOG_MISC);
    }

    return(status);
}

#ifdef __STDC__
static int get_afs_mountpoint(char *file, char *mountpoint, int size)
#else
static int get_afs_mountpoint(file, mountpoint, size)
  char *file;
  char *mountpoint;
  int size;
#endif /* __STDC__ */
{
    char our_file[MAXPATHLEN + 1];
    char *parent_dir;
    char *last_component;
    struct ViceIoctl vio;
    char cellname[BUFSIZ];

    bzero(our_file, sizeof(our_file));
    strcpy(our_file, file);

    if (last_component = strrchr(our_file, DIR)) {
	*last_component++ = 0;
	parent_dir = our_file;
    }
    else {
	last_component = our_file;
	parent_dir = ".";
    }    
    
    bzero(cellname, sizeof(cellname));

    vio.in = last_component;
    vio.in_size = strlen(last_component)+1;
    vio.out_size = size;
    vio.out = mountpoint;

    if (!pioctl(parent_dir, VIOC_AFS_STAT_MT_PT, &vio, 0)) {
	if (strchr(mountpoint, VOLMARKER) == NULL) {
	    vio.in = file;
	    vio.in_size = strlen(file) + 1;
	    vio.out_size = sizeof(cellname);
	    vio.out = cellname;
	    
	    if (!pioctl(file, VIOC_FILE_CELL_NAME, &vio, 1)) {
		strcat(cellname, VOLMARKERSTRING);
		strcat(cellname, mountpoint + 1);
		bzero(mountpoint + 1, size - 1);
		strcpy(mountpoint + 1, cellname);
	    }
	}
	return(TRUE);
    }
    else
	return(FALSE);
}

/* 
 * This routine each time it is called returns the next directory 
 * down a pathname.  It resolves all symbolic links.  The first time
 * it is called, it should be called with the name of the path
 * to be descended.  After that, it should be called with the arguemnt
 * NULL.
 */
#ifdef __STDC__
static char *next_path(char *origpath)
#else
static char *next_path(origpath)
  char *origpath;
#endif /* __STDC__ */
{
    static char path[MAXPATHLEN + 1];
    static char pathtocheck[MAXPATHLEN + 1];

    int link = FALSE;		/* Is this a symbolic link? */
    char linkbuf[MAXPATHLEN + 1];
    char tmpbuf[MAXPATHLEN + 1];

    static char *last_comp;	/* last component of directory name */
    static char *elast_comp;	/* End of last component */
    char *t;
    int len;
    
    static int symlinkcount = 0; /* We can't exceed MAXSYMLINKS */
    
    /* If we are given something for origpath, we are initializing only. */
    if (origpath) {
	bzero(path, sizeof(path));
	bzero(pathtocheck, sizeof(pathtocheck));
	strcpy(path, origpath);
	last_comp = path;
	symlinkcount = 0;
	return(NULL);
    }

    /* We were not given origpath; find then next path to check */
    
    /* If we've gotten all the way through already, return NULL */
    if (last_comp == NULL)
	return(NULL);

    do {
	while (*last_comp == DIR)
	    strncat(pathtocheck, last_comp++, 1);
	len = (elast_comp = strchr(last_comp, DIR)) 
	    ? elast_comp - last_comp : strlen(last_comp);
	strncat(pathtocheck, last_comp, len);
	bzero(linkbuf, sizeof(linkbuf));
	if (link = (params.readlink(pathtocheck, linkbuf, 
				    sizeof(linkbuf)) > 0)) {
	    if (++symlinkcount > MAXSYMLINKS) {
		sprintf(msgbuf, "%s: %s\n", progname, sys_errlist[ELOOP]);
		params.pstderr(msgbuf);
		params.exitprog(AKLOG_BADPATH);
	    }
	    bzero(tmpbuf, sizeof(tmpbuf));
	    if (elast_comp)
		strcpy(tmpbuf, elast_comp);
	    if (linkbuf[0] == DIR) {
		/* 
		 * If this is a symbolic link to an absolute path, 
		 * replace what we have by the absolute path.
		 */
		bzero(path, strlen(path));
		bcopy(linkbuf, path, sizeof(linkbuf));
		strcat(path, tmpbuf);
		last_comp = path;
		elast_comp = NULL;
		bzero(pathtocheck, sizeof(pathtocheck));
	    }
	    else {
		/* 
		 * If this is a symbolic link to a relative path, 
		 * replace only the last component with the link name.
		 */
		strncpy(last_comp, linkbuf, strlen(linkbuf) + 1);
		strcat(path, tmpbuf);
		elast_comp = NULL;
		if (t = strrchr(pathtocheck, DIR)) {
		    t++;
		    bzero(t, strlen(t));
		}
		else
		    bzero(pathtocheck, sizeof(pathtocheck));
	    }
	}
	else
	    last_comp = elast_comp;
    }
    while(link);

    return(pathtocheck);
}
	
#ifdef __STDC__
static void add_hosts_to_zsublist(char *file)
#else
static void add_hosts_to_zsublist(file)
  char *file;
#endif /* __STDC__ */
{
    struct ViceIoctl vio;
    char outbuf[BUFSIZ];
    long *hosts;
#ifdef ALLHOSTS
    int i;
#endif /* ALLHOSTS */
    struct hostent *hp;
    
    bzero(outbuf, sizeof(outbuf));

    vio.out_size = sizeof(outbuf);
    vio.in_size = 0;
    vio.out = outbuf;

    if (dflag) {
	sprintf(msgbuf, "Getting list of hosts for %s\n", file);
	params.pstdout(msgbuf);
    }
    /* Don't worry about errors. */
    if (!pioctl(file, VIOCWHEREIS, &vio, 1)) {
	hosts = (long *) outbuf;

	/*
	 * Lists hosts that we care about.  If ALLHOSTS is defined,
	 * then all hosts that you ever may possible go through are
	 * included in this list.  If not, then only hosts that are
	 * the only ones appear.  That is, if a volume you must use
	 * is replaced on only one server, that server is included.
	 * If it is replicated on many servers, then none are included.
	 * This is not perfect, but the result is that people don't
	 * get subscribed to a lot of instances of FILSRV that they
	 * probably won't need which reduces the instances of 
	 * people getting messages that don't apply to them.
	 */
#ifdef ALLHOSTS
	for (i = 0; hosts[i]; i++) 
	    if (hp = gethostbyaddr(&hosts[i], sizeof(long), AF_INET)) {
		if (dflag) {
		    sprintf(msgbuf, "Got host %s\n", hp->h_name);
		    params.pstdout(msgbuf);
		}
		ll_string(&zsublist, SL_ADD, hp->h_name);
	    }
#else
	if (hosts[1] == NULL) 
	    if (hp = gethostbyaddr(&hosts[0], sizeof(long), AF_INET)) {
		if (dflag) {
		    sprintf(msgbuf, "Got host %s\n", hp->h_name);
		    params.pstdout(msgbuf);
		}
		ll_string(&zsublist, ll_s_add, hp->h_name);
	    }
#endif /* ALLHOSTS */
    }
}
    
/*
 * This routine descends through a path to a directory, logging to 
 * every cell it encounters along the way.
 */
#ifdef __STDC__
static int auth_to_path(char *path)
#else
static int auth_to_path(path)
  char *path;			/* The path to which we try to authenticate */
#endif /* __STDC__ */
{
    int status = AKLOG_SUCCESS;

    char *nextpath;
    char pathtocheck[MAXPATHLEN + 1];
    char mountpoint[MAXPATHLEN + 1];

    char *cell;
    char *endofcell;

    u_char isdir;

    /* Initialize */
    if (path[0] == DIR)
	strcpy(pathtocheck, path);
    else {
	if (params.getwd(pathtocheck) == NULL) {
	    sprintf(msgbuf, "Unable to find current working directory.  ");
	    params.pstderr(msgbuf);
	    sprintf(msgbuf, "Try an absolute pathname:\n");
	    params.pstderr(msgbuf);
	    sprintf(msgbuf, "%s\n", pathtocheck);
	    params.pstderr(msgbuf);
	    params.exitprog(AKLOG_BADPATH);
	}
	else {
	    strcat(pathtocheck, DIRSTRING);
	    strcat(pathtocheck, path);
	}
    }
    next_path(pathtocheck);

    /* Go on to the next level down the path */
    while (nextpath = next_path(NULL)) {
	strcpy(pathtocheck, nextpath);
	if (dflag) {
	    sprintf(msgbuf, "Checking directory %s\n", pathtocheck);
	    params.pstdout(msgbuf);
	}
	/* 
	 * If this is an afs mountpoint, determine what cell from 
	 * the mountpoint name which is of the form 
	 * #cellname:volumename or %cellname:volumename.
	 */
	if (get_afs_mountpoint(pathtocheck, mountpoint, sizeof(mountpoint))) {
	    /* skip over the '#' or '%' */
	    cell = mountpoint + 1;
	    /* Add this (cell:volumename) to the list of zsubs */
	    if (zsubs) {
		ll_string(&zsublist, ll_s_add, cell);
		add_hosts_to_zsublist(pathtocheck);
	    }
	    if (endofcell = strchr(mountpoint, VOLMARKER)) {
		*endofcell = NULL;
		auth_to_cell(cell, NULL);
	    }
	}
	else
	    if (params.isdir(pathtocheck, &isdir) < 0) {
		/*
		 * If we've logged and still can't stat, there's
		 * a problem... 
		 */
		sprintf(msgbuf, "%s: %s: %s\n", progname, 
			pathtocheck, sys_errlist[errno]);
		params.pstderr(msgbuf);
		return(AKLOG_BADPATH);
	    }
	    else if (! isdir) {
		/* Allow only directories */
		sprintf(msgbuf, "%s: %s: %s\n", progname, pathtocheck,
			sys_errlist[ENOTDIR]);
		params.pstderr(msgbuf);
		return(AKLOG_BADPATH);
	    }
    }

    return(status);
}

/* Print usage message and exit */
#ifdef __STDC__
static void usage(void)
#else
static void usage()
#endif /* __STDC__ */
{
    sprintf(msgbuf, "\nUsage: %s %s%s\n", progname,
	    "[-d] [[-cell | -c] cell [-k krb_realm]] [[-path] pathname]\n",
	    "    [-zsubs] [-noauth]\n");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "    -d gives debugging information.\n");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "    krb_realm is the kerberos realm of a cell.\n");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "    pathname is the name of a directory to which ");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "you wish to authenticate.\n");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "    -zsubs gives zephyr subscription information.\n");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "    -noauth does not attempt to get tokens.\n");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "    No commandline arguments means ");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "authenticate to the local cell.\n");
    params.pstderr(msgbuf);
    sprintf(msgbuf, "\n");
    params.pstderr(msgbuf);
    params.exitprog(AKLOG_USAGE);
}

#ifdef __STDC__
void aklog(int argc, char *argv[], aklog_params *a_params)
#else
void aklog(argc, argv, a_params)
  int argc;
  char *argv[];
  aklog_params *a_params;
#endif /* __STDC__ */
{
    int status = AKLOG_SUCCESS;
    int i;
    int somethingswrong = FALSE;

    cellinfo_t cellinfo;

    extern char *progname;	/* Name of this program */

    extern int dflag;		/* Debug mode */

    int cmode = FALSE;		/* Cellname mode */
    int pmode = FALSE;		/* Path name mode */

    char realm[REALM_SZ];	/* Kerberos realm of afs server */
    char cell[BUFSIZ];		/* Cell to which we are authenticating */
    char path[MAXPATHLEN + 1];		/* Path length for path mode */

    linked_list cells;		/* List of cells to log to */
    linked_list paths;		/* List of paths to log to */
    ll_node *cur_node;

    bzero(&cellinfo, sizeof(cellinfo));

    bzero(realm, sizeof(realm));
    bzero(cell, sizeof(cell));
    bzero(path, sizeof(path));

    ll_init(&cells);
    ll_init(&paths);

    ll_init(&zsublist);

    /* Store the program name here for error messages */
    if (progname = strrchr(argv[0], DIR))
	progname++;
    else
	progname = argv[0];

    bcopy((char *)a_params, (char *)&params, sizeof(aklog_params));

    /* Initialize list of cells to which we have authenticated */
    (void)ll_init(&authedcells);

    /* Parse commandline arguments and make list of what to do. */
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-d") == 0)
	    dflag++;
	else if (strcmp(argv[i], "-noauth") == 0) 
	    noauth++;
	else if (strcmp(argv[i], "-zsubs") == 0)
	    zsubs++;
	else if (((strcmp(argv[i], "-cell") == 0) ||
		  (strcmp(argv[i], "-c") == 0)) && !pmode)
	    if (++i < argc) {
		cmode++;
		strcpy(cell, argv[i]);
	    }
	    else
		usage();
	else if ((strcmp(argv[i], "-path") == 0) && !cmode)
	    if (++i < argc) {
		pmode++;
		strcpy(path, argv[i]);
	    }
	    else
		usage();
	else if (argv[i][0] == '-')
	    usage();
	else if (!pmode && !cmode) {
	    if (strchr(argv[i], DIR) || (strcmp(argv[i], ".") == 0) ||
		(strcmp(argv[i], "..") == 0)) {
		pmode++;
		strcpy(path, argv[i]);
	    }
	    else { 
		cmode++;
		strcpy(cell, argv[i]);
	    }
	}
	else
	    usage();

	if (cmode) {
	    if (((i + 1) < argc) && (strcmp(argv[i + 1], "-k") == NULL)) {
		i+=2;
		if (i < argc)
		    strcpy(realm, argv[i]);
		else
		    usage();
	    }
	    /* Add this cell to list of cells */
	    strcpy(cellinfo.cell, cell);
	    strcpy(cellinfo.realm, realm);
	    if (cur_node = ll_add_node(&cells, ll_tail)) {
		char *new_cellinfo;
		if (new_cellinfo = copy_cellinfo(&cellinfo))
		    ll_add_data(cur_node, new_cellinfo);
		else {
		    sprintf(msgbuf, 
			    "%s: failure copying cellinfo.\n", progname);
		    params.pstderr(msgbuf);
		    params.exitprog(AKLOG_MISC);
		}
	    }
	    else {
		sprintf(msgbuf, "%s: failure adding cell to cells list.\n",
			progname);
		params.pstderr(msgbuf);
		params.exitprog(AKLOG_MISC);
	    }
	    bzero(&cellinfo, sizeof(cellinfo));
	    cmode = FALSE;
	    bzero(cell, sizeof(cell));
	    bzero(realm, sizeof(realm));
	}
	else if (pmode) {
	    /* Add this path to list of paths */
	    if (cur_node = ll_add_node(&paths, ll_tail)) {
		char *new_path; 
		if (new_path = copy_string(path)) 
		    ll_add_data(cur_node, new_path);
		else {
		    sprintf(msgbuf, "%s: failure copying path name.\n",
			    progname);
		    params.pstderr(msgbuf);
		    params.exitprog(AKLOG_MISC);
		}
	    }
	    else {
		sprintf(msgbuf, "%s: failure adding path to paths list.\n",
			progname);
		params.pstderr(msgbuf);
		params.exitprog(AKLOG_MISC);
	    }
	    pmode = FALSE;
	    bzero(path, sizeof(path));
	}
    }

    /* If nothing was given, log to the local cell. */
    if ((cells.nelements + paths.nelements) == 0)
	status = auth_to_cell(NULL, NULL);
    else {
	/* Log to all cells in the cells list first */
	for (cur_node = cells.first; cur_node; cur_node = cur_node->next) {
	    bcopy(cur_node->data, (char *)&cellinfo, sizeof(cellinfo));
	    if (status = auth_to_cell(cellinfo.cell, cellinfo.realm))
		somethingswrong++;
	}
	
	/* Then, log to all paths in the paths list */
	for (cur_node = paths.first; cur_node; cur_node = cur_node->next) {
	    if (status = auth_to_path(cur_node->data))
		somethingswrong++;
	}
	
	/* 
	 * If only one thing was logged to, we'll return the status 
	 * of the single call.  Otherwise, we'll return a generic
	 * something failed status.
	 */
	if (somethingswrong && ((cells.nelements + paths.nelements) > 1))
	    status = AKLOG_SOMETHINGSWRONG;
    }

    /* If we are keeping track of zephyr subscriptions, print them. */
    if (zsubs) 
	for (cur_node = zsublist.first; cur_node; cur_node = cur_node->next) {
	    sprintf(msgbuf, "zsub: %s\n", cur_node->data);
	    params.pstdout(msgbuf);
	}
    
    params.exitprog(status);
}
