#include <time.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>

#include "xklavier_private.h"
#include "xklavier_private_xkb.h"

#ifdef XKB_HEADERS_PRESENT
XkbDescPtr _xklXkb;

char *_xklIndicatorNames[XkbNumIndicators];

unsigned _xklPhysIndicatorsMask;

int _xklXkbEventType, _xklXkbError;

static char *groupNames[XkbNumKbdGroups];

const char **XklGetGroupNames( void )
{
  return ( const char ** ) groupNames;
}
#else
const char **XklGetGroupNames( void )
{
  static const char* dummyName = "???";
  return ( const char ** ) &dummyName;
}
#endif

int XklInit( Display * a_dpy )
{
  int opcode;
  int scr;

  if( !a_dpy )
  {
    XklDebug( 10, "XklInit : display is NULL ?\n");
    return -1;
  }

  _xklDefaultErrHandler =
    XSetErrorHandler( ( XErrorHandler ) _XklErrHandler );

  _xklDpy = a_dpy;
#ifdef XKB_HEADERS_PRESENT
  /* Lets begin */
  _xklXkbExtPresent = XkbQueryExtension( _xklDpy,
                                         &opcode, &_xklXkbEventType,
                                         &_xklXkbError, NULL, NULL );
  if( !_xklXkbExtPresent )
  {
    _xklDpy = NULL;
    XSetErrorHandler( ( XErrorHandler ) _xklDefaultErrHandler );
    return -1;
  }
#endif

  scr = DefaultScreen( _xklDpy );
  _xklRootWindow = RootWindow( _xklDpy, scr );
#ifdef XKB_HEADERS_PRESENT
  XklDebug( 160,
            "xkbEvenType: %X, xkbError: %X, display: %p, root: " WINID_FORMAT
            "\n", _xklXkbEventType, _xklXkbError, _xklDpy, _xklRootWindow );
#else
  XklDebug( 160,
            "NO XKB LIBS, display: %p, root: " WINID_FORMAT
            "\n", _xklDpy, _xklRootWindow );
#endif

  _xklAtoms[WM_NAME] = XInternAtom( _xklDpy, "WM_NAME", False );
  _xklAtoms[WM_STATE] = XInternAtom( _xklDpy, "WM_STATE", False );
  _xklAtoms[XKLAVIER_STATE] = XInternAtom( _xklDpy, "XKLAVIER_STATE", False );
  _xklAtoms[XKLAVIER_TRANSPARENT] =
    XInternAtom( _xklDpy, "XKLAVIER_TRANSPARENT", False );
#ifdef XKB_HEADERS_PRESENT
  _xklAtoms[XKB_RF_NAMES_PROP_ATOM] =
    XInternAtom( _xklDpy, _XKB_RF_NAMES_PROP_ATOM, False );
  _xklAtoms[XKB_RF_NAMES_PROP_ATOM_BACKUP] =
    XInternAtom( _xklDpy, "_XKB_RULES_NAMES_BACKUP", False );
#endif

  _xklAllowSecondaryGroupOnce = False;
  _xklSkipOneRestore = False;
  _xklDefaultGroup = -1;
  _xklSecondaryGroupsMask = 0L;
  _xklPrevAppWindow = 0;

  return _XklLoadAllInfo(  )? 0 : _xklLastErrorCode;
}

int XklPauseListen(  )
{
#ifdef XKB_HEADERS_PRESENT
  XkbSelectEvents( _xklDpy, XkbUseCoreKbd, XkbAllEventsMask, 0 );
//  XkbSelectEventDetails( _xklDpy,
  //                       XkbUseCoreKbd,
  //                     XkbStateNotify,
  //                   0,
  //                 0 );

  //!!_XklSelectInput( _xklRootWindow, 0 );
#endif
  return 0;
}

int XklResumeListen(  )
{
#ifdef XKB_HEADERS_PRESENT
  /* What events we want */
#define XKB_EVT_MASK \
         (XkbStateNotifyMask| \
          XkbNamesNotifyMask| \
          XkbControlsNotifyMask| \
          XkbIndicatorStateNotifyMask| \
          XkbIndicatorMapNotifyMask| \
          XkbNewKeyboardNotifyMask)

  XkbSelectEvents( _xklDpy, XkbUseCoreKbd, XKB_EVT_MASK, XKB_EVT_MASK );

#define XKB_STATE_EVT_DTL_MASK \
         (XkbGroupStateMask)

  XkbSelectEventDetails( _xklDpy,
                         XkbUseCoreKbd,
                         XkbStateNotify,
                         XKB_STATE_EVT_DTL_MASK, XKB_STATE_EVT_DTL_MASK );

#define XKB_NAMES_EVT_DTL_MASK \
         (XkbGroupNamesMask|XkbIndicatorNamesMask)

  XkbSelectEventDetails( _xklDpy,
                         XkbUseCoreKbd,
                         XkbNamesNotify,
                         XKB_NAMES_EVT_DTL_MASK, XKB_NAMES_EVT_DTL_MASK );
#endif
  _XklSelectInputMerging( _xklRootWindow,
                          SubstructureNotifyMask | PropertyChangeMask );
  _XklGetRealState( &_xklCurState );
  return 0;
}

