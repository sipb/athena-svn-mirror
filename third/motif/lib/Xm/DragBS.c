/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: DragBS.c,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:47 $"
#endif
#endif
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

/*****************************************************************************
 *
 *  The purpose of the routines in this module is to cache frequently needed
 *  data on window properties to reduce roundtrip server requests.
 *
 *  The data is stored on window properties of motifWindow, a persistent,
 *  override-redirect, InputOnly child window of the display's default root
 *  window.  A client looks for the motifWindow id on the "_MOTIF_DRAG_WINDOW"
 *  property of the display's default root window.  If it finds the id, the
 *  client saves it in its displayToMotifWindowContext.  Otherwise, the 
 *  client creates the motifWindow and stores its id on that root property
 *  and saves the id in its displayToMotifWindowContext.  MotifWindow is
 *  mapped but is not visible on the screen.
 *
 *  Three sets of data are stored on motifWindow properties:
 *
 *    1. a list of atom names/value pairs,
 *    2. an atom table, and
 *    3. a targets list table.
 *
 *  The first time XmInternAtom() is called with a particular display and
 *  nonNULL name, it calls _XmInitAtomPairs().  _XmInitAtomPairs(), below,
 *  attempts to read a list of atoms name/value pairs from the
 *  "_MOTIF_DRAG_ATOM_PAIRS" property on motifWindow.  If motifWindow
 *  does not yet exist, _XmInitAtomPairs() creates it.  If the property 
 *  does not exist on motifWindow, _XmInitAtomPairs() creates the list of
 *  atom name/value pairs and stores it on the property.  This list of
 *  atom name/value pairs includes most of the atoms used by Xm.  If,
 *  instead, the client finds the "_MOTIF_DRAG_ATOM_PAIRS" property it
 *  reads the list and invokes _XmInternAtomAndName() with each to save
 *  them in the nameToAtomContext and atomToNameContext contexts, avoiding
 *  multiple roundtrip server requests to intern the atoms itself. 
 *
 *  The "_MOTIF_DRAG_ATOMS" property on motifWindow contains an atoms table,
 *  consisting of pairs of atoms and timestamps.  The atoms are interned
 *  once and are available for clients to use without repeated roundtrip
 *  server requests to intern them.  A timestamp of 0 indicates that the 
 *  atom is available.  A nonzero timestamp indicates when the atom was last
 *  allocated to a client.  The atoms table initially contains only atom
 *  "_MOTIF_ATOM_0" with timestamp 0.  A client requests an atom by calling
 *  _XmAllocMotifAtom() with a timestamp.  _XmAllocMotifAtom() tries to find
 *  an available atom in the table.  If it succeeds it sets the atom's
 *  timestamp to the value specified and returns the atom.  If no atom is
 *  available, _XmAllocMotifAtom() adds an atom to the table with the
 *  specified timestamp, updates the "_MOTIF_DRAG_ATOMS" property on
 *  motifWindow, and returns the new atom.  These new atoms are named
 *  "_MOTIF_ATOM_n" where n is 1, 2, 3, ... .  The routine _XmGetMotifAtom()
 *  returns the atom from the atoms table with nonzero timestamp less than
 *  but closest to a specified value.  It does not change the atoms table.
 *  A client frees an atom by calling _XmFreeMotifAtom(), which sets the 
 *  atom's timestamp to 0 and updates the "_MOTIF_DRAG_ATOMS" property on
 *  motifWindow.  To minimize property access, the client saves the address
 *  of its current atoms table on the displayToAtomsContext context.
 *
 *  The "_MOTIF_DRAG_TARGETS" property on motifWindow contains a targets
 *  table, consisting of a sequence of target lists to be shared among
 *  clients.  These target lists are sorted into ascending order to avoid
 *  permutations.  By sharing the targets table, clients may pass target
 *  lists between themselves by passing instead the corresponding target
 *  list indexes.  The routine _XmInitTargetsTable() initializes the atoms
 *  table on the "_MOTIF_DRAG_ATOMS" property, then initializes the targets
 *  table on the "_MOTIF_DRAG_TARGETS" property to contain only two lists:
 *
 *		{ 0,		}, and
 *		{ XA_STRING,	} 
 *
 *  A client adds a target list to the targets table by passing the target
 *  list to _XmTargetsToIndex().  _XmTargetsToIndex() first sorts the target
 *  list into ascending order, then searches the targets table for a match.
 *  If it finds a match, it returns the index of the matching targets table
 *  entry.  Otherwise, it adds the sorted target list to the table, updates
 *  the "_MOTIF_DRAG_TARGETS" property on motifWindow, and returns the index
 *  of the new targets table entry.  A client uses _XmIndexToTargets() to
 *  map a targets table index to a target list.  To minimize property access,
 *  the client saves the address of its current targets table on the
 *  displayToTargetsContext context.
 *
 *  The "_MOTIF_DRAG_PROXY_WINDOW" property on motifWindow contains the
 *  window id of the DragNDrop proxy window.  The routine
 *  _XmGetDragProxyWindow() returns the window id stored there.
 ***************************************************************************/

#include <Xm/XmP.h>
#include "AtomMgrI.h"
#include "DragICCI.h"
#include "DragBSI.h"
#include "MessagesI.h"
#include <Xm/MwmUtil.h>

#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include <stdio.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif


#ifdef I18N_MSG
#include "XmMsgI.h"
#endif


#undef _XmIndexToTargets
#undef _XmTargetsToIndex

#define MAXSTACK	1200
#define MAXPROPLEN	100000L

/* structures to improve portability of WriteTargets */
typedef struct {
    CARD32 value B32;
} CARD32Item;

typedef struct {
    CARD16 value B16;
    CARD16 pad B16;
} CARD16Item;

/* atom names to cache */

#define _XA_MOTIF_WINDOW	"_MOTIF_DRAG_WINDOW"
#define _XA_MOTIF_PROXY_WINDOW	"_MOTIF_DRAG_PROXY_WINDOW"
#define _XA_MOTIF_ATOM_PAIRS	"_MOTIF_DRAG_ATOM_PAIRS"
#define _XA_MOTIF_ATOMS		"_MOTIF_DRAG_ATOMS"
#define _XA_MOTIF_TARGETS	"_MOTIF_DRAG_TARGETS"
#define _XA_MOTIF_ATOM_0	"_MOTIF_ATOM_0"

#define MAX_ATOM_NAME_LEN 30		/* >= length of longest atom name */


#ifdef I18N_MSG
#define MESSAGE1	catgets(Xm_catd,MS_DragBS,MSG_DRB_1,_XmMsgDragBS_0000)
#define MESSAGE2	catgets(Xm_catd,MS_DragBS,MSG_DRB_2,_XmMsgDragBS_0001)
#define MESSAGE3	catgets(Xm_catd,MS_DragBS,MSG_DRB_3,_XmMsgDragBS_0002)
#define MESSAGE4	catgets(Xm_catd,MS_DragBS,MSG_DRB_4,_XmMsgDragBS_0003)
#define MESSAGE5	catgets(Xm_catd,MS_DragBS,MSG_DRB_5,_XmMsgDragBS_0004)
#define MESSAGE6	catgets(Xm_catd,MS_DragBS,MSG_DRB_6,_XmMsgDragBS_0005)
#define MESSAGE7	catgets(Xm_catd,MS_DragBS,MSG_DRB_7,_XmMsgDragBS_0006)
#else
#define MESSAGE1	_XmMsgDragBS_0000
#define MESSAGE2	_XmMsgDragBS_0001
#define MESSAGE3	_XmMsgDragBS_0002
#define MESSAGE4	_XmMsgDragBS_0003
#define MESSAGE5	_XmMsgDragBS_0004
#define MESSAGE6	_XmMsgDragBS_0005
#define MESSAGE7	_XmMsgDragBS_0006
#endif


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static int LocalErrorHandler() ;
static void StartProtectedSection() ;
static void EndProtectedSection() ;
static Window GetMotifWindow() ;
static void SetMotifWindow() ;
static xmTargetsTable GetTargetsTable() ;
static void SetTargetsTable() ;
static xmAtomsTable GetAtomsTable() ;
static void SetAtomsTable() ;
static Window ReadMotifWindow() ;
static Window CreateMotifWindow() ;
static void WriteMotifWindow() ;
static void WriteAtomPairs() ;
static Boolean ReadAtomPairs() ;
static void WriteAtomsTable() ;
static Boolean ReadAtomsTable() ;
static void WriteTargetsTable() ;
static Boolean ReadTargetsTable() ;
static xmTargetsTable CreateDefaultTargetsTable() ;
static xmAtomsTable CreateDefaultAtomsTable() ;
static int AtomCompare() ;

