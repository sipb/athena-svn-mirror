#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <libxklavier/xklavier.h>
#include <libxklavier/xklavier_config.h>

static void printUsage()
{
  printf( "Usage: test_monitor (-h)|(-d <debugLevel>)\n" );
  printf( "Options:\n" );
  printf( "         -d - Set the debug level (by default, 0)\n" ); 
  printf( "         -h - Show this help\n" );
}

static void dump( XklConfigRecPtr ptr )
{
  int i,j;
  char**p;
  XklDebug( 0, "  model: [%s]\n", ptr->model );

  XklDebug( 0, "  layouts(%d):\n", ptr->numLayouts );
  p = ptr->layouts;
  for( i = ptr->numLayouts, j = 0; --i >= 0; )
    XklDebug( 0, "  %d: [%s]\n", j++, *p++ );

  XklDebug( 0, "  variants(%d):\n", ptr->numVariants );
  p = ptr->variants;
  for( i = ptr->numVariants, j = 0; --i >= 0; )
    XklDebug( 0, "  %d: [%s]\n", j++, *p++ );

  XklDebug( 0, "  options(%d):\n", ptr->numOptions );
  p = ptr->options;
  for( i = ptr->numOptions, j = 0; --i >= 0; )
    XklDebug( 0, "  %d: [%s]\n", j++, *p++ );
}

int main( int argc, char * argv[] )
{
  int c;
  int debugLevel = 0;
  XkbEvent ev;

  while (1)
  {
    c = getopt( argc, argv, "hd:" );
    if ( c == -1 )
      break;
    switch (c)
    {
      case 'h':
        printUsage();
        exit(0);
      case 'd':
        debugLevel = atoi( optarg );
        break;
      default:
        fprintf( stderr, "?? getopt returned character code 0%o ??\n", c );
        printUsage();
        exit(0);
    }
  }

  Display* dpy = XOpenDisplay( NULL );
  if ( dpy == NULL )
  {
    fprintf( stderr, "Could not open display\n" );
    exit(1);
  }
  printf( "opened display: %p\n", dpy );
  if ( !XklInit( dpy ) )
  {
    XklConfigRec currentConfig;
    XklSetDebugLevel( debugLevel );
    XklDebug( 0, "Xklavier initialized\n" );
    XklConfigInit();
    XklConfigLoadRegistry();
    XklDebug( 0, "Xklavier registry loaded\n" );

    XklConfigRecInit( &currentConfig );
    XklConfigGetFromServer( &currentConfig );

    XklStartListen();

    while (1) 
    {
      int grp;
      XNextEvent( dpy, &ev.core );
      if ( XklFilterEvents( &ev.core ) )
        XklDebug( 200, "Unknown event %d\n", ev.type );
    }

    XklStopListen();

    XklConfigRecDestroy( &currentConfig );

    XklConfigFreeRegistry();
    XklConfigTerm();
    XklDebug( 0, "Xklavier registry freed\n" );
    XklDebug( 0, "Xklavier terminating\n" );
    XklTerm();
  } else
  {
    fprintf( stderr, "Could not init Xklavier\n" );
    exit(2);
  }
  printf( "closing display: %p\n", dpy );
  XCloseDisplay(dpy);
  return 0;
}
