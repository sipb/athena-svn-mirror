/* gutf8.c - Operations on UTF-8 strings.
 *
 * Copyright (C) 1999 Tom Tromey
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <stdlib.h>
#ifdef HAVE_CODESET
#include <langinfo.h>
#endif
#include <string.h>

#include "gunicode.h"

#ifdef G_PLATFORM_WIN32
#include <stdio.h>
#define STRICT
#include <windows.h>
#undef STRICT
#endif

#include <libgnome/gnome-i18n.h>

/*
  Paranoid checking macros for utf8 strings.

  A number of functions do not check their inputs, and can segfault or
  hang if they are passed a string that isn't proper utf-8.  These
  macros add checks to all those functions, which give us some safety
  (or at least an intelligible warning message) in exchange for the
  loss of efficiency.
*/

#undef UTF8_CHECKS_ARE_FATAL
#define UTF8_CHECK_STRINGS

#ifdef UTF8_CHECKS_ARE_FATAL
#define utf8_error_msg(x) g_error(x)
#else
#define utf8_error_msg(x) g_warning(x)
#endif

#ifdef UTF8_CHECK_STRINGS
#define utf8_check(s,len) {if (s&&!g_utf8_validate(s,len,NULL)) utf8_error_msg ("processing invalid utf-8 string");}
#else
#define utf8_check(s,len)
#endif



#define UTF8_COMPUTE(Char, Mask, Len)					      \
  if (Char < 128)							      \
    {									      \
      Len = 1;								      \
      Mask = 0x7f;							      \
    }									      \
  else if ((Char & 0xe0) == 0xc0)					      \
    {									      \
      Len = 2;								      \
      Mask = 0x1f;							      \
    }									      \
  else if ((Char & 0xf0) == 0xe0)					      \
    {									      \
      Len = 3;								      \
      Mask = 0x0f;							      \
    }									      \
  else if ((Char & 0xf8) == 0xf0)					      \
    {									      \
      Len = 4;								      \
      Mask = 0x07;							      \
    }									      \
  else if ((Char & 0xfc) == 0xf8)					      \
    {									      \
      Len = 5;								      \
      Mask = 0x03;							      \
    }									      \
  else if ((Char & 0xfe) == 0xfc)					      \
    {									      \
      Len = 6;								      \
      Mask = 0x01;							      \
    }									      \
  else									      \
    Len = -1;

#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))
   

#define UTF8_GET(Result, Chars, Count, Mask, Len)			      \
  (Result) = (Chars)[0] & (Mask);					      \
  for ((Count) = 1; (Count) < (Len); ++(Count))				      \
    {									      \
      if (((Chars)[(Count)] & 0xc0) != 0x80)				      \
	{								      \
	  (Result) = -1;						      \
	  break;							      \
	}								      \
      (Result) <<= 6;							      \
      (Result) |= ((Chars)[(Count)] & 0x3f);				      \
    }

#define UNICODE_VALID(Char)                   \
    ((Char) < 0x110000 &&                     \
     ((Char) < 0xD800 || (Char) >= 0xE000) && \
     (Char) != 0xFFFE && (Char) != 0xFFFF)
   
     
gchar g_utf8_skip[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0
};

/**
 * g_utf8_find_prev_char:
 * @str: pointer to the beginning of a UTF-8 string
 * @p: pointer to some position within @str
 * 
 * Given a position @p with a UTF-8 encoded string @str, find the start
 * of the previous UTF-8 character starting before @p. Returns %NULL if no
 * UTF-8 characters are present in @p before @str.
 *
 * @p does not have to be at the beginning of a UTF-8 chracter. No check
 * is made to see if the character found is actually valid other than
 * it starts with an appropriate byte.
 *
 * Return value: a pointer to the found character or %NULL.
 **/
gchar *
g_utf8_find_prev_char (const char *str,
		       const char *p)
{
  utf8_check (str, -1);

  for (--p; p >= str; --p)
    {
      if ((*p & 0xc0) != 0x80)
	return (gchar *)p;
    }
  return NULL;
}

/**
 * g_utf8_find_next_char:
 * @p: a pointer to a position within a UTF-8 encoded string
 * @end: a pointer to the end of the string, or %NULL to indicate
 *        that the string is NULL terminated, in which case
 *        the returned value will be 
 *
 * Find the start of the next utf-8 character in the string after @p
 *
 * @p does not have to be at the beginning of a UTF-8 chracter. No check
 * is made to see if the character found is actually valid other than
 * it starts with an appropriate byte.
 * 
 * Return value: a pointer to the found character or %NULL
 **/
