/* screen-review.c
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

/*screen-review.c
 * VERSION : 0.44 (on 0.33)
 * FIXED/DONE:
 *	- no debug messages
 *	- vertical spaces working
 *	- id = 0 for acc_cells that do not belong to AccessibleObjects    
 *	- check y for a text widget (out of the visible arrea or not)
 *      - FIXED:
 * 	FIXME - line 340, (srw_acc_cell_free), for PAGE_TAB, MENU
 *	      - reproducing :1. uncomment 
 *				SPI_ROLE_PAGE_TAB (673)
 *			     2. gtk-demo, line Widget.. Source Info		  
 * 	- changed the "making lines" algorithm
 *	- added FRAME name + other toplevel windows name
 *	- instead of special cases of toplevel windows roles (FRAME, DIALOG..)
 *		use layer == SPI_LAYER_WINDOW
 *	- remove warnnings, check for leaks(unrefs)
 *	- add trailling and leading empty lines
 *	- FLAGS for trailling and leading VSP
 *	- FLAGS for trailling and leading HSP
 *	- removed "//"
 *	- instead of special cases of popup roles (MENU, MENU_ITEM...)
 *		use layer == SPI_LAYER_POPUP
 *	- menu
 *	- fixed clipping (added following case : start_inside = 0 && end_inside =0)
 *	- fixed strip_newline (in order to return correct word_start)
 *	- (srw_text_chunk_list_head_clip) : removed memory leak (free the chunks that are clipped out)
 *	- implemented a cache (0.34)
 *	- implemented a better cache (have to decide what compromise should I do : speed Vs resources)
 *	- changed API
 *	- add SOURCE (the Accessible object) to each AccCell so the user would 
 *	  press the strip sensors and would obtain information about the object
 *	  displayed in the current line at that particular location
 *	- got rid of memory leaks that where present due to calls like:
 *		string = g_strconcat (string, string2, NULL);
 *	- got rid of including diffrent delimiters for diffrent roles
 *	 (this was redundant as this information is not passed to the screen-review
 *	 client. The client will have the information about the role and that the cell
 * 	 contains a delimitor (id) inside the SRWAccCEll structure.
 *	- replaced the name of DELIMITATOR with DELIMITER (this is indeed english ;-) ) 
 *	- introduced the z-order in the TextChunk, for future use (in case of 
 *	SRW_SCOPE_APPLICATION or SRW_SCOPE_DESKTOP scope.
 *FIXME/2DO
 *	- add images, scrollbars, etc (objects that don't have a text displayed on screen)
 *	 and figure it out how would be represented on braille
 *	- optimization : do not calculate all the time the baseline ( * 2 / 3), better I should add
 *	  a baseline member to the SRWTextChunk structure
 *	- get rid of some of globals. Refactoring required
 *	- break the functions (some of them are too big and very hard to read) 
 *	- Q: what should be presented when the application is iconofied and how? 
 *	- A: NAME OF WINDOW (iconofoed).
 *	- lines should be of SRWLines type OR better SRWLineList == GList
 *	- elements should be of type SRWTextChunkList = GList
 * 	- there is no use of the align_flags parameter in function 
 *		(srw_lines_create_from_elements)
 */

#include "config.h"

#include "cspi/spi.h"
#include "SRMessages.h"

#include "screen-review.h"
/*
 *SPI_LAYER_INVALID: The layer cannot be determined or is somehow undefined.
 *SPI_LAYER_BACKGROUND: Component belongs to the destop background.
 *SPI_LAYER_CANVAS: Component is a canvas backdrop or drawing area.
 *SPI_LAYER_WIDGET: Component is a 'normal' widget.
 *SPI_LAYER_MDI: Component is drawn in the MDI layer and may have valid
 *                         Z-information relative to other MDI-layer components.
 *SPI_LAYER_POPUP: Component is in the popup layer, above other widgets and
 *                         MDI components.
 *SPI_LAYER_OVERLAY: Component is in the overlay plane - this value is reserved
 *                         for future use.
 *SPI_LAYER_WINDOW: Component is in the window layer and have valid Z-information
 *                   relative to other window-layer components.
 *SPI_LAYER_LAST_DEFINED: Used to determine the last valid value in the enum,
 *                         should not be encountered.	
 * layer1 < layer2 ->layer2 is ontop of layer 1 
*/

typedef struct _SRWBoundaryRect 
{
	long int	x;
	long int	y;
	long int	width;
	long int	height;
	/* role of last clipping element */
	AccessibleRole 	role; 
	gboolean 	is_clipped;
	gboolean 	is_empty;
} SRWBoundaryRect;

typedef struct _SRWTextChunk 
{
	char		*string;
	Accessible	*source;
	int		 start_offset;
	int		 end_offset;
	SRWBoundaryRect	 clip_bounds;
	SRWBoundaryRect	 text_bounds;
	SRWBoundaryRect	 start_char_bounds;
 	SRWBoundaryRect	 end_char_bounds;
	int 		 id;
	int		 z_order;
	int		 layer;
	gboolean	 is_text;
	gboolean	 is_focused;
	gboolean	 dummy;
} SRWTextChunk;

typedef struct _SRWLine
{
	GList		*cells;
	gint		 y1;
	gint		 y2;
	gint		 baseline;
	gint 		 layer;
	
	gboolean	 is_focused;
	gint	 	 is_empty;
	gint 		 cached;
	SRWAccLine	*acc_line;	
	char		*string;		
} SRWLine; 

typedef GList SRWLines;

typedef GList SRWElements;

#define SRW_DELTA_BASELINE_THRESHOLD	4
#define SRW_BASELINE_RATIO		0.66
#define SRW_MANAGE_DESCENDATS_STEP	5
/*__________________________< DEBUG>__________________________________________*/
#undef	SRW_COMPOSITE_DEBUG 
#undef	SRW_BENCHMARKING

#undef	SRW_STRING_DEBUG
#undef	SRW_BOUNDARY_RECT_DEBUG
#undef	SRW_TEXT_CHUNK_DEBUG
#undef	SRW_TEXT_CHUNK_LIST_DEBUG_ 
#undef	SRW_ACC_LINE_DEBUG
#undef	SRW_CLEAN_UP_DEBUG
#undef	SRW_CLIPPING_DEBUG
#undef	SRW_CLIPPING_DEBUG
#undef	SRW_ACC_LINE_CACHE	    
#undef	SRW_MENAGE_DESCENDANTS
/*__________________________</DEBUG>__________________________________________*/

/*______________________________< MACROS>_____________________________________*/

#define BOUNDS_CONTAIN_X_BOUNDS(b, p)	( ( (p).x >= (b).x ) 		&&	\
					( ( (p).x + (p).width) <= 		\
                                          ( (b).x + (b).width) )	&& 	\
	                                ( (b).width > 0) 		&& 	\
					( (b).height > 0) )

#define BOUNDS_CONTAIN_Y(b, p)  	( ( (p) > (b)->y) 		&& 	\
					( (p) < ((b)->y + (b)->height) )&&	\
					( (b)->width > 0) 		&& 	\
					( (b)->height > 0) )

#define CHUNK_BOUNDS_BEFORE_START(c, i) ( (i) 	&& 		\
					( (c)->clip_bounds.x < 	\
                                          (i)->clip_bounds.x) )

#define CHUNK_BOUNDS_END_BEFORE_START(c, i) ( (i) 	&& 		\
                                            ( ( (c)->clip_bounds.x + 	\
                                            (c)->clip_bounds.width) < 	\
                                            (i)->clip_bounds.x) )

#define CHUNK_BOUNDS_AFTER_END(c, i) 	( (i) 	&& 			\
					( (c)->clip_bounds.x >= 	\
			                ( (i)->clip_bounds.x + 		\
			                  (i)->clip_bounds.width) ) )

#define CHUNK_BOUNDS_SPANS_END(c, i) 	( (i) 	&& 			\
					( ( (c)->clip_bounds.x + 	\
					  (c)->clip_bounds.width) >= 	\
			                ( (i)->clip_bounds.x + 		\
			                  (i)->clip_bounds.width) ) )

#define IS_CLIPPING_CONTAINER(a) ( ( (a) != SPI_ROLE_PAGE_TAB ) && 	\
				   ( (a) != SPI_ROLE_MENU) 	&&	\
				   ( (a) != -1) )
#define MAX_PIXELS_PER_COLUMNS	 10000
#define MIN_PIXELS_PER_COLUMNS	 2
/*______________________________</MACROS>_____________________________________*/

/*______________________________< GLOBALS>____________________________________*/
/*FIXME : get rid of some of these globals. Refactoring required*/
static SRWElements	*elements 		= NULL;
static SRWLines		*lines			= NULL;
static GArray 		*lines_index 		= NULL;
static gint		id 			= 0,
			pixels_per_column 	= MAX_PIXELS_PER_COLUMNS;
static glong		align_flags		= 0;
static SRWBoundaryRect 	clipping_rectangle;
/*______________________________</GLOBALS>____________________________________*/

extern gboolean 
sro_get_from_accessible (Accessible 	*acc, 
			  SRObject	**obj,
			  SRObjectType	type);
extern Accessible *
sro_get_acc_at_index (const SRObject *obj, 
		      const gint index);
			  
/*_________________________< SRWString methods>_______________________________*/
static char *
srw_string_strip_newlines (char 	*string, 
			   long 	 offset, 
			   long 	*start_offset, 
			   long 	*end_offset)
{
    int i, start = 0;
    char *word_start = string;
    
    for (i = 0; string && string[i]; ++i)
    {
	if ((string[i] == '\n')  && i > (offset - *start_offset) )
	{
	    string[i] = '\0';
	    *end_offset = *start_offset + i;
	    *start_offset = *start_offset + start;
	    return word_start;
	}
	else
	{
	    if (string[i] == '\n')
	    {
		word_start = &string[i+1];
		start  = i + 1;
	    }
	
	}
    }
    *start_offset = *start_offset + start;
    return word_start;
}

static char *
srw_string_guess_clip (SRWTextChunk 	*chunk)
{
	SRWBoundaryRect 	b;
	char 		*string 	= NULL, 
			*start_pointer 	= NULL,
			*end_pointer	= NULL;
	long 		start_offset, 
			end_offset, 
			len;
	if (chunk && chunk->string)
	    start_pointer = chunk->string;
	if (start_pointer) 
	{
	    AccessibleComponent *component = Accessible_getComponent (
						chunk->source);
	    end_pointer 	= start_pointer + (strlen (start_pointer) );
	    len	= g_utf8_strlen (chunk->string, 
				 -1);
	    if (component) 
	    {
		AccessibleComponent_getExtents (component,
						&b.x, 
						&b.y,
						&b.width, 
						&b.height,
						SPI_COORD_TYPE_SCREEN);
		AccessibleComponent_unref (component);
		start_offset 	= len * 
			          (chunk->text_bounds.x - b.x) / 
			    	  b.width;
		end_offset 	= len * 
				  (chunk->text_bounds.x +
				  chunk->text_bounds.width - b.x) / 
				  b.width;
#ifdef SRW_STRING_DEBUG		
		fprintf (stderr, "\nsrw_string_guess_clip: string len=%ld, clipped to %ld-%ld (start-end)",
				 len, 
				 start_offset, 
				 end_offset);
#endif
		len = end_offset - start_offset;
		start_pointer = g_utf8_offset_to_pointer (chunk->string, 
					       start_offset);
		end_pointer = g_utf8_offset_to_pointer (chunk->string, 
					       end_offset);
	    }
	    string = g_new0 (char, end_pointer - start_pointer + 1);
	    string = g_utf8_strncpy (string, start_pointer, len);
	    string [end_pointer - start_pointer] = '\0';
	    g_assert (g_utf8_validate (string, -1, NULL));
	}
	
	return string;
}
/*_________________________</SRWString methods>_______________________________*/


