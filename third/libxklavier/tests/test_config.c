#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <libxklavier/xklavier.h>
#include <libxklavier/xklavier_config.h>

enum { ACTION_NONE, ACTION_GET, ACTION_SET };

static void printUsage()
{
  printf( "Usage: test_config (-g)|(-s -m <model> -l <layouts> -o <options>)|(-h)(-d <debugLevel>)\n" );
  printf( "Options:\n" );
  printf( "         -g - Dump the current config, load original system settings and revert back\n" ); 
  printf( "         -s - Set the configuration given my -m -l -o options. Similar to setxkbmap\n" ); 
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

int main( int argc, char * const argv[] )
{
  int c, i;
  int action = ACTION_NONE;
  const char* model = NULL;
  const char* layouts = NULL;
  const char* options = NULL;
  int debugLevel = 0;

  while (1)
  {
    c = getopt( argc, argv, "hsgm:l:o:d:" );
    if ( c == -1 )
      break;
    switch (c)
    {
      case 's':
        printf( "Set the config\n" );
        action = ACTION_SET;
        break;
      case 'g':
        printf( "Get the config\n" );
        action = ACTION_GET;
        break;
      case 'm':
        printf( "Model: [%s]\n", model = optarg );
        break;
      case 'l':
        printf( "Layouts: [%s]\n", layouts = optarg );
        break;
      case 'o':
        printf( "Options: [%s]\n", options = optarg );
        break;
      case 'h':
        printUsage();
        exit(0);
      case 'd':
        debugLevel = atoi( optarg );
        break;
      default:
        fprintf( stderr, "?? getopt returned character code 0%o ??\n", c );
        printUsage();
    }
  }

  if ( action == ACTION_NONE )
  {
    printUsage();
    exit( 0 );
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
    XklConfigRec currentConfig, r2;
    XklSetDebugLevel( debugLevel );
    XklDebug( 0, "Xklavier initialized\n" );
    XklConfigInit();
    XklConfigLoadRegistry();
    XklDebug( 0, "Xklavier registry loaded\n" );
    XklDebug( 0, "Multiple layouts are %ssupported\n",
      XklMultipleLayoutsSupported() ? "" : "not " );

    XklConfigRecInit( &currentConfig );
    XklConfigGetFromServer( &currentConfig );

    switch ( action )
    {
      case ACTION_GET:
        XklDebug( 0, "Got config from the server\n" );
        dump( &currentConfig );

        XklConfigRecInit( &r2 );

        if ( XklConfigGetFromBackup( &r2 ) )
        {
          XklDebug( 0, "Got config from the backup\n" );
          dump( &r2 );
        }

        if ( XklConfigActivate( &r2, NULL ) )
        {
          XklDebug( 0, "The backup configuration restored\n" );
          if ( XklConfigActivate( &currentConfig, NULL ) )
          {
            XklDebug( 0, "Reverting the configuration change\n" );
          } else
          {
            XklDebug( 0, "The configuration could not be reverted: %s\n", XklGetLastError() );
          }
        } else
        {
          XklDebug( 0, "The backup configuration could not be restored: %s\n", XklGetLastError() );
        }

        XklConfigRecDestroy( &r2 );
        break;
      case ACTION_SET:
        if ( model != NULL )
        {
          if ( currentConfig.model != NULL ) free ( currentConfig.model );
          currentConfig.model = strdup( model );
        }

        if ( layouts != NULL )
        {
          if ( currentConfig.layouts != NULL ) 
          {
            for ( i = currentConfig.numLayouts; --i >=0; )
              free ( currentConfig.layouts[i] );
            free ( currentConfig.layouts );
            for ( i = currentConfig.numVariants; --i >=0; )
              free ( currentConfig.variants[i] );
            free ( currentConfig.variants );
          }
          currentConfig.numLayouts = 
          currentConfig.numVariants = 1;
          currentConfig.layouts = malloc( sizeof ( char* ) );
          currentConfig.layouts[0] = strdup( layouts );
          currentConfig.variants = malloc( sizeof ( char* ) );
          currentConfig.variants[0] = strdup( "" );
        }

        if ( options != NULL )
        {
          if ( currentConfig.options != NULL ) 
          {
            for ( i = currentConfig.numOptions; --i >=0; )
              free ( currentConfig.options[i] );
            free ( currentConfig.options );
          }
          currentConfig.numOptions = 1;
          currentConfig.options = malloc( sizeof ( char* ) );
          currentConfig.options[0] = strdup( options );
        }

        XklDebug( 0, "New config:\n" );
        dump( &currentConfig );
        if ( XklConfigActivate( &currentConfig, NULL ) )
            XklDebug( 0, "Set the config\n" );
        else
            XklDebug( 0, "Could not set the config: %s\n", XklGetLastError() );
        break;
    }

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
