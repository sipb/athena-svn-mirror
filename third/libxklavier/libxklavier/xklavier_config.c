#include <errno.h>
#include <string.h>
#include <locale.h>
#include <sys/stat.h>

#include <libxml/xpath.h>

#include "config.h"

#include "xklavier_private.h"

typedef struct _XklConfigRegistry
{
  xmlDocPtr doc;
  xmlXPathContextPtr xpathContext;
}
XklConfigRegistry;

static XklConfigRegistry theRegistry;

static xmlXPathCompExprPtr modelsXPath;
static xmlXPathCompExprPtr layoutsXPath;
static xmlXPathCompExprPtr optionGroupsXPath;

#define _XklConfigRegistryIsInitialized() \
  ( theRegistry.xpathContext != NULL )

static xmlChar *_XklNodeGetXmlLangAttr( xmlNodePtr nptr )
{
  if( nptr->properties != NULL &&
      !strcmp( "lang", nptr->properties[0].name ) &&
      nptr->properties[0].ns != NULL &&
      !strcmp( "xml", nptr->properties[0].ns->prefix ) &&
      nptr->properties[0].children != NULL )
    return nptr->properties[0].children->content;
  else
    return NULL;
}

static Bool _XklReadConfigItem( xmlNodePtr iptr, XklConfigItemPtr pci )
{
  xmlNodePtr nameElement, descElement = NULL, ntDescElement =
    NULL, nptr, ptr, shortDescElement = NULL, ntShortDescElement = NULL;
  int maxDescPriority = -1;
  int maxShortDescPriority = -1;

  *pci->name = 0;
  *pci->shortDescription = 0;
  *pci->description = 0;
  if( iptr->type != XML_ELEMENT_NODE )
    return False;
  ptr = iptr->children;
  while( ptr != NULL )
  {
    switch ( ptr->type )
    {
      case XML_ELEMENT_NODE:
        if( !strcmp( ptr->name, "configItem" ) )
          break;
        return False;
      case XML_TEXT_NODE:
        ptr = ptr->next;
        continue;
      default:
        return False;
    }
    break;
  }
  if( ptr == NULL )
    return False;

  nptr = ptr->children;

  if( nptr->type == XML_TEXT_NODE )
    nptr = nptr->next;
  nameElement = nptr;
  nptr = nptr->next;

  while( nptr != NULL )
  {
    if( nptr->type != XML_TEXT_NODE )
    {
      xmlChar *lang = _XklNodeGetXmlLangAttr( nptr );

      if( lang != NULL )
      {
        int priority = _XklGetLanguagePriority( lang );
        if( !strcmp( nptr->name, "description" ) && ( priority > maxDescPriority ) )    // higher priority
        {
          descElement = nptr;
          maxDescPriority = priority;
        } else if( !strcmp( nptr->name, "shortDescription" ) && ( priority > maxShortDescPriority ) )   // higher priority
        {
          shortDescElement = nptr;
          maxShortDescPriority = priority;
        }
      } else
      {
        if( !strcmp( nptr->name, "description" ) )
          ntDescElement = nptr;
        else if( !strcmp( nptr->name, "shortDescription" ) )
          ntShortDescElement = nptr;
      }
    }
    nptr = nptr->next;
  }

  // if no language-specific description found - use the ones without lang
  if( descElement == NULL )
    descElement = ntDescElement;

  if( shortDescElement == NULL )
    shortDescElement = ntShortDescElement;

  //
  // Actually, here we should have some code to find the correct localized description...
  // 

  if( nameElement != NULL && nameElement->children != NULL )
    strncat( pci->name, nameElement->children->content,
             XKL_MAX_CI_NAME_LENGTH - 1 );

  if( shortDescElement != NULL && shortDescElement->children != NULL )
    strncat( pci->shortDescription,
             _XklLocaleFromUtf8( shortDescElement->children->content ),
             XKL_MAX_CI_SHORT_DESC_LENGTH - 1 );

  if( descElement != NULL && descElement->children != NULL )
    strncat( pci->description,
             _XklLocaleFromUtf8( descElement->children->content ),
             XKL_MAX_CI_DESC_LENGTH - 1 );
  return True;
}

