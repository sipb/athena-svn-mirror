/* wwwfetch.c */

/*
  TODO:
   
   - the DOFORKS #ifdef'ed code is never used, so no forking is
     done for fetching URLs (can libwww still sometimes hang when
     fetching them?)  The forking code is broken anyway, and utterly
     different from how forking is done in other parts of xdvi, so it
     would need to be rewritten.
     
   - redesign entire hyperref module to have a single entry point
     for fetching remote files, where we can print out error messages,
     decide what to do next etc.
*/

#include "xdvi-config.h"
#if defined(HTEX) || defined(XHDVI)
#include "kpathsea/c-fopen.h"
#include "kpathsea/c-stat.h"

#include "wwwconf.h"
#include "WWWLib.h"
#include "WWWInit.h"
#include "WWWCache.h"

#include "message-window.h"
#include "statusline.h"

#define LINE 1024
#define FILELISTCHUNK 20

typedef struct {
    char *url;
    char *savefile;
    pid_t childnum;
} Fetch_Children;

#define MAXC 10	/* Don't send off more than 10 simultaneously */
Fetch_Children fetch_children[MAXC];
static int nchildren = MAXC + 1;

static int nURLs INIT(0);

/* FIXME: why do we need this struct, *and* Fetch_Children struct?? */

static struct FiletoURLconv {
	char *url;    /* Full address of the URL we have locally */
	char *file;   /* Local file name it is stored as */
} *filelist INIT(NULL);

/* access methods to filelist for external modules */
char *
htex_file_at_index(int idx)
{
    return filelist[idx].file;
}

char *
htex_url_at_index(int idx)
{
    return filelist[idx].url;
}


/* print errors from libwww calls */
static Boolean
www_error_print(HTRequest *request,
		HTAlertOpcode op,
		int msgnum,
		const char *dfault,
		void *input,
		HTAlertPar *reply)
{
    HTParentAnchor *anchor = HTRequest_anchor(request);
    char *uri = HTAnchor_address((HTAnchor*) anchor);
    char *msg = HTDialog_errorMessage(request, op, msgnum, dfault, input);
    UNUSED(reply);
    
    if (msg) {
        do_popup_message(MSG_ERR, NULL, "Couldn't fetch `%s':\nlibwww: %s\n", uri, msg);
        free(msg);
    }
    free(uri);
    return True;
}

/* call after libwww calls to treat error messages */
static int
www_info_filter(HTRequest *request,
		 HTResponse *response,
		 void *param,
		 int status)
{
    UNUSED(param);
    UNUSED(response);
    
    switch (status) {
    case HT_LOADED:	/* success */
	break;
    default:		/* any error */
	www_error_print(request, HT_A_MESSAGE, HT_MSG_NULL, NULL,
			HTRequest_error(request), NULL);
	break;
    }
    return HT_OK;
}

/* how to get rid of the following internal? */
static const char * HTDialogs[] = {HT_MSG_ENGLISH_INITIALIZER};

static Boolean
login_proc(HTRequest *request,
	   HTAlertOpcode op, int msgnum,
	   const char *dfault, void *input,
	   HTAlertPar *reply)
{
    const char *msg = HTDialogs[msgnum]; /* it seems there is no public function for this one ??? */
    UNUSED(request);
    UNUSED(op);
    UNUSED(dfault);
    UNUSED(input);
    UNUSED(reply);

    fprintf(stderr, "LOGIN =========\n");
    fprintf(stderr, "%s\n", msg);
    do_popup_message(MSG_INFO, "Sorry, login functionality is not implemented yet", "Not implemented yet: %s", msg);
    return False;
}

static Boolean
passwd_proc(HTRequest *request,
	    HTAlertOpcode op, int msgnum,
	    const char *dfault, void *input,
	    HTAlertPar *reply)
{
    char *msg = HTDialog_errorMessage(request, op, msgnum, dfault, input);
    UNUSED(reply);
    
    fprintf(stderr, "PASSWD =========\n");
    if (msg == NULL)
	return False;
    fprintf(stderr, "%s\n", msg);
    do_popup_message(MSG_INFO,
		     "Sorry, this URI requires authentication, but "
		     "password functionality is not implemented yet", "Not implemented yet: %s", msg);
    free(msg);
    return False;
}