#else

static int LocalErrorHandler( 
                        Display *display,
                        XErrorEvent *error) ;
static void StartProtectedSection( 
                        Display *display,
                        Window window) ;
static void EndProtectedSection( 
                        Display *display) ;
static Window GetMotifWindow( 
                        Display *display) ;
static void SetMotifWindow( 
                        Display *display,
                        Window motifWindow) ;
static xmTargetsTable GetTargetsTable( 
                        Display *display) ;
static void SetTargetsTable( 
                        Display *display,
                        xmTargetsTable targetsTable) ;
static xmAtomsTable GetAtomsTable( 
                        Display *display) ;
static void SetAtomsTable( 
                        Display *display,
                        xmAtomsTable atomsTable) ;
static Window ReadMotifWindow( 
                        Display *display) ;
static Window CreateMotifWindow( 
                        Display *display) ;
static void WriteMotifWindow( 
                        Display *display,
                        Window *motifWindow) ;
static void WriteAtomPairs( 
                        Display *display) ;
static Boolean ReadAtomPairs( 
                        Display *display) ;
static void WriteAtomsTable( 
                        Display *display,
                        xmAtomsTable atomsTable) ;
static Boolean ReadAtomsTable( 
                        Display *display,
                        xmAtomsTable atomsTable) ;
static void WriteTargetsTable( 
                        Display *display,
                        xmTargetsTable targetsTable) ;
static Boolean ReadTargetsTable( 
                        Display *display,
                        xmTargetsTable targetsTable) ;
static xmTargetsTable CreateDefaultTargetsTable( 
                        Display *display) ;
static xmAtomsTable CreateDefaultAtomsTable( 
                        Display *display) ;
static int AtomCompare( 
                        XmConst void *atom1,
                        XmConst void *atom2) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


static String atomNames[] = {	
				_XA_MOTIF_WINDOW,
			  	_XA_MOTIF_ATOM_PAIRS,
			  	_XA_MOTIF_ATOMS,
			  	_XA_MOTIF_ATOM_0,
			  	_XA_MOTIF_TARGETS,
				_XA_MOTIF_WM_OFFSET, 
				_XA_MOTIF_PROXY_WINDOW,
				_XA_MOTIF_WM_MESSAGES, 
				_XA_MWM_HINTS, 
				_XA_MWM_MENU, 
				_XA_MOTIF_WM_INFO,
				_XA_MOTIF_BINDINGS,
				"ATOM_PAIR",
				"AVERAGE_WIDTH",
				"CLIPBOARD",
				"CLIP_TEMPORARY",
				"COMPOUND_TEXT",
				"DELETE",
				"INCR",
				"INSERT_SELECTION",
				"LENGTH",
				"MOTIF_DESTINATION",
				"MULTIPLE",
				"PIXEL_SIZE",
				"RESOLUTION_Y",
				"TARGETS",
				"TEXT",
				"TIMESTAMP",
				"WM_DELETE_WINDOW",
				"WM_PROTOCOLS",
				"WM_STATE",
				"XmTRANSFER_SUCCESS",
				"XmTRANSFER_FAILURE",
				"_MOTIF_CLIP_DATA_DELETE",
				"_MOTIF_CLIP_DATA_REQUEST",
				"_MOTIF_CLIP_HEADER",
				"_MOTIF_CLIP_LOCK_ACCESS_VALID",
				"_MOTIF_CLIP_MESSAGE",
				"_MOTIF_CLIP_NEXT_ID",
				"_MOTIF_CLIP_TIME",
				"_MOTIF_DROP",
				"_MOTIF_DRAG_INITIATOR_INFO",
				"_MOTIF_DRAG_RECEIVER_INFO",
				"_MOTIF_MESSAGE",
				"_XM_TEXT_I_S_PROP",
				};

static Boolean		bad_window;
static XErrorHandler	oldErrorHandler = NULL;
static unsigned long	firstProtectRequest;
static Window		errorWindow;

static XContext 	displayToMotifWindowContext = (XContext) NULL;
static XContext 	displayToTargetsContext = (XContext) NULL;
static XContext		displayToAtomsContext = (XContext) NULL;


/*****************************************************************************
 *
 *  LocalErrorHandler ()
 *
 ***************************************************************************/

static int 
#ifdef _NO_PROTO
LocalErrorHandler( display, error )
        Display *display ;
        XErrorEvent *error ;
#else
LocalErrorHandler(
        Display *display,
        XErrorEvent *error )
#endif /* _NO_PROTO */
{
    if (error->error_code == BadWindow &&
	error->resourceid == errorWindow &&
	error->serial >= firstProtectRequest) {
        bad_window = True;
	return 0;
    }

    if (oldErrorHandler == NULL) {
        return 0;  /* should never happen */
    }

    return (*oldErrorHandler)(display, error);
}

/*****************************************************************************
 *
 *  StartProtectedSection ()
 *
 *  To protect against reading or writing to a property on a window that has
 *  been destroyed.
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
StartProtectedSection( display, window )
        Display *display ;
        Window window ;
#else
StartProtectedSection(
        Display *display,
        Window window )
#endif /* _NO_PROTO */
{
    bad_window = False;
    oldErrorHandler = XSetErrorHandler (LocalErrorHandler);
    firstProtectRequest = NextRequest (display);
    errorWindow = window;
}

/*****************************************************************************
 *
 *  EndProtectedSection ()
 *
 *  Flushes any generated errors on and restores the original error handler.
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
EndProtectedSection( display )
        Display *display ;
#else
EndProtectedSection(
        Display *display )
#endif /* _NO_PROTO */
{
    XSync (display, False);
    XSetErrorHandler (oldErrorHandler);
    oldErrorHandler = NULL;
}

/*****************************************************************************
 *
 *  GetMotifWindow ()
 *
 *  Get the motifWindow id from the displayToMotifWindowContext.
 ***************************************************************************/

static Window 
#ifdef _NO_PROTO
GetMotifWindow( display )
        Display *display ;
#else
GetMotifWindow(
        Display *display )
#endif /* _NO_PROTO */
{
    Window	motifWindow;
    
    if (displayToMotifWindowContext == (XContext) NULL) {
        displayToMotifWindowContext = XUniqueContext();
    }
    
    if (XFindContext(display, 
                     DefaultRootWindow (display),
		     displayToMotifWindowContext, 
		     (char **)&motifWindow)) {
        motifWindow = None;
    }
    return (motifWindow);
}

/*****************************************************************************
 *
 *  SetMotifWindow ()
 *
 *  Set the motifWindow id into the displayToMotifWindowContext.
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
SetMotifWindow( display, motifWindow )
        Display *display ;
        Window motifWindow ;
#else
SetMotifWindow(
        Display *display,
        Window motifWindow )
#endif /* _NO_PROTO */
{
    Window oldMotifWindow;

    if (displayToMotifWindowContext == (XContext) NULL) {
        displayToMotifWindowContext = XUniqueContext();
    }

    /*
     * Save window data.
     * Delete old context if one exists.
     */
    if (XFindContext (display, DefaultRootWindow (display),
			displayToMotifWindowContext,
			(char **) &oldMotifWindow)) {
	XSaveContext(display, 
                        DefaultRootWindow (display),
			displayToMotifWindowContext, 
			(char *) motifWindow);
    }
    else if (oldMotifWindow != motifWindow) {
	XDeleteContext (display, DefaultRootWindow (display),
			displayToMotifWindowContext);
	XSaveContext(display, 
                        DefaultRootWindow (display),
			displayToMotifWindowContext, 
			(char *) motifWindow);
    }
}