gchar *
g_utf8_find_next_char (const gchar *p,
		       const gchar *end)
{
  if (*p)
    {
      if (end)
	for (++p; p < end && (*p & 0xc0) == 0x80; ++p)
	  ;
      else
	for (++p; (*p & 0xc0) == 0x80; ++p)
	  ;
    }
  return (p == end) ? NULL : (gchar *)p;
}

/**
 * g_utf8_prev_char:
 * @p: a pointer to a position within a UTF-8 encoded string
 *
 * Find the previous UTF-8 character in the string before @p
 *
 * @p does not have to be at the beginning of a UTF-8 character. No check
 * is made to see if the character found is actually valid other than
 * it starts with an appropriate byte. If @p might be the first
 * character of the string, you must use g_utf8_find_prev_char instead.
 * 
 * Return value: a pointer to the found character.
 **/
gchar *
g_utf8_prev_char (const gchar *p)
{
  while (TRUE)
    {
      p--;
      if ((*p & 0xc0) != 0x80)
	return (gchar *)p;
    }
}

/**
 * g_utf8_strlen:
 * @p: pointer to the start of a UTF-8 string.
 * @max: the maximum number of bytes to examine. If @max
 *       is less than 0, then the string is assumed to be
 *       nul-terminated.
 * 
 * Return value: the length of the string in characters
 **/
gint
g_utf8_strlen (const gchar *p, gint max)
{
  int len = 0;
  const gchar *start = p;

  utf8_check (p, max);

  if (max < 0)
    {
      while (*p)
        {
          p = g_utf8_next_char (p);
          ++len;
        }
    }
  else
    {
      if (max == 0 || !*p)
        return 0;
      
      p = g_utf8_next_char (p);          

      while (p - start < max && *p)
        {
          ++len;
          p = g_utf8_next_char (p);          
        }

      /* only do the last len increment if we got a complete
       * char (don't count partial chars)
       */
      if (p - start == max)
        ++len;
    }

  return len;
}

/**
 * g_utf8_get_char:
 * @p: a pointer to unicode character encoded as UTF-8
 * 
 * Convert a sequence of bytes encoded as UTF-8 to a unicode character.
 * 
 * Return value: the resulting character or (gunichar)-1 if @p does
 *               not point to a valid UTF-8 encoded unicode character
 **/
gunichar
g_utf8_get_char (const gchar *p)
{
  int i, mask = 0, len;
  gunichar result;
  unsigned char c = (unsigned char) *p;

  UTF8_COMPUTE (c, mask, len);
  if (len == -1)
    return (gunichar)-1;
  UTF8_GET (result, p, i, mask, len);

  return result;
}

/**
 * g_utf8_offset_to_pointer:
 * @str: a UTF-8 encoded string
 * @offset: a character offset within the string.
 * 
 * Converts from an integer character offset to a pointer to a position
 * within the string.
 * 
 * Return value: the resulting pointer
 **/
gchar *
g_utf8_offset_to_pointer  (const gchar *str,
			   gint         offset)
{
  const gchar *s = str;

  utf8_check (str, -1);

  while (offset--)
    s = g_utf8_next_char (s);
  
  return (gchar *)s;
}

/**
 * g_utf8_pointer_to_offset:
 * @str: a UTF-8 encoded string
 * @pos: a pointer to a position within @str
 * 
 * Converts from a pointer to position within a string to a integer
 * character offset
 * 
 * Return value: the resulting character offset
 **/
gint
g_utf8_pointer_to_offset (const gchar *str,
			  const gchar *pos)
{
  const gchar *s = str;
  gint offset = 0;

  utf8_check (str, -1);
  
  while (s < pos)
    {
      s = g_utf8_next_char (s);
      offset++;
    }

  return offset;
}


/**
 * g_utf8_strncpy:
 * @dest: buffer to fill with characters from @src
 * @src: UTF-8 string
 * @n: character count
 * 
 * Like the standard C strncpy() function, but copies a given number
 * of characters instead of a given number of bytes. The @src string
 * must be valid UTF-8 encoded text. (Use g_utf8_validate() on all
 * text before trying to use UTF-8 utility functions with it.)
 * 
 * Return value: @dest
 **/
