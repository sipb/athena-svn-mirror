#include <time.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>

#include "xklavier_private.h"

XklState *XklGetCurrentState(  )
{
  return &_xklCurState;
}

const char *XklGetLastError(  )
{
  return _xklLastErrorMsg;
}

char *XklGetWindowTitle( Window w )
{
  Atom type_ret;
  int format_ret;
  unsigned long nitems, rest;
  unsigned char *prop;

  if( Success == XGetWindowProperty( _xklDpy, w, _xklAtoms[WM_NAME], 0L,
                                     -1L, False, XA_STRING, &type_ret,
                                     &format_ret, &nitems, &rest, &prop ) )
    return prop;
  else
    return NULL;
}

Bool XklIsSameApp( Window win1, Window win2 )
{
  Window app1, app2;
  return _XklGetAppWindow( win1, &app1 ) &&
    _XklGetAppWindow( win2, &app2 ) && app1 == app2;
}

Bool XklGetState( Window win, XklState * state_return )
{
  Window appWin;

  if( !_XklGetAppWindow( win, &appWin ) )
  {
    if( state_return != NULL )
      state_return->group = -1;
    return False;
  }

  return _XklGetAppState( appWin, state_return );
}

void XklDelState( Window win )
{
  Window appWin;

  if( _XklGetAppWindow( win, &appWin ) )
    _XklDelAppState( appWin );
}

void XklSaveState( Window win, XklState * state )
{
  Window appWin;

  if( _XklGetAppWindow( win, &appWin ) )
    _XklSaveAppState( appWin, state );
}

/**
 *  Prepares the name of window suitable for debugging (32characters long).
 */
char *_XklGetDebugWindowTitle( Window win )
{
  static char sname[33];
  char *name;
  strcpy( sname, "NULL" );
  if( win != ( Window ) NULL )
  {
    name = XklGetWindowTitle( win );
    if( name != NULL )
    {
      snprintf( sname, sizeof( sname ), "%.32s", name );
      XFree( name );
    }
  }
  return sname;
}

Window XklGetCurrentWindow(  )
{
  return _xklCurClient;
}

/**
 * Loads subtree. 
 * All the windows with WM_STATE are added.
 * All the windows within level 0 are listened for focus and property
 */
Bool _XklLoadSubtree( Window window, int level, XklState * initState )
{
  Window rwin = ( Window ) NULL,
    parent = ( Window ) NULL, *children = NULL, *child;
  int num = 0;
  Bool retval = True;

  _xklLastErrorCode =
    _XklStatusQueryTree( _xklDpy, window, &rwin, &parent, &children, &num );

  if( _xklLastErrorCode != Success )
  {
    return False;
  }

  child = children;
  while( num )
  {
    XklDebug( 150, "Looking at child " WINID_FORMAT " '%s'\n", *child,
              _XklGetDebugWindowTitle( *child ) );
    if( _XklHasWmState( *child ) )
    {
      XklDebug( 150, "It has WM_STATE so we'll add it\n" );
      _XklAddAppWindow( *child, window, True, initState );
    } else
    {
      XklDebug( 150, "It does not have have WM_STATE so we'll not add it\n" );

      if( level == 0 )
      {
        XklDebug( 150, "But we are at level 0 so we'll spy on it\n" );
        _XklSelectInputMerging( *child,
                                FocusChangeMask | PropertyChangeMask );
      } else
        XklDebug( 150, "And we are at level %d so we'll not spy on it\n",
                  level );

      retval = _XklLoadSubtree( *child, level + 1, initState );
    }

    child++;
    num--;
  }

  if( children != NULL )
    XFree( children );

  return retval;
}

/**
 * Checks whether given window has WM_STATE property (i.e. "App window").
 */
Bool _XklHasWmState( Window win )
{                               /* ICCCM 4.1.3.1 */
  Atom type = None;
  int format;
  unsigned long nitems;
  unsigned long after;
  unsigned char *data = NULL;   /* Helps in the case of BadWindow error */

  XGetWindowProperty( _xklDpy, win, _xklAtoms[WM_STATE], 0, 0, False,
                      _xklAtoms[WM_STATE], &type, &format, &nitems, &after,
                      &data );
  if( data != NULL )
    XFree( data );              /* To avoid an one-byte memory leak because after successfull return
                                 * data array always contains at least one nul byte (NULL-equivalent) */
  return type != None;
}

/**
 * Finds out the official parent window (accortind to XQueryTree)
 */
Window _XklGetRegisteredParent( Window win )
{
  Window parent = ( Window ) NULL, rw = ( Window ) NULL, *children = NULL;
  unsigned nchildren = 0;

  _xklLastErrorCode =
    _XklStatusQueryTree( _xklDpy, win, &rw, &parent, &children, &nchildren );

  if( children != NULL )
    XFree( children );

  return _xklLastErrorCode == Success ? parent : ( Window ) NULL;
}

/**
 * Make sure about the result. Origial XQueryTree is pretty stupid beast:)
 */
Status _XklStatusQueryTree( Display * display,
                            Window w,
                            Window * root_return,
                            Window * parent_return,
                            Window ** children_return,
                            signed int *nchildren_return )
{
  Bool result;

  result = ( Bool ) XQueryTree( display,
                                w,
                                root_return,
                                parent_return,
                                children_return, nchildren_return );
  if( !result )
  {
    XklDebug( 160,
              "Could not get tree info for window " WINID_FORMAT ": %d\n", w,
              result );
    _xklLastErrorMsg = "Could not get the tree info";
  }

  return result ? Success : FirstExtensionError;
}

const char *_XklGetEventName( int type )
{
  // Not really good to use the fact of consecutivity
  // but X protocol is already standartized so...
  static const char *evtNames[] = {
    "KeyPress",
    "KeyRelease",
    "ButtonPress",
    "ButtonRelease",
    "MotionNotify",
    "EnterNotify",
    "LeaveNotify",
    "FocusIn",
    "FocusOut",
    "KeymapNotify",
    "Expose",
    "GraphicsExpose",
    "NoExpose",
    "VisibilityNotify",
    "CreateNotify",
    "DestroyNotify",
    "UnmapNotify",
    "MapNotify",
    "MapRequest",
    "ReparentNotify",
    "ConfigureNotify",
    "ConfigureRequest",
    "GravityNotify",
    "ResizeRequest",
    "CirculateNotify",
    "CirculateRequest",
    "PropertyNotify",
    "SelectionClear",
    "SelectionRequest",
    "SelectionNotify",
    "ColormapNotify", "ClientMessage", "MappingNotify", "LASTEvent"
  };
  type -= KeyPress;
  if( type < 0 || type > ( sizeof( evtNames ) / sizeof( evtNames[0] ) ) )
    return NULL;
  return evtNames[type];
}
