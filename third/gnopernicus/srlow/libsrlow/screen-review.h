/* screen-review.h
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "glib.h"
#include "SRObject.h"

#define SRW_FILL_CELL 		0
#define SRW_DELIMITER_CELL	-1

typedef struct _SRWAccCell
{
    gchar 	*ch;
    gint  	id;
    gint  	index;
    gint	role;
    SRObject	*source;
} SRWAccCell;

typedef struct
{
    GArray 	*srw_acc_line;
    gint 	 is_empty;
} SRWAccLine;
typedef  GArray	SRWAccOutput;

typedef enum _SRWAccAlignFlags
{
    SRW_ALIGNF_HSP_ADD_LEADING	= 0x00000001,
    SRW_ALIGNF_HSP_ADD_TRAILING	= 0x00000002,
    SRW_ALIGNF_HSP_ADD_EMBEDDED = 0x00000004,
    SRW_ALIGNF_HSP_ADD		= 0x00000007,

    SRW_ALIGNF_VSP_ADD_LEADING	= 0x00000010,
    SRW_ALIGNF_VSP_ADD_TRAILING	= 0x00000020,
    SRW_ALIGNF_VSP_ADD_EMBEDDED = 0x00000040,
    SRW_ALIGNF_VSP_COUNT_LINES  = 0x00000080,
    SRW_ALIGNF_VSP_ADD 		= 0x00000070,
    

    
    SRW_ALIGNF_ALL 		= 0x000000F7
} SRWAccAlignFlags;

typedef enum _SRWAccScopeFlags
{
    SRW_SCOPE_WINDOW		= 0x00000001,
    SRW_SCOPE_APPLICATION	= 0x00000002,
    SRW_SCOPE_DESKTOP		= 0x00000004,
} SRWAccScopeFlags;

int 		screen_review_init 	(SRRectangle 	*clip_rectangle,
					 SRObject	*focused_object,
					 glong 		 align_flags,
					 glong 		 scope_flags);
void		screen_review_terminate	(void);				  

SRWAccLine *	screen_review_get_line 	(int 		 line_number,
					 int 		*y1,
					 int 		*y2);
					 
gint		screen_review_get_focused_line 	(void);