gchar *
g_utf8_strncpy (gchar *dest, const gchar *src, size_t n)
{
  const gchar *s = src;

  utf8_check (src, -1);

  while (n && *s)
    {
      s = g_utf8_next_char(s);
      n--;
    }
  strncpy(dest, src, s - src);
  dest[s - src] = 0;
  return dest;
}

static gboolean
g_utf8_get_charset_internal (char **a)
{
  char *charset = getenv("CHARSET");

  if (charset && a && ! *a)
    *a = charset;

  if (charset && strstr (charset, "UTF-8"))
      return TRUE;

#ifdef HAVE_CODESET
  charset = nl_langinfo(CODESET);
  if (charset)
    {
      if (a && ! *a)
	*a = charset;
      if (strcmp (charset, "UTF-8") == 0)
	return TRUE;
    }
#endif
  
#if 0 /* #ifdef _NL_CTYPE_CODESET_NAME */
  charset = nl_langinfo (_NL_CTYPE_CODESET_NAME);
  if (charset)
    {
      if (a && ! *a)
	*a = charset;
      if (strcmp (charset, "UTF-8") == 0)
	return TRUE;
    }
#endif

#ifdef G_PLATFORM_WIN32
  if (a && ! *a)
    {
      static char codepage[10];
      
      sprintf (codepage, "CP%d", GetACP ());
      *a = codepage;
      /* What about codepage 1200? Is that UTF-8? */
      return FALSE;
    }
#else
  if (a && ! *a) 
    *a = "US-ASCII";
#endif

  /* Assume this for compatibility at present.  */
  return FALSE;
}

static int utf8_locale_cache = -1;
static char *utf8_charset_cache = NULL;

/**
 * g_get_charset:
 * @charset: return location for character set name
 * 
 * Obtains the character set for the current locale; you might use
 * this character set as an argument to g_convert(), to convert from
 * the current locale's encoding to some other encoding. (Frequently
 * g_locale_to_utf8() and g_locale_from_utf8() are nice shortcuts,
 * though.)
 *
 * The return value is %TRUE if the locale's encoding is UTF-8, in that
 * case you can perhaps avoid calling g_convert().
 *
 * The string returned in @charset is not allocated, and should not be
 * freed.
 * 
 * Return value: %TRUE if the returned charset is UTF-8
 **/
gboolean
g_get_charset (char **charset) 
{
  if (utf8_locale_cache != -1)
    {
      if (charset)
	*charset = utf8_charset_cache;
      return utf8_locale_cache;
    }
  utf8_locale_cache = g_utf8_get_charset_internal (&utf8_charset_cache);
  if (charset) 
    *charset = utf8_charset_cache;
  return utf8_locale_cache;
}

/* unicode_strchr */

/**
 * g_unichar_to_utf8:
 * @c: a ISO10646 character code
 * @outbuf: output buffer, must have at least 6 bytes of space.
 *       If %NULL, the length will be computed and returned
 *       and nothing will be written to @out.
 * 
 * Convert a single character to utf8
 * 
 * Return value: number of bytes written
 **/
int
g_unichar_to_utf8 (gunichar c, gchar *outbuf)
{
  size_t len = 0;
  int first;
  int i;

  if (c < 0x80)
    {
      first = 0;
      len = 1;
    }
  else if (c < 0x800)
    {
      first = 0xc0;
      len = 2;
    }
  else if (c < 0x10000)
    {
      first = 0xe0;
      len = 3;
    }
   else if (c < 0x200000)
    {
      first = 0xf0;
      len = 4;
    }
  else if (c < 0x4000000)
    {
      first = 0xf8;
      len = 5;
    }
  else
    {
      first = 0xfc;
      len = 6;
    }

  if (outbuf)
    {
      for (i = len - 1; i > 0; --i)
	{
	  outbuf[i] = (c & 0x3f) | 0x80;
	  c >>= 6;
	}
      outbuf[0] = c | first;
    }

  return len;
}

/**
 * g_utf8_strchr:
 * @p: a nul-terminated utf-8 string
 * @c: a iso-10646 character/
 * 
 * Find the leftmost occurence of the given iso-10646 character
 * in a UTF-8 string.
 * 
 * Return value: NULL if the string does not contain the character, otherwise, a
 *               a pointer to the start of the leftmost of the character in the string.
 **/
