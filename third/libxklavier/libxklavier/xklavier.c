#include <time.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>

#include "xklavier_private.h"

Display *_xklDpy;

Bool _xklXkbExtPresent;

XklState _xklCurState;

Window _xklCurClient;

Status _xklLastErrorCode;

const char *_xklLastErrorMsg;

XErrorHandler _xklDefaultErrHandler;

Atom _xklAtoms[TOTAL_ATOMS];

Window _xklRootWindow;

Bool _xklAllowSecondaryGroupOnce;

int _xklDefaultGroup;

Bool _xklSkipOneRestore;

int _xklSecondaryGroupsMask;

int _xklDebugLevel = 0;

Window _xklPrevAppWindow;

XklConfigCallback _xklConfigCallback = NULL;
void *_xklConfigCallbackData;

static XklStateCallback stateCallback = NULL;
static void *stateCallbackData;

static XklWinCallback winCallback = NULL;
static void *winCallbackData;

static XklLogAppender logAppender = XklDefaultLogAppender;

static Bool groupPerApp = True;

static Bool handleIndicators = False;

void XklSetIndicatorsHandling( Bool whetherHandle )
{
  handleIndicators = whetherHandle;
}

Bool XklGetIndicatorsHandling( void )
{
  return handleIndicators;
}

void XklSetDebugLevel( int level )
{
  _xklDebugLevel = level;
}

void XklSetGroupPerApp( Bool isSet )
{
  groupPerApp = isSet;
}

Bool XklIsGroupPerApp( void )
{
  return groupPerApp;
}

void XklAllowOneSwitchToSecondaryGroup( void )
{
  XklDebug( 150, "Setting allowOneSwitchToSecondaryGroup flag\n" );
  _xklAllowSecondaryGroupOnce = True;
}

void XklSetDefaultGroup( int group )
{
  _xklDefaultGroup = group;
}

int XklGetDefaultGroup( void )
{
  return _xklDefaultGroup;
}

void XklSetSecondaryGroupsMask( int mask )
{
  _xklSecondaryGroupsMask = mask;
}

int XklGetSecondaryGroupsMask( void )
{
  return _xklSecondaryGroupsMask;
}

int XklRegisterConfigCallback( XklConfigCallback fun, void *data )
{
  _xklConfigCallback = fun;
  _xklConfigCallbackData = data;
  return 0;
}

int XklRegisterStateCallback( XklStateCallback fun, void *data )
{
  stateCallback = fun;
  stateCallbackData = data;
  return 0;
}

int XklRegisterWindowCallback( XklWinCallback fun, void *data )
{
  winCallback = fun;
  winCallbackData = data;
  return 0;
}

void XklSetLogAppender( XklLogAppender fun )
{
  logAppender = fun;
}

int XklStartListen(  )
{
  XklResumeListen(  );
  _XklLoadWindowTree(  );
  XFlush( _xklDpy );
  return 0;
}

int XklStopListen(  )
{
  XklPauseListen(  );
  return 0;
}

int XklTerm(  )
{
  XSetErrorHandler( ( XErrorHandler ) _xklDefaultErrHandler );
  _xklConfigCallback = NULL;
  stateCallback = NULL;
  winCallback = NULL;

  logAppender = XklDefaultLogAppender;
  _XklFreeAllInfo(  );

  return 0;
}

Bool XklGrabKey( int key, unsigned modifiers )
{
  int keyCode;
  Bool retCode;
  char *keyName;

  keyCode = XKeysymToKeycode( _xklDpy, key );
  keyName = XKeysymToString( key );

  XklDebug( 100, "listen to the key %X(%d/%s)/%d\n", key, keyCode, keyName,
            modifiers );

  if( ( KeyCode ) NULL == keyCode )
    return False;

  _xklLastErrorCode = Success;

  retCode = XGrabKey( _xklDpy, keyCode, modifiers, _xklRootWindow,
                      True, GrabModeAsync, GrabModeAsync );
  XSync( _xklDpy, False );

  XklDebug( 100, "trying to listen: %d/%d\n", retCode, _xklLastErrorCode );

  retCode = ( _xklLastErrorCode == Success );

  if( !retCode )
    _xklLastErrorMsg = "Could not grab the key";

  return retCode;
}

Bool XklUngrabKey( int key, unsigned modifiers )
{
  int keyCode;

  keyCode = XKeysymToKeycode( _xklDpy, key );

  if( ( KeyCode ) NULL == keyCode )
    return False;

  return Success == XUngrabKey( _xklDpy, keyCode, 0, _xklRootWindow );
}