/*_____________________< SRWBoundaryRect methods>________________________________*/
static SRWBoundaryRect **
srw_boundary_rect_new0 (void)
{
	SRWBoundaryRect	**bounds;
	int 		i;
		
	bounds = g_new0 (SRWBoundaryRect *, SPI_LAYER_LAST_DEFINED);
	for (i = 0; i < SPI_LAYER_LAST_DEFINED; ++i) 
	{
		bounds[i] = g_new0 (SRWBoundaryRect, 1);
		bounds[i]->is_clipped = FALSE;
		bounds[i]->is_empty   = FALSE;
	}
	
	return bounds;
}


static SRWBoundaryRect **
srw_boundary_rect_clone (SRWBoundaryRect *bounds[])
{
	SRWBoundaryRect	**bounds_clone;
	int 		i;
		
	bounds_clone = 
		g_new0 (SRWBoundaryRect *, SPI_LAYER_LAST_DEFINED);
	for (i = 0; i < SPI_LAYER_LAST_DEFINED; ++i) 
	{
		bounds_clone[i] = g_new0 (SRWBoundaryRect, 1);
		*bounds_clone[i] = *bounds[i];
	}
	
	return bounds_clone;
}

static void
srw_boundary_rect_free (SRWBoundaryRect **bounds_clone)
{
	int 	i;
	
	for (i = 0; i < SPI_LAYER_LAST_DEFINED; ++i) 
	{
		g_free (bounds_clone[i]);
		bounds_clone[i] = NULL;
	}
	g_free (bounds_clone);
	bounds_clone = NULL;
}

static void
srw_boundary_rect_clip (SRWBoundaryRect	*bounds, 
			SRWBoundaryRect	*clip_bounds)
{
	long int x2 ;
	long int y2 ;

	if (!(bounds && clip_bounds))
	    return;
	    
	x2 = bounds->x + bounds->width;
	y2 = bounds->y + bounds->height;


#ifdef SRW_BOUNDARY_RECT_DEBUG

	fprintf (stderr, "\nsrw_boundary_rect_clip : [x] bounds %ld-%ld, clip_bounds %ld-%ld; ",
		 bounds->x, 
		 x2,
		 clip_bounds->x, 
		 clip_bounds->x + 
		 clip_bounds->width);
	fprintf (stderr, "\nsrw_boundary_rect_clip : [y] bounds %ld-%ld, clip_bounds %ld-%ld; ",
		 bounds->y, 
		 y2,
		 clip_bounds->y, 
		 clip_bounds->y + 
		 clip_bounds->height);
#endif		 
	bounds->x = MAX (bounds->x, clip_bounds->x);
	bounds->y = MAX (bounds->y, clip_bounds->y);
	x2 =  MIN (x2,  clip_bounds->x + clip_bounds->width);
	y2 =  MIN (y2, clip_bounds->y + clip_bounds->height);
	bounds->width  = MAX (x2 - bounds->x, 0);
	bounds->height = MAX (y2 - bounds->y, 0);
	
	if (!bounds->width || !bounds->height)
		bounds->is_empty = TRUE;
	if (IS_CLIPPING_CONTAINER (bounds->role)) 
	{
		*clip_bounds = *bounds;
	}
#ifdef SRW_BOUNDARY_RECT_DEBUG
	fprintf (stderr, "\nsrw_boundary_rect_clip : [x] bounds %ld-%ld	[y] bounds %ld-%ld",
		 bounds->x, 
		 bounds->x + 
		 bounds->width,
		 bounds->y,
		 bounds->y+
		 bounds->height);
#endif
}

static void
srw_boundary_rect_xclip_head (SRWBoundaryRect	*bounds, 
			      SRWBoundaryRect 	*clip_bounds)
{
	int x2;
	int cx2;
	
	if (!(bounds && clip_bounds))
	    return;
	
	cx2 = clip_bounds->x + clip_bounds->width;
	if (cx2 < bounds->x) 
	    return;
	    
	x2 = bounds->x + bounds->width;
	if (cx2 <= x2)
	     bounds->x = cx2;
	    
	bounds->width = MAX (0, x2 - cx2);
}

static void
srw_boundary_rect_xclip_tail (SRWBoundaryRect 	*bounds, 
			      SRWBoundaryRect 	*clip_bounds)
{
	int x2;

	if (!(bounds && clip_bounds))
	    return;
		
	x2 = bounds->x + bounds->width;
	if (clip_bounds->x > x2) 
	    return;
	
	bounds->width = MAX (0, clip_bounds->x - bounds->x);
}
/*_____________________</SRWBoundaryRect methods>________________________________*/



/*________________________< SRWAccCell methods>__________________________________*/
SRWAccCell *
srw_acc_cell_new (void)
{
    SRWAccCell 	*acc_cell;
    
    acc_cell = g_new0 (SRWAccCell, 1);
    
    return acc_cell;
}

void
srw_acc_cell_free (SRWAccCell *acc_cell)
{
    char *string = NULL;
    if (acc_cell)
    {
	if (acc_cell->ch)
	{
	    string = g_strdup (acc_cell->ch);
	    g_free (acc_cell->ch);
	    acc_cell->ch = NULL;
	}
	if (acc_cell->source)
	{
	    sro_release_reference (acc_cell->source);
	}
	g_free (acc_cell);
	acc_cell = NULL;

    }
}

/*________________________</SRWAccCell methods>__________________________________*/



/*________________________< SRWAccLine methods>__________________________________*/
SRWAccLine *
srw_acc_line_new (void)
{
    SRWAccLine 	*acc_line;
    
    acc_line = g_new0 (SRWAccLine , 1);
    acc_line->srw_acc_line =  g_array_new ( TRUE, 
			    		    TRUE, 
			    		    sizeof (SRWAccCell *) );
    return acc_line;
}

void
srw_acc_line_free (SRWAccLine 	*acc_line)
{
    if (!acc_line)
	return;
    if (acc_line->srw_acc_line)
    {
	int i;
	for (i = 0; i < acc_line->srw_acc_line->len; i++)
	{
	    SRWAccCell *acc_cell;
	    acc_cell = g_array_index (acc_line->srw_acc_line, SRWAccCell *, i);

	    srw_acc_cell_free (acc_cell);
	}
	g_array_free (acc_line->srw_acc_line, TRUE);
	acc_line->srw_acc_line = NULL;
	g_free (acc_line);
    }
}

void
srw_acc_line_from_string (SRWAccLine 	*acc_line, 
			  char 		*string, 
			  SRWTextChunk 	*chunk, 
			  glong 	offset,
			  glong 	start_offset)
{
    char	*crt 		= NULL, 
		*nxt 		= NULL, 
		*ch 		= NULL, 
		*s_dup 		= NULL;
    SRWAccCell 	*acc_cell 	= NULL;
    int 	i 		= 0,
		len 		= 0;
    
    
    if (acc_line && acc_line->srw_acc_line)
    {
	s_dup = g_strdup(string);
	crt = s_dup;
	len = g_utf8_strlen (string, -1);

	if (g_utf8_validate (crt, -1, NULL) )
	{
	    while (i < len)
	    {
		nxt = g_utf8_next_char (crt);
		ch  = g_new0 (gchar, nxt - crt + 1);
		ch  = g_utf8_strncpy (ch, crt, nxt - crt + 1);
		ch [nxt - crt] = '\0';
		if (ch && strlen (ch))
		{
		    acc_cell = srw_acc_cell_new ();

		    acc_cell->ch = ch;
		    acc_cell->index = offset + i;
		    if (chunk)
		    {
			acc_cell->id = chunk->id;
			acc_cell->role = chunk->clip_bounds.role;
			sro_get_from_accessible (chunk->source, 
						&(acc_cell->source),
						SR_OBJ_TYPE_VISUAL);
		    }
		    else
		    	acc_cell->id = 0;
		    
		    if (offset + i < start_offset)
			acc_cell->id = SRW_FILL_CELL;/*fill*/
		    else
			if ( (offset + i == start_offset) ||
			    (i == (len - 1) ) )
				acc_cell->id = SRW_DELIMITER_CELL;/*delimiter*/
#ifdef SRW_TEXT_CHUNK_UTF8_DEBUG
		        fprintf (stderr,"\nch =%string index=%d id=%d", 
					acc_cell->ch, 
					acc_cell->index, 
					acc_cell->id);		
#endif	
		    g_array_append_val (acc_line->srw_acc_line, acc_cell);
		}
		else
		{
		    sru_warning ("\nNOT a valid UTF8 string");    		    
		}
		i++;
		crt = nxt;
	    }
	}    
	
	if (s_dup) g_free (s_dup);	
	s_dup = NULL;
	}
}
/*________________________</SRWAccLine methods>__________________________________*/



/*______________________< SRWTextChunk methods>__________________________________*/
static SRWTextChunk *
srw_text_chunk_new0 (char 	*string,
		Accessible	*source,
		SRWBoundaryRect	*clip_bounds,
		SRWBoundaryRect	*text_bounds,
		int		id,
		int		z_order,
		int		layer)
{
    SRWTextChunk *text_chunk;
    
    text_chunk = g_new0 (SRWTextChunk, 1);

    if (string)
	text_chunk->string = g_strdup(string);
    if (source)
    {
	Accessible_ref (source);
	text_chunk->source = source;
    }
    if (clip_bounds)
	text_chunk->clip_bounds = *clip_bounds;
    if (text_bounds)
	text_chunk->text_bounds = *text_bounds;
	

    text_chunk->z_order		= z_order;
    text_chunk->layer 		= layer;
    text_chunk->id 		= id;
    text_chunk->dummy 		= FALSE;
    text_chunk->is_text 	= FALSE;
    text_chunk->is_focused	= FALSE;
    return text_chunk;
}

static void
srw_text_chunk_free (SRWTextChunk	*text_chunk)
{
    if (text_chunk)
    {
	if (text_chunk->string)
	{
	    g_free (text_chunk->string);
	    text_chunk->string = NULL;
	}

	if (text_chunk->source)
	{
	    Accessible_unref (text_chunk->source);

	}

	g_free (text_chunk);
	text_chunk = NULL;
    }
}

static SRWTextChunk *
srw_text_chunk_clone (SRWTextChunk	*text_chunk)
{
	SRWTextChunk 	*text_chunk_copy = NULL;
	
	if (!text_chunk)
	    return NULL;
	    
	text_chunk_copy = g_new0 (SRWTextChunk, 1);


	*text_chunk_copy = *text_chunk;

	if (text_chunk->string) 
	{
	    text_chunk_copy->string = g_strdup (text_chunk->string);
	}
	if (text_chunk_copy->source) 
	    Accessible_ref (text_chunk_copy->source);
	

	return text_chunk_copy;
}

