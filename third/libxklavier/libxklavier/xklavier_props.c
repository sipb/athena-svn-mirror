#include <errno.h>
#include <string.h>
#include <locale.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <libxml/xpath.h>

#include "config.h"

#include "xklavier.h"
#include "xklavier_config.h"
#include "xklavier_private.h"

void XklConfigRecInit( XklConfigRecPtr data )
{
  // clear the structure VarDefsPtr...
  memset( ( void * ) data, 0, sizeof( XklConfigRec ) );
}

static Bool PtrsEqual( char* p1, char* p2 )
{
  if ( p1 == p2 )
    return True;
  if ( ( p1 == NULL && p2 != NULL ) ||
       ( p1 != NULL && p2 == NULL ) )
    return False;
  return !strcmp( p1, p2 );
}

static Bool ListsEqual( int numItems1, char** items1, 
                        int numItems2, char** items2 )
{
  int i;
  if ( numItems1 != numItems2 )
    return False;
  if ( items1 == items2 )
    return True;
  for( i = numItems1; --i >= 0; )
     if ( !PtrsEqual( *items1++ , *items2++ ) )
       return False;
  return True;
}

Bool XklConfigRecEquals( XklConfigRecPtr data1, XklConfigRecPtr data2 )
{
  if ( data1 == data2 )
    return True;
  if ( !PtrsEqual( data1->model, data2->model ) )
    return False;
  if ( !ListsEqual( data1->numLayouts, data1->layouts, 
                    data2->numLayouts, data2->layouts ) )
    return False;
  if ( !ListsEqual( data1->numVariants, data1->variants, 
                    data2->numVariants, data2->variants ) )
    return False;
  return ListsEqual( data1->numOptions, data1->options, 
                     data2->numOptions, data2->options );
}

void XklConfigRecDestroy( XklConfigRecPtr data )
{
  int i;
  char **p;

  if( data->model != NULL )
    free( data->model );

  if( ( p = data->layouts ) != NULL )
  {
    for( i = data->numLayouts; --i >= 0; )
      free( *p++ );
    free( data->layouts );
  }

  if( ( p = data->variants ) != NULL )
  {
    for( i = data->numVariants; --i >= 0; )
      free( *p++ );
    free( data->variants );
  }

  if( ( p = data->options ) != NULL )
  {
    for( i = data->numOptions; --i >= 0; )
      free( *p++ );
    free( data->options );
  }
}

void XklConfigRecReset( XklConfigRecPtr data )
{
  XklConfigRecDestroy( data );
  XklConfigRecInit( data );
}

Bool XklConfigGetFromServer( XklConfigRecPtr data )
{
#ifdef XKB_HEADERS_PRESENT
  char *rulesFile = NULL;
  Bool rv =
    XklGetNamesProp( _xklAtoms[XKB_RF_NAMES_PROP_ATOM], &rulesFile, data );
  if( rulesFile != NULL )
    free( rulesFile );

  return rv;
#else
  return False;
#endif
}

Bool XklConfigGetFromBackup( XklConfigRecPtr data )
{
#ifdef XKB_HEADERS_PRESENT
  char *rulesFile = NULL;
  Bool rv =
    XklGetNamesProp( _xklAtoms[XKB_RF_NAMES_PROP_ATOM_BACKUP], &rulesFile,
                     data );
  if( rulesFile != NULL )
    free( rulesFile );

  return rv;
#else
  return False;
#endif
}