/*****************************************************************************
 *
 *  GetTargetsTable ()
 *
 *  Get the targets table address from the displayToTargetsContext.
 ***************************************************************************/

static xmTargetsTable 
#ifdef _NO_PROTO
GetTargetsTable( display )
        Display *display ;
#else
GetTargetsTable(
        Display *display )
#endif /* _NO_PROTO */
{
    xmTargetsTable	targetsTable;
    
    if (displayToTargetsContext == (XContext) NULL) {
        displayToTargetsContext = XUniqueContext();
    }
    
    if (XFindContext(display, 
                     DefaultRootWindow (display),
		     displayToTargetsContext, 
		     (char **)&targetsTable)) {
        targetsTable = NULL;
    }
    return (targetsTable);
}

/*****************************************************************************
 *
 *  SetTargetsTable ()
 *
 *  Set the targets table address into the displayToTargetsContext.
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
SetTargetsTable( display, targetsTable )
        Display *display ;
        xmTargetsTable targetsTable ;
#else
SetTargetsTable(
        Display *display,
        xmTargetsTable targetsTable )
#endif /* _NO_PROTO */
{
    xmTargetsTable oldTargetsTable;

    if (displayToTargetsContext == (XContext) NULL) {
        displayToTargetsContext = XUniqueContext();
    }

    /*
     * Save targets data.
     * Delete old context if one exists.
     */
    if (XFindContext (display, DefaultRootWindow (display),
			displayToTargetsContext,
			(char **) &oldTargetsTable)) {
	XSaveContext(display, 
                        DefaultRootWindow (display),
			displayToTargetsContext, 
			(char *) targetsTable);
    }
    else if (oldTargetsTable != targetsTable) {
	XDeleteContext (display, DefaultRootWindow (display),
			displayToTargetsContext);
        {
          unsigned i = 0 ;
          while(    i < oldTargetsTable->numEntries    )
            {
              XtFree( (char *)oldTargetsTable->entries[i++].targets) ;
            }
          XtFree( (char *)oldTargetsTable->entries) ;
          XtFree( (char *)oldTargetsTable) ;
        }
	XSaveContext(display, 
                        DefaultRootWindow (display),
			displayToTargetsContext, 
			(char *) targetsTable);
    }
}

/*****************************************************************************
 *
 *  GetAtomsTable ()
 *
 *  Get the atomsTable address from the displayToAtomsContext.
 ***************************************************************************/

static xmAtomsTable 
#ifdef _NO_PROTO
GetAtomsTable( display )
        Display *display ;
#else
GetAtomsTable(
        Display *display )
#endif /* _NO_PROTO */
{
    xmAtomsTable	atomsTable;
    
    if (displayToAtomsContext == (XContext) NULL) {
	displayToAtomsContext = XUniqueContext();
    }
    
    if (XFindContext (display, 
                      DefaultRootWindow (display),
		      displayToAtomsContext,
		      (XPointer *)&atomsTable)) {
        atomsTable = NULL;
    }
    return (atomsTable);
}

/*****************************************************************************
 *
 *  SetAtomsTable ()
 *
 *  Set the atoms table address into the displayToAtomsContext.
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
SetAtomsTable( display, atomsTable )
        Display *display ;
        xmAtomsTable atomsTable ;
#else
SetAtomsTable(
        Display *display,
        xmAtomsTable atomsTable )
#endif /* _NO_PROTO */
{
    xmAtomsTable oldAtomsTable;

    if (displayToAtomsContext == (XContext) NULL) {
        displayToAtomsContext = XUniqueContext();
    }

    /*
     * Save atom data.
     * Delete old context if one exists.
     */
    if (XFindContext (display, DefaultRootWindow (display),
			displayToAtomsContext,
			(char **) &oldAtomsTable)) {
	XSaveContext(display, 
                        DefaultRootWindow (display),
	                displayToAtomsContext,
			(char *) atomsTable);
    }
    else if (oldAtomsTable != atomsTable) {
	XDeleteContext (display, DefaultRootWindow (display),
			displayToAtomsContext);
        XtFree( (char *)(oldAtomsTable->entries)) ;
        XtFree( (char *)oldAtomsTable) ;
	XSaveContext(display, 
                        DefaultRootWindow (display),
	                displayToAtomsContext,
			(char *) atomsTable);
    }
}

/*****************************************************************************
 *
 *  ReadMotifWindow ()
 *
 ***************************************************************************/

static Boolean RMW_ErrorFlag;

static int
#ifdef _NO_PROTO
RMW_ErrorHandler(display, event)
     Display *display;
     XErrorEvent *event;
#else
RMW_ErrorHandler(Display *display, XErrorEvent* event)
#endif
{
    RMW_ErrorFlag = True;
    return 0 ; /* unused */
}

static Window 
#ifdef _NO_PROTO
ReadMotifWindow( display )
        Display *display ;
#else
ReadMotifWindow(
        Display *display )
#endif /* _NO_PROTO */
{
    Atom            motifWindowAtom;
    Atom            type;
    int             format;
    unsigned long   lengthRtn;
    unsigned long   bytesafter;
    Window         *property = NULL;
    Window	    motifWindow = None;
    XErrorHandler old_Handler;

    /* Setup error proc and reset error flag */
    old_Handler = XSetErrorHandler((XErrorHandler) RMW_ErrorHandler);
    RMW_ErrorFlag = False;

    motifWindowAtom = XmInternAtom (display, _XA_MOTIF_WINDOW, False);

    if ((XGetWindowProperty (display,
                             DefaultRootWindow (display),
                             motifWindowAtom,
                             0L, MAXPROPLEN,
			     False,
                             AnyPropertyType,
                             &type,
			     &format,
			     &lengthRtn,
			     &bytesafter, 
			     (unsigned char **) &property) == Success) &&
         (type == XA_WINDOW) &&
	 (format == 32) &&
	 (lengthRtn == 1)) {
	motifWindow = *property;
    }
    if (property) {
	XFree ((char *)property);
    }

    XSetErrorHandler(old_Handler);

    if (RMW_ErrorFlag) motifWindow = None;

    return (motifWindow);
}

/*****************************************************************************
 *
 *  CreateMotifWindow ()
 *
 *  Creates a persistent window to hold the target list and atom pair
 *  properties.  This window is not visible on the screen.
 *
 ***************************************************************************/

static Window 
#ifdef _NO_PROTO
CreateMotifWindow( display )
        Display *display ;
#else
CreateMotifWindow(
        Display *display )
#endif /* _NO_PROTO */
{
    XSetWindowAttributes sAttributes;
    Window	         motifWindow;

    sAttributes.override_redirect = True;
    sAttributes.event_mask = PropertyChangeMask;
    motifWindow = XCreateWindow (display,
                                 DefaultRootWindow (display),
			         -100, -100, 10, 10, 0, 0,
			         InputOnly,
			         CopyFromParent,
			         (CWOverrideRedirect |CWEventMask),
			         &sAttributes);
    XMapWindow (display, motifWindow);
    return (motifWindow);
}

/*****************************************************************************
 *
 *  WriteMotifWindow ()
 *
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
WriteMotifWindow( display, motifWindow )
        Display *display ;
        Window *motifWindow ;
#else
WriteMotifWindow(
        Display *display,
        Window *motifWindow )
#endif /* _NO_PROTO */
{
    Atom	motifWindowAtom;

    motifWindowAtom = XmInternAtom (display, _XA_MOTIF_WINDOW, False);

    XChangeProperty (display,
                     DefaultRootWindow (display),
                     motifWindowAtom,
		     XA_WINDOW,
		     32,
		     PropModeReplace,
		     (unsigned char *) motifWindow,
		     1);
}

