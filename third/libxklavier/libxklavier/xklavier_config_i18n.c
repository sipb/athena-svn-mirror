#include <stdlib.h>
#include <string.h>
#include <iconv.h>

#include "config.h"

#ifdef HAVE_LANGINFO_CODESET
# include <langinfo.h>
#endif

#ifdef HAVE_SETLOCALE
# include <locale.h>
#endif

#include "xklavier_private.h"

#define MAX_LOCALE_LEN 128

static char localeSubStrings[3][MAX_LOCALE_LEN];

/*
 *  some bad guys create LC_ALL=LC_CTYPE=ru_RU.UTF-8;LC_NUMERIC=C;LC_TIME=ru_RU.UTF-8;LC_COLLATE=ru_RU.UTF-8;LC_MONETARY=ru_RU.UTF-8;LC_MESSAGES=ru_RU.UTF-8;LC_PAPER=ru_RU.UTF-8;LC_NAME=ru_RU.UTF-8;LC_ADDRESS=ru_RU.UTF-8;LC_TELEPHONE=ru_RU.UTF-8;LC_MEASUREMENT=ru_RU.UTF-8;LC_IDENTIFICATION=ru_RU.UTF-8
 */
static const char *_XklParseLC_ALL2LC_MESSAGES( const char *lcAll )
{
  const char *lcMsgPos = strstr( lcAll, "LC_MESSAGES=" );
  const char *lcMsgEnd;
  size_t len;
  static char buf[128];
  if( lcMsgPos == NULL )
    return lcAll;
  lcMsgPos += 12;
  lcMsgEnd = strchr( lcMsgPos, ';' );
  if( lcMsgEnd == NULL )        // LC_MESSAGES is the last piece of LC_ALL
  {
    return lcMsgPos;            //safe to return!
  }
  len = lcMsgEnd - lcMsgPos;
  if( len > sizeof( buf ) )
    len = sizeof( buf );
  strncpy( buf, lcMsgPos, len );
  buf[sizeof( buf ) - 1] = '\0';
  return buf;
}

// Taken from gnome-vfs
static Bool _XklGetCharset( const char **a )
{
  static const char *charset = NULL;

  if( charset == NULL )
  {
    charset = getenv( "CHARSET" );

    if( charset == NULL || charset[0] == '\0' )
    {
// taken from gnome-vfs
#ifdef HAVE_LANGINFO_CODESET
      charset = nl_langinfo( CODESET );
      if( charset == NULL || charset[0] == '\0' )
      {
#endif
#ifdef HAVE_SETLOCALE
        charset = setlocale( LC_CTYPE, NULL );
        if( charset == NULL || charset[0] == '\0' )
        {
#endif
          charset = getenv( "LC_ALL" );
          if( charset == NULL || charset[0] == '\0' )
          {
            charset = getenv( "LC_CTYPE" );
            if( charset == NULL || charset[0] == '\0' )
              charset = getenv( "LANG" );
          }
#ifdef HAVE_SETLOCALE
        } else
        {
          XklDebug( 150, "Using charset from setlocale: [%s]\n", charset );
        }
#endif
#ifdef HAVE_LANGINFO_CODESET
      } else
      {
        XklDebug( 150, "Using charset from nl_langinfo: [%s]\n", charset );
      }
#endif
    }
  }

  if( charset != NULL && *charset != '\0' )
  {
    *a = charset;
    return ( charset != NULL && strstr( charset, "UTF-8" ) != NULL );
  }
  /* Assume this for compatibility at present.  */
  *a = "US-ASCII";
  XklDebug( 150, "Using charset fallback: [%s]\n", *a );

  return False;
}

char *_XklLocaleFromUtf8( const char *utf8string )
{
  int len;
  int bytesRead;
  int bytesWritten;

  iconv_t converter;
  static char converted[XKL_MAX_CI_DESC_LENGTH];
  char *convertedStart = converted;
  char *utfStart = ( char * ) utf8string;
  int clen = XKL_MAX_CI_DESC_LENGTH - 1;
  const char *charset;

  static Bool alreadyWarned = False;

  if( utf8string == NULL )
    return NULL;

  len = strlen( utf8string );

  if( _XklGetCharset( &charset ) )
    return strdup( utf8string );

  converter = iconv_open( charset, "UTF-8" );
  if( converter == ( iconv_t ) - 1 )
  {
    if( !alreadyWarned )
    {
      alreadyWarned = True;
      XklDebug( 0,
                "Unable to convert MIME info from UTF-8 to the current locale %s. MIME info will probably display wrong.",
                charset );
    }
    return strdup( utf8string );
  }
  //converted = convert_with_iconv( utf8string,
  //                              len, converter, &bytesRead, &bytesWritten );

  if( iconv( converter, &utfStart, &len, &convertedStart, &clen ) == -1 )
  {
    XklDebug( 0,
              "Unable to convert %s from UTF-8 to %s, this string will probably display wrong.",
              utf8string, charset );
    return strdup( utf8string );
  }
  *convertedStart = '\0';

  iconv_close( converter );

  return converted;
}

/*
 * country[_LANG[.ENCODING]] - any other ideas?
 */
void _XklI18NInit(  )
{
  char *dotPos;
  char *underscorePos;
  const char *locale = NULL;
  char *curSubstring;

  localeSubStrings[0][0] = localeSubStrings[1][0] =
    localeSubStrings[2][0] = '\0';

#ifdef HAVE_SETLOCALE
  locale = setlocale( LC_MESSAGES, NULL );
#endif
  if( locale == NULL || locale[0] == '\0' )
  {
    locale = getenv( "LC_MESSAGES" );
    if( locale == NULL || locale[0] == '\0' )
    {
      locale = getenv( "LC_ALL" );
      if( locale == NULL || locale[0] == '\0' )
        locale = getenv( "LANG" );
      else
        locale = _XklParseLC_ALL2LC_MESSAGES( locale );
    }
  }

  if( locale == NULL )
  {
    XklDebug( 0, "Could not find locale - can be problems with i18n" );
    return;
  }

  strncpy( localeSubStrings[0], locale, MAX_LOCALE_LEN );

  curSubstring = localeSubStrings[1];

  dotPos = strchr( locale, '.' );
  if( dotPos != NULL )
  {
    int idx = dotPos - locale;
    if( idx >= MAX_LOCALE_LEN )
      idx = MAX_LOCALE_LEN - 1;
    strncpy( curSubstring, locale, idx );
    curSubstring[idx] = '\0';
    curSubstring += MAX_LOCALE_LEN;
  }

  underscorePos = strchr( locale, '_' );
  if( underscorePos != NULL && ( dotPos == NULL || dotPos > underscorePos ) )
  {
    int idx = underscorePos - locale;
    if( idx >= MAX_LOCALE_LEN )
      idx = MAX_LOCALE_LEN - 1;
    strncpy( curSubstring, locale, idx );
    curSubstring[idx] = '\0';
  }

  XklDebug( 150, "Locale search order:\n" );
  XklDebug( 150, " 0: %s\n", localeSubStrings[0] );     // full locale - highest priority
  XklDebug( 150, " 1: %s\n", localeSubStrings[1] );
  XklDebug( 150, " 2: %s\n", localeSubStrings[2] );
}

int _XklGetLanguagePriority( const char *lang )
{
  int i, priority = -1;

  for( i = sizeof( localeSubStrings ) / sizeof( localeSubStrings[0] );
       --i >= 0; )
  {
    if( localeSubStrings[0][0] == '\0' )
      continue;

    if( !strcmp( lang, localeSubStrings[i] ) )
    {
      priority = i;
      break;
    }
  }
  return priority;
}