unsigned XklGetNumGroups(  )
{
#ifdef XKB_HEADERS_PRESENT
  return _xklXkb->ctrls->num_groups;
#else
  return 1;
#endif
}

#define KBD_MASK \
    ( 0 )
#define CTRLS_MASK \
  ( XkbSlowKeysMask )
#define NAMES_MASK \
  ( XkbGroupNamesMask | XkbIndicatorNamesMask )

void _XklFreeAllInfo(  )
{
#ifdef XKB_HEADERS_PRESENT
  if( _xklXkb != NULL )
  {
    int i;
    char **groupName = groupNames;
    for( i = _xklXkb->ctrls->num_groups; --i >= 0; groupName++ )
      if( *groupName )
      {
        XFree( *groupName );
        *groupName = NULL;
      }
    XkbFreeKeyboard( _xklXkb, XkbAllComponentsMask, True );
    _xklXkb = NULL;
  }
#endif
}

/**
 * Load some XKB parameters
 */
Bool _XklLoadAllInfo(  )
{
#ifdef XKB_HEADERS_PRESENT
  int i;
  unsigned bit;
  Atom *gna;
  char **groupName;

  _xklXkb = XkbGetMap( _xklDpy, KBD_MASK, XkbUseCoreKbd );
  if( _xklXkb == NULL )
  {
    _xklLastErrorMsg = "Could not load keyboard";
    return False;
  }

  _xklLastErrorCode = XkbGetControls( _xklDpy, CTRLS_MASK, _xklXkb );

  if( _xklLastErrorCode != Success )
  {
    _xklLastErrorMsg = "Could not load controls";
    return False;
  }

  XklDebug( 200, "found %d groups\n", _xklXkb->ctrls->num_groups );

  _xklLastErrorCode = XkbGetNames( _xklDpy, NAMES_MASK, _xklXkb );

  if( _xklLastErrorCode != Success )
  {
    _xklLastErrorMsg = "Could not load names";
    return False;
  }

  gna = _xklXkb->names->groups;
  groupName = groupNames;
  for( i = _xklXkb->ctrls->num_groups; --i >= 0; gna++, groupName++ )
  {
    *groupName = XGetAtomName( _xklDpy,
                               *gna == None ?
                               XInternAtom( _xklDpy, "-", False ) : *gna );
    XklDebug( 200, "group %d has name [%s]\n", i, *groupName );
  }

  _xklLastErrorCode =
    XkbGetIndicatorMap( _xklDpy, XkbAllIndicatorsMask, _xklXkb );

  if( _xklLastErrorCode != Success )
  {
    _xklLastErrorMsg = "Could not load indicator map";
    return False;
  }

  for( i = 0, bit = 1; i < XkbNumIndicators; i++, bit <<= 1 )
  {
    Atom a = _xklXkb->names->indicators[i];
    if( a != None )
      _xklIndicatorNames[i] = XGetAtomName( _xklDpy, a );
    else
      _xklIndicatorNames[i] = "";

    XklDebug( 200, "Indicator[%d] is %s\n", i, _xklIndicatorNames[i] );
  }

  XklDebug( 200, "Real indicators are %X\n",
            _xklXkb->indicators->phys_indicators );

#endif
  if( _xklConfigCallback != NULL )
    ( *_xklConfigCallback ) ( _xklConfigCallbackData );
  return True;
}

void XklLockGroup( int group )
{
  XklDebug( 100, "Posted request for change the group to %d ##\n", group );
#ifdef XKB_HEADERS_PRESENT
  XkbLockGroup( _xklDpy, XkbUseCoreKbd, group );
#endif
  XSync( _xklDpy, False );
}

/**
 * Updates current internal state from X state
 */
void _XklGetRealState( XklState * curState_return )
{
#ifdef XKB_HEADERS_PRESENT
  XkbStateRec state;
#endif

  curState_return->group = 0;
#ifdef XKB_HEADERS_PRESENT
  if( Success == XkbGetState( _xklDpy, XkbUseCoreKbd, &state ) )
    curState_return->group = state.locked_group;

  if( Success ==
      XkbGetIndicatorState( _xklDpy, XkbUseCoreKbd,
                            &curState_return->indicators ) )
    curState_return->indicators &= _xklXkb->indicators->phys_indicators;
  else
#endif
    curState_return->indicators = 0;

}

/*
 * Actually taken from mxkbledpanel, valueChangedProc
 */