static void _XklConfigEnumFromNodeSet( xmlNodeSetPtr nodes,
                                       ConfigItemProcessFunc func,
                                       void *userData )
{
  int i;
  if( nodes != NULL )
  {
    xmlNodePtr *theNodePtr = nodes->nodeTab;
    for( i = nodes->nodeNr; --i >= 0; )
    {
      XklConfigItem ci;
      if( _XklReadConfigItem( *theNodePtr, &ci ) )
        func( &ci, userData );

      theNodePtr++;
    }
  }
}

static void _XklConfigEnumSimple( xmlXPathCompExprPtr xpathCompExpr,
                                  ConfigItemProcessFunc func, void *userData )
{
  xmlXPathObjectPtr xpathObj;

  if( !_XklConfigRegistryIsInitialized(  ) )
    return;
  xpathObj = xmlXPathCompiledEval( xpathCompExpr, theRegistry.xpathContext );
  if( xpathObj != NULL )
  {
    _XklConfigEnumFromNodeSet( xpathObj->nodesetval, func, userData );
    xmlXPathFreeObject( xpathObj );
  }
}

static void _XklConfigEnumDirect( const char *format,
                                  const char *value,
                                  ConfigItemProcessFunc func, void *userData )
{
  char xpathExpr[1024];
  xmlXPathObjectPtr xpathObj;

  if( !_XklConfigRegistryIsInitialized(  ) )
    return;
  snprintf( xpathExpr, sizeof xpathExpr, format, value );
  xpathObj = xmlXPathEval( xpathExpr, theRegistry.xpathContext );
  if( xpathObj != NULL )
  {
    _XklConfigEnumFromNodeSet( xpathObj->nodesetval, func, userData );
    xmlXPathFreeObject( xpathObj );
  }
}

static Bool _XklConfigFindObject( const char *format,
                                  const char *arg1,
                                  XklConfigItemPtr ptr /* in/out */ ,
                                  xmlNodePtr * nodePtr /* out */  )
{
  xmlXPathObjectPtr xpathObj;
  xmlNodeSetPtr nodes;
  Bool rv = False;
  char xpathExpr[1024];

  if( !_XklConfigRegistryIsInitialized(  ) )
    return False;

  snprintf( xpathExpr, sizeof xpathExpr, format, arg1, ptr->name );
  xpathObj = xmlXPathEval( xpathExpr, theRegistry.xpathContext );
  if( xpathObj == NULL )
    return False;

  nodes = xpathObj->nodesetval;
  if( nodes != NULL && nodes->nodeTab != NULL )
  {
    rv = _XklReadConfigItem( *nodes->nodeTab, ptr );
    if( nodePtr != NULL )
    {
      *nodePtr = *nodes->nodeTab;
    }
  }

  xmlXPathFreeObject( xpathObj );
  return rv;
}

char *_XklConfigRecMergeLayouts( const XklConfigRecPtr data )
{
  return _XklConfigRecMergeByComma( ( const char ** ) data->layouts,
                                    data->numLayouts );
}

char *_XklConfigRecMergeVariants( const XklConfigRecPtr data )
{
  return _XklConfigRecMergeByComma( ( const char ** ) data->variants,
                                    data->numVariants );
}

char *_XklConfigRecMergeOptions( const XklConfigRecPtr data )
{
  return _XklConfigRecMergeByComma( ( const char ** ) data->options,
                                    data->numOptions );
}