static SRWTextChunk *
srw_text_chunk_from_accessible (Accessible	*accessible, 
			        SRWBoundaryRect	*bounds,
			        long 		offset,
				int 		z_order)
{
	SRWTextChunk 		*text_chunk 	= NULL;
	AccessibleText		*text 		= NULL;
	AccessibleRole 		role;
	AccessibleComponent 	*component;
	char 		*string 		= NULL, 
			*temp_string 	= NULL;
	long 		start, end,
			x2, y2;
	gboolean	no_need 	= TRUE,
			skip		= FALSE;
	int 		layer 		= 0;


	Accessible_ref (accessible);
	role = Accessible_getRole (accessible);
	
	component = Accessible_getComponent (accessible);
	layer = AccessibleComponent_getLayer (component); 
	AccessibleComponent_unref (component);
	
	text_chunk = srw_text_chunk_new0 (NULL,
				      accessible,
				      bounds,
				      NULL,
				      0,
				      z_order,
				      layer);	

	if (accessible)
	{
	    if (Accessible_isText (accessible)) 
	    {
		text = Accessible_getText (accessible);
		if (!offset)
		{
		    offset = AccessibleText_getOffsetAtPoint (text,
							      bounds->x,
							      bounds->y,
							      SPI_COORD_TYPE_SCREEN);	
		    if (offset > 0) 
			skip = TRUE;
		}
		
		temp_string = AccessibleText_getTextAtOffset (text, offset,
						    SPI_TEXT_BOUNDARY_LINE_START,
						    &start, &end);
#ifdef SRW_TEXT_CHUNK_DEBUG
		fprintf (stderr,"\nsrw_text_chunk_from_accessible : start %ld end %ld offset %ld",
				start,
				end,
				offset);
#endif
		if (temp_string)
		{
		    string = g_strdup (temp_string);
		    SPI_freeString (temp_string);
		}
		
		if (offset >= AccessibleText_getCharacterCount (text) || ((offset > start + 1) && !skip))
		{
		    g_free (string);
		    srw_text_chunk_free (text_chunk);
		    AccessibleText_unref (text);
		    Accessible_unref (accessible);
		    return NULL;
		}
	
#ifdef SRW_TEXT_CHUNK_DEBUG
		fprintf (stderr, "\nsrw_text_chunk_from_accessible: is text |%s| %20s",
				    string, 
				    Accessible_getRoleName(accessible) );
#endif				    
		if (end > start) 
		{

			AccessibleText_getCharacterExtents (
				text, start,
				&text_chunk->start_char_bounds.x,
				&text_chunk->start_char_bounds.y,
				&text_chunk->start_char_bounds.width,
				&text_chunk->start_char_bounds.height,
				SPI_COORD_TYPE_SCREEN);

#ifdef SRW_TEXT_CHUNK_DEBUG
			fprintf (stderr, "\nsrw_text_chunk_from_accessible: %s: start char (%ld) x=%ld y=%ld width=%ld height=%ld; ",
				 string,
				 start,
				 text_chunk->start_char_bounds.x,
				 text_chunk->start_char_bounds.y,
				 text_chunk->start_char_bounds.width,
				 text_chunk->start_char_bounds.height);
#endif
			if (string && strlen (string) && string[strlen (string) - 1] == '\n')
				    end--;
			if (end <= start)
			{
			    end = start + 1;
			}
			AccessibleText_getCharacterExtents (
				text, end-1,
				&text_chunk->end_char_bounds.x,
				&text_chunk->end_char_bounds.y,
				&text_chunk->end_char_bounds.width,
				&text_chunk->end_char_bounds.height,
				SPI_COORD_TYPE_SCREEN);
#ifdef SRW_TEXT_CHUNK_DEBUG			
			fprintf (stderr, "\nsrw_text_chunk_from_accessible: end char (%ld) x=%ld y=%ld width %ld height=%ld",
				 end-1,
				 text_chunk->end_char_bounds.x,
				 text_chunk->end_char_bounds.y,
				 text_chunk->end_char_bounds.width,
				 text_chunk->end_char_bounds.height);
#endif
		}
		/*\n*/
		if ((start == 0 && end == 0) 	|| 
		    start >= end 		||
		    !(string && strlen (string) ) )
		{
			text_chunk->is_text = FALSE;
			g_free (string);
			string = NULL;
		}
		else
		{
		    text_chunk->is_text = TRUE;

		
		    text_chunk->text_bounds.x = MIN (text_chunk->start_char_bounds.x,
						 text_chunk->end_char_bounds.x);
		    text_chunk->text_bounds.y = MIN (text_chunk->start_char_bounds.y,
						 text_chunk->end_char_bounds.y);
		    x2 = MAX (text_chunk->start_char_bounds.x +
			  text_chunk->start_char_bounds.width,
			  text_chunk->end_char_bounds.x +
			  text_chunk->end_char_bounds.width);
		    text_chunk->text_bounds.width = x2 - text_chunk->text_bounds.x;
		    y2 = MAX (text_chunk->start_char_bounds.y +
			  text_chunk->start_char_bounds.height,
			  text_chunk->end_char_bounds.y + 
			  text_chunk->end_char_bounds.height);
		    text_chunk->text_bounds.height = y2 - text_chunk->text_bounds.y;
		    text_chunk->start_offset = start;
		    text_chunk->end_offset = end ;

		    if (text_chunk->text_bounds.width > 0 && string && g_utf8_strlen (string, -1) )
		    {
			pixels_per_column = MIN (text_chunk->text_bounds.width / g_utf8_strlen (string, -1),
						pixels_per_column);
			pixels_per_column = MAX (pixels_per_column, MIN_PIXELS_PER_COLUMNS);
		    }
		}
		AccessibleText_unref (text);
		no_need = FALSE;
	} 
	if (!text_chunk->is_text)
	{
		if (role == SPI_ROLE_PUSH_BUTTON	||
		    role == SPI_ROLE_RADIO_BUTTON	||
		    role == SPI_ROLE_LIST 		||
		    role == SPI_ROLE_LIST_ITEM 		||
		    role == SPI_ROLE_LABEL 		||
		    role == SPI_ROLE_COLUMN_HEADER 	||
		    role == SPI_ROLE_TABLE_COLUMN_HEADER||		    
		    role == SPI_ROLE_TABLE_ROW_HEADER 	||
		    role == SPI_ROLE_TABLE_CELL		||
		    role == SPI_ROLE_DIALOG 		||
		    role == SPI_ROLE_COLOR_CHOOSER	||	
		    role == SPI_ROLE_PAGE_TAB 		||
		    role == SPI_ROLE_STATUS_BAR         ||
		    role == SPI_ROLE_CHECK_BOX          ||
		    text_chunk->layer == SPI_LAYER_WINDOW||
		    text_chunk->layer == SPI_LAYER_POPUP

		    ) 
		{/*there should be a switch here instead of the fat if*/
			temp_string= Accessible_getName (accessible);
			if (temp_string)
			{
				string = g_strdup (temp_string);
				SPI_freeString (temp_string);
				if ( !(string && strlen (string) ) )
				{	
				    if (string)
					g_free (string);
				    string = g_strdup ("------");
				}
			}
                        /* use name instead if there is no text available*/
			text_chunk->text_bounds = text_chunk->clip_bounds;
			text_chunk->start_offset = 0;
			text_chunk->end_offset = strlen (string);

			if (text_chunk->text_bounds.width > 0 && string && g_utf8_strlen (string,-1) )
			{
			    pixels_per_column = MIN (text_chunk->text_bounds.width / g_utf8_strlen (string, -1),
						pixels_per_column);
			    pixels_per_column = MAX (pixels_per_column, MIN_PIXELS_PER_COLUMNS);
			}						

#ifdef SRW_TEXT_CHUNK_DEBUG
		fprintf (stderr, "\n\nsrw_text_chunk_from_accessible: NOT text |%s| %20s",
				    string, 
				    Accessible_getRoleName(accessible) );
#endif				    
			
			no_need = FALSE;
		}
	if (no_need)
	{
/*
		 fprintf (stderr, "\nno needed role :%s", Accessible_getName (accessible) );
*/		 
		    srw_text_chunk_free (text_chunk);
		    Accessible_unref (accessible);
		    return NULL;
	}
	}
	if (text_chunk->text_bounds.x < text_chunk->clip_bounds.x) 
	{
		text_chunk->text_bounds.x = text_chunk->clip_bounds.x;
		text_chunk->text_bounds.is_clipped = TRUE;
	} 
	if ((text_chunk->text_bounds.x +
	     text_chunk->text_bounds.width)
	    > (text_chunk->clip_bounds.x +
	       text_chunk->clip_bounds.width)) 
	{
		text_chunk->text_bounds.width =
			MAX (0, (text_chunk->clip_bounds.x +
				 text_chunk->clip_bounds.width) -
			     text_chunk->text_bounds.x);
		text_chunk->text_bounds.is_clipped = TRUE;
	}
/*	have to check y (out of the visible arrea or not)*/
	if ( !(text_chunk->clip_bounds.y <= text_chunk->text_bounds.y + text_chunk->text_bounds.height/2 &&
	       text_chunk->clip_bounds.y + text_chunk->clip_bounds.height >= 
	       text_chunk->text_bounds.y + text_chunk->text_bounds.height / 2) )
	{
	    if (string)
	    {
		g_free(string);
		string = NULL;
	    }
	    if (text_chunk->is_text && !(text_chunk->clip_bounds.y + text_chunk->clip_bounds.height >= 
	       text_chunk->text_bounds.y + text_chunk->text_bounds.height / 2) )
	    {
		    srw_text_chunk_free (text_chunk);
		    Accessible_unref (accessible);
		    return NULL;
	    }
	    else
		text_chunk->start_offset = text_chunk->end_offset ;	    
	}

	if (string && strlen (string)) 
	{
		if (string[strlen(string)-1] == '\n') string[strlen(string)-1] = ' ';
		text_chunk->string = string;
	} 
	else 
	{
		text_chunk->string = NULL;
	}
    }
    Accessible_unref (accessible);

    return text_chunk;
}


static void
srw_text_chunk_tail_clip (SRWTextChunk	*bottom, 
		      SRWTextChunk	*top)
{
#ifdef SRW_CLIPPING_DEBUG
	fprintf (stderr, "\nsrw_text_chunk_tail_clip : [x] bottom %ld-%ld, top %ld-%ld;",
			bottom->clip_bounds.x,
			bottom->clip_bounds.x + bottom->clip_bounds.width,
			top->clip_bounds.x,
			top->clip_bounds.x + top->clip_bounds.width);
#endif
	srw_boundary_rect_xclip_tail (&bottom->text_bounds, 
			     &top->clip_bounds);
	srw_boundary_rect_xclip_tail (&bottom->clip_bounds, 
			     &top->clip_bounds);
	bottom->text_bounds.is_clipped = TRUE;
	bottom->clip_bounds.is_clipped = TRUE;
#ifdef SRW_CLIPPING_DEBUG
	fprintf (stderr, "\nsrw_text_chunk_tail_clip: [x] bottom result %ld-%ld\n",
			bottom->clip_bounds.x,
			bottom->clip_bounds.x + bottom->clip_bounds.width);
#endif
}

static void
srw_text_chunk_head_clip (SRWTextChunk	*bottom, 
		    	  SRWTextChunk 	*top)
{
#ifdef SRW_CLIPPING_DEBUG	
    fprintf (stderr, "\nsrw_text_chunk_head_clip: [x] bottom %ld-%ld, top %ld-%ld;",
		 bottom->clip_bounds.x,
		 bottom->clip_bounds.x + bottom->clip_bounds.width,
		 top->clip_bounds.x,
		 top->clip_bounds.x + top->clip_bounds.width);
#endif	
	srw_boundary_rect_xclip_head (&bottom->text_bounds, 
			     &top->clip_bounds);
	srw_boundary_rect_xclip_head (&bottom->clip_bounds, 
			     &top->clip_bounds);
	bottom->text_bounds.is_clipped = TRUE;
	bottom->clip_bounds.is_clipped = TRUE;
#ifdef SRW_CLIPPING_DEBUG
	fprintf (stderr, "\nsrw_text_chunk_head_clip: [x] bottom result %ld-%ld\n",
		 bottom->clip_bounds.x,
		 bottom->clip_bounds.x + bottom->clip_bounds.width);
#endif	
}