Bool _XklSetIndicator( int indicatorNum, Bool set )
{
#ifdef XKB_HEADERS_PRESENT
  XkbIndicatorMapPtr map;

  map = _xklXkb->indicators->maps + indicatorNum;

  /* The 'flags' field tells whether this indicator is automatic
   * (XkbIM_NoExplicit - 0x80), explicit (XkbIM_NoAutomatic - 0x40),
   * or neither (both - 0xC0).
   *
   * If NoAutomatic is set, the server ignores the rest of the 
   * fields in the indicator map (i.e. it disables automatic control 
   * of the LED).   If NoExplicit is set, the server prevents clients 
   * from explicitly changing the value of the LED (using the core 
   * protocol *or* XKB).   If NoAutomatic *and* NoExplicit are set, 
   * the LED cannot be changed (unless you change the map first).   
   * If neither NoAutomatic nor NoExplicit are set, the server will 
   * change the LED according to the indicator map, but clients can 
   * override that (until the next automatic change) using the core 
   * protocol or XKB.
   */
  switch ( map->flags & ( XkbIM_NoExplicit | XkbIM_NoAutomatic ) )
  {
    case XkbIM_NoExplicit | XkbIM_NoAutomatic:
    {
      // Can do nothing. Just ignore the indicator
      return True;
    }

    case XkbIM_NoAutomatic:
    {
      if( _xklXkb->names->indicators[indicatorNum] != None )
        XkbSetNamedIndicator( _xklDpy, XkbUseCoreKbd,
                              _xklXkb->names->indicators[indicatorNum], set,
                              False, NULL );
      else
      {
        XKeyboardControl xkc;
        xkc.led = indicatorNum;
        xkc.led_mode = set ? LedModeOn : LedModeOff;
        XChangeKeyboardControl( _xklDpy, KBLed | KBLedMode, &xkc );
        XSync( _xklDpy, 0 );
      }

      return True;
    }

    case XkbIM_NoExplicit:
      break;
  }

  /* The 'ctrls' field tells what controls tell this indicator to
   * to turn on:  RepeatKeys (0x1), SlowKeys (0x2), BounceKeys (0x4),
   *              StickyKeys (0x8), MouseKeys (0x10), AccessXKeys (0x20),
   *              TimeOut (0x40), Feedback (0x80), ToggleKeys (0x100),
   *              Overlay1 (0x200), Overlay2 (0x400), GroupsWrap (0x800),
   *              InternalMods (0x1000), IgnoreLockMods (0x2000),
   *              PerKeyRepeat (0x3000), or ControlsEnabled (0x4000)
   */
  if( map->ctrls )
  {
    unsigned long which = map->ctrls;

    XkbGetControls( _xklDpy, XkbAllControlsMask, _xklXkb );
    if( set )
      _xklXkb->ctrls->enabled_ctrls |= which;
    else
      _xklXkb->ctrls->enabled_ctrls &= ~which;
    XkbSetControls( _xklDpy, which | XkbControlsEnabledMask, _xklXkb );
  }

  /* The 'which_groups' field tells when this indicator turns on
   *      * for the 'groups' field:  base (0x1), latched (0x2), locked (0x4),
   *           *                          or effective (0x8).
   *                */
  if( map->groups )
  {
    int i;
    unsigned int group = 1;

    /* Turning on a group indicator is kind of tricky.  For
     * now, we will just Latch or Lock the first group we find
     * if that is what this indicator does.  Otherwise, we're
     * just going to punt and get out of here.
     */
    if( set )
    {
      for( i = XkbNumKbdGroups; --i >= 0; )
        if( ( 1 << i ) & map->groups )
        {
          group = i;
          break;
        }
      if( map->which_groups & ( XkbIM_UseLocked | XkbIM_UseEffective ) )
      {
        // Important: Groups should be ignored here - because they are handled separately!
        // XklLockGroup( group );
      } else if( map->which_groups & XkbIM_UseLatched )
        XkbLatchGroup( _xklDpy, XkbUseCoreKbd, group );
      else
      {
        // Can do nothing. Just ignore the indicator
        return True;
      }
    } else
      /* Turning off a group indicator will mean that we just
       * Lock the first group that this indicator doesn't watch.
       */
    {
      for( i = XkbNumKbdGroups; --i >= 0; )
        if( !( ( 1 << i ) & map->groups ) )
        {
          group = i;
          break;
        }
      XklLockGroup( group );
    }
  }

  /* The 'which_mods' field tells when this indicator turns on
   * for the modifiers:  base (0x1), latched (0x2), locked (0x4),
   *                     or effective (0x8).
   *
   * The 'real_mods' field tells whether this turns on when one of 
   * the real X modifiers is set:  Shift (0x1), Lock (0x2), Control (0x4),
   * Mod1 (0x8), Mod2 (0x10), Mod3 (0x20), Mod4 (0x40), or Mod5 (0x80). 
   *
   * The 'virtual_mods' field tells whether this turns on when one of
   * the virtual modifiers is set.
   *
   * The 'mask' field tells what real X modifiers the virtual_modifiers
   * map to?
   */
  if( map->mods.real_mods || map->mods.mask )
  {
    unsigned int affect, mods;

    affect = ( map->mods.real_mods | map->mods.mask );

    mods = set ? affect : 0;

    if( map->which_mods & ( XkbIM_UseLocked | XkbIM_UseEffective ) )
      XkbLockModifiers( _xklDpy, XkbUseCoreKbd, affect, mods );
    else if( map->which_mods & XkbIM_UseLatched )
      XkbLatchModifiers( _xklDpy, XkbUseCoreKbd, affect, mods );
    else
    {
      return True;
    }
  }
#endif
  return True;
}
