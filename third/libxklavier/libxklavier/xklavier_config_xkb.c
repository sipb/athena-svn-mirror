#include <errno.h>
#include <string.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <libxml/xpath.h>

#include "config.h"

#include "xklavier_private.h"

#include "xklavier_private_xkb.h"

#ifdef XKB_HEADERS_PRESENT
#include <X11/extensions/XKBfile.h>
#include <X11/extensions/XKM.h>
#endif

// For "bad" X servers we hold our own copy
#define XML_CFG_FALLBACK_PATH ( DATA_DIR "/xfree86.xml" )

#define MULTIPLE_LAYOUTS_CHECK_PATH ( XKB_BASE "/symbols/pc/en_US" )

#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#ifdef XKB_HEADERS_PRESENT
XkbRF_VarDefsRec _xklVarDefs;
static XkbRF_RulesPtr _xklRules;
static XkbComponentNamesRec componentNames;
#endif

static char *locale;

static char* _XklGetRulesSetName( void )
{
#ifdef XKB_HEADERS_PRESENT
  static char rulesSetName[_XKB_RF_NAMES_PROP_MAXLEN] = "";
  if ( !rulesSetName[0] )
  {
    char* rf = NULL;
    if( !XklGetNamesProp( _xklAtoms[XKB_RF_NAMES_PROP_ATOM], &rf, NULL ) || ( rf == NULL ) )
    {
      strncpy( rulesSetName, XKB_DEFAULT_RULESET, sizeof rulesSetName );
      XklDebug( 100, "Using default rules set: [%s]\n", rulesSetName );
      return rulesSetName;
    }
    strncpy( rulesSetName, rf, sizeof rulesSetName );
    free( rf );
  }
  XklDebug( 100, "Rules set: [%s]\n", rulesSetName );
  return rulesSetName;
#else
  return NULL;
#endif
}

#ifdef XKB_HEADERS_PRESENT
static XkbRF_RulesPtr _XklLoadRulesSet( void )
{
  char fileName[MAXPATHLEN] = "";
  char* rf = _XklGetRulesSetName();

  _xklRules = NULL;
  if( rf == NULL )
  {
    _xklLastErrorMsg = "Could not find the XKB rules set";
    return NULL;
  }

  locale = setlocale( LC_ALL, NULL );
  if( locale != NULL )
    locale = strdup( locale );

  snprintf( fileName, sizeof fileName, XKB_BASE "/rules/%s", rf );
  _xklRules = XkbRF_Load( fileName, locale, True, True );

  if( _xklRules == NULL )
  {
    _xklLastErrorMsg = "Could not load rules";
    return NULL;
  }
  return _xklRules;
}
#endif

static void _XklFreeRulesSet( void )
{
#ifdef XKB_HEADERS_PRESENT
  if ( _xklRules )
    XkbRF_Free( _xklRules, True );
#endif
}

Bool XklConfigLoadRegistry( void )
{
  struct stat statBuf;
  char fileName[MAXPATHLEN] = "";
  char* rf = _XklGetRulesSetName();

  if ( rf == NULL )
    return False;

  snprintf( fileName, sizeof fileName, XKB_BASE "/rules/%s.xml", rf );

  if( stat( fileName, &statBuf ) != 0 )
  {
    strncpy( fileName, XML_CFG_FALLBACK_PATH, sizeof fileName );
    fileName[ MAXPATHLEN - 1 ] = '\0';
  }

  return XklConfigLoadRegistryFromFile( fileName );
}

static Bool _XklConfigPrepareBeforeKbd( const XklConfigRecPtr data )
{
#ifdef XKB_HEADERS_PRESENT
  XkbRF_RulesPtr rulesPtr = _XklLoadRulesSet();

  memset( &_xklVarDefs, 0, sizeof( _xklVarDefs ) );

  if( !rulesPtr )
    return False;

  _xklVarDefs.model = ( char * ) data->model;

  if( data->layouts != NULL )
    _xklVarDefs.layout = _XklConfigRecMergeLayouts( data );

  if( data->variants != NULL )
    _xklVarDefs.variant = _XklConfigRecMergeVariants( data );

  if( data->options != NULL )
    _xklVarDefs.options = _XklConfigRecMergeOptions( data );

  if( !XkbRF_GetComponents( rulesPtr, &_xklVarDefs, &componentNames ) )
  {
    _xklLastErrorMsg = "Could not translate rules into components";
    return False;
  }

  if ( _xklDebugLevel >= 200 )
  {
    XklDebug( 200, "keymap: %s\n", componentNames.keymap );
    XklDebug( 200, "keycodes: %s\n", componentNames.keycodes );
    XklDebug( 200, "compat: %s\n", componentNames.compat );
    XklDebug( 200, "types: %s\n", componentNames.types );
    XklDebug( 200, "symbols: %s\n", componentNames.symbols );
    XklDebug( 200, "geometry: %s\n", componentNames.geometry );
  }
#endif
  return True;
}