static inline gboolean
srw_text_chunk_bounds_within (SRWTextChunk	*chunk, 
			  SRWTextChunk 	*test_chunk)
{
	int x1, x2, tx1, tx2;
	gboolean gtx1, ltx2;

	x1 	= chunk->clip_bounds.x;
	x2 	= x1 + chunk->clip_bounds.width;
	tx1 	= test_chunk->clip_bounds.x;
	tx2 	= tx1 + test_chunk->clip_bounds.width;

	gtx1 	= (x1 >= tx1);
	ltx2 	= (x2 <= tx2);
	
	return gtx1 && ltx2;
}

#define TEXT_CHUNK_BOUNDS_WITHIN(a, b) srw_text_chunk_bounds_within(a, b)

#undef SRW_CHARACTER_CLIP_DEBUG

static char*
srw_text_chunk_get_clipped_substring_by_char (SRWTextChunk	*chunk, 
					  int 		start, 
					  int 		end)
{
	SRWBoundaryRect char_bounds;
	int i;
	char *s;
	GString *string = g_string_new ("");
	gunichar c;
	AccessibleText *text = Accessible_getText (chunk->source);
	for (i = start; i < end; ++i) 
	{
		AccessibleText_getCharacterExtents (text,
						    i,
						    &char_bounds.x,
						    &char_bounds.y,
						    &char_bounds.width,
						    &char_bounds.height,
						    SPI_COORD_TYPE_SCREEN);
#ifdef SRW_CLIPPING_SUB_DEBUG
		fprintf (stderr, "\ntesting %d-%d against %d-%d\n",
			 char_bounds.x, char_bounds.x+char_bounds.width,
			 chunk->text_bounds.x,
			 chunk->text_bounds.x + chunk->text_bounds.width);
#endif
		char_bounds.width = char_bounds.width *
				    (1 - SRW_BASELINE_RATIO);
		char_bounds.x += char_bounds.width;	
		if (BOUNDS_CONTAIN_X_BOUNDS (chunk->text_bounds,
					     char_bounds)) 
		{
			c = AccessibleText_getCharacterAtOffset (
				text, i);
#ifdef SRW_CLIPPING_DEBUG
			fprintf (stderr, "[%c]", c);
#endif				
			g_string_append_unichar (string, c);
		}
	}
	AccessibleText_unref (text);

	s = string->str;
	g_string_free (string, FALSE);

	return s;
}

static char*
srw_text_chunk_get_clipped_string (SRWTextChunk	*chunk)
{
	char 	*s 		= NULL, 
		*temp_string 	= NULL,
		*string 	= NULL;
	long 	start, 
		end;
	long 	word_start, 
		word_end, 
		range_end;
	SRWBoundaryRect	start_bounds, 
			end_bounds;
	gboolean 	start_inside, 
			end_inside;
	
	string = g_strdup("");
	if (chunk)
	{
	    start 	= chunk->start_offset;
	    end 	= chunk->end_offset;
	}
	else
	    return NULL;

	if (!chunk->text_bounds.is_clipped || 
	    !chunk->string)
	    string = chunk->string;
	else 
	if (chunk->source && chunk->is_text )
	{
		/* while words at offset lie within the bounds, add them */
	    AccessibleText *text = Accessible_getText (chunk->source);
#ifdef SRW_CLIPPING_DEBUG
	    fprintf (stderr, "\nclipping1: |%s|", chunk->string);
#endif
	    do 
	    {
		temp_string = AccessibleText_getTextAtOffset (text,
						    start,
						    SPI_TEXT_BOUNDARY_WORD_END,
						    &word_start,
						    &word_end);
		range_end = word_end;
#ifdef SRW_CLIPPING_DEBUG
		fprintf (stderr, "\nclipping2: |%s|", s);
#endif
		s = g_strdup (temp_string);
		SPI_freeString (temp_string);
		temp_string = NULL;
		if (s[0] == ' ')
		    word_start++;
#ifdef SRW_CLIPPING_DEBUG
		fprintf (stderr, "\nclipping2 before strip newlines: |%s| \nword_start %ld word_end %ld", 
				    s,
				    word_start,
				    word_end);
#endif		
		s = srw_string_strip_newlines (s, start, &word_start, &word_end);
#ifdef SRW_CLIPPING_DEBUG
		fprintf (stderr, "\nclipping2 after strip newlines: |%s| \nword_start %ld word_end %ld", 
				    s,
				    word_start,
				    word_end);
				    
#endif		
		AccessibleText_getCharacterExtents (text,
					    	    word_start,
						    &start_bounds.x,
						    &start_bounds.y,
						    &start_bounds.width,
						    &start_bounds.height,
						    SPI_COORD_TYPE_SCREEN);
				    
		AccessibleText_getCharacterExtents (text,
						    word_end - 1,
						    &end_bounds.x,
						    &end_bounds.y,
						    &end_bounds.width,
						    &end_bounds.height,
						    SPI_COORD_TYPE_SCREEN);
						    
		start_inside = BOUNDS_CONTAIN_X_BOUNDS (chunk->text_bounds,
							start_bounds);
		end_inside = BOUNDS_CONTAIN_X_BOUNDS (chunk->text_bounds,
						      end_bounds);
#ifdef SRW_CLIPPING_DEBUG		
		fprintf (stderr,"\nstart_inside %d end_inside %d",						      
					    start_inside,
					    end_inside);
#endif		     				    
    		if (start_inside && 
		    end_inside) 
		{
		    /* word is contained in bounds */
		    temp_string = g_strconcat (string, s, NULL);
		    g_free (string);
		    string = temp_string;
		} 
		else 
		{ 
/*		   if (start_inside ||
		        end_inside) 
		    {
*/			/* one end of word is in OR both end are out*/
			if (word_end > end) word_end = end;
			s = srw_text_chunk_get_clipped_substring_by_char (
				    chunk,
				    MAX (word_start, 
					 chunk->start_offset),
				    MIN (word_end, 
					 chunk->end_offset));
			if (s)
			{		
			    temp_string = g_strconcat (string, 
					      s, 
					      NULL);
			    g_free (string);
			    string = temp_string;
			}
/*		    }
*/		
		}
	    start = range_end;
	    } while (start < chunk->end_offset - 1);
	    AccessibleText_unref (text);
	
	} 
	else 
	{ 
	    /* we're clipped, but don't implement AccessibleText :-( */
	    /* guess for now, maybe we can do better someday */
	    string = srw_string_guess_clip (chunk);
	}
#ifdef SRW_CLIPPING_DEBUG
	fprintf (stderr, "\nOUT String clipped :|%s|", string);	
#endif	
	if (string && !strlen (string))
	{
	    g_free (string);
	    return NULL;
	} 
	return string;
}


static char*
srw_text_chunk_pad_string (SRWTextChunk	*chunk,
		       char 		*string, 
		       glong 		offset, 
		       glong		*start_offset,
		       const char 	*pad_chars)
{/*HSP flags*/
	char 	*s = NULL, *temp_string = NULL;
	char 	*cp;
	char 	startbuf[6], 
		padbuf[6], 
		endbuf[6];
	glong 	end_padding = -1;
	static	glong	 leading = 0;
	gint 	howmany;

	s = g_strdup("");
	
	howmany = g_unichar_to_utf8 (g_utf8_get_char (pad_chars), startbuf);
	startbuf[howmany] = '\0';
	g_assert (howmany < 7 && howmany > 0);
	
	cp = g_utf8_find_next_char (pad_chars, NULL);
	howmany = g_unichar_to_utf8 (g_utf8_get_char (cp), padbuf);
	padbuf[howmany] = '\0';
	g_assert (howmany < 7 && howmany > 0);
	
	cp = g_utf8_find_next_char (cp, NULL);
	howmany = g_unichar_to_utf8 (g_utf8_get_char (cp), endbuf);
	endbuf[howmany] = '\0';
	g_assert (howmany < 7 && howmany > 0);

	offset--;

	/*|leading embedded  embedded embedded embedded      embedded trailling */
	/*|00000000[_______OK--------]========[________Cancel--------]000000000|*/
	/*|00000000111111111111111111100000000111111111111111111111111000000000|*/
/*	
	fprintf (stderr, "\nchunk %p string %s offset %ld",chunk,string, offset);
*/
	if (chunk)
    	    end_padding = (chunk->clip_bounds.x - clipping_rectangle.x) / 
			    pixels_per_column + 1;/*+1*/ 
	else
	    end_padding = clipping_rectangle.width /
			    pixels_per_column + 1;/*+1*/
	if ((align_flags & SRW_ALIGNF_HSP_ADD_LEADING ) == 0)
	{    
	    if (!offset)
	    {
		leading = end_padding;
	    }
	    else
		if (chunk)
		    end_padding -= leading;	
	}    
	if ( ( ( (align_flags & SRW_ALIGNF_HSP_ADD_LEADING ) != 0) && !offset) ||
	     ( ( (align_flags & SRW_ALIGNF_HSP_ADD_EMBEDDED) != 0) &&  offset && chunk) ||
	     ( ( (align_flags & SRW_ALIGNF_HSP_ADD_TRAILING) != 0) && !chunk ) ) 
	{
	    /*0000 */
	    while (offset < end_padding )/*-1*/ 
	    {
		temp_string = g_strconcat (s, padbuf, NULL); /* could be more efficient */
		g_free (s);
		s = temp_string;
		++offset;
	    }
	}
	if (chunk && string && strlen (string) )
	{
	    /*[*/
	    /* add left delimitator*/
	    offset ++ ;
	    *start_offset = offset;
	    temp_string = g_strconcat (s, startbuf, NULL);
	    g_free (s);
	    s = temp_string;
	}
	/*add fill between left part of the obj and location where text begins*/
        if  (align_flags & SRW_ALIGNF_HSP_ADD_EMBEDDED) 
	{
	    /*__*/
	    if (chunk)
		end_padding = (chunk->text_bounds.x - clipping_rectangle.x) / 
			    pixels_per_column - leading; 
	    else
		end_padding = 0;
	    while (offset < end_padding) 
	    {
		temp_string = g_strconcat (s, padbuf, NULL); /* could be more efficient */
		g_free (s);
		s = temp_string;
		++offset;
	    }
	}
	/*add text : OK*/
	if (chunk && string && strlen (string))
	{
	    temp_string = g_strconcat (s, string, NULL);
	    g_free (s);
	    s = temp_string;
	    offset += g_utf8_strlen (string, -1);
	}

	/*--*/
        if  (align_flags & SRW_ALIGNF_HSP_ADD_EMBEDDED)
	{
	    if (chunk)
		end_padding = (chunk->clip_bounds.x + chunk->clip_bounds.width - clipping_rectangle.x) /
			    pixels_per_column - leading;
	    else
		end_padding = 0;	
	    while (offset < end_padding )/*-1*/ 
	    {
		temp_string = g_strconcat (s, padbuf, NULL); /* could be more efficient */
		g_free (s);
		s = temp_string;
		++offset;
	    }
	}
	/*]*/
	/* add right delimitator*/
	if (chunk && string && strlen (string) )
	{
	    temp_string = g_strconcat (s, endbuf, NULL);
	    g_free (s);
	    s = temp_string;
	}
	return s;
}

static char*
srw_text_chunk_to_string (SRWTextChunk 	*chunk, 
		    	 glong 		offset,
		        SRWAccLine	*acc_line)
{
	glong 		start_offset 	= 0;
	char 		*s 		= NULL;
	
	if (chunk /*&& chunk->string*/) 
	{
		s = srw_text_chunk_get_clipped_string (chunk);
	}
	s = srw_text_chunk_pad_string (chunk, s, offset, &start_offset,"| |");
/*
	fprintf (stderr,"\npad string %s",s);
*/
	srw_acc_line_from_string (acc_line, s, chunk, offset, start_offset);

	return s;
}
gint 
srw_text_chunk_compare_layer (gconstpointer  a,
  	            	      gconstpointer  b)
{
    int 	rv = 0;
    SRWTextChunk* 	element_a = (SRWTextChunk *)a;
    SRWTextChunk* 	element_b = (SRWTextChunk *)b;
    
    
    if (element_a && element_b)
    {
	rv =  (element_a->z_order) -
	      (element_b->z_order);
    
	if (rv)
	    return rv;
	else
	{
	    rv =  (element_a->layer) -
	    	  (element_b->layer);
    	    if (rv)
		return rv;
	    else
		return ( element_a->id - element_b->id);
	}
    }
    else 
	return -1;
}