int XklGetNextGroup(  )
{
  return ( _xklCurState.group + 1 ) % XklGetNumGroups(  );
}
                                                                                          
int XklGetPrevGroup(  )
{
  int n = XklGetNumGroups(  );
  return ( _xklCurState.group + n - 1 ) % n;
}

int XklGetRestoreGroup(  )
{
  XklState state;
  if( _xklCurClient == ( Window ) NULL )
  {
    XklDebug( 150, "cannot restore without current client\n" );
  } else if( XklGetState( _xklCurClient, &state ) )
  {
    return state.group;
  } else
    XklDebug( 150,
              "Unbelievable: current client " WINID_FORMAT
              ", '%s' has no group\n", _xklCurClient,
              _XklGetDebugWindowTitle( _xklCurClient ) );
  return 0;
}

void XklSetTransparent( Window win, Bool transparent )
{
  Window appWin;
  Bool wasTransparent;
  XklDebug( 150, "setting transparent flag %d for " WINID_FORMAT "\n",
            transparent, win );

  if( !_XklGetAppWindow( win, &appWin ) )
  {
    XklDebug( 150, "No app window!\n" );
    appWin = win;
//    return;
  }

  wasTransparent = XklIsTransparent( appWin );
  XklDebug( 150, "appwin " WINID_FORMAT " was %stransparent\n", appWin,
            wasTransparent ? "" : "not " );
  if( transparent && !wasTransparent )
  {
    CARD32 prop = 1;
    XChangeProperty( _xklDpy, appWin, _xklAtoms[XKLAVIER_TRANSPARENT],
                     XA_INTEGER, 32, PropModeReplace,
                     ( const unsigned char * ) &prop, 1 );
  } else if( !transparent && wasTransparent )
  {
    XDeleteProperty( _xklDpy, appWin, _xklAtoms[XKLAVIER_TRANSPARENT] );
  }
}

Bool XklIsTransparent( Window win )
{
  Window appWin;

  if( !_XklGetAppWindow( win, &appWin ) )
    return False;
  return _XklIsTransparentAppWindow( appWin );

}

/**
 * "Adds" app window to the set of managed windows.
 * Actually, no data structures involved. The only thing we do is save app state
 * and register ourselves us listeners.
 * Note: User's callback is called
 */
void _XklAddAppWindow( Window appWin, Window parent, Bool ignoreExistingState,
                       XklState * initState )
{
  XklState state = *initState;
  int defGroupToUse = -1;

  if( appWin == _xklRootWindow )
    XklDebug( 150, "??? root app win ???\n" );

  XklDebug( 150, "Trying to add window " WINID_FORMAT "/%s with group %d\n",
            appWin, _XklGetDebugWindowTitle( appWin ), initState->group );

  if( !ignoreExistingState )
  {
    Bool have_state = _XklGetAppState( appWin, &state );

    if( have_state )
    {
      XklDebug( 150,
                "The window " WINID_FORMAT
                " does not require to be added, it already has the xklavier state \n",
                appWin );
      return;
    }
  }

  if( winCallback != NULL )
    defGroupToUse = ( *winCallback ) ( appWin, parent, winCallbackData );

  if( defGroupToUse == -1 )
    defGroupToUse = _xklDefaultGroup;

  if( defGroupToUse != -1 )
    state.group = defGroupToUse;

  _XklSaveAppState( appWin, &state );
  _XklSelectInputMerging( appWin, FocusChangeMask | PropertyChangeMask );

  if( defGroupToUse != -1 )
  {
    if( _xklCurClient == appWin )
    {
      if( ( _xklSecondaryGroupsMask & ( 1 << defGroupToUse ) ) != 0 )
        XklAllowOneSwitchToSecondaryGroup(  );
      XklLockGroup( defGroupToUse );
    }
  }

  if( parent == ( Window ) NULL )
    parent = _XklGetRegisteredParent( appWin );

  XklDebug( 150, "done\n" );
}

/**
 * Checks the window and goes up
 */
Bool _XklGetAppWindowBottomToTop( Window win, Window * appWin_return )
{
  Window parent = ( Window ) NULL, rwin = ( Window ) NULL, *children = NULL;
  int num = 0;

  if( win == ( Window ) NULL || win == _xklRootWindow )
  {
    *appWin_return = win;
    _xklLastErrorMsg = "The window is either 0 or root";
    return False;
  }

  if( _XklHasWmState( win ) )
  {
    *appWin_return = win;
    return True;
  }

  _xklLastErrorCode =
    _XklStatusQueryTree( _xklDpy, win, &rwin, &parent, &children, &num );

  if( _xklLastErrorCode != Success )
  {
    *appWin_return = ( Window ) NULL;
    return False;
  }

  if( children != NULL )
    XFree( children );

  return _XklGetAppWindowBottomToTop( parent, appWin_return );
}