Bool XklBackupNamesProp(  )
{
  Bool rv = True;
#ifdef XKB_HEADERS_PRESENT
  char *rf;
  XklConfigRec data;

  XklConfigRecInit( &data );
  if( XklGetNamesProp
      ( _xklAtoms[XKB_RF_NAMES_PROP_ATOM_BACKUP], &rf, &data ) )
  {
    XklConfigRecDestroy( &data );
    if( rf != NULL )
      free( rf );
    return True;
  }
  // "backup" property is not defined
  XklConfigRecReset( &data );
  if( XklGetNamesProp( _xklAtoms[XKB_RF_NAMES_PROP_ATOM], &rf, &data ) )
  {
#if 0
    int i;
    XklDebug( 150, "Original model: [%s]\n", data.model );

    XklDebug( 150, "Original layouts(%d):\n", data.numLayouts );
    for( i = data.numLayouts; --i >= 0; )
      XklDebug( 150, "%d: [%s]\n", i, data.layouts[i] );

    XklDebug( 150, "Original variants(%d):\n", data.numVariants );
    for( i = data.numVariants; --i >= 0; )
      XklDebug( 150, "%d: [%s]\n", i, data.variants[i] );

    XklDebug( 150, "Original options(%d):\n", data.numOptions );
    for( i = data.numOptions; --i >= 0; )
      XklDebug( 150, "%d: [%s]\n", i, data.options[i] );
#endif
    if( !XklSetNamesProp( _xklAtoms[XKB_RF_NAMES_PROP_ATOM_BACKUP], rf, &data ) )
    {
      XklDebug( 150, "Could not backup the configuration" );
      rv = False;
    }
    if( rf != NULL )
      free( rf );
  } else
  {
    XklDebug( 150, "Could not get the configuration for backup" );
    rv = False;
  }
  XklConfigRecDestroy( &data );
#endif
  return rv;
}

Bool XklRestoreNamesProp(  )
{
  Bool rv = True;
#ifdef XKB_HEADERS_PRESENT
  char *rf;
  XklConfigRec data;

  XklConfigRecInit( &data );
  if( !XklGetNamesProp( _xklAtoms[XKB_RF_NAMES_PROP_ATOM_BACKUP], &rf, &data ) )
  {
    XklConfigRecDestroy( &data );
    return False;
  }

  if( rf != NULL )
    free( rf );

  if( !XklSetNamesProp( _xklAtoms[XKB_RF_NAMES_PROP_ATOM], rf, &data ) )
  {
    XklDebug( 150, "Could not backup the configuration" );
    rv = False;
  }
  XklConfigRecDestroy( &data );
#endif
  return rv;
}

Bool XklGetNamesProp( Atom rulesAtom,
                      char **rulesFileOut, XklConfigRecPtr data )
{
  Atom realPropType;
  int fmt;
  unsigned long nitems, extraBytes;
  char *propData = NULL, *out;
  Status rtrn;

  // no such atom!
  if( rulesAtom == None )       /* property cannot exist */
  {
    _xklLastErrorMsg = "Could not find the atom";
    return False;
  }

  rtrn =
    XGetWindowProperty( _xklDpy, _xklRootWindow, rulesAtom, 0L,
                        _XKB_RF_NAMES_PROP_MAXLEN, False, XA_STRING,
                        &realPropType, &fmt, &nitems, &extraBytes,
                        ( unsigned char ** ) &propData );
  // property not found!
  if( rtrn != Success )
  {
    _xklLastErrorMsg = "Could not get the property";
    return False;
  }
  // set rules file to ""
  if( rulesFileOut )
    *rulesFileOut = NULL;

  // has to be array of strings
  if( ( extraBytes > 0 ) || ( realPropType != XA_STRING ) || ( fmt != 8 ) )
  {
    if( propData )
      XFree( propData );
    _xklLastErrorMsg = "Wrong property format";
    return False;
  }

  if( !propData )
  {
    _xklLastErrorMsg = "No properties returned";
    return False;
  }

  // rules file
  out = propData;
  if( out && ( *out ) && rulesFileOut )
    *rulesFileOut = strdup( out );
  out += strlen( out ) + 1;

  // if user is interested in rules only - don't waste the time
  if( !data )
  {
    XFree( propData );
    return True;
  }

  if( ( out - propData ) < nitems )
  {
    if( *out )
      data->model = strdup( out );
    out += strlen( out ) + 1;
  }

  if( ( out - propData ) < nitems )
  {
    _XklConfigRecSplitLayouts( data, out );
    out += strlen( out ) + 1;
  }

  if( ( out - propData ) < nitems )
  {
    int i;
    char **theLayout, **theVariant;
    _XklConfigRecSplitVariants( data, out );
    /*
       Now have to ensure that number of variants matches the number of layouts
       The 'remainder' is filled with NULLs (not ""s!)
     */
    if( data->numVariants < data->numLayouts )
    {
      data->variants =
        realloc( data->variants, data->numLayouts * sizeof( char * ) );
      memset( data->variants + data->numVariants, 0,
              ( data->numLayouts - data->numVariants ) * sizeof( char * ) );
      data->numVariants = data->numLayouts;
    }
    // take variants from layouts like ru(winkeys)
    theLayout = data->layouts;
    theVariant = data->variants;
    for( i = data->numLayouts; --i >= 0; theLayout++, theVariant++ )
    {
      if( *theLayout != NULL )
      {
        char *varstart = strchr( *theLayout, '(' );
        if( varstart != NULL )
        {
          char *varend = strchr( varstart, ')' );
          if( varend != NULL )
          {
            int varlen = varend - varstart;
            int laylen = varstart - *theLayout;
            // I am not sure - but I assume variants in layout have priority
            char *var = *theVariant = ( *theVariant != NULL ) ?
              realloc( *theVariant, varlen ) : malloc( varlen );
            memcpy( var, varstart + 1, --varlen );
            var[varlen] = '\0';
            realloc( *theLayout, laylen + 1 );
            ( *theLayout )[laylen] = '\0';
          }
        }
      }
    }
    out += strlen( out ) + 1;
  }

  if( ( out - propData ) < nitems )
  {
    _XklConfigRecSplitOptions( data, out );
//    out += strlen( out ) + 1;
  }
  XFree( propData );
  return True;
}