/*______________________</SRWTextChunk methods>__________________________________*/



/*____________________< SRWTextChunkList methods>________________________________*/

#ifdef SRW_TEXT_CHUNK_LIST_DEBUG_
static void
srw_text_chunk_list_debug (GList	*iter)
{
	SRWTextChunk *chunk;
	fprintf (stderr,"\n<DEBUG>");
	while (iter) 
	{
		chunk = (SRWTextChunk *)iter->data;
		if (chunk)
		fprintf (stderr, "\n%d Chunk %40s id %4d, z_order %4d, layer %4d, clip %4ld-%4ld [%4ld], text %4ld-%4ld [%4ld] \ns%d e%d baseline %d",
			 chunk->is_text,
			 chunk->string,
			 chunk->id,
			 chunk->z_order,
			 chunk->layer,
			 chunk->clip_bounds.y,
			 chunk->clip_bounds.y + chunk->clip_bounds.height,
			 chunk->clip_bounds.height,
			 chunk->text_bounds.y,
			 chunk->text_bounds.y + chunk->text_bounds.height,
			 chunk->text_bounds.height,
			 chunk->start_offset,
			 chunk->end_offset,
			(int)( chunk->text_bounds.y + chunk->text_bounds.height *
			SRW_BASELINE_RATIO));
		iter = iter->next;
	}
	fprintf (stderr,"\n</DEBUG>");
}
#endif

#ifdef SRW_TEXT_CHUNK_DEBUG

static void
print_chunk_debug (SRWTextChunk *chunk, char *opname, GList *prev, GList *next)
{
	fprintf (stderr, "%sing chunk %s between %s and %s; %ld-%ld\n",
		 opname,
		 chunk->string,
		 (prev ? ((SRWTextChunk *) prev->data)->string : "<null>"),
		 (next ? ((SRWTextChunk *) next->data)->string : "<null>"),
		 chunk->clip_bounds.x,
		 chunk->text_bounds.x + chunk->text_bounds.width);
}
#define PRINT_CHUNK_DEBUG(a, b, c, d) print_chunk_debug(a, b, c, d) 

#endif

static GList *
srw_text_chunk_list_split_insert (GList		*chunk_list, 
			    	  GList 	*iter, 
			          SRWTextChunk	*chunk)
{
	SRWTextChunk *iter_chunk = iter->data;
	SRWTextChunk *iter_copy = srw_text_chunk_clone (iter_chunk);
#ifdef SRW_CLIPPING_DEBUG
	fprintf (stderr, "list_split_insert :***clip insert of %s into %s\n", 
		 chunk->string ? chunk->string : "<null>",
		 iter_chunk->string ? iter_chunk->string : "<null>");
#endif	
	chunk_list = g_list_insert_before (chunk_list, 
					   iter, 
					   iter_copy);
	srw_text_chunk_tail_clip (iter_copy,	/*bottom*/ 
			          chunk		/*top   */);
	chunk_list = g_list_insert_before (chunk_list, 
					   iter, 
					   chunk);
	srw_text_chunk_head_clip (iter_chunk,	/*bottom*/ 
				  chunk		/*top   */);

	return chunk_list;
}


static GList *
srw_text_chunk_list_head_clip (GList	*text_chunk_list,
			   SRWTextChunk	*chunk,
			   GList	*next)
{
	GList *target, *iter = next, *prev;
	prev = iter->prev;
	if (chunk->string && strlen (chunk->string)) { 
		text_chunk_list =
			g_list_insert_before (text_chunk_list, 
					      next, 
					      chunk);
	}
	while (iter && 
	       CHUNK_BOUNDS_SPANS_END (chunk, 
	    			      (SRWTextChunk *)iter->data)
	       ) 
	{
#ifdef SRW_CLIPPING_DEBUG
			fprintf (stderr, "\nlist_head_clip : deleting %s\n",
				 ((SRWTextChunk *)iter->data)->string);
#endif			
	    target = iter;
	    iter = iter->next;
	    srw_text_chunk_free ( ( (SRWTextChunk *)(target->data)));		  

	    text_chunk_list = g_list_delete_link (text_chunk_list, 
						  target);


	}
	if (iter && 
	    !CHUNK_BOUNDS_END_BEFORE_START (chunk,
					    (SRWTextChunk *)iter->data)
	   ) 
	{
	    srw_text_chunk_head_clip ( (SRWTextChunk *)iter->data,
				    chunk);
	}
	if (prev &&
	    !CHUNK_BOUNDS_AFTER_END (chunk,
				     (SRWTextChunk *)prev->data)
	    ) 
	{
	    srw_text_chunk_tail_clip ( (SRWTextChunk *)prev->data,/*bottom*/
					chunk			 /*top    */);
	}
	
	return text_chunk_list;
}

static GList *
srw_text_chunk_list_clip_and_insert (GList	*text_chunk_list,
				 SRWTextChunk	*chunk,
				 GList		*prev,
				 GList		*next)
{
#ifdef SRW_CLIPPING_DEBUG
/*	if (chunk->string) */
		fprintf (stderr, "\nclip-and-insert for %s, between %s (%ld) and %s (%ld)\n",
			 chunk->string,			 
		 	 (prev && ((SRWTextChunk *)prev->data)->string ? ((SRWTextChunk *)prev->data)->string : "<null>"),
			 (prev ? ((SRWTextChunk *)prev->data)->text_bounds.x : -1),
		 	 (next && ((SRWTextChunk *)next->data)->string ? ((SRWTextChunk *)next->data)->string : "<null>"),
 			 (next ? ((SRWTextChunk *)next->data)->text_bounds.x : -1));
#endif
	/* cases: */
	if (!prev && !next) 
	{ /* first element in, no clip needed */
	    text_chunk_list = g_list_append (text_chunk_list, 
					     chunk);
	} 
	else 
	{ 
	    /* check for clip with prev */
	    /* if we split the prev */
	    if (prev &&
		TEXT_CHUNK_BOUNDS_WITHIN (chunk, /*top*/
					  (SRWTextChunk *) prev->data)/*bottom*/
		) 
	    {
		text_chunk_list = srw_text_chunk_list_split_insert (text_chunk_list,
								prev, 
								chunk);
	    } 
	    else 
		if (next) 
		{ 
		    /* we split the 'next' element */
		    if (TEXT_CHUNK_BOUNDS_WITHIN (chunk, 
						  (SRWTextChunk *)next->data)) 
		    {
			text_chunk_list = srw_text_chunk_list_split_insert ( 
						text_chunk_list,
					    	next/*bottom*/, 
					        chunk/*top*/);
		    } 
		    else 
		    {
			/* do an insert +  head clip */
			text_chunk_list = srw_text_chunk_list_head_clip (
						text_chunk_list,
						chunk,
						next);
		    }
		} 
		else 
		{
		    if (!CHUNK_BOUNDS_AFTER_END (chunk,
						 (SRWTextChunk *) prev->data)) 
		    {
			srw_text_chunk_tail_clip (prev->data, chunk);
		    }
		    text_chunk_list = g_list_append (text_chunk_list, 
						     chunk);
		}
	}

	return text_chunk_list;
}

static GList *
srw_text_chunk_list_insert_chunk (GList *text_chunk_list, 
			      SRWTextChunk *chunk)
{
	GList *iter = g_list_first (text_chunk_list);
	SRWTextChunk *iter_chunk = NULL;

#ifdef SRW_CLIPPING_DEBUG
	fprintf (stderr, "\nsrw_text_chunk_list_insert_chunk: chunk %s", chunk->string);
#endif	
	if (!chunk->string) 
	{    
	    return text_chunk_list;
	}
	
	do 
	{
	    if (iter) 
		iter_chunk = (SRWTextChunk *) iter->data;
            /* if we're ahead of the current element */
	    if (!iter) 
	    {
		text_chunk_list = srw_text_chunk_list_clip_and_insert (
					 text_chunk_list,
					 chunk,
					 iter,
					 NULL);
		break;
	    } 
	    else 
		if (CHUNK_BOUNDS_BEFORE_START (chunk, 
					       iter_chunk)) 
		{
		    text_chunk_list = srw_text_chunk_list_clip_and_insert (
					text_chunk_list,
					chunk,
					iter->prev,
					iter);
		    break;
		} 
		else 
		    if (!iter->next ) 
		    {
			text_chunk_list = srw_text_chunk_list_clip_and_insert (
					    text_chunk_list,
					    chunk,
					    iter,
					    NULL);
			break;
		    }
	    if (iter) 
		iter = iter->next;
	} while (iter);

/*	fprintf (stderr, "list_insert_chunk has %d chunks\n",
			  g_list_length (text_chunk_list) );
*/
	return text_chunk_list;
}

static char*
srw_text_chunk_list_to_string (GList *iter, 
				SRWAccLine *acc_line,
				gint line_empty )
{
	char *s = NULL;
	char *string, *temp_string = NULL;
	SRWTextChunk *chunk = NULL;

	s = g_strdup ("|");
	
	acc_line->is_empty = line_empty;
	while (iter) 
	{
		chunk = (SRWTextChunk *)iter->data;
		if (chunk) {
			string = srw_text_chunk_to_string (chunk, g_utf8_strlen (s, -1) , acc_line);
			if (string)
			{
			    temp_string = g_strconcat (s, string, NULL);
			    g_free (s);
			    s = temp_string;
			}
			
		}
		else
		{
		    fprintf (stderr, "\nlist_to_string : chunk is NULL");
		}
		iter = iter->next;
	}

	string = srw_text_chunk_to_string (NULL, g_utf8_strlen (s, -1), acc_line);
	if (string)
	{
	    temp_string = g_strconcat (s, string, NULL);
	    g_free (s);
	    s = temp_string;
	}

	temp_string = g_strconcat (s,"|",NULL);
	g_free (s);
	s = temp_string;
	return s;
}

/*_________________________</SRWTextChunkList methods>________________________*/


/*____________________________< SRWLine methods>______________________________*/
SRWLine *
srw_line_add_text_chunk (SRWLine	*line, 
			 SRWTextChunk 	*chunk)
{
    SRWLine 		*new_line	= NULL;
    SRWTextChunk 	*chunk_clone 	= NULL;
    
    if (!line)
    {
    	new_line = g_new0 (SRWLine, 1);
	new_line->is_empty 	= FALSE;
	new_line->cached 	= FALSE;
	new_line->is_focused = FALSE;
    }
    else
	new_line = line;
    
    new_line->is_focused = new_line->is_focused || chunk->is_focused;
    chunk_clone = srw_text_chunk_clone (chunk);
    new_line->cells = g_list_append (new_line->cells, 
					    chunk_clone);


    return new_line;
}

static gchar *
srw_line_toplevel_composite (SRWLine	*line, 
			     SRWAccLine	*acc_line)
{
	GList 		*chunk_list 	= NULL, 
			*iter		= NULL;
	SRWTextChunk 	*chunk		= NULL;
    
	    line->cells = g_list_sort (line->cells, srw_text_chunk_compare_layer);
	    iter = line->cells;
	
	    while (iter) 
	    {
		chunk = (SRWTextChunk *) iter->data;
		if (chunk) 
		{
#ifdef SRW_CLIP_DEBUG		
		    fprintf (stderr, "inserting chunk <%s>\n",
				 chunk->string ? chunk->string : "<null>");
#endif
		    chunk_list = srw_text_chunk_list_insert_chunk (chunk_list,
							       chunk);
		}
		iter = iter->next;
	    }
	    line->cells = chunk_list;	
	return srw_text_chunk_list_to_string (chunk_list, 
					  acc_line,
					  line->is_empty);
}