gchar *
g_utf8_strchr (const char *p, gunichar c)
{
  gchar ch[10];

  gint len = g_unichar_to_utf8 (c, ch);
  ch[len] = '\0';

  utf8_check (p, -1);
  
  return strstr(p, ch);
}

#if 0
/**
 * g_utf8_strrchr:
 * @p: a nul-terminated utf-8 string
 * @c: a iso-10646 character/
 * 
 * Find the rightmost occurence of the given iso-10646 character
 * in a UTF-8 string.
 * 
 * Return value: NULL if the string does not contain the character, otherwise, a
 *               a pointer to the start of the rightmost of the character in the string.
 **/

/* This is ifdefed out atm as there is no strrstr function in libc.
 */
gchar *
unicode_strrchr (const char *p, gunichar c)
{
  gchar ch[10];

  utf8_check (p, -1);

  len = g_unichar_to_utf8 (c, ch);
  ch[len] = '\0';
  
  return strrstr(p, ch);
}
#endif


/* UTF-8 strcasecmp - not optimized */

gint
g_utf8_strcasecmp (const gchar *s1,
		   const gchar *s2)
{
	gunichar c1, c2;

	g_return_val_if_fail (s1 != NULL && g_utf8_validate (s1, -1, NULL), 0);
	g_return_val_if_fail (s2 != NULL && g_utf8_validate (s2, -1, NULL), 0);

	while (*s1 && *s2)
		{

			c1 = g_unichar_tolower (g_utf8_get_char (s1));
			c2 = g_unichar_tolower (g_utf8_get_char (s2));

			/* Collation is locale-dependent, so this totally fails to do the right thing. */
			if (c1 != c2)
				return c1 < c2 ? -1 : 1;

			s1 = g_utf8_next_char (s1);
			s2 = g_utf8_next_char (s2);
		}

	if (*s1 == '\0' && *s2 == '\0')
		return 0;

	return *s1 ? 1 : -1;
}


/* UTF-8 strncasecmp - not optimized */

gint
g_utf8_strncasecmp (const gchar *s1,
		    const gchar *s2,
		    guint n)
{
	gunichar c1, c2;

	g_return_val_if_fail (s1 != NULL && g_utf8_validate (s1, -1, NULL), 0);
	g_return_val_if_fail (s2 != NULL && g_utf8_validate (s2, -1, NULL), 0);

	while (n && *s1 && *s2)
		{

			n -= 1;

			c1 = g_unichar_tolower (g_utf8_get_char (s1));
			c2 = g_unichar_tolower (g_utf8_get_char (s2));

			/* Collation is locale-dependent, so this totally fails to do the right thing. */
			if (c1 != c2)
				return c1 < c2 ? -1 : 1;

			s1 = g_utf8_next_char (s1);
			s2 = g_utf8_next_char (s2);
		}

	if (n == 0 || (*s1 == '\0' && *s2 == '\0'))
		return 0;

	return *s1 ? 1 : -1;
}


/* UTF-8 strdown */

gchar *
g_utf8_strdown (gchar *string)
{
  gchar *s, *p, *src;
  gchar buf[6];
  gunichar c, ctrans;
  gint sz, sz2;
  
  g_return_val_if_fail (string != NULL && g_utf8_validate (string, -1, NULL), NULL);
  
  s = string;
  p = string;
  while (*s)
    {
      c = g_utf8_get_char (s);
      ctrans = g_unichar_tolower (c);
      
      if (! g_unichar_isdefined (ctrans))
	ctrans = c;

      sz = g_unichar_to_utf8 (ctrans, buf);
      src = buf;

      /* Failsafe: Use the original character if the transformed
	 version's UTF-8 representation is larger than the original's.
	 This should never happen. */
      if (c != ctrans && (sz2 = g_unichar_to_utf8 (c, NULL)) < sz) {
	      src = s;
	      sz = sz2;
      }

      memcpy (p, src, sz);
      p += sz;

      s = g_utf8_next_char (s);
    }
  
  *p = '\0';
  
  return string;
}

/* UTF-8 strdup */

