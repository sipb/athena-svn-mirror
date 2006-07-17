/*        Misc utility functions for the Gaim-Encryption plugin           */
/*             Copyright (C) 2001-2003 William Tompkins                   */

/* This plugin is free software, distributed under the GNU General Public */
/* License.                                                               */
/* Please see the file "COPYING" distributed with the Gaim source code    */
/* for more details                                                       */
/*                                                                        */
/*                                                                        */
/*    This software is distributed in the hope that it will be useful,    */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*   General Public License for more details.                             */

/*   To compile and use:                                                  */
/*     See INSTALL file.                                                  */

#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


#include <debug.h>
#include <gaim.h>

#ifdef _WIN32
#include <win32dep.h>
#endif

#include "nls.h"
#include "cryptutil.h"
#include "rsa_nss.h"

#include <base64.h>



void GE_clear_string(unsigned char* s) {
   while(*s != 0) {
      *(s++) = 0;
   }
}

void GE_escape_name(GString* name) {
   int pos = 0;

   // Note: name->len and name->str can change as we modify name...
   while (pos < name->len) {
      switch (name->str[pos]) {
      case ' ':
         // space -> "\s"
         g_string_erase(name, pos, 1);
         g_string_insert(name, pos, "\\s");
         pos += 2;
         break;
      case ',':
         // comma -> "\c"
         g_string_erase(name, pos, 1);
         g_string_insert(name, pos, "\\c");
         pos += 2;         
         break;
      case '\\':
         // backslash -> "\\"
         g_string_erase(name, pos, 1);
         g_string_insert(name, pos, "\\");
         pos += 2;         
         break;
      default:
         ++pos;
         break;
      }
   }
}

void GE_unescape_name(char* origname) {
   GString *name = g_string_new(origname);
   int pos = 0;

   while (pos < name->len) {
      if (name->str[pos] == '\\') {
         g_string_erase(name, pos, 1);
         switch (name->str[pos]) {
         case 's':
            // \s -> space
            name->str[pos] = ' ';
            ++pos;
            break;
         case 'c':
            // \c -> comma
            name->str[pos] = ',';
            ++pos;
            break;
         default:
            // leave the char that followed the backslash
            ++pos;
            break;
         }
      } else {
         ++pos;
      }
   }

   // string can only be shorter, so this copy is ok
   strcpy(origname, name->str);

   g_string_free(name, TRUE);
}

/* Convert 'num' bytes into an ascii string.  */
void GE_bytes_to_str(unsigned char *str, unsigned char *bytes, int num) {
   unsigned char* tmp = BTOA_DataToAscii(bytes, num);
   GString* tmp2 = g_string_new(tmp);

   GE_strip_returns(tmp2);
   strcpy(str, tmp2->str);

   PORT_Free(tmp);
   g_string_free(tmp2, TRUE);
}

/* Convert ascii string back into bytes.  Returns number of bytes */
unsigned int GE_str_to_bytes(unsigned char *bytes, unsigned char *cstr) {
   unsigned int tmplen;
   unsigned char* tmp = ATOB_AsciiToData(cstr, &tmplen);
   if (tmp == NULL) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", _("Invalid Base64 data, length %d\n"),
                 strlen(cstr));
      return 0;
   }

   memcpy(bytes, tmp, tmplen);
   PORT_Free(tmp);

   return tmplen;
}


GString* GE_strip_returns(GString* s) {
   gchar *strippedStr, **strArray;
   int i;

   strArray = g_strsplit(s->str, "\n", 100);

   for (i = 0; strArray[i] != 0; ++i) {
      g_strstrip(strArray[i]);
   }

   strippedStr = g_strjoinv(0, strArray);
   
   g_string_assign(s, strippedStr);
   
   g_strfreev(strArray);
   g_free(strippedStr);
   return s;
}

gboolean GE_msg_starts_with_link(const char* c) {
   /* This seems easy.  But, we need to filter out intermediate HTML
      (like <font>) as well.  And, to be really compliant, we should
      parse things like "< a href=" (even if noone else seems to).
   */

   while (*c != 0) {
      /* If we don't start with a tag, we can't start with a link */
      if (*(c++) != '<') return FALSE;

      while (isspace(*c)) ++c;     /* skip leading whitespace in tag */
      if (*c == 'A' || *c == 'a') return TRUE;
      c = strchr(c, '>');          /* skip to end of tag */
      if (*c) ++c;                 /* watch out for unclosed tags! */
   }
   return FALSE;
}