static char *
srw_line_generate_output (SRWLine	*line, 
			    int		*y1,
			    int		*y2,
			    SRWAccLine	*acc_line)
{
	char 		*string = NULL;
	    
	    if (line) 
	    {
	    	string = srw_line_toplevel_composite (line, acc_line);
		*y1 = line->y1;
		*y2 = line->y2;
	    }
	return string;
}

/*____________________________< SRWLines methods>______________________________*/
GList*
srw_lines_create_from_elements (GList *elements, glong align_flags)
{
    SRWTextChunk	*chunk_crt	= NULL,		
			*chunk		= NULL,
			*chunk_clone	= NULL;
    SRWLine		*line		= NULL, 
			*prev_line	= NULL;

    static int		n_lines;
    gboolean 		new_line 	= FALSE;		
    GList		*lines 		= NULL,
    			*iter 		= NULL;
    int 		baseline = 0;

    n_lines = 0;
    
    while (elements)
    {
	chunk_crt = (SRWTextChunk *) elements->data;
	if (chunk_crt && !chunk_crt->text_bounds.is_empty)
	    baseline = chunk_crt->text_bounds.y + 
		       chunk_crt->text_bounds.height * SRW_BASELINE_RATIO;
	else
	    break;
	    
	if (!line)
	    new_line = TRUE;
	else
	    if (baseline - line->baseline < SRW_DELTA_BASELINE_THRESHOLD)
		new_line = FALSE;
	    else
	    	new_line = TRUE;
	  
	if (new_line)
	{/*new line*/
	    n_lines++;
	
	    if (n_lines > 0)
	    {
/*
    		fprintf (stderr, "\n%s %4d - lines %d ", 
			__FILE__,
			__LINE__,
			n_lines);
*/
	    if (prev_line && line)
	    {
		if ((line->layer < prev_line->layer) &&
		    (line->layer != SPI_LAYER_WINDOW) &&
		    ( prev_line->layer != SPI_LAYER_WINDOW))
		{
		    iter = g_list_first (prev_line->cells);
		    while (iter)
		    {
			chunk = (SRWTextChunk *)iter->data;
			if (!chunk->dummy)
			{
			    chunk_clone = srw_text_chunk_clone (chunk);
			    chunk_clone->text_bounds.x = chunk_clone->clip_bounds.x; 
			    chunk_clone->text_bounds.width = chunk_clone->clip_bounds.width; 
			    chunk_clone->dummy = TRUE;
			    chunk_clone->id =0;
			
			    if (chunk_clone->string)
			    {
			        g_free (chunk_clone->string);
				chunk_clone->string = NULL;
				chunk_clone->string = g_strdup(" ");
			    }

			    if (chunk->layer == prev_line->layer &&
				chunk->clip_bounds.y + chunk->clip_bounds.height > line->y1 
				/*+ (line->y2-line->y1)* SRW_BASELINE_RATIO*/)
				line->cells = g_list_append (line->cells, chunk_clone);
			    else
			    {
			        srw_text_chunk_free (chunk_clone);
				chunk_clone = NULL;
		    	    }
			    iter = iter->next;
			}
		    }
		}

		if ((line->layer > prev_line->layer) &&
		    (line->layer != SPI_LAYER_WINDOW) &&
		    (prev_line->layer != SPI_LAYER_WINDOW) )
		{
		    iter = g_list_first (line->cells);
		    while (iter)
		    {
			chunk = (SRWTextChunk *)iter->data;
			if (!chunk->dummy)
			{
			    chunk_clone = srw_text_chunk_clone (chunk);
			    chunk_clone->text_bounds.x = chunk_clone->clip_bounds.x;
			    chunk_clone->text_bounds.width = chunk_clone->clip_bounds.width;
			    chunk_clone->dummy = TRUE;
			    chunk_clone->id =0;
			
			    if (chunk_clone->string)
			    {
				g_free (chunk_clone->string);
				chunk_clone->string = NULL;
				chunk_clone->string = g_strdup(" ");
			    }

			    if (chunk->layer == line->layer &&
				chunk->clip_bounds.y < prev_line->y2 
				/*- (line->y2-line->y1)* SRW_BASELINE_RATIO*/)
				prev_line->cells = g_list_append (prev_line->cells, chunk_clone);
			    else
			    {
				srw_text_chunk_free (chunk_clone);
				chunk_clone = NULL;
			    }
			    iter = iter->next;
			}
		    }
		}
/*
	    fprintf (stderr,"\n -------PREV_LINE--------------------------");
	    fprintf (stderr,"\n line->y1 %d, line->y2 %d line->baseline %d line->layer %d",
			    prev_line->y1,
			    prev_line->y2,
			    prev_line->baseline,
			    prev_line->layer);
	    srw_text_chunk_list_debug (prev_line->cells);
	    fprintf (stderr,"\n --------------------------------------\n");

	    fprintf (stderr,"\n -------LINE--------------------------");
	    fprintf (stderr,"\n line->y1 %d, line->y2 %d line->baseline %d line->layer %d",
			    line->y1,
			    line->y2,
			    line->baseline,
			    line->layer);
	    srw_text_chunk_list_debug (line->cells);
	    fprintf (stderr,"\n --------------------------------------\n");
OC*/
	    }

	    prev_line = line;	
	
	    line = NULL;
	    line = srw_line_add_text_chunk (line,
						chunk_crt);
	    	
	    line->y1 = chunk_crt->text_bounds.y;
	    line->y2 = chunk_crt->text_bounds.y + chunk_crt->text_bounds.height;
	    line->baseline = line->y1 + 
			    ( chunk_crt->text_bounds.height) * SRW_BASELINE_RATIO;
	    if (line->layer)
		line->layer = MIN (line->layer, chunk_crt->layer );
	    else
		line->layer = chunk_crt->layer;
	    lines = g_list_append (lines, (SRWLine *) line);
	
/*	    fprintf (stderr,"\n -------NEW LINE--------------------------");
	    fprintf (stderr,"\n line->y1 %d, line->y2 %d line->baseline %d line->layer %d line->is_focused %d",
			    line->y1,
			    line->y2,
			    line->baseline,
			    line->layer,
			    line->is_focused);
	    srw_text_chunk_list_debug (line->cells);
	    fprintf (stderr,"\n --------------------------------------\n");
*/	    	
	    }
	    else
		fprintf (stderr, "\nThis should not happen");
	}
	else
	{/*same_line*/

		prev_line = line;
		line->y1 = MIN (prev_line->y1, chunk_crt->text_bounds.y );
		line->y2 = MAX (prev_line->y2,chunk_crt->text_bounds.y + chunk_crt->text_bounds.height);
		line->baseline = (line->baseline + baseline )/2;
		if (line->layer)
		    line->layer = MIN (line->layer, chunk_crt->layer );
		else
		    line->layer = chunk_crt->layer;
		
		line = srw_line_add_text_chunk (prev_line, 
						chunk_crt);

/*	    fprintf (stderr,"\n ----------SAME LINE------------------");
	    fprintf (stderr,"\n line->y1 %d, line->y2 %d line->baseline %d line->layer %d",
			    line->y1,
			    line->y2,
			    line->baseline,
			    line->layer);
	    srw_text_chunk_list_debug (line->cells);
	    fprintf (stderr,"\n --------------------------------------\n");
*/		
	}

	elements = elements->next;					 
    }

    return lines;
}


int 
srw_lines_compare_line_number (gconstpointer	a,
  	    	    	       gconstpointer 	b)
{
    int 		rv = 0;
    SRWLine		*element_a = (SRWLine *) a;
    SRWLine		*element_b = (SRWLine *) b;
    
    
    if (element_a && element_b)
    {
	rv = SRW_BASELINE_RATIO * element_a->y2 + 
	     (1 -  SRW_BASELINE_RATIO) * element_a->y1 - 
	     SRW_BASELINE_RATIO * element_b->y2 - 
	     (1 -  SRW_BASELINE_RATIO) * element_b->y1;
	return rv;
    }
    else 
	fprintf (stderr,"\nThis should not happen.");
    return -1;
}


static int
srw_lines_get_n_lines (GList	**lines,
		       glong	align_flags)
{ 
    int 	n_lines = 0,
		height = 0,
		i = 0,
		j = 0,
		y1 = 0,
		y2 = 0,
		empty_lines = 0;
    SRWLine 	*line_crt = NULL,
		*line_nxt = NULL;
    SRWLine	*aux_line = NULL;
    GList 	*iter =NULL,
		*aux_lines = NULL;

    n_lines = g_list_length (*lines); 
    
    iter = g_list_first (*lines);
    
    if (align_flags & SRW_ALIGNF_VSP_ADD)
    {
	if (align_flags & SRW_ALIGNF_VSP_ADD_LEADING)
	    i = -1;
	while (iter)
	{
	    if (i == -1)
	    {
		line_crt = (SRWLine *)iter->data;
		
		i++;
		y1 = clipping_rectangle.y;
		y2 = line_crt->y1;
		height = line_crt->y2 - line_crt->y1;
	    }
	    else    
	    {
    		if (!(align_flags & SRW_ALIGNF_VSP_ADD_EMBEDDED))		    
		{
		    if (i == 0)
		    {
			iter = g_list_nth (*lines, g_list_length (*lines) - 1 );
			i = n_lines - 1;
		    }
		}


		line_crt = (SRWLine *)iter->data;

		if (line_crt )
		{

		    i++;
		    y1 = line_crt->y2;
		}
		else
		    break;
		if (iter->next && (i != n_lines))
		{
		    line_nxt = (SRWLine *)iter->next->data;
		    y2 = line_nxt->y1;
	    	    height =( (line_crt->y2 - line_crt->y1) +
			  (line_nxt->y2 - line_nxt->y1)) / 2;
		}
		else
		{
		    if (align_flags & SRW_ALIGNF_VSP_ADD_TRAILING)
		    {
			y2 = clipping_rectangle.y + clipping_rectangle.height;
	    		height = line_crt->y2 - line_crt->y1;
		    }
		    else
			break;
		}
	    }
	
		    empty_lines = (int)( (y2 - y1)/height);

		    if (!empty_lines && ((y2 - y1) % height > height/2))
			empty_lines++;
		    if (empty_lines > 0)
		    {
			if (align_flags & SRW_ALIGNF_VSP_COUNT_LINES)
			{
				aux_line = g_new0 (SRWLine, 1);
				aux_line->is_empty = empty_lines;
				aux_line->y1 = y1;
				aux_line->y2 = y2; 
				aux_lines = g_list_append (aux_lines, (SRWLine *) aux_line);
			}
			else
			{
			    for (j = 1; j <= empty_lines; j++ )
			    {
				aux_line = g_new0 (SRWLine, 1);
				aux_line->is_empty = TRUE;
				aux_line->y1 = (y1 + (j-1) * height);
				if (j == empty_lines)
				{
			    	    aux_line->y2 = y2; 
				}
				else
				{	
				    aux_line->y2 = (y1 + j * height); 
				}
				aux_lines = g_list_append (aux_lines, (SRWLine *) aux_line);
			    }
			}
		    }
		    if (i > 0)
			iter = iter->next;

	    if (i == n_lines ) break;
	}    
    }

    iter = g_list_first (aux_lines);
    while (iter)
    {
	line_crt = (SRWLine *) iter->data;
	*lines = g_list_insert_sorted (*lines, line_crt, srw_lines_compare_line_number);
	iter = iter->next;
    }
    g_list_free (aux_lines);

    n_lines = g_list_length (*lines);
/*
    fprintf (stderr,"\nn_lines = %d - len %d ",n_lines, g_list_length (lines) );
*/
    return n_lines;
}