/*****************************************************************************
 *
 *  WriteAtomPairs ()
 *
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
WriteAtomPairs( display )
        Display *display ;
#else
WriteAtomPairs(
        Display *display )
#endif /* _NO_PROTO */
{
    BYTE			stackData[MAXSTACK], *fill;
    size_t				dataSize;
    struct _propertyRec {
	xmMotifAtomPairPropertyRec	info;
	xmMotifAtomPairRec		pair[1];
    } *propertyRecPtr;
    Atom                	atomPairsAtom;
    int				i;
    Cardinal			num_atoms = XtNumber (atomNames);
    Window			motifWindow;

    /* Calculate the total length of the property record. */
    dataSize = 
      sizeof(xmMotifAtomPairPropertyRec) + 
	num_atoms * sizeof(xmMotifAtomPairRec);
    for (i = 0; i < num_atoms; i++) {
	dataSize += strlen(atomNames[i]) + 1;
    }

    /* If the pre-allocated buffer isn't big enough, get a bigger one. */

    if ( dataSize > MAXSTACK ){
	propertyRecPtr = (struct _propertyRec *)XtMalloc( dataSize );
    } else {
	propertyRecPtr = (struct _propertyRec *)stackData;
    }

    propertyRecPtr->info.byte_order = (BYTE) _XmByteOrderChar;
    propertyRecPtr->info.protocol_version = (BYTE) _MOTIF_DRAG_PROTOCOL_VERSION;
    propertyRecPtr->info.num_atom_pairs = num_atoms;
    propertyRecPtr->info.heap_offset = 
      sizeof(xmMotifAtomPairPropertyRec) + 
	num_atoms * sizeof(xmMotifAtomPairRec);	/* Offset to string data */

    /*
     *  Copy each xmMotifAtomPairRec to the property buffer.
     */

    for (i = 0; i < num_atoms; i++) {
        propertyRecPtr->pair[i].atom = 
	  XmInternAtom (display, atomNames[i], False);
        propertyRecPtr->pair[i].name_length = strlen(atomNames[i]) + 1;
    }

    fill = (BYTE *)propertyRecPtr + sizeof(xmMotifAtomPairPropertyRec) +
      num_atoms * sizeof(xmMotifAtomPairRec);
    /* Now copy the string data to the property buffer. */
    for (i = 0; i < num_atoms; i++) {
	memcpy( fill, atomNames[i], propertyRecPtr->pair[i].name_length);
	fill += propertyRecPtr->pair[i].name_length;
    }

    /*
     *  Write the buffer to the property within a protected section.
     */

    atomPairsAtom = XmInternAtom (display, _XA_MOTIF_ATOM_PAIRS, False);
    motifWindow = GetMotifWindow (display);
    StartProtectedSection (display, motifWindow);
    XChangeProperty (display,
		     motifWindow,
		     atomPairsAtom,
		     atomPairsAtom,
		     8,
		     PropModeReplace, 
		     (unsigned char *)propertyRecPtr,
		     dataSize );

    /* If the buffer was malloc'ed, free it. */

    if ( propertyRecPtr != (struct _propertyRec *)stackData ) {
        XtFree( (char *)propertyRecPtr );
    }

    EndProtectedSection (display);
    if (bad_window) {
	_XmWarning ((Widget) XmGetXmDisplay (display), MESSAGE1);
    }
}

/*****************************************************************************
 *
 *  ReadAtomPairs ()
 *
 ***************************************************************************/

static Boolean 
#ifdef _NO_PROTO
ReadAtomPairs( display )
        Display *display ;
#else
ReadAtomPairs(
        Display *display )
#endif /* _NO_PROTO */
{
    struct _propertyRec {
	xmMotifAtomPairPropertyRec	info;
	xmMotifAtomPairRec		pair[1];
    } *propertyRecPtr;

    Atom                        atomPairsAtom;
    int				format;
    unsigned long 		bytesafter, lengthRtn; 
    Atom			type;
    Cardinal			i;
    Boolean			ret;
    Window			motifWindow;

    atomPairsAtom = XmInternAtom (display, _XA_MOTIF_ATOM_PAIRS, False);
    motifWindow = GetMotifWindow (display);
    StartProtectedSection (display, motifWindow);
    ret = ((XGetWindowProperty (display, 
    				motifWindow,
			        atomPairsAtom,
			        0L, MAXPROPLEN,
			        False,
			        atomPairsAtom,
			        &type,
			        &format,
			        &lengthRtn,
			        &bytesafter,
			        (unsigned char **) &propertyRecPtr) == Success) &&
           (lengthRtn >= sizeof(xmMotifAtomPairPropertyRec)));
    EndProtectedSection (display);
    if (bad_window) {
	_XmWarning ((Widget) XmGetXmDisplay (display), MESSAGE1);
	ret = False;
    }

    if (ret) {
	if (propertyRecPtr->info.protocol_version !=
	    _MOTIF_DRAG_PROTOCOL_VERSION) {
	    _XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE2);
	}

	if (propertyRecPtr->info.byte_order != _XmByteOrderChar) {
	    swap2bytes(propertyRecPtr->info.num_atom_pairs);
	    swap4bytes(propertyRecPtr->info.heap_offset);
	}

	if (propertyRecPtr->info.num_atom_pairs) {
	    char	*buf;

	    buf = (char *)propertyRecPtr + propertyRecPtr->info.heap_offset;
	    
	    for (i = 0; i < propertyRecPtr->info.num_atom_pairs; i++) {
	        if (propertyRecPtr->info.byte_order != _XmByteOrderChar) {
		    swap2bytes(propertyRecPtr->pair[i].name_length);
		    swap4bytes(propertyRecPtr->pair[i].atom);
	        }

                _XmInternAtomAndName (display, 
				      (Atom)propertyRecPtr->pair[i].atom, 
				      buf);
		buf += propertyRecPtr->pair[i].name_length;
	    }
	}
    }      

    /*
     *  Free any memory that Xlib passed us.
     */

    if (propertyRecPtr) {
        XFree((char *)propertyRecPtr);
    }
    return (ret);
}

/*****************************************************************************
 *
 *  WriteAtomsTable ()
 *
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
WriteAtomsTable( display, atomsTable )
        Display *display ;
        xmAtomsTable atomsTable ;
#else
WriteAtomsTable(
        Display *display,
        xmAtomsTable atomsTable )
#endif /* _NO_PROTO */
{
    BYTE			stackData[MAXSTACK];
    struct _propertyRec {
	xmMotifAtomsPropertyRec	info;
	xmMotifAtomsTableRec	entry[1];
    } *propertyRecPtr;

    Atom                	atomsTableAtom;
    int				i;
    Window			motifWindow;
    size_t			dataSize;

    if (!atomsTable) {
	_XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE4);
	return;
    }

    /* If the data is bigger than the default stack allocation, then 
     * allocate heap storage, else use automatic storage.
     */
    dataSize = sizeof(xmMotifAtomsPropertyRec) + 
      atomsTable->numEntries * sizeof(xmMotifAtomsTableRec) ;

    if ( dataSize > MAXSTACK ) {
	propertyRecPtr = (struct _propertyRec *)XtMalloc( dataSize );
    } else {
	propertyRecPtr = (struct _propertyRec *)stackData;
    }

    propertyRecPtr->info.byte_order = (BYTE) _XmByteOrderChar;
    propertyRecPtr->info.protocol_version = (BYTE) _MOTIF_DRAG_PROTOCOL_VERSION;
    propertyRecPtr->info.num_atoms = atomsTable->numEntries;
    propertyRecPtr->info.heap_offset = dataSize;

    /* write each entry's atom and time */

    for (i = 0; i < atomsTable->numEntries; i++) {
        propertyRecPtr->entry[i].atom = atomsTable->entries[i].atom;
        propertyRecPtr->entry[i].time = atomsTable->entries[i].time;
    }

    /*
     *  Write the buffer to the property within a protected section.
     */

    atomsTableAtom = XmInternAtom (display, _XA_MOTIF_ATOMS, False);
    motifWindow = GetMotifWindow (display);
    StartProtectedSection (display, motifWindow);
    XChangeProperty (display, 
                     motifWindow,
		     atomsTableAtom,
		     atomsTableAtom,
		     8,
		     PropModeReplace, 
		     (unsigned char *)propertyRecPtr,
		     dataSize );

    /* If we had to use a heap buffer, free it. */
    if (propertyRecPtr != (struct _propertyRec *)stackData) {
        XtFree((char *)propertyRecPtr);
    }
    EndProtectedSection (display);
    if (bad_window) {
	_XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE1);
    }
}