/* Removed from Gaim, so added in here :)  */

GSList *GE_message_split(char *message, int limit) {
	static GSList *ret = NULL;
	int lastgood = 0, curgood = 0, curpos = 0, len = strlen(message);
	gboolean intag = FALSE;

	if (ret) {
		GSList *tmp = ret;
		while (tmp) {
			g_free(tmp->data);
			tmp = g_slist_remove(tmp, tmp->data);
		}
		ret = NULL;
	}

	while (TRUE) {
		if (lastgood >= len)
			return ret;

		if (len - lastgood < limit) {
			ret = g_slist_append(ret, g_strdup(&message[lastgood]));
			return ret;
		}

		curgood = curpos = 0;
		intag = FALSE;
		while (curpos <= limit) {
			if (isspace(message[curpos + lastgood]) && !intag)
				curgood = curpos;
			if (message[curpos + lastgood] == '<')
				intag = TRUE;
			if (message[curpos + lastgood] == '>')
				intag = FALSE;
			curpos++;
		}

		if (curgood) {
			ret = g_slist_append(ret, g_strndup(&message[lastgood], curgood));
			if (isspace(message[curgood + lastgood]))
				lastgood += curgood + 1;
			else
				lastgood += curgood;
		} else {
			/* whoops, guess we have to fudge it here */
			ret = g_slist_append(ret, g_strndup(&message[lastgood], limit));
			lastgood += limit;
		}
	}
}



/* Old versions of utility functions, using Base16 rather than Base64 */


/* Note: str will _NOT_ be null terminated.  Its length will be returned
   from the function */
   
/* int GE_bytes_to_colonstr(unsigned char *str, unsigned char *bytes, int num) { */
/*    int bytes_cursor=0, str_cursor=0; */
   
/*    while (bytes_cursor < num) { */
/*       sprintf(str + str_cursor, "%02x", bytes[bytes_cursor++]); */
/*       str_cursor += 2; */
/*       if (bytes_cursor < num) str[str_cursor++] = ':'; */
/*    } */

/*    return str_cursor; */
/* } */

/* /\* Note: str will _NOT_ be null terminated.  Its length will be returned */
/*    from the function *\/    */
/* int GE_bytes_to_str(unsigned char *str, unsigned char *bytes, int num) { */
/*    int bytes_cursor=0, str_cursor=0; */
   
/*    while (bytes_cursor < num) { */
/*       sprintf(str + str_cursor, "%02x", bytes[bytes_cursor++]); */
/*       str_cursor += 2; */
/*    } */
/*    return str_cursor; */
/* } */

/* /\* Note: str does not need to be null terminated.  num bytes are pulled */
/*          off the string (unless the end of the string is reached first). */
/*          The number of chars pulled off the string is returned, or -1 */
/*          if there is an error or the end of the string is reached first.     *\/ */
   
/* int GE_nstr_to_bytes(unsigned char *bytes, unsigned char *nstr, int num) { */
/*    int bytes_cursor, str_cursor = 0; */
/*    unsigned char minibuf[3] = "00"; */

/*    for (bytes_cursor = 0; bytes_cursor < num; ++bytes_cursor) { */
/*       minibuf[0] = nstr[str_cursor++]; */
/*       if (minibuf[0] == 0) return -1; */
/*       minibuf[1] = nstr[str_cursor++]; */
/*       if (minibuf[1] == 0) return -1;       */
/*       bytes[bytes_cursor] = (unsigned char) strtoul(minibuf, 0, 16); */
/*    } */
/*    return str_cursor; */
/* } */

/* /\* Note: cstr must be null terminated.  Up to num bytes are pulled */
/*          off the string (unless the end of the string is reached first). */
/*          The number of bytes gotten is returned.          */
/*          A parse error returns -1                                        *\/ */
   
/* int GE_cstr_to_bytes(unsigned char *bytes, unsigned char *cstr, int num) { */
/*    int bytes_cursor, str_cursor = 0; */
/*    unsigned char minibuf[3] = "00"; */

/*    for (bytes_cursor = 0; bytes_cursor < num; ++bytes_cursor) { */
/*       minibuf[0] = cstr[str_cursor++]; */
/*       if (minibuf[0] == 0) return bytes_cursor; */
/*       minibuf[1] = cstr[str_cursor++]; */
/*       if (minibuf[1] == 0) return -1;       */
/*       bytes[bytes_cursor] = (unsigned char) strtoul(minibuf, 0, 16); */
/*    } */
/*    return bytes_cursor; */
/* } */