gchar *
g_utf8_strup (gchar *string)
{
  gchar *s, *p, *src;
  gchar buf[6];
  gunichar c, ctrans;
  gint sz, sz2;
  
  g_return_val_if_fail (string != NULL && g_utf8_validate (string, -1, NULL), NULL);
  
  s = string;
  p = string;
  while (*s)
    {
      c = g_utf8_get_char (s);
      ctrans = g_unichar_toupper (c);
      
      if (! g_unichar_isdefined (ctrans))
	ctrans = c;

      sz = g_unichar_to_utf8 (ctrans, buf);
      src = buf;

      /* Failsafe: Use the original character if the transformed
	 version's UTF-8 representation is larger than the original's.
	 This should never happen. */
      if (c != ctrans && (sz2 = g_unichar_to_utf8 (c, NULL)) < sz) {
	      src = s;
	      sz = sz2;
      }

      memcpy (p, src, sz);
      p += sz;

      s = g_utf8_next_char (s);
    }
  
  *p = '\0';
  
  return string;
}

gchar *
g_utf8_strtitle (gchar *string)
{
  gchar *s, *p, *src;
  gchar buf[6];
  gunichar c, ctrans;
  gint sz, sz2;
  
  g_return_val_if_fail (string != NULL && g_utf8_validate (string, -1, NULL), NULL);
  
  s = string;
  p = string;
  while (*s)
    {
      c = g_utf8_get_char (s);
      ctrans = g_unichar_totitle (c);
      
      if (! g_unichar_isdefined (ctrans))
	ctrans = c;

      sz = g_unichar_to_utf8 (ctrans, buf);
      src = buf;

      /* Failsafe: Use the original character if the transformed
	 version's UTF-8 representation is larger than the original's.
	 This should never happen. */
      if (c != ctrans && (sz2 = g_unichar_to_utf8 (c, NULL)) < sz) {
	      src = s;
	      sz = sz2;
      }

      memcpy (p, src, sz);
      p += sz;

      s = g_utf8_next_char (s);
    }
  
  *p = '\0';
  
  return string;
}


/* Like g_utf8_get_char, but take a maximum length
 * and return (gunichar)-2 on incomplete trailing character
 */
static inline gunichar
g_utf8_get_char_extended (const gchar *p, int max_len)
{
  gint i, len;
  gunichar wc = (guchar) *p;

  if (wc < 0x80)
    {
      return wc;
    }
  else if (wc < 0xc0)
    {
      return (gunichar)-1;
    }
  else if (wc < 0xe0)
    {
      len = 2;
      wc &= 0x1f;
    }
  else if (wc < 0xf0)
    {
      len = 3;
      wc &= 0x0f;
    }
  else if (wc < 0xf8)
    {
      len = 4;
      wc &= 0x07;
    }
  else if (wc < 0xfc)
    {
      len = 5;
      wc &= 0x03;
    }
  else if (wc < 0xfe)
    {
      len = 6;
      wc &= 0x01;
    }
  else
    {
      return (gunichar)-1;
    }
  
  if (len == -1)
    return (gunichar)-1;
  if (max_len >= 0 && len > max_len)
    {
      for (i = 1; i < max_len; i++)
	{
	  if ((((guchar *)p)[i] & 0xc0) != 0x80)
	    return (gunichar)-1;
	}
      return (gunichar)-2;
    }

  for (i = 1; i < len; ++i)
    {
      gunichar ch = ((guchar *)p)[i];
      
      if ((ch & 0xc0) != 0x80)
	{
	  if (ch)
	    return (gunichar)-1;
	  else
	    return (gunichar)-2;
	}

      wc <<= 6;
      wc |= (ch & 0x3f);
    }

  if (UTF8_LENGTH(wc) != len)
    return (gunichar)-1;
  
  return wc;
}

/**
 * g_utf8_get_char_validated:
 * @p: a pointer to Unicode character encoded as UTF-8
 * @max_len: the maximum number of bytes to read, or -1, for no maximum.
 * 
 * Convert a sequence of bytes encoded as UTF-8 to a Unicode character.
 * This function checks for incomplete characters, for invalid characters
 * such as characters that are out of the range of Unicode, and for
 * overlong encodings of valid characters.
 * 
 * Return value: the resulting character. If @p points to a partial
 *    sequence at the end of a string that could begin a valid character,
 *    returns (gunichar)-2; otherwise, if @p does not point to a valid
 *    UTF-8 encoded Unicode character, returns (gunichar)-1.
 **/
gunichar
g_utf8_get_char_validated (const  gchar *p,
			   gssize max_len)
{
  gunichar result = g_utf8_get_char_extended (p, max_len);

  if (result & 0x80000000)
    return result;
  else if (!UNICODE_VALID (result))
    return (gunichar)-1;
  else
    return result;
}