static Boolean
void_proc(HTRequest *request,
	  HTAlertOpcode op, int msgnum,
	  const char *dfault, void *input,
	  HTAlertPar *reply)
{
    char *msg = HTDialog_errorMessage(request, op, msgnum, dfault, input);
    if (msg == NULL)
	return False;
    do_popup_message(MSG_ERR, NULL, "Libwww error: %s", msg);
    free(msg);
    return HTAlert_setReplyMessage(reply, "anonymous");
}

/* progress messages */
static Boolean
statusline_cb(HTRequest *request,
	      HTAlertOpcode op, int msgnum,
	      const char *dfault, void *input,
	      HTAlertPar *reply)
{
    HTParentAnchor *anchor = HTRequest_anchor(request);
    char *uri = HTAnchor_address((HTAnchor*) anchor);
    char *msg = HTDialog_progressMessage(request, op, msgnum, dfault, input);
    XEvent event;

    UNUSED(reply);

    print_statusline(STATUS_LONG, "Retrieving \"%s\": %s", uri, msg);

    free(uri);
    free(msg);
    
#ifdef MOTIF
    XmUpdateDisplay(top_level);
#else
    XSync(DISP, 0);
    while (XCheckMaskEvent(DISP, ExposureMask, &event))
	XtDispatchEvent(&event);
#endif
    return True;
}

/* wrapper for HTLoadToFile to work around a bug in that function */
static int
libwww_wrapper_HTLoadToFile(const char *url, HTRequest *request, _Xconst char *filename)
{
    if (url && filename && request) {
	FILE * fp = NULL;
	
	/* file existance check omitted */

	/* open the file */
	if ((fp = xfopen_local(filename, "wb")) == NULL) {
	    HTRequest_addError(request, ERR_NON_FATAL, NO, HTERR_NO_FILE, 
			       (char *) filename, strlen(filename),
			       "HTLoadToFile"); 
	    return 0;
	}

	/* Set the output stream and start the request */
	HTRequest_setOutputFormat(request, WWW_SOURCE);
	HTRequest_setOutputStream(request, HTFWriter_new(request, fp, NO));
	if (HTLoadAbsolute(url, request) == NO) {
	    /* SU 2001/09/22: omitted closing this fp, since it will be closed a second
	       time in HTFWriter_free() (called by HTRequest_delete()), leading
	       to a segfault (this is the bug mentioned above).
	    */
	    /* fclose(fp); */
	    return 0;
	} else
	    return 1;
    }
    return 0;
}

extern Boolean dragcurs; /* defined in events.c */

