#ifndef __XKLAVIER_PRIVATE_H__
#define __XKLAVIER_PRIVATE_H__

#include <stdio.h>

#include <libxklavier/xklavier_config.h>

extern void _XklGetRealState( XklState * curState_return );
extern void _XklAddAppWindow( Window win, Window parent, Bool force,
                              XklState * initState );
extern Bool _XklGetAppWindowBottomToTop( Window win, Window * appWin_return );
extern Bool _XklGetAppWindow( Window win, Window * appWin_return );

extern void _XklFocusInEvHandler( XFocusChangeEvent * fev );
extern void _XklFocusOutEvHandler( XFocusChangeEvent * fev );
extern void _XklPropertyEvHandler( XPropertyEvent * rev );
extern void _XklCreateEvHandler( XCreateWindowEvent * cev );

extern void _XklErrHandler( Display * dpy, XErrorEvent * evt );

extern Window _XklGetRegisteredParent( Window win );
extern Bool _XklLoadAllInfo( void );
extern void _XklFreeAllInfo( void );
extern Bool _XklLoadWindowTree( void );
extern Bool _XklLoadSubtree( Window window, int level, XklState * initState );

extern Bool _XklHasWmState( Window win );

extern Bool _XklGetAppState( Window appWin, XklState * state_return );
extern void _XklDelAppState( Window appWin );
extern void _XklSaveAppState( Window appWin, XklState * state );

extern void _XklSelectInputMerging( Window win, long mask );

extern char *_XklGetDebugWindowTitle( Window win );

extern Status _XklStatusQueryTree( Display * display,
                                   Window w,
                                   Window * root_return,
                                   Window * parent_return,
                                   Window ** children_return,
                                   signed int *nchildren_return );

extern Bool _XklSetIndicator( int indicatorNum, Bool set );

extern void _XklTryCallStateCallback( XklStateChange changeType,
                                      XklState * oldState );

extern void _XklI18NInit(  );

extern char *_XklLocaleFromUtf8( const char *utf8string );

extern int _XklGetLanguagePriority( const char *language );

extern char *_XklConfigRecMergeByComma( const char **array,
                                        const int arrayLength );

extern char *_XklConfigRecMergeLayouts( const XklConfigRecPtr data );

extern char *_XklConfigRecMergeVariants( const XklConfigRecPtr data );

extern char *_XklConfigRecMergeOptions( const XklConfigRecPtr data );

extern void _XklConfigRecSplitByComma( char ***array,
                                       int *arraySize, const char *merged );

extern void _XklConfigRecSplitLayouts( XklConfigRecPtr data,
                                       const char *merged );

extern void _XklConfigRecSplitVariants( XklConfigRecPtr data,
                                        const char *merged );

extern void _XklConfigRecSplitOptions( XklConfigRecPtr data,
                                       const char *merged );

extern const char *_XklGetEventName( int type );

extern Bool _XklIsTransparentAppWindow( Window appWin );

extern Display *_xklDpy;

extern Window _xklRootWindow;

extern XklState _xklCurState;

extern Window _xklCurClient;

extern Status _xklLastErrorCode;

extern const char *_xklLastErrorMsg;

extern XErrorHandler _xklDefaultErrHandler;

extern char *_xklIndicatorNames[];

#define WM_NAME 0
#define WM_STATE 1
#define XKLAVIER_STATE 2
#define XKLAVIER_TRANSPARENT 3

#ifdef XKB_HEADERS_PRESENT
  #define XKB_RF_NAMES_PROP_ATOM 4
  #define XKB_RF_NAMES_PROP_ATOM_BACKUP 5
  #define TOTAL_ATOMS 6
#else
  #define TOTAL_ATOMS 4
#endif

#define XKLAVIER_STATE_PROP_LENGTH 2

// taken from XFree86 maprules.c
#define _XKB_RF_NAMES_PROP_MAXLEN 1024

extern Atom _xklAtoms[];

extern Bool _xklAllowSecondaryGroupOnce;

extern int _xklDefaultGroup;

extern Bool _xklSkipOneRestore;

extern int _xklSecondaryGroupsMask;

extern int _xklDebugLevel;

extern Window _xklPrevAppWindow;

#define WINID_FORMAT "%lx"

extern XklConfigCallback _xklConfigCallback;

extern void *_xklConfigCallbackData;

#endif