/**
 * g_utf8_validate:
 * @str: a pointer to character data
 * @max_len: max bytes to validate, or -1 to go until nul
 * @end: return location for end of valid data
 * 
 * Validates UTF-8 encoded text. @str is the text to validate;
 * if @str is nul-terminated, then @max_len can be -1, otherwise
 * @max_len should be the number of bytes to validate.
 * If @end is non-NULL, then the end of the valid range
 * will be stored there (i.e. the address of the first invalid byte
 * if some bytes were invalid, or the end of the text being validated
 * otherwise).
 *
 * Returns TRUE if all of @str was valid. Many GLib and GTK+
 * routines <emphasis>require</emphasis> valid UTF8 as input;
 * so data read from a file or the network should be checked
 * with g_utf8_validate() before doing anything else with it.
 * 
 * Return value: TRUE if the text was valid UTF-8.
 **/
gboolean
g_utf8_validate (const gchar  *str,
                 gssize       max_len,
                 const gchar **end)
{

  const gchar *p;

  g_return_val_if_fail (str != NULL, FALSE);
  
  if (end)
    *end = str;
  
  p = str;
  
  while ((max_len < 0 || (p - str) < max_len) && *p)
    {
      int i, mask = 0, len;
      gunichar result;
      unsigned char c = (unsigned char) *p;
      
      UTF8_COMPUTE (c, mask, len);

      if (len == -1)
        break;

      /* check that the expected number of bytes exists in str */
      if (max_len >= 0 &&
          ((max_len - (p - str)) < len))
        break;
        
      UTF8_GET (result, p, i, mask, len);

      if (UTF8_LENGTH (result) != len) /* Check for overlong UTF-8 */
	break;

      if (result == (gunichar)-1)
        break;

      if (!UNICODE_VALID (result))
	break;
      
      p += len;
    }

  if (end)
    *end = p;

  /* See that we covered the entire length if a length was
   * passed in, or that we ended on a nul if not
   */
  if (max_len >= 0 &&
      p != (str + max_len))
    return FALSE;
  else if (max_len < 0 &&
           *p != '\0')
    return FALSE;
  else
    return TRUE;
}

/**
 * g_unichar_validate:
 * @ch: a Unicode character
 * 
 * Checks whether @ch is a valid Unicode character. Some possible
 * integer values of @ch will not be valid. 0 is considered a valid
 * character, though it's normally a string terminator.
 * 
 * Return value: %TRUE if @ch is a valid Unicode character
 **/
gboolean
g_unichar_validate (gunichar ch)
{
  return UNICODE_VALID (ch);
}

/**
 * g_ucs4_to_utf8:
 * @str: a UCS-4 encoded string
 * @len: the maximum length of @str to use. If < 0, then
 *       the string is %NULL terminated.
 * @items_read: location to store number of characters read read, or %NULL.
 * @items_written: location to store number of bytes written or %NULL.
 *                 The value here stored does not include the trailing 0
 *                 byte. 
 * @error: location to store the error occuring, or %NULL to ignore
 *         errors. Any of the errors in #GConvertError other than
 *         %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from a 32-bit fixed width representation as UCS-4.
 * to UTF-8. The result will be terminated with a 0 byte.
 * 
 * Return value: a pointer to a newly allocated UTF-8 string.
 *               This value must be freed with g_free(). If an
 *               error occurs, %NULL will be returned and
 *               @error set.
 **/
gchar *
g_ucs4_to_utf8 (const gunichar *str,
		glong           len,              
		glong          *items_read,       
		glong          *items_written)
{
  gint result_length;
  gchar *result = NULL;
  gchar *p;
  gint i;

  result_length = 0;
  for (i = 0; len < 0 || i < len ; i++)
    {
      if (!str[i])
	break;

      if (str[i] >= 0x80000000)
	{
	  if (items_read)
	    *items_read = i;
	  
	  g_error (_("Character out of range for UTF-8"));
	  goto err_out;
	}
      
      result_length += UTF8_LENGTH (str[i]);
    }

  result = g_malloc (result_length + 1);
  p = result;

  i = 0;
  while (p < result + result_length)
    p += g_unichar_to_utf8 (str[i++], p);
  
  *p = '\0';

  if (items_written)
    *items_written = p - result;

 err_out:
  if (items_read)
    *items_read = i;

  return result;
}