/* Given absolute URL, open a temporary filename, and do the transfer */
static int
www_fetch(char *url, char *savefile)
{
    int status;
    XEvent event;
    HTRequest *request;
    HTFormat content_type;

    if (debug & DBG_HYPER) {
	fprintf(stderr, "www_fetch called with: |%s|, savefile: |%s|\n", url,
		savefile);
    }

    /* we can't just set
       
    HTAlert_setInteractive(NO);

     since this would disable all alerts/callbacks. All of the following counts as `interactive':
     - confirm before overwriting local files on disk
     - passwords for ftp servers
     - general progress messages (!)
     We want to preserve at least the latter category, maybe in the future also the
     password authentication stuff. So we need to register callbacks for all these
     types separately.
    */

    /* FIXME: is this one still needed? Do some regression testing on this. */
    HTNet_addAfter(www_info_filter, NULL, NULL, HT_ALL, HT_FILTER_LATE);
    HTAlert_add(www_error_print, HT_A_MESSAGE);
    /* all these don't require user input: */
    HTAlert_add(statusline_cb,
		HT_PROG_DNS | HT_PROG_CONNECT | HT_PROG_ACCEPT |
		HT_PROG_READ | HT_PROG_WRITE | HT_PROG_DONE |
		HT_PROG_INTERRUPT | HT_PROG_OTHER | HT_PROG_TIMEOUT |
		HT_PROG_LOGIN | HT_A_PROGRESS);
    /* these however do: */
    HTAlert_add(login_proc, HT_A_USER_PW);
    HTAlert_add(passwd_proc, HT_A_SECRET | HT_A_PROMPT);
    HTAlert_add(void_proc, HT_A_MESSAGE | HT_A_CONFIRM);
    
    request = HTRequest_new();

    if (debug & DBG_HYPER) {
	fprintf(stderr, "calling HTLoadToFile\n");
    }

    dragcurs = True;
    while (XCheckMaskEvent(DISP, ExposureMask, &event))
	XtDispatchEvent(&event);

    /*
      Use this wrapper to avoid bug with HTLoadToFile (see
      http://lists.w3.org/Archives/Public/www-lib/2001AprJun/0152.html),
      and problems with interactive confirmation before overwriting `savefile'
      (which really is a temporary file in our case):
    */
    status = libwww_wrapper_HTLoadToFile(url, request, savefile);
    
    /* Is this still needed? */
    if (status == 0) {
 	char *errmsg = HTDialog_errorMessage(request, 0, 0, NULL, HTRequest_error(request));
	do_popup_message(MSG_ERR, NULL,
			 "Couldn't open URL `%s':\nlibwww: %s\n",
			 dvi_name, errmsg);
	free(errmsg);
    }
    
    /* do we need this??? */
    /*     HTEventList_loop(request); */
    
    if (debug & DBG_HYPER) {
	fprintf(stderr, "after request: status %d\n", status);
    }
    /* Extract the content_type before deleting the request. */
    if (debug & DBG_HYPER) {
	/* content_type = request->response->content_type; */
	content_type = HTResponse_format(HTRequest_response(request));

	if (content_type == HTAtom_for("application/x-dvi"))
	    fprintf(stderr, "www_fetch(%s->%s) returned a dvi file.\n",
		    url, savefile);
	else
	    fprintf(stderr, "www_fetch(%s->%s) returned content-type: %s\n",
		    url, savefile, HTAtom_name(content_type));
    }
    HTRequest_delete(request);

#ifdef DOFORKS
    exit(1);	/* No cleanup! */
#else /* DOFORKS */
    dragcurs = False;
    return status;	/* return status: YES (1) or NO (0) */
#endif /* DOFORKS */
}

/* Turn a relative URL into an absolute one: */
void
make_absolute(char *rel, char *base, int len)
{
    char *cp, *parsed;

    if (base == NULL)
	return;
    cp = strchr(rel, '\n');
    if (cp)
	*cp = '\0';	/* Eliminate newline char */

    parsed = HTParse(rel, base, PARSE_ALL);
    strncpy(rel, parsed, len);
    free(parsed);
}

void
wait_for_urls(void)
{	/* Wait for all the children to finish... */
    int i;
#ifdef DOFORKS
    int ret, status, j;
#endif

    if (nchildren > MAXC) {
	/* Initialization needed: */
	for (i = 0; i < MAXC; i++) {
	    fetch_children[i].url = NULL;
	    fetch_children[i].savefile = NULL;
	}
    }
    else {
	for (i = 0; i < nchildren; i++) {
#ifdef DOFORKS
	    ret = wait(&status);	/* Wait for one to finish */
	    for (j = 0; j < nchildren; j++) {
		if (ret == fetch_children[j].childnum) {
		    if (debug & DBG_HYPER)
			fprintf(stderr,
				"wait_for_urls(): URL %s in file %s: status %d\n",
				fetch_children[j].url,
				fetch_children[j].savefile, status);
		    break;
		}
	    }
#else /* DOFORKS */
	    if (debug & DBG_HYPER)
		fprintf(stderr, "wait_for_urls(): URL %s in file %s\n",
			fetch_children[i].url, fetch_children[i].savefile);
#endif /* DOFORKS */
	}
    }
    nchildren = 0;
}