/*****************************************************************************
 *
 *  ReadAtomsTable ()
 *
 ***************************************************************************/

static Boolean 
#ifdef _NO_PROTO
ReadAtomsTable( display, atomsTable )
        Display *display ;
        xmAtomsTable atomsTable ;
#else
ReadAtomsTable(
        Display *display,
        xmAtomsTable atomsTable )
#endif /* _NO_PROTO */
{
    struct { 
	xmMotifAtomsPropertyRec info;
	xmMotifAtomsTableRec	entry[1];
    } *propertyRecPtr = NULL;
    Atom                        atomsTableAtom;
    int				format;
    unsigned long 		bytesafter, lengthRtn; 
    Atom			type;
    int				i;
    Boolean			ret;
    Window			motifWindow;

    atomsTableAtom = XmInternAtom (display, _XA_MOTIF_ATOMS, False);
    motifWindow = GetMotifWindow (display);
    StartProtectedSection (display, motifWindow);
    ret = ((XGetWindowProperty (display, 	/* display* */
    				motifWindow,	/* window */
			        atomsTableAtom,	/* property atom */
			        0L, MAXPROPLEN,	/* long_offset, long_length */
			        False,		/* delete flag */
			        atomsTableAtom,	/* property type */
			        &type,		/* returned actual type */
			        &format,	/* returned actual format */
			        &lengthRtn,	/* returned item count */
			        &bytesafter,	/* returned bytes remaining */
			        (unsigned char **) &propertyRecPtr)
						/* returned data */
	    == Success) &&
           (lengthRtn >= sizeof(xmMotifAtomsPropertyRec)));
    EndProtectedSection (display);
    if (bad_window) {
	_XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE1);
	ret = False;
    }

    if (ret) {
	if (propertyRecPtr->info.protocol_version != 
	    _MOTIF_DRAG_PROTOCOL_VERSION) {
	    _XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE2);
	}

	if (propertyRecPtr->info.byte_order != _XmByteOrderChar) {
	    swap2bytes(propertyRecPtr->info.num_atoms);
	    swap4bytes(propertyRecPtr->info.heap_offset);
	}

        if (atomsTable == NULL)
        {
            atomsTable = (xmAtomsTable) XtMalloc(sizeof(xmAtomsTableRec));
            atomsTable->numEntries = 0;
            atomsTable->entries = NULL;

            SetAtomsTable (display, atomsTable);
        }

	if (propertyRecPtr->info.num_atoms > atomsTable->numEntries) {

            /*
	     *  expand the atoms table
	     */

            atomsTable->entries = (xmAtomsTableEntry) XtRealloc(
	        (char *)atomsTable->entries,	/* NULL ok */
		sizeof(xmAtomsTableEntryRec) * propertyRecPtr->info.num_atoms);
	}

	/*
	 *  Read the atom table entries.
	 */

	for (i = 0; i < propertyRecPtr->info.num_atoms; i++) {
	    if (propertyRecPtr->info.byte_order != _XmByteOrderChar) {
		swap4bytes(propertyRecPtr->entry[i].atom);
		swap4bytes(propertyRecPtr->entry[i].time);
	    }

            atomsTable->entries[i].atom = (Atom) propertyRecPtr->entry[i].atom;
            atomsTable->entries[i].time = (Time) propertyRecPtr->entry[i].time;
	}
        atomsTable->numEntries = propertyRecPtr->info.num_atoms;
    }      

    /*
     *  Free any memory that Xlib passed us.
     */

    if (propertyRecPtr) {
        XFree((char *)propertyRecPtr);
    }
    return (ret);
}

/*****************************************************************************
 *
 *  WriteTargetsTable ()
 *
 ***************************************************************************/

static void 
#ifdef _NO_PROTO
WriteTargetsTable( display, targetsTable )
        Display *display ;
        xmTargetsTable targetsTable ;
#else
WriteTargetsTable(
        Display *display,
        xmTargetsTable targetsTable )
#endif /* _NO_PROTO */
{
    BYTE		stackData[MAXSTACK], *fill;
    struct _propertyRec {
	xmMotifTargetsPropertyRec	info;
    } *propertyRecPtr;

    Atom                targetsTableAtom;
    int			i, j;
    Window		motifWindow;
    size_t		dataSize;
    CARD16Item		shortItem;
    CARD32Item		longItem;

    if (!targetsTable) {
	_XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE5);
	return;
    }

    /* rohit - ensuring consistency with 2.0 */
    /* dataSize = sizeof(xmMotifTargetsPropertyRec) + 
			targetsTable->numEntries * 2; */

    dataSize = sizeof(xmMotifTargetsPropertyRec);

    for (i = 0; i < targetsTable->numEntries; i++) {

	/* rohit - ensuring consistency with 2.0 */
	/* dataSize += targetsTable->entries[i].numTargets * 4 ; */

	dataSize += targetsTable->entries[i].numTargets * 4 + 2;
    }

    if ( dataSize > MAXSTACK ){
	propertyRecPtr = (struct _propertyRec *)XtMalloc( dataSize );
    } else {
	propertyRecPtr = (struct _propertyRec *)stackData ;
    }

    propertyRecPtr->info.byte_order = (BYTE) _XmByteOrderChar;
    propertyRecPtr->info.protocol_version = (BYTE) _MOTIF_DRAG_PROTOCOL_VERSION;
    propertyRecPtr->info.num_target_lists = targetsTable->numEntries;
    propertyRecPtr->info.heap_offset = dataSize;

    /* write each target list's count and atoms */

    fill = (BYTE *)propertyRecPtr + sizeof(xmMotifTargetsPropertyRec);

    for (i = 0; i < targetsTable->numEntries; i++) {
	shortItem.value = targetsTable->entries[i].numTargets;

	/* rohit - fix for CR 7839 */
	/* memcpy( fill, &shortItem.value, 2 ); */

	memcpy( fill, &shortItem, 2 );
	fill += 2;

	/*
	 *  Write each Atom out one at a time as a CARD32.
	 */
	for (j = 0; j < targetsTable->entries[i].numTargets; j++) {
	    longItem.value = targetsTable->entries[i].targets[j];
	    memcpy( fill, &longItem, 4 );
	    fill += 4;
	}
    }
    /*
     *  Write the buffer to the property within a protected section.
     */

    targetsTableAtom = XmInternAtom (display, _XA_MOTIF_TARGETS, False);
    motifWindow = GetMotifWindow (display);
    StartProtectedSection (display, motifWindow);
    XChangeProperty (display, 
                     motifWindow,
		     targetsTableAtom,
		     targetsTableAtom,
		     8,
		     PropModeReplace, 
		     (unsigned char *)propertyRecPtr,
		     dataSize);

    /* If a buffer was allocated, free it. */
    if (propertyRecPtr != (struct _propertyRec *)stackData) {
        XtFree((char *)propertyRecPtr);
    }
    EndProtectedSection (display);
    if (bad_window) {
	_XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE1);
    }
}

/*****************************************************************************
 *
 *  ReadTargetsTable ()
 *
 ***************************************************************************/

static Boolean 
#ifdef _NO_PROTO
ReadTargetsTable( display, targetsTable )
        Display *display ;
        xmTargetsTable targetsTable ;
#else
ReadTargetsTable(
        Display *display,
        xmTargetsTable targetsTable )