static void
srw_lines_free (GList	*lines) 
{
    int 		i = 0;
    SRWLine 		*line;
    SRWTextChunk	*chunk;
    GList		*lines_aux = lines;
    
    
#ifdef SRW_CLEAN_UP_DEBUG
    fprintf (stderr, "\n\n<FREE LINES> %p",lines);
#endif
    while (lines)
    {
	i++;
	line = (SRWLine *)lines->data;
#ifdef SRW_CLEAN_UP_DEBUG
	fprintf (stderr, "\n\n<FREE LINE %d>",i);
	srw_text_chunk_list_debug (line->cells);
#endif
	if (line->string)
	{
	    g_free (line->string);
	    line->string = NULL;
	}
	srw_acc_line_free (line->acc_line);
	line->acc_line = NULL;
	while (line->cells) 
	{
	    chunk = (SRWTextChunk *)(line->cells->data);
	    if (chunk)
	    {
		srw_text_chunk_free (chunk);
	    }
	    line->cells = line->cells->next;
	}
	g_list_free (line->cells);
	line->cells = NULL;	
	lines = lines->next;
    }
#ifdef SRW_CLEAN_UP_DEBUG    
    fprintf (stderr, "\n\n</FREE LINES>");
#endif
    g_list_free (lines_aux);
    lines = NULL; 
}
/*____________________________< /SRWLines methods>______________________________*/


/*________________________< SRWElements methods>__________________________________*/

gint 
srw_elements_compare_text_chunk_y (gconstpointer	a,
  	    	    	    	    gconstpointer 	b)
{
    int 		rv = 0;
    SRWTextChunk* 	element_a = (SRWTextChunk *) a;
    SRWTextChunk* 	element_b = (SRWTextChunk *) b;
    
    
    if (element_a && element_b)
    {
	rv = element_a->text_bounds.y + 
	     element_a->text_bounds.height * SRW_BASELINE_RATIO  - 
	    (element_b->text_bounds.y + 
	    element_b->text_bounds.height * SRW_BASELINE_RATIO);
    	if (rv)/*-Treshold -> + TRESHOLD - equal???*/
	    return rv;
	else
	    return ( element_a->clip_bounds.x - element_b->clip_bounds.x);
    }
    else 
	return -1;
}

static void
srw_elements_from_accessible (Accessible 	*accessible,  
			      Accessible	*focused_accessible,
			      SRWBoundaryRect	*parentClipBounds[],
			      gboolean		parent_selected,
			      int		toplevel_z_order,
			      int 		parent_layer)
{
	Accessible 		*child;
	AccessibleRole		role;
	AccessibleComponent 	*component,
				*child_component;
	AccessibleStateSet	*states;
	SRWBoundaryRect 	bounds, child_bounds;
	SRWBoundaryRect		**clip_bounds;
	SRWTextChunk 		*text_chunk;	
	int 			n_children, 
				child_n,
				layer = -1;	
	long 			offset = 0;

	Accessible_ref (accessible);
        clip_bounds = srw_boundary_rect_clone (parentClipBounds);

	states = Accessible_getStateSet (accessible);

	/*SELECTED objects don't have the SHOWING state, but they are visible*/
	if (AccessibleStateSet_contains (states, SPI_STATE_SHOWING)  ||
	    AccessibleStateSet_contains (states, SPI_STATE_SELECTED)  ||
	    AccessibleStateSet_contains (states, SPI_STATE_TRANSIENT) ) 
	{
	    if (Accessible_isComponent (accessible)) 
	    {
		role = Accessible_getRole (accessible);
		component = Accessible_getComponent (accessible);
		layer = AccessibleComponent_getLayer (component);
		bounds = *clip_bounds[layer];
		if (!bounds.is_empty) 
		{

			AccessibleComponent_getExtents (component,
							&bounds.x,
							&bounds.y,
							&(bounds.width),
							&(bounds.height),
							SPI_COORD_TYPE_SCREEN);							    
			bounds.role = role;
			if (clip_bounds[layer])
				srw_boundary_rect_clip (&bounds, clip_bounds[layer]);
			offset = 0;

			do
			{
    				text_chunk = srw_text_chunk_from_accessible 
					    (accessible, 
					     &bounds, 
					     offset,
					     toplevel_z_order);
				if (text_chunk)
				{
					gboolean only_spaces = TRUE;
					gint i, cnt;
						
					if (accessible == focused_accessible)
					{
					    text_chunk->is_focused = TRUE;
					}
					offset = text_chunk->end_offset + 1;
					
					/* WORKAROUND for bug 144404:
					   a line of only spaces should be ignored 
					   (treated as an empty line) */
					if (text_chunk->string != NULL)
					{
					    cnt = strlen (text_chunk->string);   
					    for (i = 0; i < cnt; i++)
						if ((guint16)*(text_chunk->string + i) != 0x20 &&
					    	    (guint16)*(text_chunk->string + i) != 0x160)   
						{
						    only_spaces = FALSE;
						    break;
						}
					}	
					if (text_chunk->string && !only_spaces)
					{
						id++;
						text_chunk->id 		= id;
						/*special case for the window-title :
						 There is no other way to determine it */
						if (text_chunk->layer == SPI_LAYER_WINDOW)
						{
						    child = Accessible_getChildAtIndex (accessible, 0);
						    child_component = Accessible_getComponent (child);
						    AccessibleComponent_getExtents (child_component,
										    &bounds.x,
										    &bounds.y,
										    &(bounds.width),
										    &(bounds.height),
										    SPI_COORD_TYPE_SCREEN);							    
						    text_chunk->text_bounds.height = bounds.y - text_chunk->text_bounds.y; 
						    AccessibleComponent_unref (child_component);
						    Accessible_unref (child);
						}
						/* special logic for the POPUP layer :
						 if the menu item was selected the menu items persist to have
						 SHOWING and VISIBLE states, even if they are not REALLY VISIBLE*/
						if (text_chunk->layer == SPI_LAYER_POPUP && 
						    parent_layer == SPI_LAYER_POPUP)
						{
						    if (!parent_selected) 
						    {
							srw_text_chunk_free (text_chunk);					
							break;
						    }
						}
						if (!text_chunk->clip_bounds.is_empty && 
						    text_chunk->text_bounds.height > 0)
						{
						    elements = g_list_insert_sorted (elements, 
										text_chunk, 
										srw_elements_compare_text_chunk_y);
						}
						else
						    srw_text_chunk_free (text_chunk);					
					}
					else
					srw_text_chunk_free (text_chunk);					
				}


			}
			while (text_chunk && text_chunk->is_text);
		} 
		Accessible_unref (component);
	    }
	    if (AccessibleStateSet_contains (states, SPI_STATE_ICONIFIED))
		return;
	/*
	 * we always descend into children in case they are in a higher layer
	 * this can of course be optimized for the topmost layer...
	 * but nobody uses that one! (SPI_LAYER_OVERLAY)
	 */
	    n_children = Accessible_getChildCount (accessible);
	    parent_selected = AccessibleStateSet_contains (states, SPI_STATE_SELECTED) || 
	        	(!AccessibleStateSet_contains (states, SPI_STATE_SELECTABLE) &&
		          AccessibleStateSet_contains (states, SPI_STATE_SHOWING));
	    parent_layer = layer;
	    if (!AccessibleStateSet_contains (states, SPI_STATE_MANAGES_DESCENDANTS))
	    {

		for (child_n = 0; child_n < n_children; ++child_n) 
		{
		    child = Accessible_getChildAtIndex (accessible, child_n);
		    srw_elements_from_accessible (child, 
						  focused_accessible,
						  clip_bounds,
						  parent_selected,
						  toplevel_z_order,
						  parent_layer); 
		    Accessible_unref (child);
		}
	    }
	    else
	    {
		int x, y, step_y;
		gboolean support_at_point = FALSE;
		GList 	*children = NULL,  *iter = NULL;

		component = Accessible_getComponent (accessible);
		x = bounds.x ;
		y = bounds.y ;

		child = AccessibleComponent_getAccessibleAtPoint 
				(component,
			        bounds.x + bounds.width/2,
				bounds.y + bounds.height/2,
				SPI_COORD_TYPE_SCREEN);
		if (child)
		{
			Accessible_unref (child);
			support_at_point = TRUE;
		}				
		else
		{
		    sru_warning ("AccessibleComponent_getAccessibleAtPoint  is NOT working");		    			    			
		}
#ifdef SRW_MANAGE_DESCENDENTS
		sru_message ("parent [%d, %d, %ld, %ld] ", x, y, bounds.width, bounds.height);
#endif
		while (support_at_point && component && y < bounds.y + bounds.height )
		{
		    step_y = -1;
		    while (x < bounds.x + bounds.width)
		    {
			child = AccessibleComponent_getAccessibleAtPoint 
				(component,
			        x,
				y,
				SPI_COORD_TYPE_SCREEN);
			if (child)
			{
#ifdef SRW_MANAGE_DESCENDENTS			
			    sru_message ("Manage descendents - child %p %s", child,  Accessible_getName (child));		    			    
			    sru_message ("next column :  %d %d", x,y);
#endif			
			    child_component =  Accessible_getComponent (child);
			    if (child_component)
			    {
				AccessibleComponent_getExtents (child_component,
							&child_bounds.x,
							&child_bounds.y,
							&(child_bounds.width),
							&(child_bounds.height),
							SPI_COORD_TYPE_SCREEN);	
#ifdef SRW_MANAGE_DESCENDENTS			
				sru_message ("child [%ld, %ld, %ld, %ld] ", 
				    child_bounds.x, 
				    child_bounds.y,
				    child_bounds.width, 
				    child_bounds.height);													    
#endif
				Accessible_unref (child_component);
				if (step_y == -1)
				{
			    	    step_y = child_bounds.height;
#ifdef SRW_MANAGE_DESCENDENTS			
				    sru_message ("step_y = %d",step_y);
#endif
				}
				if ( child_bounds.x > 0)
			    	    x = child_bounds.x + child_bounds.width;			    
				else
				{
			    	    x += SRW_MANAGE_DESCENDATS_STEP;
				    Accessible_unref (child);			    		    
				    break;
				}
				children = g_list_prepend (children, child);
			} /*if (child_component)*/

			x += SRW_MANAGE_DESCENDATS_STEP; 
		    }/*if (child)*/
		    else
		    {
			static int break_cond = 20;
			if (step_y < 0) 
			    step_y = SRW_MANAGE_DESCENDATS_STEP;
			if (--break_cond < 0)
			{
			    break;
			}
			else
			{
				x += SRW_MANAGE_DESCENDATS_STEP;
#ifdef SRW_MANAGE_DESCENDENTS							
				sru_message ("Manage descendents - NO child %p", child);		    			    
				sru_message ("Point %d %d", x,y);
#endif
			}
		    }		    
		    }/*while x*/
		y += step_y;
		x = bounds.x ;
#ifdef SRW_MANAGE_DESCENDENTS			
		sru_message ("next line: Point %d %d | step_y %d", x,y, step_y);
#endif
		}/*while y*/
	    Accessible_unref (component);

	    for (iter = g_list_first (children); iter; iter = iter->next) 
	    {
		child = (Accessible *) iter->data;

		srw_elements_from_accessible (child, 
						  focused_accessible,
						  clip_bounds,
						  parent_selected,
						  toplevel_z_order,
						  parent_layer); 
		Accessible_unref (child);			    			    

	    }
	    g_list_free (children);
	}/*else - MD*/

	    
	}/*if states*/

	AccessibleStateSet_unref (states);

	/*free the parent clip bounds */
	srw_boundary_rect_free (clip_bounds);
	Accessible_unref (accessible);
}