void
htex_cleanup ARGS((void))
{
    /* Delete all the temp files we created */
    for (; nURLs > 0; nURLs--) {
	if (debug & DBG_HYPER)
	    fprintf(stderr,"htex: trying to unlink %s\n", filelist[nURLs-1].file);
	if (unlink(filelist[nURLs - 1].file) < 0) {
	    fprintf(stderr, "Xdvi: Couldn't unlink ");
	    perror(filelist[nURLs - 1].file);
	}
    }
    HTCache_flushAll();
    HTProfile_delete();
}


/* Start a fetch of a relative URL */
int
fetch_relative_url(char *base_url, _Xconst char *rel_url)
{
    int resp;
    char *tmpfile = NULL;
    char *cp;
    char buf[LINE];
    int fd;

    if (debug & DBG_HYPER) {
	fprintf(stderr, "fetch_relative_url called: |%s|%s|!\n",
		base_url ? base_url : "NULL", rel_url);
    }

    /* SU: to disable all the WWW fetching stuff, activate the following line: */
    /* return -1; */
    
    /* Step 1: make the relative URL absolute: */
    strncpy(buf, rel_url, LINE);	/* Put it in buf */
    make_absolute(buf, base_url, LINE);

    /* Step 2: Find a temporary file to store the output in */
    fd = xdvi_temp_fd(&tmpfile);
    if (debug & DBG_HYPER) {
	fprintf(stderr, "htex: created temporary file |%s|\n", tmpfile);
    }
    if (fd == -1) {
	do_popup_message(MSG_ERR,
			 "This is serious. Either you've run out of file \
descriptors, or your disk is full. Sorry, you'll need to resolve this first ...",
			 "Couldn't create temporary file");
	return -1;
    }
    else {
	/* it would be nice if we could use the old Unix trick
	   unlink(file);
	   to ensure the file is deleted even if xdvi crashes,
	   but since all communication with libwww and child
	   processes is done via the file name, this is not
	   possible, and we have to rely on htex_cleanup()
	   instead.
	*/
	close(fd);
    }

    /* Step 3: Fork to fetch the URL */
    if (nchildren >= MAXC)
	wait_for_urls();	/* Wait for the old ones */

    cp = NULL;
    fetch_children[nchildren].url = MyStrAllocCopy(&cp, buf);
    cp = NULL;
    fetch_children[nchildren].savefile = MyStrAllocCopy(&cp, tmpfile);

    /* Step 4: Update the URL-filename list */
    if (nURLs == 0) {
	filelist = xmalloc(FILELISTCHUNK * sizeof *filelist);
	bzero(filelist, FILELISTCHUNK * sizeof *filelist);
    }
    else if (nURLs % FILELISTCHUNK == 0) {
	filelist = xrealloc(filelist,
			    (nURLs + FILELISTCHUNK) * sizeof *filelist);
	bzero(filelist + nURLs, FILELISTCHUNK * sizeof *filelist);
    }
    MyStrAllocCopy(&(filelist[nURLs].url), buf);
    MyStrAllocCopy(&(filelist[nURLs].file), tmpfile);
    nURLs++;
#ifdef DOFORKS
    fetch_children[nchildren].childnum = fork();
    if (fetch_children[nchildren].childnum == 0) {	/* Child process */
	www_fetch(buf, tmpfile);	/* Get the URL! */
	free(tmpfile);
	exit(0);	/* Make sure this process quits... */
    }
    nchildren++;
#else /* DOFORKS */
    resp = www_fetch(buf, tmpfile);	/* Get the URL! */
    free(tmpfile);
    if (resp == 0) {
	print_statusline(STATUS_LONG, "Error requesting URL: %s", buf);
	nURLs--;
	if (debug & DBG_HYPER) {
	    fprintf(stderr, "htex: unlinking |%s|\n", filelist[nURLs].file);
	}
	/* get rid of this temp file */
	if (unlink(filelist[nURLs].file) < 0) {
	    fprintf(stderr, "Xdvi: Couldn't unlink ");
	    perror(filelist[nURLs].file);
	}
	return -1;
    }
    else {
	nchildren++;
    }
#endif /* DOFORKS */
    close(fd);
    return nURLs - 1;
}

#endif /* HTEX || XHDVI */