// taken from XFree86 maprules.c
Bool XklSetNamesProp( Atom rulesAtom,
                      char *rulesFile, const XklConfigRecPtr data )
{
  int len, i, rv;
  char *pval;
  char *next;
  char *allLayouts = _XklConfigRecMergeLayouts( data );
  char *allVariants = _XklConfigRecMergeVariants( data );
  char *allOptions = _XklConfigRecMergeOptions( data );

  len = ( rulesFile ? strlen( rulesFile ) : 0 );
  len += ( data->model ? strlen( data->model ) : 0 );
  len += ( allLayouts ? strlen( allLayouts ) : 0 );
  len += ( allVariants ? strlen( allVariants ) : 0 );
  len += ( allOptions ? strlen( allOptions ) : 0 );
  if( len < 1 )
    return True;

  len += 5;                     /* trailing NULs */

  pval = next = ( char * ) malloc( len + 1 );
  if( !pval )
  {
    _xklLastErrorMsg = "Could not allocate buffer";
    return False;
  }
  if( rulesFile )
  {
    strcpy( next, rulesFile );
    next += strlen( rulesFile );
  }
  *next++ = '\0';
  if( data->model )
  {
    strcpy( next, data->model );
    next += strlen( data->model );
  }
  *next++ = '\0';
  if( data->layouts )
  {
    strcpy( next, allLayouts );
    next += strlen( allLayouts );
  }
  *next++ = '\0';
  if( data->variants )
  {
    strcpy( next, allVariants );
    next += strlen( allVariants );
  }
  *next++ = '\0';
  if( data->options )
  {
    strcpy( next, allOptions );
    next += strlen( allOptions );
  }
  *next++ = '\0';
  if( ( next - pval ) != len )
  {
    XklDebug( 150, "Illegal final position: %d/%d\n", ( next - pval ), len );
    if( allOptions != NULL )
      free( allOptions );
    free( pval );
    _xklLastErrorMsg = "Internal property parsing error";
    return False;
  }

  rv = XChangeProperty( _xklDpy, _xklRootWindow, rulesAtom, XA_STRING, 8,
                        PropModeReplace, ( unsigned char * ) pval, len );
  XSync( _xklDpy, False );
#if 0
  for( i = len - 1; --i >= 0; )
    if( pval[i] == '\0' )
      pval[i] = '?';
  XklDebug( 150, "Stored [%s] of length %d to [%s] of %X: %d\n", pval, len,
            propName, _xklRootWindow, rv );
#endif
  if( allOptions != NULL )
    free( allOptions );
  free( pval );
  return True;
}
