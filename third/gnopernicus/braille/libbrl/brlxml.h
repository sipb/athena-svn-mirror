/* brlxml.h
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __BRLXML_H__
#define __BRLXML_H__

#include <glib.h>

#define TT_SIZE 256

typedef enum
{
    BPS_IDLE,
    BPS_BRL_OUT,
    BPS_BRL_DISP,
    BPS_DOTS,
    BPS_TEXT,	
    BPS_SCROLL,
    BPS_UNKNOWN
} BPSParserState;

typedef enum
{
    BRL_8_DOTS,
    BRL_6_DOTS
} BRLStyle;

typedef enum
{
    BRL_CS_UNDERLINE,
    BRL_CS_BLOCK,
    BRL_CS_USER_DEFINED	
} BRLCursorStyle;

typedef struct
{
    gboolean		clear_display;
    gint8		id;	
    gchar		*role;
    gint16		start;	
    gint16		offset;	
    gint16		cursor_position;
    BRLCursorStyle	cursor_style;
    guint8		cursor_mask;
    guint8		cursor_pattern;
    guint8		attribute;
    guint8		*translation_table;
    GByteArray		*dots;
} BRLDisp;

typedef struct
{
    gboolean		clear_all_cells;
    GArray		*displays;
    guint8		*translation_table;						/* the actual translation table */
    BRLStyle     	braille_style;
} BRLOut;

/* BRLDisp Methods */
BRLDisp* brl_disp_new        ();
BRLDisp* brl_disp_copy       (BRLDisp *brl_disp);
void     brl_disp_free       (BRLDisp *brl_disp);
void     brl_disp_add_dots   (BRLDisp *brl_disp,
			     guint8   *dots,
			     gint     len);	

/* BRLOut Methods */
BRLOut* brl_out_new          ();
void    brl_out_free         (BRLOut  *brl_out);
void    brl_out_add_display  (BRLOut  *brl_out,
			     BRLDisp  *brl_disp);
void    brl_out_to_driver    (BRLOut  *brl_out);

#endif