/**
 * Recursively finds "App window" (window with WM_STATE) for given window.
 * First, checks the window itself
 * Then, for first level of recursion, checks childen,
 * Then, goes to parent.
 * NOTE: root window cannot be "App window" under normal circumstances
 */
Bool _XklGetAppWindow( Window win, Window * appWin_return )
{
  Window parent = ( Window ) NULL,
    rwin = ( Window ) NULL, *children = NULL, *child;
  int num = 0;
  Bool rv;

  if( win == ( Window ) NULL || win == _xklRootWindow )
  {
    *appWin_return = ( Window ) NULL;
    _xklLastErrorMsg = "The window is either 0 or root";
    XklDebug( 150,
              "Window " WINID_FORMAT
              " is either 0 or root so could not get the app window for it\n",
              win );
    return False;
  }

  if( _XklHasWmState( win ) )
  {
    *appWin_return = win;
    return True;
  }

  _xklLastErrorCode =
    _XklStatusQueryTree( _xklDpy, win, &rwin, &parent, &children, &num );

  if( _xklLastErrorCode != Success )
  {
    *appWin_return = ( Window ) NULL;
    XklDebug( 150,
              "Could not get tree for window " WINID_FORMAT
              " so could not get the app window for it\n", win );
    return False;
  }

  /**
   * Here we first check the children (in case win is just above some "App Window")
   * and then go upstairs
   */
  child = children;
  while( num )
  {
    if( _XklHasWmState( *child ) )
    {
      *appWin_return = *child;
      if( children != NULL )
        XFree( children );
      return True;
    }
    child++;
    num--;
  }

  if( children != NULL )
    XFree( children );

  rv = _XklGetAppWindowBottomToTop( parent, appWin_return );

  if( !rv )
    XklDebug( 200, "Could not get the app window for " WINID_FORMAT "/%s\n",
              win, _XklGetDebugWindowTitle( win ) );

  return rv;
}

/**
 * Loads the tree recursively.
 */
Bool _XklLoadWindowTree(  )
{
  Window focused;
  int revert;
  Bool retval, haveAppWindow;

  retval = _XklLoadSubtree( _xklRootWindow, 0, &_xklCurState );

  XGetInputFocus( _xklDpy, &focused, &revert );

  XklDebug( 160, "initially focused: " WINID_FORMAT ", '%s'\n", 
            focused, _XklGetDebugWindowTitle( focused ) );

  haveAppWindow = _XklGetAppWindow( focused, &_xklCurClient );

  if( haveAppWindow )
  {
    Bool haveState = _XklGetAppState( _xklCurClient, &_xklCurState );
    XklDebug( 160,
              "initial _xklCurClient: " WINID_FORMAT
              ", '%s' %s state %d/%X\n", _xklCurClient,
              _XklGetDebugWindowTitle( _xklCurClient ),
              ( haveState ? "with" : "without" ),
              ( haveState ? _xklCurState.group : -1 ),
              ( haveState ? _xklCurState.indicators : -1 ) );
  } else
  {
    XklDebug( 160,
              "could not find initial app. Probably, focus belongs to some WM service window. Will try to survive:)" );
  }

  return retval;
}

void _XklDebug( const char file[], const char function[], int level,
                const char format[], ... )
{
  va_list lst;

  if( level > _xklDebugLevel )
    return;

  va_start( lst, format );
  if( logAppender != NULL )
    ( *logAppender ) ( file, function, level, format, lst );
  va_end( lst );
}

void XklDefaultLogAppender( const char file[], const char function[],
                            int level, const char format[], va_list args )
{
  time_t now = time( NULL );
  fprintf( stdout, "[%08ld,%03d,%s:%s/] \t", now, level, file, function );
  vfprintf( stdout, format, args );
}

/**
 * Gets the state from the window property
 */