#endif /* _NO_PROTO */
{
    struct _propertyRec {
	xmMotifTargetsPropertyRec	info;
    } *propertyRecPtr;

    char			*bufptr;
    short			num_targets;
    Atom                        targetsTableAtom;
    int				format;
    unsigned long 		bytesafter, lengthRtn; 
    Atom			type;
    int				i, j;
    Atom		        *targets;
    Boolean			ret;
    Window			motifWindow;
    CARD16Item			shortItem;
    CARD32Item			longItem;

    targetsTableAtom = XmInternAtom (display, _XA_MOTIF_TARGETS, False);
    motifWindow = GetMotifWindow (display);
    StartProtectedSection (display, motifWindow);
    ret = ((XGetWindowProperty (display, 
    				motifWindow,
			        targetsTableAtom,
			        0L, MAXPROPLEN,
			        False,
			        targetsTableAtom,
			        &type,
			        &format,
			        &lengthRtn,
			        &bytesafter,
			        (unsigned char **) &propertyRecPtr) == Success) &&
           (lengthRtn >= sizeof(xmMotifTargetsPropertyRec)));
    EndProtectedSection (display);
    if (bad_window) {
	_XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE1);
	ret = False;
    }

    if (ret) {
	if (propertyRecPtr->info.protocol_version != 
	    _MOTIF_DRAG_PROTOCOL_VERSION) {
	    _XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE2);
	}

	if (propertyRecPtr->info.byte_order != _XmByteOrderChar) {
	    swap2bytes(propertyRecPtr->info.num_target_lists);
	    swap4bytes(propertyRecPtr->info.heap_offset);
	}

        if (targetsTable == NULL)
        {
            targetsTable = (xmTargetsTable)XtMalloc(sizeof(xmTargetsTableRec));
            targetsTable->numEntries = 0;
            targetsTable->entries = NULL;

            SetTargetsTable (display, targetsTable);
        }

	if (propertyRecPtr->info.num_target_lists > targetsTable->numEntries) {

	    /*
	     *  expand the target table
	     */

            targetsTable->entries = (xmTargetsTableEntry) 
	      XtRealloc(
			(char *)targetsTable->entries,	/* NULL ok */
			sizeof(xmTargetsTableEntryRec) * 
			propertyRecPtr->info.num_target_lists);

	    /*
	     *  read the new entries
	     */

	    bufptr = (char *)propertyRecPtr + sizeof(xmMotifTargetsPropertyRec);
	    for (i = 0; i < targetsTable->numEntries; i++) {

		/* rohit - fix for CR 7839 */
		/* memcpy( &shortItem.value, bufptr, 2 ); */

		memcpy( &shortItem, bufptr, 2 );
	        if (propertyRecPtr->info.byte_order != _XmByteOrderChar) {
		    swap2bytes(shortItem.value);
		}
		num_targets = shortItem.value;

		bufptr += 2 + 4 * num_targets;

		if (num_targets != targetsTable->entries[i].numTargets) {
		    _XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE6);
		}

	    }
	    for (; i < propertyRecPtr->info.num_target_lists; i++) {

		/* rohit - fix for CR 7839 */
		/* memcpy( &shortItem.value, bufptr, 2 ); */

		memcpy( &shortItem, bufptr, 2 );
		bufptr += 2;
	        if (propertyRecPtr->info.byte_order != _XmByteOrderChar) {
	            swap2bytes(shortItem.value);
		}
		num_targets = shortItem.value;

	        targets = (Atom *) XtMalloc(sizeof(Atom) * num_targets);
		/*
	 	 *  Read each Atom in one at a time.
	 	 */
		for (j = 0; j < num_targets; j++) {

		    /* rohit - fix for CR 7839 */
		    /* memcpy( &longItem.value, bufptr, 4 ); */

		    memcpy( &longItem, bufptr, 4 );
		    bufptr += 4;
	            if (propertyRecPtr->info.byte_order != _XmByteOrderChar) {
			swap4bytes(longItem.value);
		    }
		    targets[j] = (Atom) longItem.value ;
		}

                targetsTable->numEntries++;
                targetsTable->entries[i].numTargets = num_targets;
                targetsTable->entries[i].targets = targets;
	    }
	}
    }      

    /*
     *  Free any memory that Xlib passed us.
     */

    if (propertyRecPtr) {
        XFree((char *)propertyRecPtr);
    }
    return (ret);
}

/*****************************************************************************
 *
 *  CreateDefaultTargetsTable ()
 *
 *  Create the default targets table.
 ***************************************************************************/

static Atom nullTargets[] = 	{ 0,		};
static Atom stringTargets[] = 	{ XA_STRING,	};

static xmTargetsTable 
#ifdef _NO_PROTO
CreateDefaultTargetsTable( display )
        Display *display ;
#else
CreateDefaultTargetsTable(
        Display *display )
#endif /* _NO_PROTO */
{
    xmTargetsTable	targetsTable;

    targetsTable = (xmTargetsTable) XtMalloc(sizeof(xmTargetsTableRec));

    targetsTable->numEntries = 2;
    targetsTable->entries =
	(xmTargetsTableEntry) XtMalloc(sizeof(xmTargetsTableEntryRec) * 2);

    targetsTable->entries[0].numTargets = XtNumber(nullTargets);
    targetsTable->entries[0].targets = nullTargets;
    targetsTable->entries[1].numTargets = XtNumber(stringTargets);
    targetsTable->entries[1].targets = stringTargets;

    SetTargetsTable (display, targetsTable);
    return (targetsTable);
}

/*****************************************************************************
 *
 *  CreateDefaultAtomsTable ()
 *
 *  Create the default atoms table.
 ***************************************************************************/

static xmAtomsTable 
#ifdef _NO_PROTO
CreateDefaultAtomsTable( display )
        Display *display ;
#else
CreateDefaultAtomsTable(
        Display *display )
#endif /* _NO_PROTO */
{
    xmAtomsTable	atomsTable;

    atomsTable = (xmAtomsTable) XtMalloc(sizeof(xmAtomsTableRec));

    atomsTable->numEntries = 1;
    atomsTable->entries =
	(xmAtomsTableEntry) XtMalloc(sizeof(xmAtomsTableEntryRec));

    atomsTable->entries[0].atom =
	XmInternAtom (display, _XA_MOTIF_ATOM_0, False);
    atomsTable->entries[0].time = 0;

    SetAtomsTable (display, atomsTable);
    return (atomsTable);
}

/*****************************************************************************
 *
 *  _XmInitAtomPairs ()
 *
 ***************************************************************************/

void 
#ifdef _NO_PROTO
_XmInitAtomPairs( display )
	Display	*display ;
#else
_XmInitAtomPairs(
	Display	*display )
#endif /* _NO_PROTO */
{
    Display	*ndisplay;
    Window	motifWindow;

    _XmInitByteOrderChar();

    /*
     *  Read the motifWindow property on the root.  If the property is not
     *    there, create a persistant motifWindow and put it on the property.
     *  Reading the atom pair property on motifWindow is delayed so it can
     *    be saved in a context indexed by the original display.
     */

    if ((motifWindow = ReadMotifWindow (display)) == None) {

	if ((ndisplay = XOpenDisplay(XDisplayString(display))) == NULL) {
	    _XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE3);
	    return;
	}

	XGrabServer(ndisplay);
        if ((motifWindow = ReadMotifWindow (ndisplay)) == None) {
	    XSetCloseDownMode (ndisplay, RetainPermanent);
	    motifWindow = CreateMotifWindow (ndisplay);
            WriteMotifWindow (ndisplay, &motifWindow);
	}
	XCloseDisplay(ndisplay);
    }
    SetMotifWindow (display, motifWindow);

    /*
     *  Read the atom pair property on motifWindow.
     *  If it is not there, create it and put it there.
     */

    if (!ReadAtomPairs (display)) {
	XGrabServer(display);
        if (!ReadAtomPairs (display)) {
            WriteAtomPairs (display);
	}
	XUngrabServer (display);
        XFlush (display);
    }
}

/*****************************************************************************
 *
 *  _XmInitTargetsTable ()
 *
 ***************************************************************************/

