/* filefmt.h: declarations for file formats we know about.

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

#ifndef KPATHSEA_FILEFMT_H
#define KPATHSEA_FILEFMT_H

#include <kpathsea/init-path.h>
#include <kpathsea/paths.h>


/* We put the glyphs first so we don't waste space in an array.  A new
   format here should be accompanied by a new initialization
   abbreviation below and a new entry in `tex-make.c'.  */
typedef enum
{
  kpse_gf_format,
  kpse_pk_format,
  kpse_any_glyph_format, /* ``any'' meaning any of the above */
  kpse_bib_format, 
  kpse_bst_format, 
  kpse_mf_format, 
  kpse_tex_format, 
  kpse_tfm_format, 
  kpse_vf_format
} kpse_file_format_type;


/* Do initialization for the various file formats.  We separate the
   envvar lists so they can be used in other contexts with a different
   default, as in dvipsk (until we have config file support).  */
#define KPSE_BASE_ENVS "MFBASES", NULL
#define KPSE_BASE_PATH() \
  kpse_init_path (NULL, DEFAULT_BASE_PATH, KPSE_BASE_ENVS)

#define KPSE_BIB_ENVS "BIBINPUTS", NULL
#define KPSE_BIB_PATH() kpse_init_path (NULL, DEFAULT_BIB_PATH, KPSE_BIB_ENVS)

#define KPSE_BST_ENVS "BSTINPUTS", "TEXINPUTS", NULL
#define KPSE_BST_PATH() kpse_init_path (NULL, DEFAULT_BST_PATH, KPSE_BST_ENVS)

#define KPSE_FMT_ENVS "TEXFORMATS", NULL
#define KPSE_FMT_PATH() kpse_init_path (NULL, DEFAULT_FMT_PATH, KPSE_FMT_ENVS)

#define KPSE_GF_ENVS "GFFONTS", KPSE_GLYPH_ENVS
#define KPSE_GF_PATH() kpse_init_path (NULL, DEFAULT_GF_PATH, KPSE_GF_ENVS)

#define KPSE_GLYPH_ENVS "GLYPHFONTS", "TEXFONTS", NULL
#define KPSE_GLYPH_PATH() \
  kpse_init_path (NULL, DEFAULT_GLYPH_PATH, KPSE_GLYPH_ENVS)

#define KPSE_MF_ENVS "MFINPUTS", NULL
#define KPSE_MF_PATH() kpse_init_path (NULL, DEFAULT_MF_PATH, KPSE_MF_ENVS)

#define KPSE_MFPOOL_ENVS "MFPOOL", NULL
#define KPSE_MFPOOL_PATH() \
  kpse_init_path (NULL, DEFAULT_MFPOOL_PATH, KPSE_MFPOOL_ENVS)

#define KPSE_PK_ENVS "PKFONTS", "TEXPKS", KPSE_GLYPH_ENVS
#define KPSE_PK_PATH() kpse_init_path (NULL, DEFAULT_PK_PATH, KPSE_PK_ENVS)

#define KPSE_TEX_ENVS "TEXINPUTS", NULL
#define KPSE_TEX_PATH() kpse_init_path (NULL, DEFAULT_TEX_PATH, KPSE_TEX_ENVS)

#define KPSE_TEXPOOL_ENVS "TEXPOOL", NULL
#define KPSE_TEXPOOL_PATH() \
  kpse_init_path (NULL, DEFAULT_TEXPOOL_PATH, KPSE_TEXPOOL_ENVS)

#define KPSE_TFM_ENVS "TFMFONTS", "TEXFONTS", NULL
#define KPSE_TFM_PATH() kpse_init_path (NULL, DEFAULT_TFM_PATH, KPSE_TFM_ENVS)

#define KPSE_VF_ENVS "VFFONTS", "TEXFONTS", NULL
#define KPSE_VF_PATH() kpse_init_path (NULL, DEFAULT_VF_PATH, KPSE_VF_ENVS)

#endif /* not KPATHSEA_FILEFMT_H */