Bool _XklGetAppState( Window appWin, XklState * state_return )
{
  Atom type_ret;
  int format_ret;
  unsigned long nitems, rest;
  CARD32 *prop = NULL;
  Bool ret = False;
                                                                                            
  int grp = -1;
  unsigned inds = 0;
                                                                                            
  if( ( XGetWindowProperty
        ( _xklDpy, appWin, _xklAtoms[XKLAVIER_STATE], 0L,
          XKLAVIER_STATE_PROP_LENGTH, False,
          XA_INTEGER, &type_ret, &format_ret, &nitems, &rest,
          ( unsigned char ** ) &prop ) == Success )
      && ( type_ret == XA_INTEGER ) && ( format_ret == 32 ) )
  {
    grp = prop[0];
    if( grp >= XklGetNumGroups(  ) || grp < 0 )
      grp = 0;
                                                                                            
    inds = prop[1];
                                                                                            
    if( state_return != NULL )
    {
      state_return->group = grp;
      state_return->indicators = inds;
    }
    if( prop != NULL )
      XFree( prop );
                                                                                            
    ret = True;
  }
                                                                                            
  if( ret )
    XklDebug( 150,
              "Appwin " WINID_FORMAT
              ", '%s' has the group %d, indicators %X\n", appWin,
              _XklGetDebugWindowTitle( appWin ), grp, inds );
  else
    XklDebug( 150, "Appwin " WINID_FORMAT ", '%s' does not have state\n",
              appWin, _XklGetDebugWindowTitle( appWin ) );
                                                                                            
  return ret;
}
                                                                                            
/**
 * Deletes the state from the window properties
 */
void _XklDelAppState( Window appWin )
{
  XDeleteProperty( _xklDpy, appWin, _xklAtoms[XKLAVIER_STATE] );
}

/**
 * Saves the state into the window properties
 */
void _XklSaveAppState( Window appWin, XklState * state )
{
  CARD32 prop[XKLAVIER_STATE_PROP_LENGTH];

  prop[0] = state->group;
  prop[1] = state->indicators;

  XChangeProperty( _xklDpy, appWin, _xklAtoms[XKLAVIER_STATE], XA_INTEGER,
                   32, PropModeReplace, ( const unsigned char * ) prop,
                   XKLAVIER_STATE_PROP_LENGTH );

  XklDebug( 160,
            "Saved the group %d, indicators %X for appwin " WINID_FORMAT "\n",
            state->group, state->indicators, appWin );
}

/**
 * Just selects some events from the window.
 */
void _XklSelectInput( Window win, long mask )
{
  if( _xklRootWindow == win )
    XklDebug( 160,
              "Someone is looking for " WINID_FORMAT " on root window ***\n",
              mask );

  XSelectInput( _xklDpy, win, mask );
}

void _XklSelectInputMerging( Window win, long mask )
{
  XWindowAttributes attrs;
  long oldmask = 0L, newmask;
  memset( &attrs, 0, sizeof( attrs ) );
  if( XGetWindowAttributes( _xklDpy, win, &attrs ) )
    oldmask = attrs.your_event_mask;

  newmask = oldmask | mask;
  if( newmask != oldmask )
    _XklSelectInput( win, newmask );
}

void _XklTryCallStateCallback( XklStateChange changeType,
                               XklState * oldState )
{
  int group = _xklCurState.group;
  Bool restore = oldState->group == group;

  XklDebug( 150,
            "changeType: %d, group: %d, secondaryGroupMask: %X, allowsecondary: %d\n",
            changeType, group, _xklSecondaryGroupsMask,
            _xklAllowSecondaryGroupOnce );

  if( changeType == GROUP_CHANGED )
  {
    if( !restore )
    {
      if( ( _xklSecondaryGroupsMask & ( 1 << group ) ) != 0 &&
          !_xklAllowSecondaryGroupOnce )
      {
        XklDebug( 150, "secondary -> go next\n" );
        group = XklGetNextGroup(  );
        XklLockGroup( group );
        return;                 // we do not need to revalidate
      }
    }
    _xklAllowSecondaryGroupOnce = False;
  }
  if( stateCallback != NULL )
  {

    ( *stateCallback ) ( changeType, _xklCurState.group,
                         restore, stateCallbackData );
  }
}

Bool _XklIsTransparentAppWindow( Window appWin )
{
  Atom type_ret;
  int format_ret;
  unsigned long nitems, rest;
  CARD32 *prop = NULL;
  Bool retVal;
  if( ( XGetWindowProperty
        ( _xklDpy, appWin, _xklAtoms[XKLAVIER_TRANSPARENT], 0L, 1, False,
          XA_INTEGER, &type_ret, &format_ret, &nitems, &rest,
          ( unsigned char ** ) &prop ) == Success )
      && ( type_ret == XA_INTEGER ) && ( format_ret == 32 ) )
  {
    if( prop != NULL )
      XFree( prop );
    return True;
  }
  return False;
}