char *_XklConfigRecMergeByComma( const char **array, const int arrayLength )
{
  int len = 0;
  int i;
  char *merged;
  const char **theString;

  if( ( theString = array ) == NULL )
    return NULL;

  for( i = arrayLength; --i >= 0; theString++ )
  {
    if( *theString != NULL )
      len += strlen( *theString );
    len++;
  }

  if( len < 1 )
    return NULL;

  merged = ( char * ) malloc( len );
  merged[0] = '\0';

  theString = array;
  for( i = arrayLength; --i >= 0; theString++ )
  {
    if( *theString != NULL )
      strcat( merged, *theString );
    if( i != 0 )
      strcat( merged, "," );
  }
  return merged;
}

void _XklConfigRecSplitLayouts( XklConfigRecPtr data, const char *merged )
{
  _XklConfigRecSplitByComma( &data->layouts, &data->numLayouts, merged );
}

void _XklConfigRecSplitVariants( XklConfigRecPtr data, const char *merged )
{
  _XklConfigRecSplitByComma( &data->variants, &data->numVariants, merged );
}

void _XklConfigRecSplitOptions( XklConfigRecPtr data, const char *merged )
{
  _XklConfigRecSplitByComma( &data->options, &data->numOptions, merged );
}

void _XklConfigRecSplitByComma( char ***array,
                                int *arraySize, const char *merged )
{
  const char *pc = merged;
  char **ppc, *npc;
  *arraySize = 0;
  *array = NULL;

  if( merged == NULL || merged[0] == '\0' )
    return;

  // first count the elements 
  while( ( npc = strchr( pc, ',' ) ) != NULL )
  {
    ( *arraySize )++;
    pc = npc + 1;
  }
  ( *arraySize )++;

  if( ( *arraySize ) != 0 )
  {
    int len;
    *array = ( char ** ) malloc( ( sizeof( char * ) ) * ( *arraySize ) );

    ppc = *array;
    pc = merged;
    while( ( npc = strchr( pc, ',' ) ) != NULL )
    {
      int len = npc - pc;
      //*ppc = ( char * ) strndup( pc, len );
      *ppc = ( char * ) malloc( len + 1 );
      if ( *ppc != NULL )
      {
        strncpy( *ppc, pc, len );
        (*ppc)[len] = '\0';
      }

      ppc++;
      pc = npc + 1;
    }

    //len = npc - pc;
    len = strlen( pc );
    //*ppc = ( char * ) strndup( pc, len );
    *ppc = ( char * ) malloc( len + 1 );
    if ( *ppc != NULL )
      strcpy( *ppc, pc );
  }
}

void XklConfigInit( void )
{
  xmlXPathInit(  );
  modelsXPath = xmlXPathCompile( "/xkbConfigRegistry/modelList/model" );
  layoutsXPath = xmlXPathCompile( "/xkbConfigRegistry/layoutList/layout" );
  optionGroupsXPath =
    xmlXPathCompile( "/xkbConfigRegistry/optionList/group" );
  _XklI18NInit(  );
}

void XklConfigTerm( void )
{
  if( modelsXPath != NULL )
  {
    xmlXPathFreeCompExpr( modelsXPath );
    modelsXPath = NULL;
  }
  if( layoutsXPath != NULL )
  {
    xmlXPathFreeCompExpr( layoutsXPath );
    layoutsXPath = NULL;
  }
  if( optionGroupsXPath != NULL )
  {
    xmlXPathFreeCompExpr( optionGroupsXPath );
    optionGroupsXPath = NULL;
  }
}

Bool XklConfigLoadRegistryFromFile( const char * fileName )
{
  theRegistry.doc = xmlParseFile( fileName );
  if( theRegistry.doc == NULL )
  {
    theRegistry.xpathContext = NULL;
    _xklLastErrorMsg = "Could not parse XKB configuration registry";
  } else
    theRegistry.xpathContext = xmlXPathNewContext( theRegistry.doc );
  return _XklConfigRegistryIsInitialized(  );
}

void XklConfigFreeRegistry( void )
{
  if( _XklConfigRegistryIsInitialized(  ) )
  {
    xmlXPathFreeContext( theRegistry.xpathContext );
    xmlFreeDoc( theRegistry.doc );
    theRegistry.xpathContext = NULL;
    theRegistry.doc = NULL;
  }
}