static void _XklConfigCleanAfterKbd(  )
{
#ifdef XKB_HEADERS_PRESENT
  _XklFreeRulesSet();

  free( locale );
  locale = NULL;

  free( _xklVarDefs.layout );
  free( _xklVarDefs.variant );
  free( _xklVarDefs.options );
  memset( &_xklVarDefs, 0, sizeof( _xklVarDefs ) );

  free(componentNames.keymap);
  free(componentNames.keycodes);
  free(componentNames.compat);
  free(componentNames.types);
  free(componentNames.symbols);
  free(componentNames.geometry);
#endif
}

// check only client side support
Bool XklMultipleLayoutsSupported( void )
{
  enum { NON_SUPPORTED, SUPPORTED, UNCHECKED };

  static int supportState = UNCHECKED;

  if ( supportState == UNCHECKED )
  {
    XklDebug( 100, "!!! Checking multiple layouts support\n" );
    supportState = NON_SUPPORTED;
#ifdef XKB_HEADERS_PRESENT
    XkbRF_RulesPtr rulesPtr = _XklLoadRulesSet();
    if ( rulesPtr )
    {
      XkbRF_VarDefsRec varDefs;
      XkbComponentNamesRec cNames;
      memset( &varDefs, 0, sizeof( varDefs ) );

      varDefs.model = "pc105";
      varDefs.layout = "a,b";
      varDefs.variant = "";
      varDefs.options = "";

      if( XkbRF_GetComponents( rulesPtr, &varDefs, &cNames ) )
      {
        XklDebug( 100, "!!! Multiple layouts ARE supported\n" );
        supportState = SUPPORTED;
      } else
      {
        XklDebug( 100, "!!! Multiple layouts ARE NOT supported\n" );
      }
      free(cNames.keymap);
      free(cNames.keycodes);
      free(cNames.compat);
      free(cNames.types);
      free(cNames.symbols);
      free(cNames.geometry);

      _XklFreeRulesSet();
    }
#endif
  }
  return supportState == SUPPORTED;
}

Bool XklConfigActivate( const XklConfigRecPtr data,
                        void *userData )
{
  Bool rv = False;
#if 0
  {
  int i;
  XklDebug( 150, "New model: [%s]\n", data->model );
  XklDebug( 150, "New layouts: %p\n", data->layouts );
  for( i = data->numLayouts; --i >= 0; )
    XklDebug( 150, "New layout[%d]: [%s]\n", i, data->layouts[i] );
  XklDebug( 150, "New variants: %p\n", data->variants );
  for( i = data->numVariants; --i >= 0; )
    XklDebug( 150, "New variant[%d]: [%s]\n", i, data->variants[i] );
  XklDebug( 150, "New options: %p\n", data->options );
  for( i = data->numOptions; --i >= 0; )
    XklDebug( 150, "New option[%d]: [%s]\n", i, data->options[i] );
  }
#endif

#ifdef XKB_HEADERS_PRESENT
  if( _XklConfigPrepareBeforeKbd( data ) )
  {
    XkbDescPtr xkb;
    xkb =
      XkbGetKeyboardByName( _xklDpy, XkbUseCoreKbd, &componentNames,
                            XkbGBN_AllComponentsMask,
                            XkbGBN_AllComponentsMask &
                            ( ~XkbGBN_GeometryMask ), True );

    //!! Do I need to free it anywhere?
    if( xkb != NULL )
    {
      if( XklSetNamesProp
          ( _xklAtoms[XKB_RF_NAMES_PROP_ATOM], _XklGetRulesSetName(), data ) )
          // We do not need to check the result of _XklGetRulesSetName - 
          // because PrepareBeforeKbd did it for us
        rv = True;
      else
        _xklLastErrorMsg = "Could not set names property";
      XkbFreeKeyboard( xkb, XkbAllComponentsMask, True );
    } else
    {
      _xklLastErrorMsg = "Could not load keyboard description";
    }
  }
  _XklConfigCleanAfterKbd(  );
#endif
  return rv;
}

Bool XklConfigWriteXKMFile( const char *fileName, const XklConfigRecPtr data,
                            void *userData )
{
  Bool rv = False;

#ifdef XKB_HEADERS_PRESENT
  FILE *output = fopen( fileName, "w" );
  XkbFileInfo dumpInfo;

  if( output == NULL )
  {
    _xklLastErrorMsg = "Could not open the XKB file";
    return False;
  }

  if( _XklConfigPrepareBeforeKbd( data ) )
  {
    XkbDescPtr xkb;
    xkb =
      XkbGetKeyboardByName( _xklDpy, XkbUseCoreKbd, &componentNames,
                            XkbGBN_AllComponentsMask,
                            XkbGBN_AllComponentsMask &
                            ( ~XkbGBN_GeometryMask ), False );
    if( xkb != NULL )
    {
      dumpInfo.defined = 0;
      dumpInfo.xkb = xkb;
      dumpInfo.type = XkmKeymapFile;
      rv = XkbWriteXKMFile( output, &dumpInfo );
      XkbFreeKeyboard( xkb, XkbGBN_AllComponentsMask, True );
    } else
      _xklLastErrorMsg = "Could not load keyboard description";
  }
  _XklConfigCleanAfterKbd(  );
  fclose( output );
#endif
  return rv;
}
