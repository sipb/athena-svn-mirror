#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/Mu.h>    
#include <Mrm/MrmAppl.h>
#include <X11/MwmUtil.h>
#include <X11/Protocols.h>
#include <sys/file.h>
#include <setjmp.h>
    
#ifdef XTCOMM_CLIENT
#include <XtComm.h>
#endif

static MrmHierarchy	Hierarchy;	/* MRM database hierarchy id */
static MrmCode		class ;
static Widget toplevel, main_widget;
static jmp_buf load;


typedef struct AppResRec{
    char *widgetName;
    char *reloadBinding;
    Boolean reportUndefined;
} *AppResPtr;

struct AppResRec AppRes;

XtResource AppResourceList[] = {
    {"widgetName","WidgetName",XtRString,
     sizeof(String), XtOffset(AppResPtr,widgetName),
     XtRString, "main"},
    {"reloadBinding","ReloadBinding",XtRString,
      sizeof(String), XtOffset(AppResPtr,reloadBinding),
      XtRString, "Alt Shift<Key>F1"},
    {"reportUndefined","ReportUndefined",XtRBoolean,
     sizeof(Boolean), XtOffset(AppResPtr,reportUndefined),
     XtRImmediate, (caddr_t)True},
};

static XrmOptionDescRec options[] = {
    {"-w", ".widgetName",XrmoptionSepArg, NULL},
    {"-reload",".reloadBinding",XrmoptionSepArg,NULL},
    {"-noReport", ".reportUndefined",XrmoptionNoArg,"False"},
};



void QuitCallback();

static MRMRegisterArg	regvec[] = {
    {"Quit", (caddr_t)QuitCallback},
};


void NewWarningHandler(msg)
String msg;
{
    char *start, *end;
    /* neaten up warning messages that tell us about undefined
       callbacks, because it is expected that some will be undefined */

    if (strncmp(msg,"Urm__CW_FixupCallback",21) == 0) {
	if (AppRes.reportUndefined) {
	    start = index(msg,'\'') + 1;
	    end = index(start,'\'');
	    *end = '\0';
	    fprintf(stderr,"Callback undefined: %s\n", start);
	}
    }
    else 
	fprintf(stderr,"X Toolkit Warning: %s\n", msg);
}


void Reload()
{
    MrmCloseHierarchy(Hierarchy);
    XtDestroyWidget(main_widget);
    longjmp(load,1);
}

void QuitCallback(w, tag, call_data)
Widget w;
caddr_t tag;
XmAnyCallbackStruct *call_data;
{
    exit(0);
}



/*
 *  Main program
 */
int main(argc, argv)
int argc;
char **argv;
{
    Atom _motif_wm_messages;
    Atom reload;
    char buf[500];
    Arg arglist[5] ;


    MrmInitialize();

    toplevel = XtInitialize(
	"mockup", 			/* application name */
	"Mockup",                       /* application class */
	options, XtNumber(options),     /* how to parse the command line*/
	&argc, argv);                   /* the command line */

    XtSetArg (arglist[0], XtNallowShellResize, TRUE) ;
    XtSetValues (toplevel, arglist, 1) ;

    MuInitialize(toplevel);

    /* filter out 'callback not defined' warnings */
    XtSetWarningHandler(NewWarningHandler);

    /* look for resources in resource database */
    XtGetApplicationResources(toplevel,&AppRes,
			      AppResourceList,XtNumber(AppResourceList),
			      NULL,0);

    
    if ((argc !=  2) || (argv[1][0] == '-')) {
	fprintf(stderr,
	  "Usage: mockup uidfile [-w widgetname] [-reload keybindings]\n");
	exit(10);
    }

    /*
     * set up 'Reload' item in the system menu.
     * arrange to be called back when it is selected.
     */
    _motif_wm_messages=XInternAtom(XtDisplay(toplevel),_XA_MWM_MESSAGES,True);
    reload = XInternAtom(XtDisplay(toplevel),"RELOAD",False);
    XmAddWMProtocols(toplevel,&_motif_wm_messages,1);
    sprintf(buf,"Reload _e %s f.send_msg %d",
	    AppRes.reloadBinding,(int)reload);
    XtSetArg (arglist[0], XmNmwmMenu, buf);
    XtSetValues(toplevel,arglist,1);
    XmAddProtocolCallback(toplevel,_motif_wm_messages,reload,Reload,NULL);


    /* register the quit callback and the Mu routines. */
    if (MrmRegisterNames (regvec, XtNumber(regvec)) != MrmSUCCESS) {
	fprintf(stderr,"mockup: can't register names\n");
	exit(1);
    }
    MuRegisterNames();

    /* remember where to jump back to on reload */
    setjmp(load);
    
    /* check that the UID file exists and is readable */
    /* if we don't do this, MrmOpenHierarchy will give gross error messages */
    if (access(argv[1],R_OK) == -1) {
	perror(argv[1]);
	exit(1);
    }
   
    if (MrmOpenHierarchy (1,		            /* number of files	    */
			&argv[1],		    /* files     	    */
			NULL,			    /* os_ext_list (null)   */
			&Hierarchy)   	            /* ptr to returned id   */
			!= MrmSUCCESS) {
	exit(1);
     }

    /*
     *  Call MRM to fetch and create the interface
     */

    if (MrmFetchWidget (Hierarchy,
			AppRes.widgetName,
			toplevel,
			&main_widget,
			&class)
			!= MrmSUCCESS) {
	fprintf(stderr,"mockup: Can't find widget named '%s' in file %s.\n",
		AppRes.widgetName, argv[1]);
	exit(1);
    }

    XtManageChild(main_widget);
    XtRealizeWidget(toplevel);
    XtMainLoop();

    /* UNREACHABLE */
    return (0);
}