void 
#ifdef _NO_PROTO
_XmInitTargetsTable( display )
        Display *display ;
#else
_XmInitTargetsTable(
        Display *display )
#endif /* _NO_PROTO */
{
    Display	*ndisplay;
    Window	motifWindow;
    Boolean	grabbed = False;

    /*
     *  Read the motifWindow property on the root.  If the property is not
     *    there, create a persistant motifWindow and put it on the property.
     *  Reading the atom pair, atoms table, and targets table properties
     *    on motifWindow is delayed so they can be saved in contexts indexed
     *    by the original display.
     */

    if ((motifWindow = ReadMotifWindow (display)) == None) {

	if ((ndisplay = XOpenDisplay(XDisplayString(display))) == NULL) {
	    _XmWarning( (Widget) XmGetXmDisplay (display), MESSAGE3);
	    return;
	}

	XGrabServer(ndisplay);
	if ((motifWindow = ReadMotifWindow (display)) == None) {
	    XSetCloseDownMode (ndisplay, RetainPermanent);
	    motifWindow = CreateMotifWindow (ndisplay);
            WriteMotifWindow (ndisplay, &motifWindow);
	}
	XCloseDisplay(ndisplay);
    }
    SetMotifWindow (display, motifWindow);

    /*
     *  Read the atom pair, atoms table, and target table properties on 
     *  motifWindow.  If any of these properties are not there, create them
     *  and put them on motifWindow.  Grab the server here at most once.
     */

    if (!ReadAtomPairs (display)) {
	XGrabServer(display);
        grabbed = True;
        if (!ReadAtomPairs (display)) {
            WriteAtomPairs (display);
	}
    }

    if (!ReadAtomsTable (display, GetAtomsTable (display))) {
        if (!grabbed) {
	    XGrabServer(display);
            grabbed = True;
            if (!ReadAtomsTable (display, GetAtomsTable (display))) {
                WriteAtomsTable (display, CreateDefaultAtomsTable (display));
	    }
	}
	else {
            WriteAtomsTable (display, CreateDefaultAtomsTable (display));
	}
    }

    if (!ReadTargetsTable (display, GetTargetsTable (display))) {
        if (!grabbed) {
	    XGrabServer(display);
            grabbed = True;
	    if (!ReadTargetsTable (display, GetTargetsTable (display))) {
                WriteTargetsTable (display,
				   CreateDefaultTargetsTable (display));
	    }
	}
	else {
            WriteTargetsTable (display, CreateDefaultTargetsTable (display));
	}
    }

    if (grabbed) {
	XUngrabServer (display);
        XFlush (display);
    }
}

/*****************************************************************************
 *
 *  _XmIndexToTargets ()
 *
 ***************************************************************************/

Cardinal 
#ifdef _NO_PROTO
_XmIndexToTargets( shell, t_index, targetsRtn )
        Widget shell ;
        Cardinal t_index ;
        Atom **targetsRtn ;
#else
_XmIndexToTargets(
        Widget shell,
        Cardinal t_index,
        Atom **targetsRtn )
#endif /* _NO_PROTO */
{
    Display		*display = XtDisplay (shell);
    xmTargetsTable	targetsTable;

    if (!(targetsTable = GetTargetsTable (display))) {
        _XmInitTargetsTable (display);
        targetsTable = GetTargetsTable (display);
    }

    if (t_index >= targetsTable->numEntries) {
        /*
	 *  Retrieve the targets table from motifWindow.
	 *  If this fails, then either the motifWindow or the targets table
	 *  property on motifWindow has been destroyed, so reinitialize.
	 */
        if (!ReadTargetsTable (display, targetsTable)) {
            _XmInitTargetsTable (display);
            targetsTable = GetTargetsTable (display);
	}
    }

    if (t_index >= targetsTable->numEntries) {
	_XmWarning ((Widget) XmGetXmDisplay (display), MESSAGE7);
        *targetsRtn = NULL;
        return 0;
    }

    *targetsRtn = targetsTable->entries[t_index].targets;
    return targetsTable->entries[t_index].numTargets;
}

/*****************************************************************************
 *
 *  _XmAtomCompare ()
 *
 *  The routine must return an integer less than, equal to, or greater than
 *  0 according as the first argument is to be considered less
 *  than, equal to, or greater than the second.
 ***************************************************************************/

static int 
#ifdef _NO_PROTO
AtomCompare( atom1, atom2 )
        XmConst void *atom1 ;
        XmConst void *atom2 ;
#else
AtomCompare(
        XmConst void *atom1,
        XmConst void *atom2 )
#endif /* _NO_PROTO */
{
    return (*((Atom *) atom1) - *((Atom *) atom2));
}

/*****************************************************************************
 *
 *  _XmTargetsToIndex ()
 *
 ***************************************************************************/

Cardinal 
#ifdef _NO_PROTO
_XmTargetsToIndex( shell, targets, numTargets )
        Widget shell ;
        Atom *targets ;
        Cardinal numTargets ;
#else
_XmTargetsToIndex(
        Widget shell,
        Atom *targets,
        Cardinal numTargets )
#endif /* _NO_PROTO */
{
    Display		*display = XtDisplay (shell);
    Cardinal		i, j;
    Cardinal		size;
    Cardinal		oldNumEntries;
    Atom		*newTargets;
    xmTargetsTable	targetsTable;

    if (!(targetsTable = GetTargetsTable (display))) {
        _XmInitTargetsTable (display);
        targetsTable = GetTargetsTable (display);
    }

    /*
     *  Create a new targets list, sorted in ascending order.
     */

    size =  sizeof(Atom) * numTargets;
    newTargets = (Atom *) XtMalloc(size);
    memcpy (newTargets, targets, size);
    qsort ((void *)newTargets, (size_t)numTargets, (size_t)sizeof(Atom),
           AtomCompare);
    /*
     *  Try to find the targets list in the targets table.
     */

    for (i = 0; i < targetsTable->numEntries; i++) {
	if (numTargets == targetsTable->entries[i].numTargets) {
            for (j = 0; j < numTargets; j++) {
	        if (newTargets[j] != targetsTable->entries[i].targets[j]) {
	            break;
		}
	    }
	    if (j == numTargets) {
	        XtFree ((char *)newTargets);
	        return i;
	    }
	}
    }
    oldNumEntries = targetsTable->numEntries;

    /*
     *  Lock and retrieve the target table from motifWindow.
     *  If this fails, then either the motifWindow or the targets table
     *  property on motifWindow has been destroyed, so reinitialize.
     *  If the target list is still not in the table, add the target list
     *  to the table and write the table out to its property.
     */

    XGrabServer (display);
    if (!ReadTargetsTable (display, targetsTable)) {
	XUngrabServer (display);
        _XmInitTargetsTable (display);
	XGrabServer (display);
        targetsTable = GetTargetsTable (display);
    }

    for (i = oldNumEntries; i < targetsTable->numEntries; i++) {
	if (numTargets == targetsTable->entries[i].numTargets) {
	    for (j = 0; j < numTargets; j++) {
		if (newTargets[j] != targetsTable->entries[i].targets[j]) {
	            break;
		}
	    }
	    if (j == numTargets) {
	        XtFree ((char *)newTargets);
		break;
            }
	}
    }
    if (i == targetsTable->numEntries) {
        targetsTable->numEntries++;

        targetsTable->entries = (xmTargetsTableEntry) XtRealloc(
	    (char *)targetsTable->entries,	/* NULL ok */
	    sizeof(xmTargetsTableEntryRec) * (targetsTable->numEntries));

        targetsTable->entries[i].numTargets = numTargets;
        targetsTable->entries[i].targets = newTargets;
        WriteTargetsTable (display, targetsTable);
    }

    XUngrabServer (display);
    XFlush (display);
    return i;
}

/*****************************************************************************
 *
 *  _XmAllocMotifAtom ()
 *
 *  Allocate an atom in the atoms table with the specified time stamp.
 ***************************************************************************/

