/* tex-font.h: declarations for all TeX font formats.

Copyright (C) 1993 Karl Berry.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef KPATHSEA_TEX_FONT_H
#define KPATHSEA_TEX_FONT_H

#include <kpathsea/c-proto.h>
#include <kpathsea/filefmt.h>
#include <kpathsea/types.h>


/* NULL if don't use.  (That is the default.)  */
extern const_string kpse_fallback_font;


/* If non-NULL, check these if can't find (within a few percent of) the
   given resolution.  List must end with a zero element.  */
extern unsigned *kpse_fallback_resolutions;

/* This initializes the fallback resolution list.  If ENVVAR
   is set, it is used; otherwise, the envvar `TEXSIZES' is looked at; if
   that's not set either, a default set by the Makefile is used.  */
extern void kpse_init_fallback_resolutions P1H(string envvar);


/* This type describes a font that we have found.  Maybe it would work
   for all kinds of files.  */

typedef enum
{
  kpse_source_normal, kpse_source_alias,
  kpse_source_maketex, kpse_source_fallback
} kpse_source_type;

typedef struct
{
  string name;			/* font name found */
  unsigned dpi;			/* size found, for glyphs */
  kpse_file_format_type format;	/* glyph format found */
  kpse_source_type source;	/* where we found it */
} kpse_font_file_type;		

#define KPSE_FONT_FILE_NAME(f) ((f).name)
#define KPSE_FONT_FILE_DPI(f) ((f).dpi)
#define KPSE_FONT_FILE_FORMAT(f) ((f).format)
#define KPSE_FONT_FILE_SOURCE(f) ((f).source)

#endif /* not KPATHSEA_TEX_FONT_H */