void XklConfigEnumModels( ConfigItemProcessFunc func, void *userData )
{
  _XklConfigEnumSimple( modelsXPath, func, userData );
}

void XklConfigEnumLayouts( ConfigItemProcessFunc func, void *userData )
{
  _XklConfigEnumSimple( layoutsXPath, func, userData );
}

void XklConfigEnumLayoutVariants( const char *layoutName,
                                  ConfigItemProcessFunc func, void *userData )
{
  _XklConfigEnumDirect
    ( "/xkbConfigRegistry/layoutList/layout/variantList/variant[../../configItem/name = '%s']",
      layoutName, func, userData );
}

void XklConfigEnumOptionGroups( GroupProcessFunc func, void *userData )
{
  xmlXPathObjectPtr xpathObj;
  int i;

  if( !_XklConfigRegistryIsInitialized(  ) )
    return;
  xpathObj =
    xmlXPathCompiledEval( optionGroupsXPath, theRegistry.xpathContext );
  if( xpathObj != NULL )
  {
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    xmlNodePtr *theNodePtr = nodes->nodeTab;
    for( i = nodes->nodeNr; --i >= 0; )
    {
      XklConfigItem ci;

      if( _XklReadConfigItem( *theNodePtr, &ci ) )
      {
        Bool allowMC = True;
        xmlChar *allowMCS =
          xmlGetProp( *theNodePtr, "allowMultipleSelection" );
        if( allowMCS != NULL )
        {
          allowMC = strcmp( "false", allowMCS );
          xmlFree( allowMCS );
        }

        func( &ci, allowMC, userData );
      }

      theNodePtr++;
    }
    xmlXPathFreeObject( xpathObj );
  }
}

void XklConfigEnumOptions( const char *optionGroupName,
                           ConfigItemProcessFunc func, void *userData )
{
  _XklConfigEnumDirect
    ( "/xkbConfigRegistry/optionList/group/option[../configItem/name = '%s']",
      optionGroupName, func, userData );
}

Bool XklConfigFindModel( XklConfigItemPtr ptr /* in/out */  )
{
  return
    _XklConfigFindObject
    ( "/xkbConfigRegistry/modelList/model[configItem/name = '%s%s']", "",
      ptr, NULL );
}

Bool XklConfigFindLayout( XklConfigItemPtr ptr /* in/out */  )
{
  return
    _XklConfigFindObject
    ( "/xkbConfigRegistry/layoutList/layout[configItem/name = '%s%s']", "",
      ptr, NULL );
}

Bool XklConfigFindVariant( const char *layoutName,
                           XklConfigItemPtr ptr /* in/out */  )
{
  return
    _XklConfigFindObject
    ( "/xkbConfigRegistry/layoutList/layout/variantList/variant"
      "[../../configItem/name = '%s' and configItem/name = '%s']",
      layoutName, ptr, NULL );
}

Bool XklConfigFindOptionGroup( XklConfigItemPtr ptr /* in/out */ ,
                               Bool * allowMultipleSelection /* out */  )
{
  xmlNodePtr node;
  Bool rv =
    _XklConfigFindObject
    ( "/xkbConfigRegistry/optionList/group[configItem/name = '%s%s']", "",
      ptr, &node );

  if( rv && allowMultipleSelection != NULL )
  {
    xmlChar *val = xmlGetProp( node, "allowMultipleSelection" );
    *allowMultipleSelection = False;
    if( val != NULL )
    {
      *allowMultipleSelection = !strcmp( val, "true" );
      xmlFree( val );
    }
  }
  return rv;
}

Bool XklConfigFindOption( const char *optionGroupName,
                          XklConfigItemPtr ptr /* in/out */  )
{
  return
    _XklConfigFindObject
    ( "/xkbConfigRegistry/optionList/group/option"
      "[../configItem/name = '%s' and configItem/name = '%s']",
      optionGroupName, ptr, NULL );
}