Atom 
#ifdef _NO_PROTO
_XmAllocMotifAtom( shell, time )
        Widget shell ;
        Time time ;
#else
_XmAllocMotifAtom(
        Widget shell,
        Time time )
#endif /* _NO_PROTO */
{
    Display		*display = XtDisplay (shell);
    xmAtomsTable	atomsTable;
    xmAtomsTableEntry	p;
    Cardinal		i;
    char		atomname[80];
    Atom		atomReturn = None;

    if (!(atomsTable = GetAtomsTable (display))) {
        _XmInitTargetsTable (display);
        atomsTable = GetAtomsTable (display);
    }

    /*
     *  Lock and retrieve the atoms table from motifWindow.
     *  If this fails, then either the motifWindow or the atoms table
     *  property on motifWindow has been destroyed, so reinitialize.
     *  Try to find an available atom in the table (time == 0).
     *  If no atom is available, add an atom to the table.
     *  Write the atoms table out to its property.
     */

    XGrabServer (display);
    if (!ReadAtomsTable (display, atomsTable)) {
	XUngrabServer (display);
        _XmInitTargetsTable (display);
	XGrabServer (display);
        atomsTable = GetAtomsTable (display);
    }

    for (p = atomsTable->entries, i = atomsTable->numEntries; i; p++, i--) {
        if ((p->time) == 0) {
            p->time = time;
            atomReturn = p->atom;
	    break;
        }
    }

    if (atomReturn == None) {
	i = atomsTable->numEntries++;

        atomsTable->entries = (xmAtomsTableEntry) XtRealloc(
	    (char *)atomsTable->entries,	/* NULL ok */
  	    (atomsTable->numEntries * sizeof(xmAtomsTableEntryRec)));

        sprintf(atomname, "%s%d", "_MOTIF_ATOM_", i);
        atomsTable->entries[i].atom = XmInternAtom (display, atomname, False);
        atomsTable->entries[i].time = time;
        atomReturn = atomsTable->entries[i].atom;
    }

    WriteAtomsTable (display, atomsTable);
    XUngrabServer (display);
    XFlush (display);
    return (atomReturn);
}

/*****************************************************************************
 *
 *  _XmGetMotifAtom ()
 *
 *  Get the atom from the atoms table with nonzero timestamp less than but
 *  closest to the specified value.
 ***************************************************************************/

Atom 
#ifdef _NO_PROTO
_XmGetMotifAtom( shell, time )
        Widget shell ;
        Time time ;
#else
_XmGetMotifAtom(
        Widget shell,
        Time time )
#endif /* _NO_PROTO */
{
    Display		*display = XtDisplay (shell);
    xmAtomsTable	atomsTable;
    Cardinal		i;
    Atom		atomReturn = None;
    Time		c_time;

    /*
     *  Get the atoms table saved in the display's context.
     *  This table will be updated from the motifWindow property.
     */

    if (!(atomsTable = GetAtomsTable (display))) {
        _XmInitTargetsTable (display);
        atomsTable = GetAtomsTable (display);
    }

    /*
     *  Lock and retrieve the atoms table from motifWindow.
     *  If this fails, then either the motifWindow or the atoms table
     *  property on motifWindow has been destroyed, so reinitialize.
     *  Try to find the atom with nonzero timestamp less than but closest
     *  to the specified value.
     */

    XGrabServer (display);
    if (!ReadAtomsTable (display, atomsTable)) {
	XUngrabServer (display);
        _XmInitTargetsTable (display);
	XGrabServer (display);
        atomsTable = GetAtomsTable (display);
    }

    for (i = 0; i < atomsTable->numEntries; i++) {
        if ((atomsTable->entries[i].time) &&
            (atomsTable->entries[i].time <= time)) {
	    break;
	}
    }

    if (i < atomsTable->numEntries) {
        atomReturn = atomsTable->entries[i].atom;
        c_time = atomsTable->entries[i++].time;
        for (; i < atomsTable->numEntries; i++) {
            if ((atomsTable->entries[i].time > c_time) &&
                (atomsTable->entries[i].time < time)) {
                atomReturn = atomsTable->entries[i].atom;
                c_time = atomsTable->entries[i].time;
	    }
	}
    }

    XUngrabServer (display);
    XFlush (display);
    return (atomReturn);
}

/*****************************************************************************
 *
 *  _XmFreeMotifAtom ()
 *
 *  Free an atom in the atoms table by giving it a zero timestamp.
 ***************************************************************************/

void 
#ifdef _NO_PROTO
_XmFreeMotifAtom( shell, atom )
        Widget shell ;
        Atom atom ;
#else
_XmFreeMotifAtom(
        Widget shell,
        Atom atom )
#endif /* _NO_PROTO */
{
    Display		*display = XtDisplay (shell);
    xmAtomsTable	atomsTable;
    xmAtomsTableEntry	p;
    Cardinal		i;

    if (atom == None) {
	return;
    }

    /*
     *  Get the atoms table saved in the display's context.
     *  This table will be updated from the motifWindow property.
     */

    if (!(atomsTable = GetAtomsTable (display))) {
        _XmInitTargetsTable (display);
        atomsTable = GetAtomsTable (display);
    }

    /*
     *  Lock and retrieve the atoms table from its property.
     *  If this fails, then either the motifWindow or the atoms table
     *  property on motifWindow has been destroyed, so reinitialize.
     *  Free the matched atom, if present, and write the atoms table out
     *  to its property.
     */

    XGrabServer (display);
    if (!ReadAtomsTable (display, atomsTable)) {
	XUngrabServer (display);
        _XmInitTargetsTable (display);
	XGrabServer (display);
        atomsTable = GetAtomsTable (display);
    }

    for (p = atomsTable->entries, i = atomsTable->numEntries; i; p++, i--) {
        if (p->atom == atom) {
            p->time = (Time) 0;
            WriteAtomsTable (display, atomsTable);
	    break;
        }
    }

    XUngrabServer (display);
    XFlush (display);
}

/*****************************************************************************
 *
 *  _XmDestroyMotifWindow ()
 *
 ***************************************************************************/

void 
#ifdef _NO_PROTO
_XmDestroyMotifWindow( display )
        Display *display ;
#else
_XmDestroyMotifWindow(
        Display  *display )
#endif /* _NO_PROTO */
{
    Window	motifWindow;
    Atom	motifWindowAtom;

    if ((motifWindow = ReadMotifWindow (display)) != None) {
        motifWindowAtom = XmInternAtom (display, _XA_MOTIF_WINDOW, False);
        XDeleteProperty (display,
                         DefaultRootWindow (display),
                         motifWindowAtom);
	XDestroyWindow (display, motifWindow);
    }
}

/*****************************************************************************
 *
 *  _XmGetDragProxyWindow ()
 *
 ***************************************************************************/

Window
#ifdef _NO_PROTO
_XmGetDragProxyWindow( display )
        Display *display ;
#else
_XmGetDragProxyWindow(
        Display  *display )
#endif /* _NO_PROTO */
{
    Atom		motifProxyWindowAtom;
    Atom		type;
    int			format;
    unsigned long	lengthRtn;
    unsigned long	bytesafter;
    Window		*property = NULL;
    Window		motifWindow;
    Window		motifProxyWindow = None;

    if ((motifWindow = ReadMotifWindow (display)) != None) {

	motifProxyWindowAtom =
	    XmInternAtom (display, _XA_MOTIF_PROXY_WINDOW, False);

	if ((XGetWindowProperty (display,
                                 motifWindow,
                                 motifProxyWindowAtom,
                                 0L, MAXPROPLEN,
			         False,
                                 AnyPropertyType,
                                 &type,
			         &format,
			         &lengthRtn,
			         &bytesafter, 
			         (unsigned char **) &property) == Success) &&
             (type == XA_WINDOW) &&
	     (format == 32) &&
	     (lengthRtn == 1)) {
	    motifProxyWindow = *property;
	}
	if (property) {
	    XFree ((char *)property);
	}
    }
    return (motifProxyWindow);
}