static void
srw_elements_free (GList *elements)
{
    SRWTextChunk	*chunk;
    
    if (!elements)
	return;
#ifdef SRW_CLEAN_UP_DEBUG    
    fprintf (stderr, "\n\n<FREE ELEMENTS>");
#endif
    while (elements) 
    {
	    chunk = (SRWTextChunk *)elements->data;
	    if (chunk)
	    {
		srw_text_chunk_free (chunk);
	    }
	    elements = elements->next;
    }
    g_list_free (elements);
    elements = NULL;	
#ifdef SRW_CLEAN_UP_DEBUG    
    fprintf (stderr, "\n</FREE ELEMENTS>");
#endif
}
/*________________________</SRWElements methods>__________________________________*/



/*_______________________________<AUX methods>______________________________________*/
GList *
srw_get_toplevels 	(Accessible 	*focused_acc, 
			 glong 		scope_flags)
{
    Accessible	*desktop, *crt;
    GList 	*toplevels = NULL;
			
    int 	n_toplevels,
		toplevel_n;
    
    
    srl_return_val_if_fail (focused_acc, FALSE);
    srl_return_val_if_fail (!Accessible_isApplication (focused_acc), FALSE);
    
    if (scope_flags & SRW_SCOPE_DESKTOP)
    {
	int n_apps = 0,
	    app_n;
	    	    
	/*desktop 0 is always the active one*/
    	desktop = SPI_getDesktop (0);
	n_apps = Accessible_getChildCount (desktop);
	for (app_n = 0; app_n < n_apps; ++app_n)
	{
	    Accessible *app = NULL;
	    app = Accessible_getChildAtIndex (desktop, app_n);
	    if (app)
	    {
		n_toplevels = Accessible_getChildCount (app);
		for (toplevel_n = 0; toplevel_n < n_toplevels; ++toplevel_n)
		{
		    Accessible *toplevel = NULL;
		    toplevel = Accessible_getChildAtIndex (app, toplevel_n);
		    if (toplevel && Accessible_isComponent (toplevel) )
		    {
			toplevels = g_list_prepend (toplevels, toplevel);
		    }
		    else
		    {
			Accessible_unref (toplevel);
			fprintf (stderr, "warning, app toplevel not a component\n");
		    }
		} /* end for (toplevel_n ...) */
		Accessible_unref (app);
	    } /* end if (app) */
	} /*end for (app_n ...) */

	return toplevels;
    }
	
    crt = focused_acc;
    Accessible_ref (crt);
    for ( ; ; )
    {
	Accessible *parent;
	parent = Accessible_getParent (crt);
	if (parent && Accessible_isApplication (parent))
	{
	    if (scope_flags & SRW_SCOPE_WINDOW)
	    {
		toplevels = g_list_prepend (toplevels, crt);
	    }
	    if (scope_flags & SRW_SCOPE_APPLICATION)
	    {
		n_toplevels = Accessible_getChildCount (parent);
	    	for (toplevel_n = 0; toplevel_n < n_toplevels; ++toplevel_n) 
	    	{
	        	Accessible *toplevel = Accessible_getChildAtIndex (parent, toplevel_n);
			toplevels = g_list_prepend (toplevels, toplevel);
		}
		Accessible_unref (crt);
	    }
	    Accessible_unref (parent);
	    
	    return toplevels;
	}
	if (!parent)
	{
	    srl_warning ("no object wich is application in parent line");
	    Accessible_unref (crt);
	    return NULL;
	}
	Accessible_unref (crt);
	crt = parent;
    }
	
    srl_assert_not_reached ();
    return NULL;
}
/*_______________________________</AUX methods>______________________________________*/

/*_______________________________< API>_______________________________________*/
/**
 * screen_review_init:
 * @clip_rectangle: a #SRRectangle representing the rectangle which is wanted to be splitted in lines. 
 * @alignment_flags: flags indicating what kind of arrangement are required .
 *
 * Call this function to initialize the screen review.
 **/
int
screen_review_init (SRRectangle *clip_rectangle,
		    SRObject	*focused_object,
		    glong 	alignment_flags,
		    glong 	scope_flags)
{

    Accessible 		*toplevel, *focused_accessible, *desktop;
    AccessibleComponent	*component;
    AccessibleStateSet	*states;

    SRWBoundaryRect	**clip_bounds;
    SRWBoundaryRect 	toplevel_bounds, desktop_bounds;

    GList 		*toplevels 	= NULL,
			*iter		= NULL;
    int 		i,
			n_lines = 0,
			z_order = G_MAXINT;
#ifdef SRW_BENCHMARKING
    GTimer 		*timer = g_timer_new ();
#endif


  /*new && init*/
    clip_bounds	 = srw_boundary_rect_new0 ();
    id = 0;
    pixels_per_column = MAX_PIXELS_PER_COLUMNS;
    lines_index = NULL;
    if (scope_flags & SRW_SCOPE_WINDOW)
    {
	clipping_rectangle.x 		= clip_rectangle->x;
	clipping_rectangle.y 		= clip_rectangle->y;
	clipping_rectangle.width 	= clip_rectangle->width;
	clipping_rectangle.height 	= clip_rectangle->height;
    }
    else
    {
	clipping_rectangle.x 		= desktop_bounds.x;
	clipping_rectangle.y 		= desktop_bounds.y;
	clipping_rectangle.width 	= desktop_bounds.width;
	clipping_rectangle.height 	= desktop_bounds.height;
    }
    align_flags			= alignment_flags;

    /* get desktop's size in order to clip the toplevels against it*/    
    desktop = SPI_getDesktop (0);
    if (desktop && Accessible_isComponent (desktop))
    {
    	component = Accessible_getComponent (desktop);
	AccessibleComponent_getExtents (component,
	    			        &desktop_bounds.x,
					&desktop_bounds.y,
					&desktop_bounds.width,
					&desktop_bounds.height,
					SPI_COORD_TYPE_SCREEN);
	AccessibleComponent_unref (component);
	Accessible_unref (desktop);
    }
    
  /* for each toplevel, ending with the active one(s),
   * clip against children.
   */
    focused_accessible = sro_get_acc_at_index (focused_object, 0);

    toplevels = srw_get_toplevels (focused_accessible, scope_flags);
    if (toplevels && g_list_first (toplevels))
    for (iter = g_list_first (toplevels); iter; iter = iter->next)
    {
	toplevel = (Accessible *) iter->data;
	if (Accessible_isComponent (toplevel))
	{
  	    /* make sure toplevel is visible and not iconified or shaded */
	    states = Accessible_getStateSet (toplevel);
	    /* iconofied windows are visible, too */
	    if (AccessibleStateSet_contains (states, SPI_STATE_VISIBLE) )
	    { 
		component = Accessible_getComponent (toplevel);
		z_order = AccessibleComponent_getMDIZOrder (component);
		AccessibleComponent_getExtents (component,
		    			        &toplevel_bounds.x,
						&toplevel_bounds.y,
						&toplevel_bounds.width,
						&toplevel_bounds.height,
						SPI_COORD_TYPE_SCREEN);
		toplevel_bounds.is_empty = FALSE;
		toplevel_bounds.role = -1;/*invalid role*/
    		srw_boundary_rect_clip (&toplevel_bounds, &desktop_bounds);

		for (i = 0; i < SPI_LAYER_LAST_DEFINED; ++i)
		{
		        *clip_bounds[i] = toplevel_bounds;
		}
		srw_elements_from_accessible (toplevel,
					  focused_accessible,
					  clip_bounds,
					  TRUE,
					  z_order,
					  -1);
		AccessibleComponent_unref (component);
	    }/*states*/
	    AccessibleStateSet_unref (states);
	}/*isComponent*/
	Accessible_unref (toplevel);
    }
#ifdef SRW_TEXT_CHUNK_DEBUG
	    srw_text_chunk_list_debug (elements);
#endif

    lines =  srw_lines_create_from_elements (elements, align_flags);
    if (lines)
    {
	n_lines = srw_lines_get_n_lines (&lines, align_flags);
    }

    g_list_free (toplevels);
    toplevels = NULL;

#ifdef SRW_BENCHMARKING
    g_timer_stop (timer);
    fprintf (stderr, "elapsed time = %f s\n", g_timer_elapsed (timer, NULL));
#endif

  return n_lines;
}

/**
 * screen_review_get_line:
 * @line_number: an integer .
 * @y1: the location to store the line coordinate.
 * @y2: the location to store the line coordinate.
 *
 * Return value: screen review accessible line. The resulting
 * value has already been allocated.
 **/
SRWAccLine *
screen_review_get_line (int line_number,
			int *y1,
			int *y2)
{
    char 	*string		= NULL;
    GList	*line		= NULL;
    SRWLine	*line_data	= NULL;
    SRWAccLine	*acc_line	= NULL;


    acc_line = srw_acc_line_new ();

    if (lines)
    {
	line = g_list_nth (lines, line_number - 1);
    }
    else
	return NULL;
    if (line && line->data)
    {
	line_data = (SRWLine *)line->data;
	if (!line_data)
	    return NULL;
	if (!line_data->cached)
	{
	    string = srw_line_generate_output (line_data, y1, y2, acc_line);
	    line_data->cached = TRUE;
	    line_data->acc_line = acc_line;
	    line_data->string = string;
	}
	else
	{
	    *y1 = line_data->y1;
	    *y2 = line_data->y2;
	    acc_line = line_data->acc_line;
	    string = line_data->string;
#ifdef SRW_ACC_LINE_CACHE	    
	    fprintf (stderr, "\ncached %d %d %p",*y1, *y2, acc_line);
#endif
	}
    }
    else
	return NULL;

#ifdef SRW_ACC_LINE_DEBUG
    if (acc_line)
    {
	SRWAccCell 	*acc_cell = NULL;
	int 		i;

	fprintf (stderr, "\nGET LINE len = %d",acc_line->srw_acc_line->len);

	for (i = 0; i < acc_line->srw_acc_line->len; i++)
	{
    	    acc_cell = g_array_index (acc_line->srw_acc_line, SRWAccCell *, i);
	    if (acc_cell)
		fprintf (stderr,"\nch =%s index=%d id=%d", acc_cell->ch, acc_cell->index, acc_cell->id);
	}
    }
    fprintf (stderr,"\n%d %d %s",acc_line->is_empty,line_data->is_focused, string);
#endif

    return acc_line;
}

gint
screen_review_get_focused_line (void)
{
    GList	*line		= NULL,
		*iter		= NULL;
    SRWLine	*line_data	= NULL;
    gint 	line_number	= 0;

    if (lines)
    {
	iter = g_list_first (lines);
	line_number = 1;
	while (iter)
	{

	    line_data = (SRWLine *) iter->data;
	    if (line_data->is_focused)
	    {
		line = iter;
		break;
	    }
	    line_number++;
	    iter = iter->next;
	}
    }
    else
    /* start from top if there is no focused object*/
	return 1;
    
    /* the focused object is not a really visible object so it won't be in any
    line; there is not focused_line, so start from top  */
    if (line_number > g_list_length (lines) )
	return 1;
    
    return line_number;
}

/**
 * screen_review_terminate:
 *
 * Cleans up all structures that were used.
 **/
void
screen_review_terminate (void)
{
    srw_lines_free (lines) ;
    lines = NULL;
    srw_elements_free (elements);
    elements = NULL;
    if (lines_index)
    {
	g_array_free (lines_index, TRUE);
	lines_index = NULL;
    }
}

/*_______________________________</API>_______________________________________*/
