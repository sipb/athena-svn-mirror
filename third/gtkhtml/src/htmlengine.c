/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1999 Anders Carlsson (andersca@gnu.org)
    Copyright (C) 1999, 2000, Helix Code, Inc.
    Copyright (C) 2001, Ximian Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

/* RULE: You should never create a new flow without inserting anything in it.
   If `e->flow' is not NULL, it must contain something.  */


#include <config.h>
#include "gtkhtml-compat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include "gtkhtml-embedded.h"
#include "gtkhtml-private.h"
#include "gtkhtml-stream.h"

#include "gtkhtmldebug.h"

#include "htmlengine.h"
#include "htmlengine-search.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cursor.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-selection-updater.h"
#include "htmlengine-print.h"
#include "htmlselection.h"
#include "htmlcolor.h"
#include "htmlinterval.h"
#include "htmlsettings.h"
#include "htmltext.h"
#include "htmltokenizer.h"
#include "htmltype.h"
#include "htmlundo.h"
#include "htmldrawqueue.h"
#include "htmlgdkpainter.h"
#include "htmlreplace.h"
#include "htmlentity.h"

#include "htmlanchor.h"
#include "htmlrule.h"
#include "htmlobject.h"
#include "htmlclueh.h"
#include "htmlcluev.h"
#include "htmlcluealigned.h"
#include "htmlvspace.h"
#include "htmlimage.h"
#include "htmllinktext.h"
#include "htmllist.h"
#include "htmltable.h"
#include "htmltablecell.h"
#include "htmltext.h"
#include "htmltextslave.h"
#include "htmlclueflow.h"
#include "htmlstack.h"
#include "htmlstringtokenizer.h"
#include "htmlform.h"
#include "htmlbutton.h"
#include "htmltextinput.h"
#include "htmlradio.h"
#include "htmlcheckbox.h"
#include "htmlhidden.h"
#include "htmlselect.h"
#include "htmltextarea.h"
#include "htmlimageinput.h"
#include "htmlstack.h"
#include "htmlsearch.h"
#include "htmlframeset.h"
#include "htmlframe.h"
#include "htmliframe.h"
#include "htmlshape.h"
#include "htmlmap.h"


static void      html_engine_class_init       (HTMLEngineClass     *klass);
static void      html_engine_init             (HTMLEngine          *engine);
static gboolean  html_engine_timer_event      (HTMLEngine          *e);
static gboolean  html_engine_update_event     (HTMLEngine          *e);
static char **   html_engine_stream_types     (GtkHTMLStream       *stream,
					       gpointer            data);
static void      html_engine_stream_write     (GtkHTMLStream       *stream,
					       const gchar         *buffer,
					       size_t               size,
					       gpointer             data);
static void      html_engine_stream_end       (GtkHTMLStream       *stream,
					       GtkHTMLStreamStatus  status,
					       gpointer             data);
static void      html_engine_set_class_data   (HTMLEngine          *e,
					       const gchar         *class_name,
					       const gchar         *key,
					       const gchar         *value);
static void      html_engine_clear_class_data (HTMLEngine          *e,
					       const gchar         *class_name,
					       const gchar         *key);
static void      html_engine_set_object_data  (HTMLEngine          *e,
					       HTMLObject          *o);

static void      parse_one_token           (HTMLEngine *p,
					    HTMLObject *clue,
					    const gchar *str);
static void      parse_input               (HTMLEngine *e,
					    const gchar *s,
					    HTMLObject *_clue);
static void      parse_iframe              (HTMLEngine *e,
					    const gchar *s,
					    HTMLObject *_clue);
static void      parse_f                   (HTMLEngine *p,
					    HTMLObject *clue,
					    const gchar *str);

static void      html_object_changed       (GtkHTMLEmbedded *eb,
					    HTMLEngine *e);

static void      update_embedded           (GtkWidget *widget,
					    gpointer );

static GtkLayoutClass *parent_class = NULL;

enum {
	SET_BASE_TARGET,
	SET_BASE,
	LOAD_DONE,
	TITLE_CHANGED,
	URL_REQUESTED,
	DRAW_PENDING,
	REDIRECT,
	SUBMIT,
	OBJECT_REQUESTED,
	LAST_SIGNAL
};
	
static guint signals [LAST_SIGNAL] = { 0 };

#define TIMER_INTERVAL 300

enum ID {
	ID_ADDRESS, ID_B, ID_BIG, ID_BLOCKQUOTE, ID_CAPTION, ID_CITE, ID_CODE,
	ID_DIR, ID_DIV, ID_EM, ID_FONT, ID_HEADER, ID_I, ID_KBD, ID_OL, ID_PRE,
	ID_SMALL, ID_STRONG, ID_U, ID_UL, ID_TEXTAREA, ID_TD, ID_TH, ID_TT, ID_VAR,
	ID_SUB, ID_SUP, ID_STRIKEOUT
};



/*
 *  Font handling.
 */

/* Font faces */

static HTMLFontFace *
current_font_face (HTMLEngine *e)
{
	return (HTMLFontFace *) (html_stack_is_empty (e->font_face_stack)
		? NULL
		: html_stack_top (e->font_face_stack));
}

static void
push_font_face (HTMLEngine *e, const HTMLFontFace *face)
{
	html_stack_push (e->font_face_stack, g_strdup (face));
}

static void
pop_font_face (HTMLEngine *e)
{
	g_free (html_stack_pop (e->font_face_stack));
}

/* Font styles */

static GtkHTMLFontStyle
current_font_style (HTMLEngine *e)
{
	GtkHTMLFontStyle style;

	if (html_stack_is_empty (e->font_style_stack))
		return GTK_HTML_FONT_STYLE_DEFAULT;

	style = GPOINTER_TO_INT (html_stack_top (e->font_style_stack));
	return style;
}

static GtkHTMLFontStyle
push_font_style (HTMLEngine *e,
		 GtkHTMLFontStyle new_attrs)
{
	GtkHTMLFontStyle current;
	GtkHTMLFontStyle new;

	current = current_font_style (e);
	new = gtk_html_font_style_merge (current, new_attrs);

	html_stack_push (e->font_style_stack, GINT_TO_POINTER (new));

	return new;
}

static void
pop_font_style (HTMLEngine *e)
{
	html_stack_pop (e->font_style_stack);
}


/* Color handling.  */

static HTMLColor *
current_color (HTMLEngine *e)
{
	HTMLColor *color;

	if (html_stack_is_empty (e->color_stack))
		color = html_colorset_get_color (e->settings->color_set, HTMLTextColor);
	else
		color = html_stack_top (e->color_stack);

	return color;
}

static void
push_color (HTMLEngine *e,
	    HTMLColor *color)
{
	html_stack_push (e->color_stack, color);
	html_color_ref (color);
}

static void
pop_color (HTMLEngine *e)
{
        HTMLColor *color = (HTMLColor *)html_stack_pop (e->color_stack);
	g_return_if_fail (color != NULL);

	html_color_unref (color);
}

static gboolean
parse_color (const gchar *text,
	     GdkColor *color)
{
	gchar c [8];
	gint  len = strlen (text);

	if (gdk_color_parse (text, color))
		return TRUE;

	c [7] = 0;
	if (*text != '#') {
		c[0] = '#'; 
		strncpy (c + 1, text, 6);
		len++;
	} else {
		strncpy (c, text, 7);
	}
	
	if (len < 7)
		memset (c + len, '0', 7-len);

	return gdk_color_parse (c, color);
}

static GtkPolicyType 
parse_scroll (const char *token) 
{
	GtkPolicyType scroll;
	
	if (strncasecmp (token, "yes", 3) == 0) {
		scroll = GTK_POLICY_ALWAYS;
	} else if (strncasecmp (token, "no", 2) == 0) {
		scroll = GTK_POLICY_NEVER;
	} else /* auto */ {
		scroll = GTK_POLICY_AUTOMATIC;
	}
	return scroll;
}

static HTMLHAlignType
parse_halign (const char *token, HTMLHAlignType default_val)
{
	if (strcasecmp (token, "center") == 0)
		return HTML_HALIGN_CENTER;
	else if (strcasecmp (token, "right") == 0)
		return HTML_HALIGN_RIGHT;
	else if (strcasecmp (token, "left") == 0)
		return HTML_HALIGN_LEFT;
	else
		return default_val;
}
	

/* ClueFlow style handling.  */

static HTMLClueFlowStyle
current_clueflow_style (HTMLEngine *e)
{
	HTMLClueFlowStyle style;

	if (html_stack_is_empty (e->clueflow_style_stack))
		return HTML_CLUEFLOW_STYLE_NORMAL;

	style = (HTMLClueFlowStyle) GPOINTER_TO_INT (html_stack_top (e->clueflow_style_stack));
	return style;
}

static void
push_clueflow_style (HTMLEngine *e,
		     HTMLClueFlowStyle style)
{
	html_stack_push (e->clueflow_style_stack, GINT_TO_POINTER (style));
}

static void
pop_clueflow_style (HTMLEngine *e)
{
	html_stack_pop (e->clueflow_style_stack);
}



/* Utility functions.  */

static void new_flow (HTMLEngine *e, HTMLObject *clue, HTMLObject *first_object);
static void close_flow (HTMLEngine *e, HTMLObject *clue);

static HTMLObject *
text_new (HTMLEngine *e, const gchar *text, GtkHTMLFontStyle style, HTMLColor *color)
{
	HTMLObject *o;

	o = html_text_new (text, style, color);
	html_engine_set_object_data (e, o);

	return o;
}

static HTMLObject *
flow_new (HTMLEngine *e, HTMLClueFlowStyle style, guint8 level, HTMLListType item_type, gint item_number)
{
	HTMLObject *o;

	o = html_clueflow_new (style, level, item_type, item_number);
	html_engine_set_object_data (e, o);

	return o;
}

static HTMLObject *
create_empty_text (HTMLEngine *e)
{
	HTMLObject *o;

	o = text_new (e, "", current_font_style (e), current_color (e));
	html_text_set_font_face (HTML_TEXT (o), current_font_face (e));

	return o;
}

static void
insert_paragraph_break (HTMLEngine *e,
		     HTMLObject *clue)
{
	
	close_flow (e, clue);
	new_flow (e, clue, create_empty_text (e));
	close_flow (e, clue);
}

static void
add_pending_paragraph_break (HTMLEngine *e,
		  HTMLObject *clue)
{
	if (e->pending_para) {
		insert_paragraph_break (e, clue);
		e->pending_para = FALSE;
	}
}

static void
add_line_break (HTMLEngine *e,
		HTMLObject *clue,
		HTMLClearType clear)
{
	if (!e->flow && !HTML_CLUE (clue)->head)
		new_flow (e, clue, create_empty_text (e));
	new_flow (e, clue, NULL);
}

static void
close_anchor (HTMLEngine *e)
{
	if (e->url == NULL && e->target == NULL)
		return;

	g_free (e->url);
	e->url = NULL;

	g_free (e->target);
	e->target = NULL;

	pop_color (e);
}

static void
close_flow (HTMLEngine *e,
	    HTMLObject *clue)
{
	HTMLObject *last;

	if (e->flow == NULL)
		return;

	last = HTML_CLUE (e->flow)->tail;
	if (last == NULL) {
		html_clue_append (HTML_CLUE (e->flow), create_empty_text (e));
	} else if (HTML_OBJECT_TYPE (last) == HTML_TYPE_VSPACE) {
		html_clue_remove (HTML_CLUE (e->flow), last);
		html_object_destroy (last);
	} else if (HTML_CLUE (e->flow)->tail != HTML_CLUE (e->flow)->head
		   && html_object_is_text (last)
		   && HTML_TEXT (last)->text_len == 1
		   && HTML_TEXT (last)->text [0] == ' ') {
		html_clue_remove (HTML_CLUE (e->flow), last);
		html_object_destroy (last);
	}

	e->flow = NULL;
}

static void
new_flow (HTMLEngine *e, HTMLObject *clue, HTMLObject *first_object)
{
	close_flow (e, clue);

	e->flow = flow_new (e, current_clueflow_style (e), e->indent_level, HTML_LIST_TYPE_UNORDERED, 0);

	HTML_CLUE (e->flow)->halign = e->divAlign;

	if (first_object)
		html_clue_append (HTML_CLUE (e->flow), first_object);

	html_clue_append (HTML_CLUE (clue), e->flow);
}

static void
append_element (HTMLEngine *e,
		HTMLObject *clue,
		HTMLObject *obj)
{
	add_pending_paragraph_break (e, clue);

	e->avoid_para = FALSE;

	if (e->flow == NULL)
		new_flow (e, clue, obj);
	else
		html_clue_append (HTML_CLUE (e->flow), obj);
}


static gboolean
check_prev (const HTMLObject *p, HTMLType type, GtkHTMLFontStyle font_style, HTMLColor *color, gchar *face)
{
	if (p == NULL)
		return FALSE;

	if (HTML_OBJECT_TYPE (p) != type)
		return FALSE;

	if (HTML_TEXT (p)->font_style != font_style)
		return FALSE;

	if (! html_color_equal (HTML_TEXT (p)->color, color))
		return FALSE;

	if ((face && !HTML_TEXT (p)->face) || (!face && HTML_TEXT (p)->face)
	    || (!face && !HTML_TEXT (p)->face) || !strcmp (face, HTML_TEXT (p)->face))
		return FALSE;

	return TRUE;
}

static void
insert_text (HTMLEngine *e,
	     HTMLObject *clue,
	     const gchar *text)
{
	GtkHTMLFontStyle font_style;
	HTMLObject *prev;
	HTMLType type;
	HTMLColor *color;
	gchar *face;
	gboolean create_link;

	if (text [0] == ' ' && text [1] == 0) {
		if (e->eat_space)
			return;
		else
			e->eat_space = TRUE;
	} else
		e->eat_space = FALSE;
	

	if (e->url != NULL || e->target != NULL)
		create_link = TRUE;
	else
		create_link = FALSE;

	font_style = current_font_style (e);
	color = current_color (e);
	face = current_font_face (e);

	if ((create_link || e->pending_para || e->flow == NULL || HTML_CLUE (e->flow)->head == NULL) && !e->inPre) {
		while (*text == ' ')
			text++;
		if (*text == 0)
			return;
	}

	if (e->flow == NULL)
		prev = NULL;
	else
		prev = HTML_CLUE (e->flow)->tail;

	if (e->url != NULL || e->target != NULL)
		type = HTML_TYPE_LINKTEXT;
	else
		type = HTML_TYPE_TEXT;

	if (! check_prev (prev, type, font_style, color, face) || e->pending_para) {
		HTMLObject *obj;

		if (create_link)
			obj = html_link_text_new (text, font_style, color, e->url, e->target);
		else
			obj = text_new (e, text, font_style, color);
		html_text_set_font_face (HTML_TEXT (obj), current_font_face (e));

		append_element (e, clue, obj);
	} else
		html_text_append (HTML_TEXT (prev), text, -1);
}


/* Block stack.  */

typedef void (*BlockFunc)(HTMLEngine *e, HTMLObject *clue, HTMLBlockStackElement *el);

struct _HTMLBlockStackElement {
	BlockFunc exitFunc;

	gint id;
	gint level;
	gint miscData1;
	gint miscData2;
	HTMLBlockStackElement *next;
};

static HTMLBlockStackElement *
block_stack_element_new (gint id, gint level, BlockFunc exitFunc, 
			 gint miscData1, gint miscData2, HTMLBlockStackElement *next)
{
	HTMLBlockStackElement *se;

	se = g_new0 (HTMLBlockStackElement, 1);
	se->id = id;
	se->level = level;
	se->miscData1 = miscData1;
	se->miscData2 = miscData2;
	se->next = next;
	se->exitFunc = exitFunc;
	return se;
}

static void
block_stack_element_free (HTMLBlockStackElement *elem)
{
	g_free (elem);
}

static void
push_block (HTMLEngine *e, gint id, gint level,
	    BlockFunc exitFunc,
	    gint miscData1,
	    gint miscData2)
{
	HTMLBlockStackElement *elem;

	elem = block_stack_element_new (id, level, exitFunc, miscData1, miscData2, e->blockStack);
	e->blockStack = elem;
}

static void
free_block (HTMLEngine *e)
{
	HTMLBlockStackElement *elem = e->blockStack;

	while (elem != 0) {
		HTMLBlockStackElement *tmp = elem;

		elem = elem->next;
		block_stack_element_free (tmp);
	}
	e->blockStack = 0;
}

static void
pop_block (HTMLEngine *e, gint id, HTMLObject *clue)
{
	HTMLBlockStackElement *elem, *tmp;
	gint maxLevel;

	elem = e->blockStack;
	maxLevel = 0;

	while ((elem != NULL) && (elem->id != id)) {
		if (maxLevel < elem->level) {
			maxLevel = elem->level;
		}
		elem = elem->next;
	}
	if (elem == NULL)
		return;
	if (maxLevel > elem->level)
		return;
	
	elem = e->blockStack;
	
	while (elem) {
		tmp = elem;
		if (elem->exitFunc != NULL)
			(*(elem->exitFunc))(e, clue, elem);
		
		if (elem->id == id) {
			e->blockStack = elem->next;
			elem = NULL;
		}
		else {
			elem = elem->next;
		}

		block_stack_element_free (tmp);
	}
}


/* The following are callbacks that are called at the end of a block.  */

static void
block_end_font (HTMLEngine *e, HTMLObject *clue, HTMLBlockStackElement *elem)
{
	pop_font_style (e);
}

static void
block_end_clueflow_style (HTMLEngine *e,
			  HTMLObject *clue,
			  HTMLBlockStackElement *elem)
{
	close_flow (e, clue);
	pop_clueflow_style (e);

	e->divAlign = elem->miscData1;
}

static void
block_end_pre ( HTMLEngine *e, HTMLObject *_clue, HTMLBlockStackElement *elem)
{
	block_end_clueflow_style (e, _clue, elem);
	e->inPre = FALSE;
}

static void
block_end_color_font (HTMLEngine *e, HTMLObject *clue, HTMLBlockStackElement *elem)
{
	pop_color (e);
	pop_font_face (e);
	block_end_font (e, clue, elem);
}

static void
block_end_list (HTMLEngine *e, HTMLObject *clue, HTMLBlockStackElement *elem)
{
	html_list_destroy (html_stack_pop (e->listStack));

	close_flow (e, clue);
	
	e->indent_level = elem->miscData1;

	if (e->indent_level == 0) {
		e->pending_para = FALSE;
		e->avoid_para = TRUE;
	}
}

static void
block_end_quote (HTMLEngine *e, HTMLObject *clue, HTMLBlockStackElement *elem)
{
	close_flow (e, clue);

	e->indent_level = elem->miscData1;

	e->pending_para = FALSE;
	e->avoid_para = TRUE;
}

static void
block_end_div (HTMLEngine *e, HTMLObject *clue, HTMLBlockStackElement *elem)
{
	close_flow (e, clue);

	e->divAlign =  (HTMLHAlignType) elem->miscData1;
}


static gchar *
parse_body (HTMLEngine *e, HTMLObject *clue, const gchar *end[], gboolean toplevel)
{
	gchar *str;

	e->eat_space = FALSE;
	while (html_tokenizer_has_more_tokens (e->ht) && e->parsing) {
		str = html_tokenizer_next_token (e->ht);

		if (*str == '\0')
			continue;

		if ( *str == ' ' && *(str+1) == '\0' ) {
			/* if in* is set this text belongs in a form element */
			if (e->inTextArea || e->inOption)
				e->formText = g_string_append (e->formText, " ");
			else if (e->inTitle)
				g_string_append (e->title, " ");
			else
				insert_text (e, clue, str);
		} else if (*str != TAG_ESCAPE) {
			if (e->inOption || e->inTextArea)
				g_string_append (e->formText, str);
			else if (e->inTitle) {
				g_string_append (e->title, str);
			}
			else {
				insert_text (e, clue, str);
			}
		} else {
			gint i  = 0;
			str++;

			while (end [i] != 0) {
				if (strncasecmp (str, end[i], strlen(end[i])) == 0) {
					return str;
				}
				i++;
			}
			
			/* The tag used for line break when we are in <pre>...</pre> */
			if (*str == '\n')
				add_line_break (e, clue, HTML_CLEAR_NONE);
			else
				parse_one_token (e, clue, str);
		}
	}

	if (!html_tokenizer_has_more_tokens (e->ht) && toplevel && !e->writing)
		html_engine_stop_parser (e);

	return NULL;
}

static gchar *
discard_body (HTMLEngine *p, const gchar *end[])
{
	gchar *str = NULL;

	while (html_tokenizer_has_more_tokens (p->ht) && p->parsing) {
		str = html_tokenizer_next_token (p->ht);

		if (*str == '\0')
			continue;

		if ((*str == ' ' && *(str+1) == '\0')
		    || (*str != TAG_ESCAPE)) {
			/* do nothing */
		}
		else {
			gint i  = 0;
			str++;
			
			while (end [i] != 0) {
				if (strncasecmp (str, end[i], strlen(end[i])) == 0) {
					return str;
				}
				i++;
			}
		}
	}

	return 0;
}

/* EP CHECK: finished except for the settings stuff (see `FIXME').  */
static const gchar *
parse_table (HTMLEngine *e, HTMLObject *clue, gint max_width,
	     const gchar *attr)
{
	static const gchar *endthtd[] = { "</th", "</td", "</tr", "<th", "<td", "<tr", "</table", "</body", 0 };
	static const char *endcap[] = { "</caption>", "</table>", "<tr", "<td", "<th", "</body", 0 };    
	static const gchar *endall[] = { "</caption>", "</table", "<tr", "<td", "<th", "</th", "</td", "</tr","</body", 0 };
	HTMLTable *table;
	const gchar *str = 0;
	gint width = 0;
	gint percent = 0;
	gint padding = 1;
	gint spacing = 2;
	gint border = 0;
	gchar has_cell = 0;
	gboolean done = FALSE;
	gboolean tableTag = TRUE;
	gboolean firstRow = TRUE;
	gboolean noCell = TRUE;
	gboolean tableEntry;
	HTMLVAlignType rowvalign = HTML_VALIGN_NONE;
	HTMLHAlignType rowhalign = HTML_HALIGN_NONE;
	HTMLHAlignType align = HTML_HALIGN_NONE;
	HTMLClueV *caption = 0;
	HTMLVAlignType capAlign = HTML_VALIGN_BOTTOM;
	HTMLHAlignType olddivalign = e->divAlign;
	HTMLClue *oldflow = HTML_CLUE (e->flow);
	gint old_indent_level = e->indent_level;
	GdkColor tableColor, rowColor, bgColor;
	gboolean have_tableColor, have_rowColor, have_bgColor;
	gboolean have_tablePixmap, have_rowPixmap, have_bgPixmap;
	gint rowSpan;
	gint colSpan;
	gint cellwidth;
	gint cellheight;
	gboolean cellwidth_percent;
	gboolean cellheight_percent;
	gboolean no_wrap;
	gboolean fixedWidth;
	gboolean fixedHeight;
	HTMLVAlignType valign;
	HTMLHAlignType halign;
	HTMLTableCell *cell;
	gpointer tablePixmapPtr = NULL;
	gpointer rowPixmapPtr = NULL;
	gpointer bgPixmapPtr = NULL;

	have_tablePixmap = FALSE;
	have_rowPixmap = FALSE;
	have_bgPixmap = FALSE;
 
	have_tableColor = FALSE;
	have_rowColor = FALSE;
	have_bgColor = FALSE;

	gtk_html_debug_log (e->widget, "start parse\n");

	html_string_tokenizer_tokenize (e->st, attr, " >");
	while (html_string_tokenizer_has_more_tokens (e->st)) {
		const gchar *token = html_string_tokenizer_next_token (e->st);
		if (strncasecmp (token, "cellpadding=", 12) == 0) {
			padding = atoi (token + 12);
		}
		else if (strncasecmp (token, "cellspacing=", 12) == 0) {
			spacing = atoi (token + 12);
		}
		else if (strncasecmp (token, "border", 6) == 0) {
			if (*(token + 6) == '=')
				border = atoi (token + 7);
			else
				border = 1;
		}
		else if (strncasecmp (token, "width=", 6) == 0) {
			if (strchr (token + 6, '%')) {
				percent = atoi (token + 6);
			} else if (strchr (token + 6, '*')) {
				/* Ignore */
			} else if (isdigit (*(token + 6))) {
				width = atoi (token + 6);
			}
		}
		else if (strncasecmp (token, "align=", 6) == 0) {
			align = parse_halign (token + 6, align);
		}
		else if (strncasecmp (token, "bgcolor=", 8) == 0
			 && !e->defaultSettings->forceDefault) {
			if (parse_color (token + 8, &tableColor)) {
				rowColor = tableColor;
				have_rowColor = have_tableColor = TRUE;
			}
		}
		else if (strncasecmp (token, "background=", 11) == 0
			 && token [12]
			 && !e->defaultSettings->forceDefault) {
			tablePixmapPtr = html_image_factory_register(e->image_factory, NULL, token + 11, FALSE);

			if(tablePixmapPtr) {
				rowPixmapPtr = tablePixmapPtr;
				have_tablePixmap = have_rowPixmap = TRUE;
			}
		}
	}

	table = HTML_TABLE (html_table_new (width, 
					    percent, padding,
					    spacing, border));
	if (have_tableColor)
		table->bgColor = gdk_color_copy (&tableColor);
	if (have_tablePixmap)
		table->bgPixmap = HTML_IMAGE_POINTER (tablePixmapPtr);
	e->indent_level = 0;

	while (!done && html_tokenizer_has_more_tokens (e->ht)) {
		str = html_tokenizer_next_token (e->ht);
		
		/* Every tag starts with an escape character */
		if (str[0] == TAG_ESCAPE) {
			str++;

			tableTag = TRUE;

			for (;;) {
				if (strncmp (str, "</table", 7) == 0) {
					close_anchor (e);
					done = TRUE;
					break;
				}

				if ( strncmp( str, "<caption", 8 ) == 0 ) {
					html_string_tokenizer_tokenize( e->st, str + 9, " >" );
					while ( html_string_tokenizer_has_more_tokens (e->st) ) {
						const char* token = html_string_tokenizer_next_token(e->st);
						if ( strncasecmp( token, "align=", 6 ) == 0) {
							if ( strncasecmp( token+6, "top", 3 ) == 0)
								capAlign = HTML_VALIGN_TOP;
						}
					}

					caption = HTML_CLUEV (html_cluev_new (0, 0, 100));

					e->divAlign = HTML_HALIGN_CENTER;
					e->flow = 0;

					push_block (e, ID_CAPTION, 3, NULL, 0, 0);
					str = parse_body ( e, HTML_OBJECT (caption), endcap, FALSE );
					pop_block (e, ID_CAPTION, HTML_OBJECT (caption) );

					table->caption = caption;
					table->capAlign = capAlign;

					e->flow = 0;

					if (!str)
						break;
					else if (strncmp( str, "</caption", 9) == 0 ) {
						/* HTML Ok! */
						break; /* Get next token from 'ht' */
					}
					else {
						/* Bad HTML
						   caption ended with </table> <td> <tr> or <th> */
						continue; /* parse the returned tag */
					}
				}

				if (strncmp (str, "<tr", 3) == 0) {
					if (!firstRow)
						html_table_end_row (table);
					html_table_start_row (table);
					firstRow = FALSE;
					rowvalign = HTML_VALIGN_NONE;
					rowhalign = HTML_HALIGN_NONE;

					have_rowColor = FALSE;
					have_rowPixmap = FALSE;

					html_string_tokenizer_tokenize (e->st, str + 4, " >");
					while (html_string_tokenizer_has_more_tokens (e->st)) {
						const gchar *token = html_string_tokenizer_next_token (e->st);
						if (strncasecmp (token, "valign=", 7) == 0) {
							if (strncasecmp (token + 7, "top", 3) == 0)
								rowvalign = HTML_VALIGN_TOP;
							else if (strncasecmp (token + 7, "bottom", 6) == 0)
								rowvalign = HTML_VALIGN_BOTTOM;
							else
								rowvalign = HTML_VALIGN_MIDDLE;
						} else if (strncasecmp (token, "align=", 6) == 0) {
							rowhalign = parse_halign (token + 6, rowhalign);
						} else if (strncasecmp (token, "bgcolor=", 8) == 0) {
							have_rowColor |= parse_color (token + 8, &rowColor);
						} else if (strncasecmp (token, "background=", 11) == 0
							   && token [12]
							   && !e->defaultSettings->forceDefault) {
							rowPixmapPtr = html_image_factory_register(e->image_factory, NULL, token + 11, FALSE);
							if(rowPixmapPtr)
								have_rowPixmap = TRUE;
						}
					}
					break;
				} /* Hack to fix broken html in bonsai */
				else if (strncmp (str, "<form", 5) == 0 || strncmp (str, "</form", 6) == 0) {
					parse_f (e, clue, str + 1);
				}

				/* Check for <td> and <th> */
				tableEntry = *str == '<' && *(str + 1) == 't' &&
					(*(str + 2) == 'd' || *(str + 2) == 'h');
				if (tableEntry || noCell) {
					gboolean heading = FALSE;
					noCell = FALSE;

					close_anchor (e);
					if (tableEntry && *(str + 2) == 'h') {
						gtk_html_debug_log (e->widget, "<th>\n");
						heading = TRUE;
					}
					
					/* <tr> may not be specified for the first row */
					if (firstRow) {
						/* Bad HTML: No <tr> tag present */
						html_table_start_row (table);
						firstRow = FALSE;
					}

					no_wrap     = FALSE;
					rowSpan     = 1;
					colSpan     = 1;
					cellwidth   = clue->max_width;
					cellheight  = -1;
					cellwidth_percent = FALSE;
					cellheight_percent = FALSE;
					fixedWidth  = FALSE;
					fixedHeight = FALSE;

					if (have_rowColor) {
						bgColor = rowColor;
						have_bgColor = TRUE;
					} else {
						have_bgColor = FALSE;
					}

					if (have_rowPixmap) {
						bgPixmapPtr = rowPixmapPtr;
						have_bgPixmap = TRUE;
					} else {
						have_bgPixmap = FALSE;
					}

					e->divAlign = HTML_HALIGN_NONE;
					valign = rowvalign == HTML_VALIGN_NONE ? HTML_VALIGN_MIDDLE : rowvalign;
					halign = rowhalign == HTML_HALIGN_NONE ? HTML_HALIGN_NONE   : rowhalign;

					if (tableEntry) {
						html_string_tokenizer_tokenize (e->st, str + 4, " >");
						while (html_string_tokenizer_has_more_tokens (e->st)) {
							const gchar *token = html_string_tokenizer_next_token (e->st);
							if (strncasecmp (token, "rowspan=", 8) == 0) {
								rowSpan = atoi (token + 8);
								if (rowSpan < 1)
									rowSpan = 1;
							}
							else if (strncasecmp (token, "colspan=", 8) == 0) {
								colSpan = atoi (token + 8);
								if (colSpan < 1)
									colSpan = 1;
							}
							else if (strncasecmp (token, "valign=", 7) == 0) {
								if (strncasecmp (token + 7, "top", 3) == 0)
									valign = HTML_VALIGN_TOP;
								else if (strncasecmp (token + 7, "bottom", 6) == 0)
									valign = HTML_VALIGN_BOTTOM;
								else 
									valign = HTML_VALIGN_MIDDLE;
							}
							else if (strncasecmp (token, "align=", 6) == 0) {
								halign = parse_halign (token + 6, halign);
							}
							else if (strncasecmp (token, "height=", 7) == 0) {
								if (strchr (token + 7, '%')) {
									/* gtk_html_debug_log (e->widget, "percent!\n");
									cellheight = atoi (token + 7);
									cellheight_percent = TRUE;
									fixedHeight = TRUE; */
								}
								else if (strchr (token + 7, '*')) {
									/* ignore */
								}
								else if (isdigit (*(token + 7))) {
									cellheight = atoi (token + 7);
									cellheight_percent = FALSE;
									fixedHeight = TRUE;
								}
							}
							else if (strncasecmp (token, "width=", 6) == 0) {
								if (strchr (token + 6, '%')) {
									gtk_html_debug_log (e->widget, "percent!\n");
									cellwidth = atoi (token + 6);
									cellwidth_percent = TRUE;
									fixedWidth = TRUE;
								}
								else if (strchr (token + 6, '*')) {
									/* ignore */
								}
								else if (isdigit (*(token + 6))) {
									cellwidth = atoi (token + 6);
									cellwidth_percent = FALSE;
									fixedWidth = TRUE;
								}
							}
							else if (strncasecmp (token, "bgcolor=", 8) == 0
								 && !e->defaultSettings->forceDefault) {
								have_bgColor |= parse_color (token + 8,
											     &bgColor);
							}
							else if (strncasecmp (token, "nowrap", 6) == 0) {
								no_wrap = TRUE;
							}
							else if (strncasecmp (token, "background=", 11) == 0
								 && token [12]
								 && !e->defaultSettings->forceDefault) {
								
								bgPixmapPtr = html_image_factory_register(e->image_factory, 
													  NULL, token + 11,
													  FALSE);
								if(bgPixmapPtr)
									have_bgPixmap = TRUE;

							}
						}
					}

					add_pending_paragraph_break (e, clue);

					cell = HTML_TABLE_CELL (html_table_cell_new (rowSpan, colSpan, padding));
					cell->no_wrap = no_wrap;
					cell->heading = heading;
					html_object_set_bg_color (HTML_OBJECT (cell),
								  have_bgColor ? &bgColor : NULL);

					if(have_bgPixmap)
						html_table_cell_set_bg_pixmap(cell, bgPixmapPtr);

					HTML_CLUE (cell)->valign = valign;
					HTML_CLUE (cell)->halign = halign;
					if (fixedWidth)
						html_table_cell_set_fixed_width (cell, cellwidth, cellwidth_percent);
					if (fixedHeight)
						html_table_cell_set_fixed_height (cell, cellheight, cellheight_percent);
 
					html_table_add_cell (table, cell);
					has_cell = 1;
					e->flow = NULL;

					e->avoid_para = TRUE;

					if (!tableEntry) {
						/* Put all the junk between <table>
						   and the first table tag into one row */
						push_block (e, ID_TD, 3, NULL, 0, 0);
						str = parse_body (e, HTML_OBJECT (cell), endall, FALSE);
						pop_block (e, ID_TD, HTML_OBJECT (cell));

						add_pending_paragraph_break (e, HTML_OBJECT (cell));

						html_table_end_row (table);
						html_table_start_row (table);
					} else {
						push_block (e, heading ? ID_TH : ID_TD, 3, NULL, 0, 0);
						str = parse_body (e, HTML_OBJECT (cell), endthtd, FALSE);
						if (HTML_CLUE (cell)->head == NULL)
							insert_paragraph_break (e, HTML_OBJECT (cell));
						pop_block (e, heading ? ID_TH : ID_TD, HTML_OBJECT (cell));
						add_pending_paragraph_break (e, HTML_OBJECT (cell));
					}

					if (!str)
						break;
					else if ((strncmp (str, "</td", 4) == 0) ||
						    (strncmp (str, "</th", 4) == 0)) {
						/* HTML ok! */
						break; /* Get next token from 'ht' */
					} else {
						/* Bad HTML */
						continue;
					}
				}

				/* Unknown or unhandled table-tag: ignore */
				break;
				
			}
		}
	}
		
	e->indent_level = old_indent_level;
	e->divAlign = olddivalign;
	e->flow = HTML_OBJECT (oldflow);

	if (has_cell) {
		/* The ending "</table>" might be missing, so we close the table
		   here...  */
		if (!firstRow)
			html_table_end_row (table);
		html_table_end_table (table);

		if (align != HTML_HALIGN_LEFT && align != HTML_HALIGN_RIGHT) {
			close_flow (e, clue);

			if (align != HTML_HALIGN_NONE) {
				olddivalign = e->divAlign;
				e->divAlign = align;
			}
			append_element (e, clue, HTML_OBJECT (table));

			close_flow (e, clue);
			if (align != HTML_HALIGN_NONE)
				e->divAlign = olddivalign;
		} else {
			HTMLClueAligned *aligned = HTML_CLUEALIGNED (html_cluealigned_new (NULL, 0, 0, clue->max_width, 100));
			HTML_CLUE (aligned)->halign = align;
			html_clue_append (HTML_CLUE (aligned), HTML_OBJECT (table));
			append_element (e, clue, HTML_OBJECT (aligned));
		}
	} else {
		/* Last resort: remove tables that do not contain any cells */
		html_object_destroy (HTML_OBJECT (table));
	}

	gtk_html_debug_log (e->widget, "Returning: %s\n", str);
	return str;
}

static gboolean
is_leading_space (guchar *str)
{
	while (*str != '\0') {
		if (!(isspace (*str) || IS_UTF8_NBSP (str))) 
			return FALSE;
		
		str = g_utf8_next_char (str);
	}
	return TRUE;
}

static gchar * 
parse_object_params(HTMLEngine *p, HTMLObject *clue) 
{
	gchar *str;

	/* we peek at tokens looking for <param> elements and
	 * as soon as we find something that is not whitespace or a param
	 * element we bail and the caller deal with the rest
	 */
	while (html_tokenizer_has_more_tokens (p->ht) && p->parsing) {
		str = html_tokenizer_peek_token (p->ht);
		
		if (*str == '\0' || 
		    *str == '\n' ||
		    is_leading_space (str)) {
				str = html_tokenizer_next_token (p->ht);
				/* printf ("\"%s\": was the string\n", str); */
				continue;
		} else if (*str == TAG_ESCAPE) {
			str++;
			if (strncasecmp ("<param", str, 6) == 0) {
				/* go ahead and remove the token */
				html_tokenizer_next_token (p->ht);

				parse_one_token (p, clue, str);
				continue;
			}
		}
		return str;
	}
	
	return NULL;	
}

static void
parse_object (HTMLEngine *e, HTMLObject *clue, gint max_width,
	     const gchar *attr)
{
	char *classid=NULL;
	char *name=NULL;
	char *type = NULL;
	char *str = NULL;
	char *data = NULL;
	int width=-1,height=-1;
	static const gchar *end[] = { "</object", 0};
	GtkHTMLEmbedded *eb;
	HTMLEmbedded *el;
	gboolean object_found;
	
	
	html_string_tokenizer_tokenize( e->st, attr, " >" );
	
	/* this might have to do something different for form object
	   elements - check the spec MPZ */
	while (html_string_tokenizer_has_more_tokens (e->st) ) {
		const char* token;
		
		token = html_string_tokenizer_next_token (e->st);
		if (strncasecmp (token, "classid=", 8) == 0) {
			classid = g_strdup (token + 8);
		} else if (strncasecmp (token, "name=", 5) == 0 ) {
			name = g_strdup (token + 5);
		} else if ( strncasecmp (token, "width=", 6) == 0) {
			width = atoi (token + 6);
		} else if (strncasecmp (token, "height=", 7) == 0) {
			height = atoi (token + 7);
		} else if (strncasecmp (token, "type=", 5) == 0) {
			type = g_strdup (token + 5);
		} else if (strncasecmp (token, "data=", 5) == 0) {
			data = g_strdup (token + 5);
		}
	}
	
	eb = (GtkHTMLEmbedded *) gtk_html_embedded_new (classid, name, type, data, width, height);
	html_stack_push (e->embeddedStack, eb);
	
	el = html_embedded_new_widget (GTK_WIDGET (e->widget), eb);		
	gtk_signal_connect (GTK_OBJECT(eb), "changed", html_object_changed, e);
	
	/* evaluate params */
	parse_object_params (e, clue);

	/* create the object */
        object_found = FALSE;
	gtk_signal_emit (GTK_OBJECT (e), signals [OBJECT_REQUESTED], eb, &object_found);
	
	/* show alt text on TRUE */ 
	if (object_found) {
		append_element(e, clue, HTML_OBJECT(el));
		/* automatically add this to a form if it is part of one */
		if (e->form)
			html_form_add_element (e->form, HTML_EMBEDDED (el));

		/* throw away the contents we can deal with the object */
		str = discard_body (e, end);
	} else {
		/* parse the body of the tag to display the alternative */
		str = parse_body (e, clue, end, FALSE);

		html_object_destroy (HTML_OBJECT (el));
	}
	
	if ((!str || (strncasecmp (str, "</object", 8) == 0)) && 
	    (!html_stack_is_empty (e->embeddedStack))) {
		html_stack_pop (e->embeddedStack);
	}
	
	g_free (type);
	g_free (data);
	g_free (classid);
	g_free (name);
}

static void
parse_input (HTMLEngine *e, const gchar *str, HTMLObject *_clue)
{
	enum InputType { CheckBox, Hidden, Radio, Reset, Submit, Text, Image,
			 Button, Password, Undefined };
	HTMLObject *element = NULL;
	const char *p;
	enum InputType type = Text;
	gchar *name = NULL;
	gchar *value = NULL;
	gchar *imgSrc = NULL;
	gboolean checked = FALSE;
	int size = 20;
	int maxLen = -1;
	int imgHSpace = 0;
	int imgVSpace = 0;

	html_string_tokenizer_tokenize (e->st, str, " >");

	while (html_string_tokenizer_has_more_tokens (e->st)) {
		const gchar *token = html_string_tokenizer_next_token (e->st);

		if ( strncasecmp( token, "type=", 5 ) == 0 ) {
			p = token + 5;
			if ( strncasecmp( p, "checkbox", 8 ) == 0 )
				type = CheckBox;
			else if ( strncasecmp( p, "password", 8 ) == 0 )
				type = Password;
			else if ( strncasecmp( p, "hidden", 6 ) == 0 )
				type = Hidden;
			else if ( strncasecmp( p, "radio", 5 ) == 0 )
				type = Radio;
			else if ( strncasecmp( p, "reset", 5 ) == 0 )
				type = Reset;
			else if ( strncasecmp( p, "submit", 5 ) == 0 )
				type = Submit;
			else if ( strncasecmp( p, "button", 6 ) == 0 )
				type = Button;
			else if ( strncasecmp( p, "text", 5 ) == 0 )
				type = Text;
			else if ( strncasecmp( p, "image", 5 ) == 0 )
				type = Image;
		}
		else if ( strncasecmp( token, "name=", 5 ) == 0 ) {
			name = g_strdup(token + 5);
		}
		else if ( strncasecmp( token, "value=", 6 ) == 0 ) {
			value = g_strdup(token + 6);
		}
		else if ( strncasecmp( token, "size=", 5 ) == 0 ) {
			size = atoi( token + 5 );
		}
		else if ( strncasecmp( token, "maxlength=", 10 ) == 0 ) {
			maxLen = atoi( token + 10 );
		}
		else if ( strncasecmp( token, "checked", 7 ) == 0 ) {
			checked = TRUE;
		}
		else if ( strncasecmp( token, "src=", 4 ) == 0 ) {
			imgSrc = g_strdup (token + 4);
		}
		else if ( strncasecmp( token, "onClick=", 8 ) == 0 ) {
			/* TODO: Implement Javascript */
		}
		else if ( strncasecmp( token, "hspace=", 7 ) == 0 ) {
			imgHSpace = atoi (token + 7);
		}
		else if ( strncasecmp( token, "vspace=", 7 ) == 0 ) {
			imgVSpace = atoi (token + 7);
		}
	}
	switch ( type ) {
	case CheckBox:
		element = html_checkbox_new(GTK_WIDGET(e->widget), name, value, checked);
		break;
	case Hidden:
		{
		HTMLObject *hidden = html_hidden_new(name, value);

		html_form_add_hidden (e->form, HTML_HIDDEN (hidden));

		break;
		}
	case Radio:
		element = html_radio_new(GTK_WIDGET(e->widget), name, value, checked, e->form);
		break;
	case Reset:
		element = html_button_new(GTK_WIDGET(e->widget), name, value, BUTTON_RESET);
		break;
	case Submit:
		element = html_button_new(GTK_WIDGET(e->widget), name, value, BUTTON_SUBMIT);
		break;
	case Button:
		element = html_button_new(GTK_WIDGET(e->widget), name, value, BUTTON_NORMAL);
		break;
	case Text:
	case Password:
		element = html_text_input_new(GTK_WIDGET(e->widget), name, value, size, maxLen, (type == Password));
		break;
	case Image:
		element = html_imageinput_new (e->image_factory, name, imgSrc);
		html_image_set_spacing (HTML_IMAGE (HTML_IMAGEINPUT (element)->image), imgHSpace, imgVSpace);

		break;
	case Undefined:
		g_warning ("Unknown <input type>\n");
		break;
	}
	if (element) {

		append_element (e, _clue, element);
		html_form_add_element (e->form, HTML_EMBEDDED (element));
	}

	if (name)
		g_free (name);
	if (value)
		g_free (value);
	if (imgSrc)
		g_free (imgSrc);
}

static void
parse_frameset (HTMLEngine *e, HTMLObject *clue, gint max_width, const gchar *attr)
{
	HTMLObject *set;
	char *rows = NULL;
	char *cols = NULL;

	html_string_tokenizer_tokenize (e->st, attr, " >");

	while (html_string_tokenizer_has_more_tokens (e->st)) {
		const gchar *token = html_string_tokenizer_next_token (e->st);
	
		if (strncasecmp (token, "rows=", 5) == 0) {
			rows = g_strdup (token + 5);
		} else if (strncasecmp (token, "cols=", 5) == 0) {
			cols = g_strdup (token + 5);
		} else if (strncasecmp (token, "onload=", 7) == 0) {
			/* all frames in set loaded */
			/* unimplemented */
		} else if (strncasecmp (token, "onunload=", 9) == 0) {
			/* all frames unloaded */
			/* unimplemented */
		}		
		
	}
	
	/* clear the borders */
	e->bottomBorder = 0;
	e->topBorder = 0;
	e->leftBorder = 0;
	e->rightBorder = 0;
	
	set = html_frameset_new (e->widget, rows, cols);

	if (html_stack_is_empty (e->frame_stack)) {
		append_element (e, clue, set);
	} else {
		html_frameset_append (html_stack_top (e->frame_stack), set);
	}
		
	html_stack_push (e->frame_stack, set);
	       
	g_free (rows);
	g_free (cols);

}

static void
parse_iframe (HTMLEngine *e, const gchar *str, HTMLObject *_clue) 
{	char *src = NULL;
	char *align = NULL;
	HTMLObject *iframe;
	static const gchar *end[] = { "</iframe", 0};
	gint width           = -1;
	gint height          = -1;
	gint border          = TRUE;
	GtkPolicyType scroll = GTK_POLICY_AUTOMATIC;
	gint margin_width    = -1;
	gint margin_height   = -1;

	html_string_tokenizer_tokenize (e->st, str, " >");

	while (html_string_tokenizer_has_more_tokens (e->st)) {
		const gchar *token = html_string_tokenizer_next_token (e->st);

		if (strncasecmp (token, "src=", 4) == 0) {
			src = g_strdup (token + 4);
		} else if (strncasecmp (token, "width=", 6) == 0) {
			width = atoi (token + 6);
		} else if (strncasecmp (token, "height=", 7) == 0) {
			height = atoi (token + 7);
		} else if (strncasecmp (token, "align=", 6) == 0) {
			align = g_strdup (token + 6);
		} else if (strncasecmp (token, "longdesc=", 9) == 0) {
			/* TODO: Ignored */
		} else if (strncasecmp (token, "name=", 5) == 0) {
			/* TODO: Ignored */
		} else if (strncasecmp (token, "scrolling=", 10) == 0) {
			scroll = parse_scroll (token + 10);
		} else if (strncasecmp (token, "marginwidth=", 12) == 0) {
			margin_width = atoi (token + 12);
		} else if (strncasecmp (token, "marginheight=", 13) == 0) {
			margin_height = atoi (token + 13);
		} else if (strncasecmp (token, "frameborder=", 12) == 0) {
			border = atoi (token + 12);
		}

	}	
		
	if (src) {
#if 1
		iframe = html_iframe_new (GTK_WIDGET (e->widget),
					 src, width, height, border);
		g_free (src);
		append_element (e, _clue, iframe);
#else 
		{
		HTMLObject *frame = html_frame_new (GTK_WIDGET (e->widget),
					 src, width, height, border);
		
		html_frame_set_size (HTML_FRAME (frame), width, height);

		if (margin_height > 0)
			html_frame_set_margin_height (HTML_FRAME (frame), margin_height);
		if (margin_width > 0)
			html_frame_set_margin_width (HTML_FRAME (frame), margin_width);
		if (scroll != GTK_POLICY_AUTOMATIC)
			html_frame_set_scrolling (HTML_FRAME (frame), scroll);

		g_free (src);
		append_element (e, _clue, frame);
		}
#endif
		discard_body (e, end);
	} else {
		parse_body (e, _clue, end, FALSE);
	}
	g_free (align);
}


/*
  <a               </a>
  <address>        </address>
  <area            </area>
*/
static void
parse_a (HTMLEngine *e, HTMLObject *_clue, const gchar *str)
{
	if (strncmp (str, "area", 4) == 0) {
		HTMLShape *shape;
		char *type = NULL;
		char *href = NULL;
		char *coords = NULL;
		char *target = NULL;

		if (e->map == NULL)
			return;

		html_string_tokenizer_tokenize (e->st, str + 5, " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {   
			gchar *token = html_string_tokenizer_next_token (e->st);

			if (strncasecmp (token, "shape=", 6) == 0) {
				type = g_strdup (token + 6);
			} else if (strncasecmp (token, "href=", 5) == 0) {
				href = g_strdup (token +5);
			} else if ( strncasecmp (token, "target=", 7) == 0) {
				target = g_strdup (token + 7);
			} else if ( strncasecmp (token, "coords=", 7) == 0) {
				coords = g_strdup (token + 7);
			}
		}
		
		if (type || coords) {
			shape = html_shape_new (type, coords, href, target);
			if (shape != NULL) {
				html_map_add_shape (e->map, shape);
			}
		}

		g_free (type);
		g_free (href);
		g_free (coords);
		g_free (target);
	} else if ( strncmp( str, "address", 7) == 0 ) {
		push_clueflow_style (e, HTML_CLUEFLOW_STYLE_ADDRESS);
		close_flow (e, _clue);
		push_block (e, ID_ADDRESS, 2, block_end_clueflow_style,
			    e->divAlign, 0);
	} else if ( strncmp( str, "/address", 8) == 0 ) {
		pop_block (e, ID_ADDRESS, _clue);
	} else if ( strncmp( str, "a ", 2 ) == 0 ) {
		gchar *url = NULL;
		gchar *id = NULL;

		const gchar *p;
		
		close_anchor (e);
		
		html_string_tokenizer_tokenize( e->st, str + 2, " >" );
		
		while ((p = html_string_tokenizer_next_token (e->st)) != 0) {
			if (strncasecmp (p, "href=", 5) == 0) {
				url = g_strdup (p + 5);
				
				/* FIXME visited? */
			} else if (strncasecmp (p, "id=", 3) == 0) {
				/*
				 * FIXME this doesn't handle the 
				 * case where id and name are both set
				 * properly but it will do for now
				 */
				if (id == NULL)
					id = g_strdup (p + 3);
			} else if (strncasecmp (p, "name=", 5) == 0) {
				if (id == NULL)
					id = g_strdup (p + 5);
			} else if (strncasecmp (p, "shape=", 6) == 0) {
				/* FIXME todo */
			} else if (strncasecmp (p, "target=", 7) == 0) {
#if 0				/* FIXME TODO */
				target = g_strdup (p + 7);
				parsedTargets.append( target );
#endif
			}
		}
		

		if (id != NULL) {
			if (e->flow == 0)
				html_clue_append (HTML_CLUE (_clue),
						  html_anchor_new (id));
			else
				html_clue_append (HTML_CLUE (e->flow),
						  html_anchor_new (id));
			g_free (id);
		}

#if 0
		if ( !target
		     && e->baseTarget != NULL
		     && e->baseTarget[0] != '\0' ) {
			target = g_strdup (e->baseTarget);
				/*  parsedTargets.append( target ); FIXME TODO */		}
#endif
		
		if (url != NULL) {
			g_free (e->url);
			e->url = url;
		}
		
		if (e->url != NULL || e->target != NULL)
			push_color (e, html_colorset_get_color (e->settings->color_set, HTMLLinkColor));
	} else if ( strncmp( str, "/a", 2 ) == 0 ) {
		close_anchor (e);
		e->eat_space = FALSE;
	}
}


/*
  <b>              </b>
  <base
  <basefont                        unimplemented
  <big>            </big>
  <blockquote>     </blockquote>
  <body
  <br
*/
/* EP CHECK All done except for the color specifications in the `<body>'
   tag.  */
static void
parse_b (HTMLEngine *e, HTMLObject *clue, const gchar *str)
{
	GdkColor color;

	if (strncmp (str, "basefont", 8) == 0) {
	} else if ( strncmp(str, "base", 4 ) == 0 ) {
		html_string_tokenizer_tokenize( e->st, str + 5, " >" );
		while ( html_string_tokenizer_has_more_tokens (e->st) ) {
			const char* token = html_string_tokenizer_next_token(e->st);
			if ( strncasecmp( token, "target=", 7 ) == 0 ) {
				gtk_signal_emit (GTK_OBJECT (e), signals[SET_BASE_TARGET], token + 7);
			} else if ( strncasecmp( token, "href=", 5 ) == 0 ) {
				gtk_signal_emit (GTK_OBJECT (e), signals[SET_BASE], token + 5);
			}
		}
	} else if ( strncmp(str, "big", 3 ) == 0 ) {
		push_font_style (e, GTK_HTML_FONT_STYLE_SIZE_4);
		push_block (e, ID_BIG, 1, block_end_font, 0, 0);
	} else if ( strncmp(str, "/big", 4 ) == 0 ) {
		pop_block (e, ID_BIG, clue);
	} else if ( strncmp(str, "blockquote", 10 ) == 0 ) {
		push_block (e, ID_BLOCKQUOTE, 2, block_end_quote, e->indent_level, e->indent_level);
		e->avoid_para = TRUE;
		e->pending_para = FALSE;
		e->indent_level = e->indent_level + 1;
		close_flow (e, clue);
	} else if ( strncmp(str, "/blockquote", 11 ) == 0 ) {
		e->avoid_para = TRUE;
		pop_block (e, ID_BLOCKQUOTE, clue);
	} else if (strncmp (str, "body", 4) == 0) {
		html_string_tokenizer_tokenize (e->st, str + 5, " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			gchar *token;

			token = html_string_tokenizer_next_token (e->st);
			gtk_html_debug_log (e->widget, "token is: %s\n", token);

			if (strncasecmp (token, "bgcolor=", 8) == 0) {
				gtk_html_debug_log (e->widget, "setting color\n");
				if (parse_color (token + 8, &color)) {
					gtk_html_debug_log (e->widget, "bgcolor is set\n");
					html_colorset_set_color (e->settings->color_set, &color, HTMLBgColor);
				} else {
					gtk_html_debug_log (e->widget, "Color `%s' could not be parsed\n", token);
				}
			} else if (strncasecmp (token, "background=", 11) == 0
				   && token [12]
				   && ! e->defaultSettings->forceDefault) {
				gchar *bgurl;

				bgurl = g_strdup (token + 11);
				if (e->bgPixmapPtr != NULL)
					html_image_factory_unregister(e->image_factory, e->bgPixmapPtr, NULL);
				e->bgPixmapPtr = html_image_factory_register(e->image_factory, NULL, bgurl, FALSE);
				g_free (bgurl);
			} else if ( strncasecmp( token, "text=", 5 ) == 0
				    && !e->defaultSettings->forceDefault ) {
				if (parse_color (token + 5, &color)) {
					if (! html_stack_is_empty (e->color_stack))
						pop_color (e);
					html_colorset_set_color (e->settings->color_set, &color, HTMLTextColor);
					push_color (e, html_colorset_get_color (e->settings->color_set, HTMLTextColor));
				}
			} else if ( strncasecmp( token, "link=", 5 ) == 0
				    && !e->defaultSettings->forceDefault ) {
				parse_color (token + 5, &color);
				html_colorset_set_color (e->settings->color_set, &color, HTMLLinkColor);
			} else if ( strncasecmp( token, "vlink=", 6 ) == 0
				    && !e->defaultSettings->forceDefault ) {
				parse_color (token + 6, &color);
				html_colorset_set_color (e->settings->color_set, &color, HTMLVLinkColor);
			} else if ( strncasecmp( token, "alink=", 6 ) == 0
				    && !e->defaultSettings->forceDefault ) {
				parse_color (token + 6, &color);
				html_colorset_set_color (e->settings->color_set, &color, HTMLALinkColor);
			} else if ( strncasecmp( token, "leftmargin=", 11 ) == 0) {
				e->leftBorder = atoi (token + 11);
			} else if ( strncasecmp( token, "rightmargin=", 12 ) == 0) {
				e->rightBorder = atoi (token + 12);
			} else if ( strncasecmp( token, "topmargin=", 10 ) == 0) {
				e->topBorder = atoi (token + 10);
			} else if ( strncasecmp( token, "bottommargin=", 13 ) == 0) {
				e->bottomBorder = atoi (token + 13);
			} else if ( strncasecmp( token, "marginwidth=", 12 ) == 0) {
				e->leftBorder = e->rightBorder = atoi (token + 12);
			} else if ( strncasecmp( token, "marginheight=", 13 ) == 0) {
				e->topBorder = e->bottomBorder = atoi (token + 13);
			}
		}

		gtk_html_debug_log (e->widget, "parsed <body>\n");
	}
	else if (strncmp (str, "br", 2) == 0 || strncmp (str, "/br", 3) == 0) {
		HTMLClearType clear;

		clear = HTML_CLEAR_NONE;

		html_string_tokenizer_tokenize (e->st, str + 3, " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			gchar *token = html_string_tokenizer_next_token (e->st);
			
			if (strncasecmp (token, "clear=", 6) == 0) {
				gtk_html_debug_log (e->widget, "%s\n", token);
				if (strncasecmp (token + 6, "left", 4) == 0)
					clear = HTML_CLEAR_LEFT;
				else if (strncasecmp (token + 6, "right", 5) == 0)
					clear = HTML_CLEAR_RIGHT;
				else if (strncasecmp (token + 6, "all", 3) == 0)
					clear = HTML_CLEAR_ALL;
			}
		}

		add_line_break (e, clue, clear);
	}
	else if (strncmp (str, "b", 1) == 0) {
		if (str[1] == '>' || str[1] == ' ') {
			push_font_style (e, GTK_HTML_FONT_STYLE_BOLD);
			push_block (e, ID_B, 1, block_end_font, FALSE, FALSE);
		}
	}
	else if (strncmp (str, "/b", 2) == 0) {
		pop_block (e, ID_B, clue);
	}
}


/*
  <center>         </center>
  <cite>           </cite>
  <code>           </code>
  <cell>           </cell>
  <comment>        </comment>      unimplemented
*/
/* EP CHECK OK except for the font in `<code>'.  */
static void
parse_c (HTMLEngine *e, HTMLObject *clue, const gchar *str)
{
	if (strncmp (str, "center", 6) == 0) {
		close_flow (e, clue);
		e->divAlign = HTML_HALIGN_CENTER;
	}
	else if (strncmp (str, "/center", 7) == 0) {
		close_flow (e, clue);
		e->divAlign = HTML_HALIGN_NONE;
	}
	else if (strncmp( str, "cite", 4 ) == 0) {
		push_font_style (e, GTK_HTML_FONT_STYLE_ITALIC | GTK_HTML_FONT_STYLE_BOLD);
		push_block(e, ID_CITE, 1, block_end_font, 0, 0);
	}
	else if (strncmp( str, "/cite", 5) == 0) {
		pop_block (e, ID_CITE, clue);
	} else if (strncmp(str, "code", 4 ) == 0 ) {
		push_font_style (e, GTK_HTML_FONT_STYLE_FIXED);
		push_block (e, ID_CODE, 1, block_end_font, 0, 0);
	} else if (strncmp(str, "/code", 5 ) == 0 ) {
		pop_block (e, ID_CODE, clue);
	}
}


/*
  <dir             </dir>          partial
  <div             </div>
  <dl>             </dl>
  <dt>             </dt>
  <data>           </data>
*/
/* EP CHECK: dl/dt might be wrong.  */
/* EP CHECK: dir might be wrong.  */
static void
parse_d ( HTMLEngine *e, HTMLObject *_clue, const char *str )
{
	if ( strncmp( str, "dir", 3 ) == 0 ) {
		close_anchor(e);
		push_block (e, ID_DIR, 2, block_end_list, e->indent_level, FALSE);
		html_stack_push (e->listStack, html_list_new (HTML_LIST_TYPE_DIR));
		e->indent_level++;
		/* FIXME shouldn't it create a new flow? */
	} else if ( strncmp( str, "/dir", 4 ) == 0 ) {
		pop_block (e, ID_DIR, _clue);
	} else if ( strncmp( str, "div", 3 ) == 0 ) {
		push_block (e, ID_DIV, 1, block_end_div, e->divAlign, FALSE);

		html_string_tokenizer_tokenize( e->st, str + 4, " >" );
		while ( html_string_tokenizer_has_more_tokens (e->st) ) {
			const char* token = html_string_tokenizer_next_token (e->st);
			if ( strncasecmp( token, "align=", 6 ) == 0 ) {
				e->divAlign = parse_halign (token + 6, e->divAlign);
			}
		}

		if (e->flow != NULL) {
			if (HTML_CLUE (e->flow)->head != NULL)
				close_flow (e, _clue);
			else
				HTML_CLUE (e->flow)->halign = e->divAlign;
		}
	} else if ( strncmp( str, "/div", 4 ) == 0 ) {
		pop_block (e, ID_DIV, _clue );
	} else if ( strncmp( str, "dl", 2 ) == 0 ) {
		close_anchor (e);
		if ( html_stack_top(e->glossaryStack) != NULL )
			e->indent_level++;
		html_stack_push (e->glossaryStack, GINT_TO_POINTER (HTML_GLOSSARY_DL));
		/* FIXME shouldn't it create a new flow? */
		add_line_break (e, _clue, HTML_CLEAR_ALL);
	} else if ( strncmp( str, "/dl", 3 ) == 0 ) {
		if ( html_stack_top (e->glossaryStack) == NULL)
			return;

		if ( GPOINTER_TO_INT (html_stack_top (e->glossaryStack)) == HTML_GLOSSARY_DD ) {
			html_stack_pop (e->glossaryStack);
			if (e->indent_level > 0)
				e->indent_level--;
		}

		html_stack_pop (e->glossaryStack);
		if ( html_stack_top (e->glossaryStack) != NULL ) {
			if (e->indent_level > 0)
				e->indent_level--;
		}

		add_line_break (e, _clue, HTML_CLEAR_ALL);
	} else if (strncmp( str, "dt", 2 ) == 0) {
		if (GPOINTER_TO_INT (html_stack_top (e->glossaryStack)) == HTML_GLOSSARY_DD) {
			html_stack_pop (e->glossaryStack);
			if (e->indent_level > 0)
				e->indent_level--;
		}

		close_flow (e, _clue);
	} else if (strncmp( str, "dd", 2 ) == 0) {
		gpointer top = html_stack_top (e->glossaryStack);
			
		if (top && GPOINTER_TO_INT (top) != HTML_GLOSSARY_DD) {
			html_stack_push (e->glossaryStack,
					 GINT_TO_POINTER (HTML_GLOSSARY_DD));
			e->indent_level++;
		}

		close_flow (e, _clue);
	} else if (strncmp (str, "data ", 5) == 0) {
		gchar *key = NULL;
		gchar *class_name = NULL;

		html_string_tokenizer_tokenize (e->st, str + 5, " >" );
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			const gchar *token = html_string_tokenizer_next_token (e->st);
			if (strncasecmp (token, "class=", 6 ) == 0) {
				g_free (class_name);
				class_name = g_strdup (token + 6);
			} else if (strncasecmp (token, "key=", 4 ) == 0) {
				g_free (key);
				key = g_strdup (token + 4);
			} else if (class_name && key && strncasecmp (token, "value=", 6) == 0) {
				html_engine_set_class_data (e, class_name, key, token + 6);
				if (!strcmp (class_name, "ClueFlow") && e->flow)
					html_engine_set_object_data (e, e->flow);
			} else if (strncasecmp (token, "clear=", 6) == 0)
				html_engine_clear_class_data (e, class_name, token + 6);
			/* TODO clear flow data */
		}
		g_free (class_name);
		g_free (key);
	}
}


/*
  <em>             </em>
*/
/* EP CHECK: OK.  */
static void
parse_e (HTMLEngine *e, HTMLObject *_clue, const gchar *str)
{
	if ( strncmp( str, "em", 2 ) == 0 ) {
		push_font_style (e, GTK_HTML_FONT_STYLE_ITALIC);
		push_block (e, ID_EM, 1, block_end_font, FALSE, FALSE);
	} else if ( strncmp( str, "/em", 3 ) == 0 ) {
		pop_block (e, ID_EM, _clue);
	}
}


/*
  <font>           </font>
  <form>           </form>         partial
  <frame           <frame>
  <frameset        </frameset>
*/
/* EP CHECK: Fonts are done wrong, the rest is missing.  */
static void
parse_f (HTMLEngine *e, HTMLObject *clue, const gchar *str)
{
	if (strncmp (str, "font", 4) == 0) {
		GdkColor *color;
		HTMLColor *html_color = NULL;
		const HTMLFontFace *face = NULL;
		gint newSize;

		newSize = current_font_style (e) & GTK_HTML_FONT_STYLE_SIZE_MASK;

		/* The GdkColor API is not const safe!  */
		color = gdk_color_copy ((GdkColor *) current_color (e));

		html_string_tokenizer_tokenize (e->st, str + 5, " >");

		while (html_string_tokenizer_has_more_tokens (e->st)) {
			const gchar *token = html_string_tokenizer_next_token (e->st);
			if (strncasecmp (token, "size=", 5) == 0) {
				gint num = atoi (token + 5);

				/* FIXME implement basefont */
				if (*(token + 5) == '+' || *(token + 5) == '-')
					newSize = GTK_HTML_FONT_STYLE_SIZE_3 + num;
				else
					newSize = num;
				if (newSize > GTK_HTML_FONT_STYLE_SIZE_MAX)
					newSize = GTK_HTML_FONT_STYLE_SIZE_MAX;
				else if (newSize < GTK_HTML_FONT_STYLE_SIZE_1)
					newSize = GTK_HTML_FONT_STYLE_SIZE_1;
			} else if (strncasecmp (token, "face=", 5) == 0) {
				face = token + 5;
			} else if (strncasecmp (token, "color=", 6) == 0) {
				parse_color (token + 6, color);
				html_color = html_color_new_from_gdk_color (color);
			}
		}

		push_color (e, html_color ? html_color : current_color (e));

		if (html_color) 
			html_color_unref (html_color);

		push_font_face (e, face);
		push_font_style (e, newSize);

		push_block  (e, ID_FONT, 1, block_end_color_font, FALSE, FALSE);
	} else if (strncmp (str, "/font", 5) == 0) {
		pop_block (e, ID_FONT, clue);
	} else if (strncmp (str, "form", 4) == 0) {
                gchar *action = NULL;
                gchar *method = "GET";
                gchar *target = NULL;

 		html_string_tokenizer_tokenize (e->st, str + 5, " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			const gchar *token = html_string_tokenizer_next_token (e->st);

                        if ( strncasecmp( token, "action=", 7 ) == 0 ) {
                                action = g_strdup (token + 7);
                        } else if ( strncasecmp( token, "method=", 7 ) == 0 ) {
                                if ( strncasecmp( token + 7, "post", 4 ) == 0 )
                                        method = "POST";
                        } else if ( strncasecmp( token, "target=", 7 ) == 0 ) {
				target = g_strdup(token + 7);
                        }
                }

                e->form = html_form_new (e, action, method);
                e->formList = g_list_append (e->formList, e->form);
		
		g_free(action);
		g_free(target);

		if (! e->avoid_para) {
			close_anchor (e);
			e->avoid_para = TRUE;
			e->pending_para = TRUE;
		}
	} else if (strncmp (str, "/form", 5) == 0) {
		e->form = NULL;

		if (! e->avoid_para) {
			close_anchor (e);
			e->avoid_para = TRUE;
			e->pending_para = TRUE;
		}
	} else if (strncmp (str, "frameset", 8) == 0) {
		parse_frameset (e, clue, clue->max_width, str + 8);
	} else if (strncasecmp (str, "/frameset", 9) == 0) {
		if (!html_stack_is_empty (e->frame_stack))
			html_stack_pop (e->frame_stack);
	} else if (strncasecmp (str, "frame", 5) == 0) {
		char *src = NULL;
		HTMLObject *frame = NULL;
		gint margin_height = -1;
		gint margin_width = -1;
		GtkPolicyType scroll = GTK_POLICY_AUTOMATIC;

		src = NULL;
		html_string_tokenizer_tokenize (e->st, str + 5, " >");
		
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			const gchar *token = html_string_tokenizer_next_token (e->st);
			
			if (strncasecmp (token, "src=", 4) == 0) {
				src = g_strdup (token + 4);
			} else if (strncasecmp (token, "noresize", 8) == 0) {
			} else if (strncasecmp (token, "frameborder=", 12) == 0) {
			} else if (strncasecmp (token, "border=", 7) == 0) {
				/*
				 * Netscape and Mozilla recognize this to turn of all the between
				 * frame decoration.
				 */
			} else if (strncasecmp(token, "marginwidth=", 12) == 0) {
				margin_width = atoi (token + 12);
			} else if (strncasecmp(token, "marginheight=", 13) == 0) {
				margin_height = atoi (token + 13);
			} else if (strncasecmp(token, "scrolling=", 10) == 0) {
				scroll = parse_scroll (token + 10);
			}
		}
		
		frame = html_frame_new (GTK_WIDGET (e->widget), src, -1 , -1, FALSE);
		if (!html_frameset_append (html_stack_top (e->frame_stack), frame))
			html_object_destroy (frame);
		
		if (margin_height > 0)
			html_frame_set_margin_height (HTML_FRAME (frame), margin_height);
		if (margin_width > 0)
			html_frame_set_margin_width (HTML_FRAME (frame), margin_width);
		if (scroll != GTK_POLICY_AUTOMATIC)
			html_frame_set_scrolling (HTML_FRAME (frame), scroll);

		g_free (src);
	}
	
}


/*
  <h[1-6]>         </h[1-6]>
  <hr
*/
/* EP CHECK: OK */
static void
parse_h (HTMLEngine *p, HTMLObject *clue, const gchar *str)
{
	if (*str == 'h'
	    && (str[1] >= '1' && str[1] <= '6')) {
		HTMLHAlignType align;

		align = p->divAlign;

		html_string_tokenizer_tokenize (p->st, str + 3, " >");
		while (html_string_tokenizer_has_more_tokens (p->st)) {
			const gchar *token;

			token = html_string_tokenizer_next_token (p->st);
			if ( strncasecmp( token, "align=", 6 ) == 0 ) {
				align = parse_halign (token + 6, align);
			}
		}
		
		/* Start a new flow box */

		push_clueflow_style (p, HTML_CLUEFLOW_STYLE_H1 + (str[1] - '1'));
		close_flow (p, clue);

		push_block (p, ID_HEADER, 2, block_end_clueflow_style, p->divAlign, 0);

		p->divAlign = align;

		p->pending_para = FALSE;
		p->avoid_para = TRUE;
	} else if (*(str) == '/' && *(str + 1) == 'h'
		   && (*(str + 2) >= '1' && *(str + 2) <= '6')) {
		/* Close tag.  */
		pop_block (p, ID_HEADER, clue);

		p->avoid_para = TRUE;
		p->pending_para = FALSE;
	}
	else if (strncmp (str, "hr", 2) == 0) {
		gint size = 2;
		gint length = clue->max_width;
		gint percent = 100;
		HTMLHAlignType align = HTML_HALIGN_CENTER;
		gboolean shade = TRUE;

		html_string_tokenizer_tokenize (p->st, str + 3, " >");
		while (html_string_tokenizer_has_more_tokens (p->st)) {
			gchar *token = html_string_tokenizer_next_token (p->st);
			if (strncasecmp (token, "align=", 6) == 0) {
				align = parse_halign (token + 6, align);
			}
			else if (strncasecmp (token, "size=", 5) == 0) {
				size = atoi (token + 5);
			}
			else if (strncasecmp (token, "width=", 6) == 0) {
				if (strchr (token + 6, '%'))
					percent = atoi (token + 6);
				else if (isdigit (*(token + 6))) {
					length = atoi (token + 6);
					percent = 0;
				}
			}
			else if (strncasecmp (token, "noshade", 7) == 0) {
				shade = FALSE;
			}
		}

		append_element (p, clue, html_rule_new (length, percent, size, shade, align));
	}
}


/*
  <i>              </i>
  <img                             partial
  <input                           partial
  <iframe                          partial
*/
/* EP CHECK: map support missing.  `<input>' missing.  */
static void
parse_i (HTMLEngine *e, HTMLObject *_clue, const gchar *str)
{
	if (strncmp (str, "img", 3) == 0) {
		HTMLObject *image = 0;
		HTMLHAlignType align = HTML_HALIGN_NONE;
		HTMLVAlignType valign = HTML_VALIGN_NONE;
		HTMLColor *color = NULL;
		gchar *token = 0; 
		gchar *tmpurl = NULL;
		gchar *mapname = NULL;
		gchar *id = NULL;
		gchar *alt = NULL;
		gint width = -1;
		gint height = -1;
		gint border = 0;
		gint hspace = 0;
		gint vspace = 0;
		gboolean percent_width  = FALSE;
		gboolean percent_height = FALSE;
		gboolean ismap = FALSE;
		color        = current_color (e);
		if (e->url != NULL || e->target != NULL)
			border = 2;

		if (e->url != NULL || e->target != NULL)
			border = 2;

		html_string_tokenizer_tokenize (e->st, str + 4, " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			token = html_string_tokenizer_next_token (e->st);
			if (strncasecmp (token, "src=", 4) == 0) {
				tmpurl = g_strdup (token + 4);
			} else if (strncasecmp (token, "width=", 6) == 0) {
				if (isdigit (*(token + 6)))
					width = atoi (token + 6);
				percent_width = strchr (token + 6, '%') ? TRUE : FALSE;
			} else if (strncasecmp (token, "height=", 7) == 0) {
				if (isdigit (*(token + 7)))
					height = atoi (token + 7);
				percent_height = strchr (token + 7, '%') ? TRUE : FALSE;
			} else if (strncasecmp (token, "border=", 7) == 0) {
				border = atoi (token + 7);
			} else if (strncasecmp (token, "hspace=", 7) == 0) {
				hspace = atoi (token + 7);
			} else if (strncasecmp (token, "vspace=", 7) == 0) {
				vspace = atoi (token + 7);
			} else if (strncasecmp (token, "align=", 6) == 0) {
				if (strcasecmp (token + 6, "left") == 0)
					align = HTML_HALIGN_LEFT;
				else if (strcasecmp (token + 6, "right") == 0)
					align = HTML_HALIGN_RIGHT;
				else if (strcasecmp (token + 6, "top") == 0)
					valign = HTML_VALIGN_TOP;
				else if (strcasecmp (token + 6, "middle") == 0)
					valign = HTML_VALIGN_MIDDLE;
				else if (strcasecmp (token + 6, "bottom") ==0)
					valign = HTML_VALIGN_BOTTOM;
			} else if (strncasecmp (token, "id=", 3) == 0) {
				id = token + 3;
			} else if (strncasecmp (token, "alt=", 4) == 0) {
				alt = g_strdup (token + 4);
			} else if (strncasecmp (token, "usemap=", 7) == 0) {
				mapname = g_strdup (token + 7);
			} else if (strncasecmp (token, "ismap", 5) == 0) {
				ismap = TRUE;
			}
		}

		if (tmpurl != 0) {
			if (align != HTML_HALIGN_NONE)
				valign = HTML_VALIGN_BOTTOM;
			else if (valign == HTML_VALIGN_NONE)
				valign = HTML_VALIGN_BOTTOM;

			image = html_image_new (e->image_factory, tmpurl,
						e->url, e->target,
						width, height,
						percent_width, percent_height, border, color, valign);

			if (id) {
				html_engine_add_object_with_id (e, id, (HTMLObject *) image);
			}

			if (hspace < 0)
				hspace = 0;
			if (vspace < 0)
				vspace = 0;

			html_image_set_spacing (HTML_IMAGE (image), hspace, vspace);
			
			if (alt) {
				html_image_set_alt (HTML_IMAGE (image), alt);
				g_free (alt);
			}
			
			html_image_set_map (HTML_IMAGE (image), mapname, ismap);

			g_free (tmpurl);
			g_free (mapname);
				
			if (align == HTML_HALIGN_NONE) {
				append_element (e, _clue, image);
			} else {
				/* We need to put the image in a HTMLClueAligned.  */
				/* Man, this is *so* gross.  */
				HTMLClueAligned *aligned = HTML_CLUEALIGNED (html_cluealigned_new (NULL, 0, 0, _clue->max_width, 100));
				HTML_CLUE (aligned)->halign = align;
				html_clue_append (HTML_CLUE (aligned), HTML_OBJECT (image));
				append_element (e, _clue, HTML_OBJECT (aligned));
			}
		}		       
	}
	else if (strncmp( str, "input", 5 ) == 0) {
		if (e->form == NULL)
			return;
		
		parse_input (e, str + 6, _clue );
	} else if (strncmp( str, "iframe", 6) == 0) {
		parse_iframe (e, str + 7, _clue);
	} else if ( strncmp (str, "i", 1 ) == 0 ) {
		if ( str[1] == '>' || str[1] == ' ' ) {
			push_font_style (e, GTK_HTML_FONT_STYLE_ITALIC);
			push_block (e, ID_I, 1, block_end_font, FALSE, FALSE);
		}
	} else if ( strncmp( str, "/i", 2 ) == 0 ) {
		pop_block (e, ID_I, _clue);
	}
}


/*
  <kbd>            </kbd>
*/
/* EP CHECK: OK but font is wrong.  */
static void
parse_k (HTMLEngine *e, HTMLObject *_clue, const gchar *str)
{
	if ( strncmp(str, "kbd", 3 ) == 0 ) {
		push_font_style (e, GTK_HTML_FONT_STYLE_FIXED);
		push_block (e, ID_KBD, 1, block_end_font, 0, 0);
	} else if ( strncmp(str, "/kbd", 4 ) == 0 ) {
		pop_block (e, ID_KBD, _clue);
	}
}

static HTMLListType
get_list_type (gchar c)
{
	switch (c) {
	case 'i':
		return HTML_LIST_TYPE_ORDERED_LOWER_ROMAN;
	case 'I':
		return HTML_LIST_TYPE_ORDERED_UPPER_ROMAN;
	case 'a':
		return HTML_LIST_TYPE_ORDERED_LOWER_ALPHA;
	case 'A':
		return HTML_LIST_TYPE_ORDERED_UPPER_ALPHA;
	case '1':
	default:
		return HTML_LIST_TYPE_ORDERED_ARABIC;		
	}
}

/*
  <listing   unimplemented.
  <link      unimpemented.
  <li>
*/
/* EP CHECK: OK */
static void
parse_l (HTMLEngine *p, HTMLObject *clue, const gchar *str)
{
	if (strncmp (str, "link", 4) == 0) {
	} else if (strncmp (str, "li", 2) == 0) {
		HTMLListType listType;
		gint listLevel;
		gint itemNumber;
		
		listType = HTML_LIST_TYPE_UNORDERED;
		listLevel = 1;
		itemNumber = 1;
		
		close_anchor (p);
		
		if (! html_stack_is_empty (p->listStack)) {
			HTMLList *top;
			
			top = html_stack_top (p->listStack);
			
			listType = top->type;
			itemNumber = top->itemNumber;
			
			listLevel = html_stack_count (p->listStack);
		}

		html_string_tokenizer_tokenize (p->st, str + 3, " >");
		while (html_string_tokenizer_has_more_tokens (p->st)) {
			const gchar *token = html_string_tokenizer_next_token (p->st);

			if (!strncasecmp (token, "value=", 6))
				itemNumber = atoi (token + 6);
			else if (!strncasecmp (token, "type=", 5))
				listType = get_list_type (token [5]);
		}

		add_pending_paragraph_break (p, clue);
			
		close_flow (p, clue);
		
		p->flow = flow_new (p, HTML_CLUEFLOW_STYLE_LIST_ITEM, p->indent_level, listType, itemNumber);
		html_clue_append (HTML_CLUE (clue), p->flow);

		p->avoid_para = TRUE;

		if (! html_stack_is_empty (p->listStack)) {
			HTMLList *list;

			list = html_stack_top (p->listStack);
			list->itemNumber = itemNumber + 1;
		}
	}
}

/*
 <meta
 <map
 </map
*/

static void
parse_m (HTMLEngine *e, HTMLObject *_clue, const gchar *str )
{
	if (strncmp (str, "meta", 4) == 0) {
		int refresh = 0;
		int refresh_delay = 0;
		gchar *refresh_url = NULL;
		
		html_string_tokenizer_tokenize( e->st, str + 5, " >" );
		while ( html_string_tokenizer_has_more_tokens (e->st) ) {

			const gchar* token = html_string_tokenizer_next_token(e->st);
			if ( strncasecmp( token, "http-equiv=", 11 ) == 0 ) {
				if ( strncasecmp( token + 11, "refresh", 7 ) == 0 )
					refresh = 1;
			} else if ( strncasecmp( token, "content=", 8 ) == 0 ) {
				if (refresh) {
					const gchar *content;
					content = token + 8;

					/* The time in seconds until the refresh */
					refresh_delay = atoi(content);

					html_string_tokenizer_tokenize(e->st, content, ",;> ");
					while ( html_string_tokenizer_has_more_tokens (e->st) ) {
						const gchar* token = html_string_tokenizer_next_token(e->st);
						if ( strncasecmp( token, "url=", 4 ) == 0 )
							refresh_url = g_strdup (token + 4);
					}
					
					gtk_signal_emit (GTK_OBJECT (e), signals[REDIRECT], refresh_url, refresh_delay);
					if(refresh_url)
						g_free(refresh_url);
				}
			}
		}
	} else if (strncmp (str, "map", 3) == 0) {
		html_string_tokenizer_tokenize (e->st, str + 3, " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			const char* token = html_string_tokenizer_next_token (e->st);
			if (strncasecmp (token, "name=", 5) == 0) {
				const char *name = token + 5;

				e->map = HTML_MAP (html_map_new (name));

				if (e->map == NULL)
					return;

				html_engine_add_object_with_id (e, name, 
								HTML_OBJECT (e->map));
				if (e->flow == NULL) {
					html_clue_append (HTML_CLUE (_clue),
							  HTML_OBJECT (e->map));
				} else { 
					html_clue_append (HTML_CLUE (e->flow),
							  HTML_OBJECT (e->map));  
				}
			}
		}
	} else if (strncmp (str, "/map", 4) == 0) {
		e->map = NULL;
	}
}

static void
parse_n (HTMLEngine *e, HTMLObject *_clue, const gchar *str )
{
	if (strncasecmp (str, "noframe", 7) == 0) {
		static const char *end[] = {"</noframe", NULL};
		discard_body (e, end);
	}
}

/* called when some state in an embedded html object has changed ... do a redraw */
static void
html_object_changed(GtkHTMLEmbedded *eb, HTMLEngine *e)
{
	HTMLObject *object;

	object = gtk_object_get_data(GTK_OBJECT(eb), "embeddedelement");
	if (object)
		html_object_calc_size (object, e->painter);
	
	html_engine_schedule_update(e);
}


/*
<ol>             </ol>           partial
<option
<object
*/
/* EP CHECK: `<ol>' does not handle vspace correctly.  */
static void
parse_o (HTMLEngine *e, HTMLObject *_clue, const gchar *str )
{
	if ( strncmp( str, "ol", 2 ) == 0 ) {
		HTMLList *list;
		HTMLListType listType = HTML_LIST_TYPE_ORDERED_ARABIC;

		close_anchor (e);

		/* FIXME */
		push_block (e, ID_OL, 2, block_end_list, e->indent_level, html_stack_is_empty (e->listStack));

		html_string_tokenizer_tokenize( e->st, str + 3, " >" );

		while ( html_string_tokenizer_has_more_tokens (e->st) ) {
			const char* token;

			token = html_string_tokenizer_next_token (e->st);
			if ( strncasecmp( token, "type=", 5 ) == 0 )
				listType = get_list_type (token [5]);
		}

		list = html_list_new (listType);
		html_stack_push (e->listStack, list);

		e->indent_level++;
	}
	else if ( strncmp( str, "/ol", 3 ) == 0 ) {
		pop_block (e, ID_OL, _clue);
	}
	else if ( strncmp( str, "option", 6 ) == 0 ) {
		gchar *value = NULL;
		gboolean selected = FALSE;

		if ( !e->formSelect )
			return;

		html_string_tokenizer_tokenize( e->st, str + 3, " >" );

		while ( html_string_tokenizer_has_more_tokens (e->st) ) {
			const char* token;
			
			token = html_string_tokenizer_next_token (e->st);
			
			if ( strncasecmp( token, "value=", 6 ) == 0 ) {

				value = g_strdup (token + 6);
			}
			else if ( strncasecmp( token, "selected", 8 ) == 0 ) {

				selected = TRUE;
			}
		}

		if ( e->inOption )
			html_select_set_text (e->formSelect, e->formText->str);

		html_select_add_option (e->formSelect, value, selected );

		e->inOption = TRUE;
		g_string_assign (e->formText, "");
	} else if ( strncmp( str, "/option", 7 ) == 0 ) {
		if ( e->inOption )
			html_select_set_text (e->formSelect, e->formText->str);

		e->inOption = FALSE;
	} else if ( strncmp( str, "object", 6 ) == 0 ) {
		parse_object (e, _clue, _clue->max_width, str + 6);
	}
}


/*
  <p
  <pre             </pre>
  <param
*/
static void
parse_p (HTMLEngine *e, HTMLObject *clue, const gchar *str)
{
	if ( strncmp( str, "pre", 3 ) == 0 ) {
		close_flow (e, clue);
		push_clueflow_style (e, HTML_CLUEFLOW_STYLE_PRE);
		e->inPre = TRUE;
		push_block (e, ID_PRE, 2, block_end_pre, e->divAlign, 0);
	} else if ( strncmp( str, "/pre", 4 ) == 0 ) {
		pop_block (e, ID_PRE, clue);
		close_flow (e, clue);
	} else if ( strncmp( str, "param", 5) == 0 ) {
		if (! html_stack_is_empty (e->embeddedStack)) {
			GtkHTMLEmbedded *eb;
			char *name = NULL, *value = NULL;

			eb = html_stack_top (e->embeddedStack);

			html_string_tokenizer_tokenize (e->st, str + 6, " >");
			while ( html_string_tokenizer_has_more_tokens (e->st) ) {
				const char *token = html_string_tokenizer_next_token (e->st);
				if ( strncasecmp( token, "name=", 5 ) == 0 ) {
					name = g_strdup(token+5);
				} else if ( strncasecmp( token, "value=", 6 ) == 0 ) {
					value = g_strdup(token+6);
				}
			}

			if (name!=NULL)
				gtk_html_embedded_set_parameter(eb, name, value);

			g_free(name);
			g_free(value);
		}					
	} else if (*(str) == 'p' && ( *(str + 1) == ' ' || *(str + 1) == '>')) {
		gchar *token;

		html_string_tokenizer_tokenize (e->st, (gchar *)(str + 2), " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			token = html_string_tokenizer_next_token (e->st);
			if (strncasecmp (token, "align=", 6) == 0) {
				e->divAlign = parse_halign (token + 6, e->divAlign);
			}
		}

		if (! e->avoid_para) {
			close_anchor (e);
			e->avoid_para = TRUE;
			e->pending_para = TRUE;
		}
	} else if (*(str) == '/' && *(str + 1) == 'p'
		   && (*(str + 2) == ' ' || *(str + 2) == '>')) {

		if (! e->avoid_para) {
			e->avoid_para = TRUE;
			e->pending_para = TRUE;
		}
	}
}


/*
  <select>            </select>
  <small>             </small>
  <strong>            </strong>
  <sub>               </sub>
  <sup>               </sup>
  <s>                 </s>
  <strike>            </strike>
*/
static void
parse_s (HTMLEngine *e, HTMLObject *clue, const gchar *str)
{
	if (strncmp (str, "small", 5) == 0) {
		push_font_style (e, GTK_HTML_FONT_STYLE_SIZE_2);
		push_block (e, ID_SMALL, 1, block_end_font, 0, 0);
	} else if (strncmp (str, "/small", 6) == 0 ) {
		pop_block (e, ID_SMALL, clue);
	} else if (strncmp (str, "strong", 6) == 0) {
		push_font_style (e, GTK_HTML_FONT_STYLE_BOLD);
		push_block (e, ID_STRONG, 1, block_end_font, 0, 0);
	} else if (strncmp (str, "/strong", 7) == 0) {
		pop_block (e, ID_STRONG, clue);
	} else if (strncmp (str, "select", 6) == 0) {
                gchar *name = NULL;
		gint size = 0;
		gboolean multi = FALSE;

		if (!e->form)
			return;
                    
		html_string_tokenizer_tokenize (e->st, str + 7, " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			const gchar *token = html_string_tokenizer_next_token (e->st);

                        if ( strncasecmp( token, "name=", 5 ) == 0 ) {
				name = g_strdup(token + 5);
                        } else if ( strncasecmp( token, "size=", 5 ) == 0 ) {
				size = atoi (token + 5);
                        } else if ( strncasecmp( token, "multiple", 8 ) == 0 ) {
				multi = TRUE;
                        }
                }
                
                e->formSelect = HTML_SELECT (html_select_new (GTK_WIDGET(e->widget), name, size, multi));
                html_form_add_element (e->form, HTML_EMBEDDED ( e->formSelect ));

		append_element (e, clue, HTML_OBJECT (e->formSelect));
		
		g_free(name);
	}
	else if (strncmp (str, "/select", 7) == 0) {
		if ( e->inOption )
			html_select_set_text (e->formSelect, e->formText->str);

		e->inOption = FALSE;
		e->formSelect = NULL;
		e->eat_space = FALSE;
	} else if (strncmp (str, "sub", 3) == 0) {
		if (str[3] == '>' || str[3] == ' ') {
			push_font_style (e, GTK_HTML_FONT_STYLE_SUBSCRIPT);
			push_block (e, ID_SUB, 1, block_end_font, FALSE, FALSE);
		}
	} else if (strncmp (str, "/sub", 4) == 0) {
		pop_block (e, ID_SUB, clue);
	} else if (strncmp (str, "sup", 3) == 0) {
		if (str[3] == '>' || str[3] == ' ') {
			push_font_style (e, GTK_HTML_FONT_STYLE_SUPERSCRIPT);
			push_block (e, ID_SUP, 1, block_end_font, FALSE, FALSE);
		}
	} else if (strncmp (str, "/sup", 4) == 0) {
		pop_block (e, ID_SUP, clue);
	} else 	if (strncmp (str, "strike", 6) == 0
		    || (strncmp (str, "s", 1) == 0 && (str[1] == '>' || str[1] == ' '))) {
		push_font_style (e, GTK_HTML_FONT_STYLE_STRIKEOUT);
		push_block (e, ID_STRIKEOUT, 1, block_end_font, 0, 0);
	} else if (strncmp (str, "/strike", 7) == 0
		   || (strncmp (str, "/s", 2) == 0 && (str[2] == '>' || str[2] == ' '))) {
		pop_block (e, ID_STRIKEOUT, clue);
	}
}


/*
  <table           </table>        most
  <textarea        </textarea>
  <title>          </title>
  <tt>             </tt>
*/
/* EP CHECK: `<tt>' uses the wrong font.  `<textarea>' is missing.  Rest is
   OK.  */
static void
parse_t (HTMLEngine *e, HTMLObject *clue, const gchar *str)
{
	if (strncmp (str, "table", 5) == 0) {
		close_anchor (e);

		close_flow (e, clue);
		parse_table (e, clue, clue->max_width, str + 6);
		close_flow (e, clue);

		e->avoid_para = FALSE;
	}
	else if (strncmp (str, "title", 5) == 0) {
		e->inTitle = TRUE;
		e->title = g_string_new ("");
	}
	else if (strncmp (str, "/title", 6) == 0) {
		/*
		 * only emit the title changed signal if we have a 
		 * valid title 
		 */
		if (e->inTitle && e->title) 
			gtk_signal_emit (GTK_OBJECT (e), signals[TITLE_CHANGED]);
		e->inTitle = FALSE;
	}
	else if ( strncmp( str, "tt", 2 ) == 0 ) {
		push_font_style (e, GTK_HTML_FONT_STYLE_FIXED);
		push_block (e, ID_TT, 1, block_end_font, 0, 0);
	} else if ( strncmp( str, "/tt", 3 ) == 0 ) {
		pop_block (e, ID_TT, clue);
	}
	else if (strncmp (str, "textarea", 8) == 0) {
                gchar *name = NULL;
		gint rows = 5, cols = 40;

		if (!e->form)
			return;
                    
		html_string_tokenizer_tokenize (e->st, str + 9, " >");
		while (html_string_tokenizer_has_more_tokens (e->st)) {
			const gchar *token = html_string_tokenizer_next_token (e->st);

                        if ( strncasecmp( token, "name=", 5 ) == 0 )
                        {
				name = g_strdup(token + 5);
                        }
                        else if ( strncasecmp( token, "rows=", 5 ) == 0 )
                        {
				rows = atoi (token + 5);
                        }
                        else if ( strncasecmp( token, "cols=", 5 ) == 0 )
                        {
				cols = atoi (token + 5);
                        }
                }
                
                e->formTextArea = HTML_TEXTAREA (html_textarea_new (GTK_WIDGET(e->widget), name, rows, cols));
                html_form_add_element (e->form, HTML_EMBEDDED ( e->formTextArea ));

		append_element (e, clue, HTML_OBJECT (e->formTextArea));

		g_string_assign (e->formText, "");
		e->inTextArea = TRUE;

		push_block(e, ID_TEXTAREA, 3, NULL, 0, 0);
		
		if(name)
			g_free(name);
	}
	else if (strncmp (str, "/textarea", 9) == 0) {
		pop_block(e, ID_TEXTAREA, clue);

		if ( e->inTextArea )
			html_textarea_set_text (e->formTextArea, e->formText->str);

		e->inTextArea = FALSE;
		e->formTextArea = NULL;
		e->eat_space = FALSE;
	}

}


/*
  <u>              </u>
  <ul              </ul>
*/
/* EP CHECK: OK */
static void
parse_u (HTMLEngine *e, HTMLObject *clue, const gchar *str)
{
	if (strncmp (str, "ul", 2) == 0) {

		close_anchor (e);
		close_flow (e, clue);

		push_block (e, ID_UL, 2, block_end_list, e->indent_level, html_stack_is_empty (e->listStack));

		html_string_tokenizer_tokenize (e->st, str + 3, " >");
		while (html_string_tokenizer_has_more_tokens (e->st))
			html_string_tokenizer_next_token (e->st);
		
		html_stack_push (e->listStack, html_list_new (HTML_LIST_TYPE_UNORDERED));
		e->flow = NULL;

		if (e->indent_level > 0)
			add_pending_paragraph_break (e, clue);

		e->indent_level++;

		e->avoid_para = TRUE;
	} else if (strncmp (str, "/ul", 3) == 0) {
		pop_block (e, ID_UL, clue);
	}
	else if (strncmp (str, "u", 1) == 0) {
		if (str[1] == '>' || str[1] == ' ') {
			push_font_style (e, GTK_HTML_FONT_STYLE_UNDERLINE);
			push_block (e, ID_U, 1, block_end_font, FALSE, FALSE);
		}
	}
	else if (strncmp (str, "/u", 2) == 0) {
		pop_block (e, ID_U, clue);
	}
}


/*
  <var>            </var>
*/
/* EP CHECK: OK */
static void
parse_v (HTMLEngine *e, HTMLObject * _clue, const char *str )
{
	if ( strncmp(str, "var", 3 ) == 0 ) {
		push_font_style (e, GTK_HTML_FONT_STYLE_FIXED);
	   	push_block(e, ID_VAR, 1, block_end_font, 0, 0);
	} else if ( strncmp( str, "/var", 4 ) == 0) {
		pop_block(e, ID_VAR, _clue);
	}
}


/* Parsing vtable.  */

typedef void (*HTMLParseFunc)(HTMLEngine *p, HTMLObject *clue, const gchar *str);
static HTMLParseFunc parseFuncArray[26] = {
	parse_a,
	parse_b,
	parse_c,
	parse_d,
	parse_e,
	parse_f,
	NULL,
	parse_h,
	parse_i,
	NULL,
	parse_k,
	parse_l,
	parse_m,
	parse_n,
	parse_o,
	parse_p,
	NULL,
	NULL,
	parse_s,
	parse_t,
	parse_u,
	parse_v,
	NULL,
	NULL,
	NULL,
	NULL
};

static void
parse_one_token (HTMLEngine *p, HTMLObject *clue, const gchar *str)
{
	if (*str == '<') {
		gint indx;
		
		str++;
		
		if (*str == '/')
			indx = *(str + 1) - 'a';
		else
			indx = *str - 'a';

		if (indx >= 0 && indx < 26) {
			/* FIXME: This should be removed */
			if (parseFuncArray[indx] != NULL) {
				(* parseFuncArray[indx])(p, clue, str);
			} 
		}
	}
}


guint
html_engine_get_type (void)
{
	static guint html_engine_type = 0;

	if (!html_engine_type) {
		static const GtkTypeInfo html_engine_info = {
			"HTMLEngine",
			sizeof (HTMLEngine),
			sizeof (HTMLEngineClass),
			(GtkClassInitFunc) html_engine_class_init,
			(GtkObjectInitFunc) html_engine_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		html_engine_type = gtk_type_unique (GTK_TYPE_OBJECT, &html_engine_info);
	}

	return html_engine_type;
}

static void
clear_selection (HTMLEngine *e)
{
	if (e->selection) {
		html_interval_destroy (e->selection);
		e->selection = NULL;
	}
}

static void
html_engine_destroy (GtkObject *object)
{
	HTMLEngine *engine;
	GList *p;

	engine = HTML_ENGINE (object);

	html_undo_destroy (engine->undo);
	html_engine_clipboard_clear (engine);

	if (engine->invert_gc != NULL)
		gdk_gc_destroy (engine->invert_gc);

	html_cursor_destroy (engine->cursor);
	if (engine->mark != NULL)
		html_cursor_destroy (engine->mark);

	html_tokenizer_destroy (engine->ht);
	html_string_tokenizer_destroy (engine->st);
	html_settings_destroy (engine->settings);
	html_settings_destroy (engine->defaultSettings);
	html_color_unref (engine->insertion_color);
	if (engine->clue != NULL)
		html_object_destroy (engine->clue);
	engine->clue = NULL;
	html_image_factory_free (engine->image_factory);

	gtk_object_unref (GTK_OBJECT (engine->painter));

	html_stack_destroy (engine->color_stack);
	html_stack_destroy (engine->font_face_stack);
	html_stack_destroy (engine->font_style_stack);
	html_stack_destroy (engine->clueflow_style_stack);
	html_stack_destroy (engine->frame_stack);

	html_stack_destroy (engine->listStack);
	html_stack_destroy (engine->glossaryStack);
	html_stack_destroy (engine->embeddedStack);

	for (p = engine->tempStrings; p != NULL; p = p->next)
		g_free (p->data);
	g_list_free (engine->tempStrings);

	html_draw_queue_destroy (engine->draw_queue);

	html_engine_edit_selection_updater_destroy (engine->selection_updater);

	if (engine->search_info)
		html_search_destroy (engine->search_info);
	clear_selection (engine);

        /* Finally, destroy timers.  */

	if (engine->timerId != 0)
		gtk_idle_remove (engine->timerId);
	if (engine->blinking_timer_id != 0)
		gtk_timeout_remove (engine->blinking_timer_id);
	if (engine->updateTimer != 0)
		gtk_timeout_remove (engine->updateTimer);

	if (engine->insertion_url) g_free (engine->insertion_url);
	if (engine->insertion_target) g_free (engine->insertion_target);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
html_engine_set_arg (GtkObject        *object,
		     GtkArg           *arg,
		     guint             arg_id)
{
	HTMLEngine *engine = HTML_ENGINE (object);

	if (arg_id == 1) {
		GtkHTMLClassProperties *prop;

		engine->widget          = GTK_HTML (GTK_VALUE_OBJECT (*arg));
		engine->settings        = html_settings_new (GTK_WIDGET (engine->widget));
		engine->defaultSettings = html_settings_new (GTK_WIDGET (engine->widget));
		html_colorset_add_slave (engine->settings->color_set, engine->painter->color_set);

		engine->insertion_color = html_colorset_get_color (engine->settings->color_set, HTMLTextColor);
		html_color_ref (engine->insertion_color);

		prop = GTK_HTML_CLASS (GTK_OBJECT (engine->widget)->klass)->properties;
	}
}

static void
html_engine_class_init (HTMLEngineClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *)klass;

	parent_class = gtk_type_class (GTK_TYPE_OBJECT);

	signals [SET_BASE] =
		gtk_signal_new ("set_base",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, set_base),
				gtk_marshal_NONE__STRING,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_STRING);

	signals [SET_BASE_TARGET] =
		gtk_signal_new ("set_base_target",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, set_base_target),
				gtk_marshal_NONE__STRING,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_STRING);

	signals [LOAD_DONE] = 
		gtk_signal_new ("load_done",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, load_done),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	signals [TITLE_CHANGED] = 
		gtk_signal_new ("title_changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, title_changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	signals [URL_REQUESTED] =
		gtk_signal_new ("url_requested",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, url_requested),
				gtk_marshal_NONE__POINTER_POINTER,
				GTK_TYPE_NONE, 2,
				GTK_TYPE_STRING,
				GTK_TYPE_POINTER);

	signals [DRAW_PENDING] =
		gtk_signal_new ("draw_pending",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, draw_pending),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	signals [REDIRECT] =
		gtk_signal_new ("redirect",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, redirect),
				gtk_marshal_NONE__POINTER_INT,
				GTK_TYPE_NONE, 2,
				GTK_TYPE_STRING,
				GTK_TYPE_INT);

	signals [SUBMIT] =
		gtk_signal_new ("submit",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, submit),
				gtk_marshal_NONE__POINTER_POINTER_POINTER,
				GTK_TYPE_NONE, 3,
				GTK_TYPE_STRING,
				GTK_TYPE_STRING,
				GTK_TYPE_STRING);

	signals [OBJECT_REQUESTED] =
		gtk_signal_new ("object_requested",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (HTMLEngineClass, object_requested),
				gtk_marshal_BOOL__POINTER,
				GTK_TYPE_BOOL, 1,
				GTK_TYPE_POINTER);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	gtk_object_add_arg_type ("HTMLEngine::html",
				 GTK_TYPE_HTML,
				 GTK_ARG_WRITABLE | GTK_ARG_CONSTRUCT_ONLY,
				 1);

	object_class->set_arg = html_engine_set_arg;
	object_class->destroy = html_engine_destroy;

	html_engine_init_magic_links ();

	/* Initialize the HTML objects.  */
	html_types_init ();
}

static void
html_engine_init (HTMLEngine *engine)
{
	engine->clue = NULL;

	/* STUFF might be missing here!   */
	engine->freeze_count = 0;
	engine->thaw_idle_id = 0;

	engine->window = NULL;
	engine->invert_gc = NULL;

	/* settings, colors and painter init */
	engine->painter = html_gdk_painter_new (TRUE);

	engine->newPage = FALSE;

	engine->editable = FALSE;
	engine->clipboard = NULL;
	engine->clipboard_stack = NULL;
	engine->selection_stack  = NULL;

	engine->ht = html_tokenizer_new ();
	engine->st = html_string_tokenizer_new ();
	engine->image_factory = html_image_factory_new(engine);

	engine->undo = html_undo_new ();

	engine->font_style_stack = html_stack_new (NULL);
	engine->font_face_stack = html_stack_new (g_free);
	engine->color_stack = html_stack_new ((HTMLStackFreeFunc) html_color_unref);
	engine->clueflow_style_stack = html_stack_new (NULL);
	engine->frame_stack = html_stack_new (NULL);

	engine->listStack = html_stack_new ((HTMLStackFreeFunc) html_list_destroy);
	engine->glossaryStack = html_stack_new (NULL);
	/* FIXME rodo engine->embeddedStack = html_stack_new ((HTMLStackFreeFunc) gtk_object_unref); */
	engine->embeddedStack = html_stack_new (NULL);

	engine->url = NULL;
	engine->target = NULL;

	engine->leftBorder = LEFT_BORDER;
	engine->rightBorder = RIGHT_BORDER;
	engine->topBorder = TOP_BORDER;
	engine->bottomBorder = BOTTOM_BORDER;

	engine->inPre = FALSE;
	engine->inTitle = FALSE;

	engine->tempStrings = NULL;

	engine->draw_queue = html_draw_queue_new (engine);

	engine->map = NULL;
	engine->formList = NULL;

	engine->avoid_para = TRUE;
	engine->pending_para = FALSE;

	engine->indent_level = 0;

	engine->have_focus = FALSE;

	engine->cursor = html_cursor_new ();
	engine->mark = NULL;
	engine->cursor_hide_count = 0;
	engine->blinking_timer_id = 0;
	engine->blinking_status = FALSE;
	engine->insertion_font_style = GTK_HTML_FONT_STYLE_DEFAULT;
	engine->insertion_url = NULL;
	engine->insertion_target = NULL;
	engine->selection = NULL;
	engine->shift_selection = FALSE;
	engine->selection_mode = FALSE;
	engine->block_selection = 0;
	engine->cursor_position_stack = NULL;

	engine->selection_updater = html_engine_edit_selection_updater_new (engine);

	engine->search_info = NULL;
	engine->need_spell_check = FALSE;

	html_engine_print_set_min_split_index (engine, .75);
}

HTMLEngine *
html_engine_new (GtkWidget *w)
{
	GtkObject *engine;

	engine = gtk_object_new (html_engine_get_type (), "html", w, NULL);

	return HTML_ENGINE (engine);
}

void
html_engine_realize (HTMLEngine *e,
		     GdkWindow *window)
{
	GdkGCValues gc_values;

	g_return_if_fail (e != NULL);
	g_return_if_fail (window != NULL);
	g_return_if_fail (e->window == NULL);

	e->window = window;

	html_gdk_painter_realize (HTML_GDK_PAINTER (e->painter), window);

	gc_values.function = GDK_INVERT;
	e->invert_gc = gdk_gc_new_with_values (e->window, &gc_values, GDK_GC_FUNCTION);
}

void
html_engine_unrealize (HTMLEngine *e)
{
	html_gdk_painter_unrealize (HTML_GDK_PAINTER (e->painter));

	e->window = NULL;
}


/* This function makes sure @engine can be edited properly.  In order
   to be editable, the beginning of the document must have the
   following structure:
   
     HTMLClueV (cluev)
       HTMLClueFlow (head)
 	 HTMLObject (child) */
void
html_engine_ensure_editable (HTMLEngine *engine)
{
	HTMLObject *cluev;
	HTMLObject *head;
	HTMLObject *child;

	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	cluev = engine->clue;
	if (cluev == NULL)
		engine->clue = cluev = html_cluev_new (0, 0, 100);

	head = HTML_CLUE (cluev)->head;
	if (head == NULL || HTML_OBJECT_TYPE (head) != HTML_TYPE_CLUEFLOW) {
		HTMLObject *clueflow;

		clueflow = flow_new (engine, HTML_CLUEFLOW_STYLE_NORMAL, 0, HTML_LIST_TYPE_UNORDERED, 0);
		html_clue_prepend (HTML_CLUE (cluev), clueflow);

		head = clueflow;
	}

	child = HTML_CLUE (head)->head;
	if (child == NULL) {
		HTMLObject *text;

		text = text_new (engine, "", engine->insertion_font_style, engine->insertion_color);
		html_text_set_font_face (HTML_TEXT (text), current_font_face (engine));
		html_clue_prepend (HTML_CLUE (head), text);
	}
}


void
html_engine_draw_background (HTMLEngine *e,
			     gint x, gint y, gint w, gint h)
{
	HTMLImagePointer *bgpixmap;
	GdkPixbuf *pixbuf = NULL;

	/* return if no background pixmap is set */
	bgpixmap = e->bgPixmapPtr;
	if (bgpixmap && bgpixmap->pixbuf) {
		pixbuf = bgpixmap->pixbuf;
	}

	html_painter_draw_background (e->painter, 
				      &html_colorset_get_color_allocated (e->painter, HTMLBgColor)->color,
				      pixbuf,
				      x, y,
				      w, h,
				      e->x_offset + x, e->y_offset + y);
}

void
html_engine_stop_parser (HTMLEngine *e)
{
	if (!e->parsing)
		return;

	if (e->timerId != 0) {
		gtk_idle_remove (e->timerId);
		e->timerId = 0;
	}
	
	e->parsing = FALSE;

	html_stack_clear (e->color_stack);
	html_stack_clear (e->font_style_stack);
	html_stack_clear (e->font_face_stack);
	html_stack_clear (e->clueflow_style_stack);
	html_stack_clear (e->frame_stack);
}

/* used for cleaning up the id hash table */
static gboolean
id_table_free_func (gpointer key, gpointer val, gpointer data)
{
	g_free (key);
	return TRUE;
}

static void 
html_engine_id_table_clear (HTMLEngine *e)
{
	if (e->id_table) {
		g_hash_table_freeze (e->id_table);
		g_hash_table_foreach_remove (e->id_table, id_table_free_func, NULL);
		g_hash_table_thaw (e->id_table);
		g_hash_table_destroy (e->id_table);
		e->id_table = NULL;
	}
}

static gboolean
class_data_free_func (gpointer key, gpointer val, gpointer data)
{
	g_free (key);
	g_free (val);

	return TRUE;
}

static gboolean
class_data_table_free_func (gpointer key, gpointer val, gpointer data)
{
	GHashTable *t;

	t = (GHashTable *) val;
	g_hash_table_freeze (t);
	g_hash_table_foreach_remove (t, class_data_free_func, NULL);
	g_hash_table_thaw (t);
	g_hash_table_destroy (t);

	g_free (key);

	return TRUE;
}

static void 
html_engine_class_data_clear (HTMLEngine *e)
{
	if (e->class_data) {
		g_hash_table_freeze (e->class_data);
		g_hash_table_foreach_remove (e->class_data, class_data_table_free_func, NULL);
		g_hash_table_thaw (e->class_data);
		g_hash_table_destroy (e->class_data);
		e->class_data = NULL;
	}
}

/* #define LOG_INPUT */

GtkHTMLStream *
html_engine_begin (HTMLEngine *e, char *content_type)
{
	GtkHTMLStream *new_stream;

	html_tokenizer_begin (e->ht, content_type);
	
	free_block (e); /* Clear the block stack */

	html_engine_stop_parser (e);
	e->writing = TRUE;

	html_engine_id_table_clear (e);
	html_engine_class_data_clear (e);
	html_image_factory_stop_animations (e->image_factory);
	html_image_factory_cleanup (e->image_factory);


	new_stream = gtk_html_stream_new (GTK_HTML (e->widget),
					  html_engine_stream_types,
					  html_engine_stream_write,
					  html_engine_stream_end,
					  e);
#ifdef LOG_INPUT
	new_stream = gtk_html_stream_log_new (GTK_HTML (e->widget), new_stream);
#endif
	
	e->newPage = TRUE;
	clear_selection (e);

	html_engine_thaw_idle_reset (e);

	g_slist_free (e->cursor_position_stack);
	e->cursor_position_stack = NULL;

	return new_stream;
}

char *engine_content_types[]= {"text/html", NULL};

static char **
html_engine_stream_types (GtkHTMLStream *handle,
			  gpointer data)
{
	return engine_content_types;
}

static void
html_engine_stream_write (GtkHTMLStream *handle,
			  const gchar *buffer,
			  size_t size,
			  gpointer data)
{
	HTMLEngine *e;

	e = HTML_ENGINE (data);

	if (buffer == NULL)
		return;

	html_tokenizer_write (e->ht, buffer, size);

	if (e->parsing && e->timerId == 0) {
		e->timerId = gtk_idle_add ((GtkFunction) html_engine_timer_event, e);
	}
}

static void
update_embedded (GtkWidget *widget, gpointer data)
{
	GtkHTML *html = data;
	HTMLObject *obj;

	/* FIXME: this is a hack to update all the embedded widgets when
	 * they get moved off screen it isn't gracefull, but it should be a effective
	 * it also duplicates the draw_obj function in the drawqueue function very closely
	 * the common code in these functions should be merged and removed, but until then
	 * enjoy having your objects out of the way :)
	 */
	
	obj = gtk_object_get_data (GTK_OBJECT (widget), "embeddedelement");
	if (obj) {
		HTMLEngine *e;
		HTMLObject *p;
		gint tx, ty;
		gint x, y, width, height;

		e = html->engine;
		
		tx = 0;
		ty = 0;

		for (p = obj->parent; p != NULL && HTML_OBJECT_TYPE (p) != HTML_TYPE_IFRAME; p = p->parent) {
			tx += p->x;
			ty += p->y - p->ascent;
		}
		
		tx = tx + e->leftBorder - e->x_offset;
		ty = ty + e->topBorder - e->y_offset;
		
		/* Then prepare for drawing.  We will only update this object, so we
		   only allocate enough size for it.  */
		x = obj->x;
		y = obj->y - obj->ascent;
		width = obj->width;
		height = obj->ascent + obj->descent;

		html_painter_begin (e->painter, tx + x, ty + y, tx+ x + width, ty + y + height);

		html_object_draw (obj,
				  e->painter, 
				  x, y,
				  width, height,
				  tx, ty);	

		html_painter_end (e->painter);
	}
}

static gboolean
html_engine_update_event (HTMLEngine *e)
{
	e->updateTimer = 0;

	if (html_engine_get_editable (e))
		html_engine_hide_cursor (e);
	html_engine_calc_size (e);

	if (GTK_LAYOUT (e->widget)->vadjustment == NULL
	    || ! html_gdk_painter_realized (HTML_GDK_PAINTER (e->painter)))
		return FALSE;
	
	/* Adjust the scrollbars */
	gtk_html_private_calc_scrollbars (e->widget, NULL, NULL);

	/* Scroll page to the top on first display */
	if (e->newPage) {
		gtk_adjustment_set_value (GTK_LAYOUT (e->widget)->vadjustment, 0);
		e->newPage = FALSE;
		if (! e->parsing && e->editable)
			html_cursor_home (e->cursor, e);
	}

	/* Is y_offset too big? */
	if (html_engine_get_doc_height (e) - e->y_offset < e->height) {
		e->y_offset = html_engine_get_doc_height (e) - e->height;
		if (e->y_offset < 0)
			e->y_offset = 0;
	}
		
	/* Is x_offset too big? */
	if (html_engine_get_doc_width (e) - e->x_offset < e->width) {
		e->x_offset = html_engine_get_doc_width (e) - e->width;
		if (e->x_offset < 0)
			e->x_offset = 0;
	}
	
	gtk_adjustment_set_value (GTK_LAYOUT (e->widget)->vadjustment, e->y_offset);
	gtk_adjustment_set_value (GTK_LAYOUT (e->widget)->hadjustment, e->x_offset);

	html_image_factory_deactivate_animations (e->image_factory);
	gtk_container_forall (GTK_CONTAINER (e->widget), update_embedded, e->widget);
	html_engine_draw (e, 0, 0, e->width, e->height);

	if (html_engine_get_editable (e))
		html_engine_show_cursor (e);
	
	return FALSE;
}


void
html_engine_schedule_update (HTMLEngine *p)
{
	if(p->updateTimer == 0)
		p->updateTimer = gtk_idle_add ((GtkFunction) html_engine_update_event, p);
}


gboolean
html_engine_goto_anchor (HTMLEngine *e,
			 const gchar *anchor)
{
	GtkAdjustment *vadj;
	HTMLAnchor *a;
	gint x, y;

	g_return_val_if_fail (anchor != NULL, FALSE);

	if (!e->clue)
		return FALSE;

	x = y = 0;
	a = html_object_find_anchor (e->clue, anchor, &x, &y);

	if (a == NULL) {
		/* g_warning ("Anchor: \"%s\" not found", anchor); */
		return FALSE;
	}

	vadj = GTK_LAYOUT (e->widget)->vadjustment;

	if (y < vadj->upper - vadj->page_size)
		gtk_adjustment_set_value (vadj, y);
	else
		gtk_adjustment_set_value (vadj, vadj->upper - vadj->page_size);

	return TRUE;
}

#if 0
struct {
	HTMLEngine *e;
	HTMLObject *o;
} respon

static void 
find_engine (HTMLObject *o, HTMLEngine *e, HTMLEngine **parent_engine)
{
	
}

gchar *
html_engine_get_object_base (HTMLEngine *e, HTMLObject *o) 
{
	HTMLEngine *parent_engine = NULL;
	
	g_return_if_fail (e != NULL);
	g_return_if_fail (o != NULL);
	
	html_object_forall (o, e, find_engine, &parent_engine);

	
}
#endif 

static gboolean
html_engine_timer_event (HTMLEngine *e)
{
	static const gchar *end[] = { "</body>", 0};
	gint lastHeight;
	gboolean retval = TRUE;

	/* Has more tokens? */
	if (!html_tokenizer_has_more_tokens (e->ht) && e->writing) {
		retval = FALSE;
		goto out;
	}

	/* Getting height */
	lastHeight = html_engine_get_doc_height (e);

	e->parseCount = e->granularity;

	/* Parsing body height */
	if (parse_body (e, e->clue, end, TRUE))
	  	html_engine_stop_parser (e);

	html_engine_schedule_update (e);

	if (!e->parsing)
		retval = FALSE;

 out:
	if (!retval) {
		if(e->updateTimer != 0) {
			gtk_idle_remove (e->updateTimer);
			html_engine_update_event (e);
		}
			
		e->timerId = 0;
	}

	return retval;
}

/* This makes sure that the last HTMLClueFlow is non-empty.  */
static void
ensure_last_clueflow (HTMLEngine *engine)
{
	HTMLObject *new_text;
	HTMLClue *clue;
	HTMLClue *last_clueflow;

	clue = HTML_CLUE (engine->clue);
	if (clue == NULL)
		return;

	last_clueflow = HTML_CLUE (clue->tail);
	if (last_clueflow == NULL)
		return;

	if (last_clueflow->tail != NULL)
		return;

	new_text = text_new (engine, "", GTK_HTML_FONT_STYLE_DEFAULT,
			     html_colorset_get_color (engine->settings->color_set, HTMLTextColor));
	html_text_set_font_face (HTML_TEXT (new_text), current_font_face (engine));
	html_clue_prepend (last_clueflow, new_text);
}

static void
html_engine_stream_end (GtkHTMLStream *stream,
			GtkHTMLStreamStatus status,
			gpointer data)
{
	HTMLEngine *e;

	e = HTML_ENGINE (data);

	e->writing = FALSE;

	html_tokenizer_end (e->ht);
	while (html_engine_timer_event (e))
		;

	if (e->timerId != 0) {		gtk_idle_remove (e->timerId);
		e->timerId = 0;
	}

	ensure_last_clueflow (e);
	html_engine_class_data_clear (e);
	
	if (e->editable) {
		html_engine_ensure_editable (e);
		html_cursor_home (e->cursor, e);
	}

	gtk_signal_emit (GTK_OBJECT (e), signals[LOAD_DONE]);
}



void
html_engine_draw (HTMLEngine *e,
		  gint x, gint y,
		  gint width, gint height)
{
	gint tx, ty;

	/* This case happens when the widget has not been shown yet.  */
	if (width == 0 || height == 0)
		return;

	tx = -e->x_offset + e->leftBorder;
	ty = -e->y_offset + e->topBorder;

	html_painter_begin (e->painter, x, y, x + width, y + height);

	html_engine_draw_background (e, x, y, width, height);

	if (e->clue)
		html_object_draw (e->clue,
				  e->painter,
				  x + e->x_offset - e->leftBorder,
				  y + e->y_offset - e->topBorder,
				  width,
				  height,
				  tx, ty);

	html_painter_end (e->painter);

	if (e->editable)
		html_engine_draw_cursor_in_area (e, x, y, width, height);
}

static gint
redraw_idle (HTMLEngine *e)
{
	e->redraw_idle_id = 0;
	html_engine_draw (e, 0, 0, e->width, e->height);

	return FALSE;
}

void
html_engine_schedule_redraw (HTMLEngine *e)
{
	if (e->block_redraw)
		e->need_redraw = TRUE;
	else if (e->redraw_idle_id == 0)
		e->redraw_idle_id = gtk_idle_add ((GtkFunction) redraw_idle, e);
}

void
html_engine_block_redraw (HTMLEngine *e)
{
	e->block_redraw ++;
}

void
html_engine_unblock_redraw (HTMLEngine *e)
{
	g_assert (e->block_redraw > 0);

	e->block_redraw --;
	if (!e->block_redraw && e->need_redraw) {
		html_engine_draw (e, 0, 0, e->width, e->height);
		e->need_redraw = FALSE;
	}
}


gint
html_engine_get_doc_width (HTMLEngine *e)
{
	if (e->clue)
		return e->clue->width + e->leftBorder + e->rightBorder;
	else
		return e->leftBorder + e->rightBorder;
}

gint
html_engine_get_doc_height (HTMLEngine *e)
{
	gint height;

	if (e->clue) {
		height = e->clue->ascent;
		height += e->clue->descent;
		height += e->topBorder;
		height += e->bottomBorder;

		return height;
	}
	
	return 0;
}

gint
html_engine_calc_min_width (HTMLEngine *e)
{
	return html_object_calc_min_width (e->clue, e->painter) + e->leftBorder + e->rightBorder;
}

gint
html_engine_get_max_width (HTMLEngine *e)
{
	gint max_width;

	if (e->widget->iframe_parent)
		max_width = e->widget->frame->max_width
			- (e->leftBorder + e->rightBorder) * html_painter_get_pixel_size (e->painter);
	else
		max_width = html_painter_get_page_width (e->painter, e)
			- (e->leftBorder + e->rightBorder) * html_painter_get_pixel_size (e->painter);

	return MAX (0, max_width);
}

void
html_engine_calc_size (HTMLEngine *e)
{
	if (e->clue == 0)
		return;

	html_object_reset (e->clue);

	html_object_set_max_width (e->clue, e->painter,
				   MIN (html_engine_get_max_width (e),
					html_painter_get_pixel_size (e->painter)
					* (MAX_WIDGET_WIDTH - e->leftBorder - e->rightBorder)));
	/* printf ("calc size %d\n", e->clue->max_width); */
	html_object_calc_size (e->clue, e->painter);

	e->clue->x = 0;
	e->clue->y = e->clue->ascent;
}

static void
destroy_form (gpointer data, gpointer user_data)
{
	html_form_destroy (HTML_FORM(data));
}

void
html_engine_parse (HTMLEngine *e)
{
	html_engine_stop_parser (e);

	/* reset search & replace */
	if (e->search_info) {
		html_search_destroy (e->search_info);
		e->search_info = NULL;
	}
	if (e->replace_info) {
		html_replace_destroy (e->replace_info);
		e->replace_info = NULL;
	}

	if (e->clue != NULL)
		html_object_destroy (e->clue);

	g_list_foreach (e->formList, destroy_form, NULL);

	g_list_free (e->formList);

	e->map = NULL;
	e->formList = NULL;
	e->form = NULL;
	e->formSelect = NULL;
	e->formTextArea = NULL;
	e->inOption = FALSE;
	e->inTextArea = FALSE;
	e->formText = g_string_new ("");

	e->flow = NULL;
	e->divAlign = HTML_HALIGN_NONE;
	e->indent_level = 0;

	/* reset to default border size */
	e->leftBorder   = LEFT_BORDER;
	e->rightBorder  = RIGHT_BORDER;
	e->topBorder    = TOP_BORDER;
	e->bottomBorder = BOTTOM_BORDER;

	/* reset settings to default ones */
	html_colorset_set_by (e->settings->color_set, e->defaultSettings->color_set);

	e->clue = html_cluev_new (0, 0, 100);
	HTML_CLUE (e->clue)->valign = HTML_VALIGN_TOP;
	HTML_CLUE (e->clue)->halign = HTML_HALIGN_LEFT;

	e->cursor->object = e->clue;

	/* Free the background pixmap */
	if (e->bgPixmapPtr) {
		html_image_factory_unregister(e->image_factory, e->bgPixmapPtr, NULL);
		e->bgPixmapPtr = NULL;
	}

	e->parsing = TRUE;
	e->avoid_para = TRUE;
	e->pending_para = FALSE;

	e->pending_para_alignment = HTML_HALIGN_LEFT;

	e->timerId = gtk_idle_add ((GtkFunction) html_engine_timer_event, e);
}


HTMLObject *
html_engine_get_object_at (HTMLEngine *e,
			   gint x, gint y,
			   guint *offset_return,
			   gboolean for_cursor)
{
	HTMLObject *clue;
	HTMLObject *obj;

	clue = HTML_OBJECT (e->clue);
	if (clue == NULL)
		return NULL;

	if (for_cursor) {
		gint width, height;

		width = clue->width;
		height = clue->ascent + clue->descent;

		if (width == 0 || height == 0)
			return NULL;

		if (x < e->leftBorder)
			x = e->leftBorder;
		else if (x >= e->leftBorder + width)
			x = e->leftBorder + width - 1;

		if (y < e->topBorder) {
			x = e->leftBorder;
			y = e->topBorder;
		} else if (y >= e->topBorder + height) {
			x = e->leftBorder + width - 1;
			y = e->topBorder + height - 1;
		}
	}

	obj = html_object_check_point (clue,
				       e->painter,
				       x - e->leftBorder, y - e->topBorder,
				       offset_return,
				       for_cursor);

	return obj;
}

HTMLPoint *
html_engine_get_point_at (HTMLEngine *e,
			  gint x, gint y,
			  gboolean for_cursor)
{
	HTMLObject *o;
	guint off;

	o = html_engine_get_object_at (e, x, y, &off, for_cursor);

	return o ? html_point_new (o, off) : NULL;
}

const gchar *
html_engine_get_link_at (HTMLEngine *e, gint x, gint y)
{
	HTMLObject *obj;

	if (e->clue == NULL)
		return NULL;

	obj = html_engine_get_object_at (e, x, y, NULL, FALSE);

	if (obj != NULL)
		return html_object_get_url (obj);

	return NULL;
}


/**
 * html_engine_set_editable:
 * @e: An HTMLEngine object
 * @editable: A flag specifying whether the object must be editable
 * or not
 * 
 * Make @e editable or not, according to the value of @editable.
 **/
void
html_engine_set_editable (HTMLEngine *e,
			  gboolean editable)
{
	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));

	if ((e->editable && editable) || (! e->editable && ! editable))
		return;

	if (editable)
		html_engine_spell_check (e);
	html_engine_disable_selection (e);

	html_engine_draw (e, 0, 0, e->width, e->height);

	e->editable = editable;

	if (editable) {
		html_engine_ensure_editable (e);
		html_cursor_home (e->cursor, e);

		if (e->have_focus)
			html_engine_setup_blinking_cursor (e);
	} else {
		if (e->have_focus)
			html_engine_stop_blinking_cursor (e);
	}
}

gboolean
html_engine_get_editable (HTMLEngine *e)
{
	g_return_val_if_fail (e != NULL, FALSE);
	g_return_val_if_fail (HTML_IS_ENGINE (e), FALSE);

	if (e->editable && ! e->parsing && e->timerId == 0)
		return TRUE;
	else
		return FALSE;
}


void
html_engine_set_focus (HTMLEngine *engine,
		       gboolean have_focus)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	if (engine->editable) {
		if (! engine->have_focus && have_focus)
			html_engine_setup_blinking_cursor (engine);
		else if (engine->have_focus && ! have_focus)
			html_engine_stop_blinking_cursor (engine);
	}

	engine->have_focus = have_focus;
}


/*
  FIXME: It might be nice if we didn't allow the tokenizer to be
  changed once tokenizing has begin.
*/
void
html_engine_set_tokenizer (HTMLEngine *engine,
			   HTMLTokenizer *tok)
{
	g_return_if_fail (engine && HTML_IS_ENGINE (engine));
	g_return_if_fail (tok && HTML_IS_TOKENIZER (tok));

	gtk_object_ref (GTK_OBJECT (tok));
	gtk_object_unref (GTK_OBJECT (engine->ht));
	engine->ht = tok;
}


gboolean
html_engine_make_cursor_visible (HTMLEngine *e)
{
	gint x1, y1, x2, y2, xo, yo;

	g_return_val_if_fail (e != NULL, FALSE);

	if (! e->editable)
		return FALSE;

	if (e->cursor->object == NULL)
		return FALSE;

	html_object_get_cursor (e->cursor->object, e->painter, e->cursor->offset, &x1, &y1, &x2, &y2);

	x1 += e->leftBorder;
	y1 += e->topBorder;
	x2 += e->leftBorder;
	y2 += e->topBorder;

	xo = e->x_offset;
	yo = e->y_offset;

	if (x1 < e->x_offset + e->leftBorder)
		e->x_offset = x1 - e->leftBorder;
	if (x1 + e->leftBorder > e->x_offset + e->width)
		e->x_offset = x1 + e->leftBorder - e->width;

	if (y1 < e->y_offset + e->topBorder)
		e->y_offset = y1 - e->topBorder;
	if (y2 + e->topBorder >= e->y_offset + e->height)
		e->y_offset = y2 + e->topBorder - e->height + 1;

	return xo != e->x_offset || yo != e->y_offset;
}


/* Draw queue handling.  */

void
html_engine_flush_draw_queue (HTMLEngine *e)
{
	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));

	html_draw_queue_flush (e->draw_queue);
}

void
html_engine_queue_draw (HTMLEngine *e, HTMLObject *o)
{
	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));
	g_return_if_fail (o != NULL);

	if (e->freeze_count == 0)
		html_draw_queue_add (e->draw_queue, o);
}

void
html_engine_queue_clear (HTMLEngine *e,
			 gint x, gint y,
			 guint width, guint height)
{
	g_return_if_fail (e != NULL);

	if (e->freeze_count == 0)
		html_draw_queue_add_clear (e->draw_queue, x, y, width, height,
					   &html_colorset_get_color_allocated (e->painter, HTMLBgColor)->color);
}


void
html_engine_form_submitted (HTMLEngine *e,
			    const gchar *method,
			    const gchar *action,
			    const gchar *encoding)
{
	gtk_signal_emit (GTK_OBJECT (e), signals[SUBMIT], method, action, encoding);

}


/* Retrieving the selection as a string.  */

gchar *
html_engine_get_selection_string (HTMLEngine *engine)
{
	GString *buffer;
	gchar *string;

	g_return_val_if_fail (engine != NULL, NULL);
	g_return_val_if_fail (HTML_IS_ENGINE (engine), NULL);

	if (engine->clue == NULL)
		return NULL;

	buffer = g_string_new (NULL);
	html_object_append_selection_string (engine->clue, buffer);

	string = buffer->str;
	g_string_free (buffer, FALSE);

	return string;
}


/* Cursor normalization.  */

void
html_engine_normalize_cursor (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	html_cursor_normalize (engine->cursor);
	html_engine_edit_selection_updater_update_now (engine->selection_updater);
}


/* Freeze/thaw.  */

gboolean
html_engine_frozen (HTMLEngine *engine)
{
	g_return_val_if_fail (engine != NULL, FALSE);
	g_return_val_if_fail (HTML_IS_ENGINE (engine), FALSE);

	return engine->freeze_count > 0;
}

void
html_engine_freeze (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	html_engine_hide_cursor (engine);
	engine->freeze_count++;
}

static gint
thaw_idle (gpointer data)
{
	HTMLEngine *e = HTML_ENGINE (data);

	e->thaw_idle_id = 0;

	html_engine_calc_size (e);

	gtk_html_private_calc_scrollbars (e->widget, NULL, NULL);
	gtk_html_edit_make_cursor_visible (e->widget);
	/* html_engine_make_cursor_visible (e);

	gtk_adjustment_set_value (GTK_LAYOUT (e->widget)->hadjustment, (gfloat) e->x_offset);
	gtk_adjustment_set_value (GTK_LAYOUT (e->widget)->vadjustment, (gfloat) e->y_offset);
	*/
	html_draw_queue_clear (e->draw_queue);
	html_engine_schedule_redraw (e);
	html_engine_show_cursor (e);


	return FALSE;
}

void
html_engine_thaw (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));
	g_return_if_fail (engine->freeze_count > 0);

	engine->freeze_count--;

	if (engine->freeze_count == 0 && engine->thaw_idle_id == 0)
		engine->thaw_idle_id = gtk_idle_add (thaw_idle, engine);
	else
		html_engine_show_cursor (engine);
}

void
html_engine_thaw_idle_reset (HTMLEngine *e)
{
	if (e->thaw_idle_id) {
		gtk_idle_remove (e->thaw_idle_id);
		e->thaw_idle_id = 0;
		html_engine_show_cursor (e);
	}
}


/**
 * html_engine_load_empty:
 * @engine: An HTMLEngine object
 * 
 * Load an empty document into the engine.
 **/
void
html_engine_load_empty (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	/* FIXME: "slightly" hackish.  */
	html_engine_stop_parser (engine);
	html_engine_parse (engine);
	html_engine_stop_parser (engine);

	html_engine_ensure_editable (engine);
}

void
html_engine_replace (HTMLEngine *e, const gchar *text, const gchar *rep_text,
		     gboolean case_sensitive, gboolean forward, gboolean regular,
		     void (*ask)(HTMLEngine *, gpointer), gpointer ask_data)
{
	if (e->replace_info)
		html_replace_destroy (e->replace_info);
	e->replace_info = html_replace_new (rep_text, ask, ask_data);

	if (html_engine_search (e, text, case_sensitive, forward, regular))
		ask (e, ask_data);
}

static void
replace (HTMLEngine *e)
{
	HTMLObject *first = HTML_OBJECT (e->search_info->found->data);
	HTMLObject *new_text;

	html_engine_edit_selection_updater_update_now (e->selection_updater);

	new_text = text_new (e, e->replace_info->text,
			     HTML_TEXT (first)->font_style,
			     HTML_TEXT (first)->color);
	html_text_set_font_face (HTML_TEXT (new_text), HTML_TEXT (first)->face);
	html_engine_paste_object (e, new_text, html_object_get_length (HTML_OBJECT (new_text)));

	/* update search info to point just behind replaced text */
	g_list_free (e->search_info->found);
	e->search_info->found = g_list_append (NULL, e->cursor->object);
	e->search_info->start_pos = e->search_info->stop_pos = e->cursor->offset - 1;
	e->search_info->found_len = 0;
	html_search_pop  (e->search_info);
	html_search_push (e->search_info, e->cursor->object->parent);
}

void
html_engine_replace_do (HTMLEngine *e, HTMLReplaceQueryAnswer answer)
{
	g_assert (e->replace_info);

	switch (answer) {
	case RQA_ReplaceAll:
		html_undo_level_begin (e->undo, "Replace all", "Revert replace all");
		replace (e);
		while (html_engine_search_next (e))
			replace (e);
		html_undo_level_end (e->undo);
	case RQA_Cancel:
		html_replace_destroy (e->replace_info);
		e->replace_info = NULL;
		html_engine_disable_selection (e);
		break;

	case RQA_Replace:
		html_undo_level_begin (e->undo, "Replace", "Revert replace");
		replace (e);
		html_undo_level_end (e->undo);
	case RQA_Next:
		if (html_engine_search_next (e))
			e->replace_info->ask (e, e->replace_info->ask_data);
		else
			html_engine_disable_selection (e);
		break;
	}
}

/* spell checking */

static void
check_paragraph (HTMLObject *o, HTMLEngine *unused, HTMLEngine *e)
{
	if (HTML_OBJECT_TYPE (o) == HTML_TYPE_CLUEFLOW)
		html_clueflow_spell_check (HTML_CLUEFLOW (o), e, NULL);
}

void
html_engine_spell_check (HTMLEngine *e)
{
	g_assert (HTML_IS_ENGINE (e));
	g_assert (e->clue);

	e->need_spell_check = FALSE;

	if (e->widget->editor_api && e->widget->editor_api->check_word)
		html_object_forall (e->clue, NULL, (HTMLObjectForallFunc) check_paragraph, e);
}

gchar *
html_engine_get_word (HTMLEngine *e)
{
	GString *text;
	HTMLCursor *cursor;
	gchar *word;
	gint pos;
	gunichar uc;

	if (!html_is_in_word (html_cursor_get_current_char (e->cursor))
	    && !html_is_in_word (html_cursor_get_prev_char (e->cursor)))
		return NULL;

	cursor = html_cursor_dup (e->cursor);
	pos    = cursor->position;
	text   = g_string_new (NULL);

	/* move to the beginning of word */
	while (html_is_in_word (html_cursor_get_prev_char (cursor)))
		html_cursor_backward (cursor, e);

	/* move to the end of word */
	while (html_is_in_word (uc = html_cursor_get_current_char (cursor))) {
		gchar out [7];
		gint size;

		size = g_unichar_to_utf8 (uc, out);
		g_assert (size < 7);
		out [size] = 0;
		text = g_string_append (text, out);
		html_cursor_forward (cursor, e);
	}

	word = text->str;
	g_string_free (text, FALSE);
	html_cursor_destroy (cursor);

	return word;
}

gboolean
html_engine_word_is_valid (HTMLEngine *e)
{
	HTMLObject *obj;
	HTMLText   *text;
	GList *cur;
	gboolean valid = TRUE;
	gint offset;
	gchar prev, curr;

	prev = html_cursor_get_prev_char (e->cursor);
	curr = html_cursor_get_current_char (e->cursor);

	/* if we are not in word always return TRUE so we care only about invalid words */
	if (!html_is_in_word (prev) && !html_is_in_word (curr))
		return TRUE;

	if (html_is_in_word (curr)) {
		gboolean end;

		end    = (e->cursor->offset == html_object_get_length (e->cursor->object));
		obj    = (end) ? html_object_next_not_slave (e->cursor->object) : e->cursor->object;
		offset = (end) ? 0 : e->cursor->offset;
	} else {
		obj    = (e->cursor->offset) ? e->cursor->object : html_object_prev_not_slave (e->cursor->object);
		offset = (e->cursor->offset) ? e->cursor->offset - 1 : html_object_get_length (obj) - 1;
	}

	g_assert (html_object_is_text (obj));
	text = HTML_TEXT (obj);

	/* now we have text, so let search for spell_error area in it */
	cur = text->spell_errors;
	while (cur) {
		SpellError *se = (SpellError *) cur->data;
		if (se->off <= offset && offset <= se->off + se->len) {
			valid = FALSE;
			break;
		}
		if (offset < se->off)
			break;
		cur = cur->next;
	}

	/* printf ("is_valid: %d\n", valid); */

	return valid;
}

void
html_engine_replace_word_with (HTMLEngine *e, const gchar *word)
{
	HTMLObject *replace = NULL;
	HTMLText   *orig;
	gint pos;

	if (!html_is_in_word (html_cursor_get_current_char (e->cursor))
	    && !html_is_in_word (html_cursor_get_prev_char (e->cursor)))
		return;

	pos = e->cursor->position;
	html_engine_selection_push (e);

	/* move to the beginning of word */
	while (html_is_in_word (html_cursor_get_prev_char (e->cursor)))
		html_cursor_backward (e->cursor, e);
	html_engine_set_mark (e);

	/* move to the end of word */
	while (html_is_in_word (html_cursor_get_current_char (e->cursor)))
		html_cursor_forward (e->cursor, e);

	orig = HTML_TEXT (e->mark->object);
	switch (HTML_OBJECT_TYPE (e->mark->object)) {
	case HTML_TYPE_TEXT:
		replace = text_new (e, word, orig->font_style, orig->color);
		break;
	case HTML_TYPE_LINKTEXT:
		replace = html_link_text_new (word, orig->font_style, orig->color,
					      HTML_LINK_TEXT (orig)->url,
					      HTML_LINK_TEXT (orig)->target);
		break;
	default:
		g_assert_not_reached ();
	}
	html_text_set_font_face (HTML_TEXT (replace), HTML_TEXT (orig)->face);
	html_engine_edit_selection_updater_update_now (e->selection_updater);
	html_engine_paste_object (e, replace, html_object_get_length (replace));
	html_engine_selection_pop (e);
}

HTMLCursor *
html_engine_get_cursor (HTMLEngine *e)
{
	HTMLCursor *cursor;

	cursor = html_cursor_new ();
	cursor->object = html_engine_get_object_at (e, e->widget->selection_x1, e->widget->selection_y1,
						    &cursor->offset, FALSE);
	return cursor;
}

void
html_engine_set_painter (HTMLEngine *e, HTMLPainter *painter)
{
	g_return_if_fail (painter != NULL);
	g_return_if_fail (e != NULL);

	gtk_object_ref (GTK_OBJECT (painter));
	gtk_object_unref (GTK_OBJECT (e->painter));
	e->painter = painter;
	
	html_object_set_painter (e->clue, painter);
	html_object_change_set_down (e->clue, HTML_CHANGE_ALL);
	html_object_reset (e->clue);
	html_engine_calc_size (e);
}

gint
html_engine_get_view_width (HTMLEngine *e)
{
	return MAX (0, (e->widget->iframe_parent
		? html_engine_get_view_width (GTK_HTML (e->widget->iframe_parent)->engine)
		: GTK_WIDGET (e->widget)->allocation.width) - e->leftBorder - e->rightBorder);
}

gint
html_engine_get_view_height (HTMLEngine *e)
{
	return MAX (0, (e->widget->iframe_parent
		? html_engine_get_view_height (GTK_HTML (e->widget->iframe_parent)->engine)
		: GTK_WIDGET (e->widget)->allocation.height) - e->topBorder - e->bottomBorder);
}

/* beginnings of ID support */
void
html_engine_add_object_with_id (HTMLEngine *e, const gchar *id, HTMLObject *obj)
{
	gpointer old_key;
	gpointer old_val;

	if (e->id_table == NULL)
		e->id_table = g_hash_table_new (g_str_hash, g_str_equal);

	if (!g_hash_table_lookup_extended (e->id_table, id, &old_key, &old_val))
		old_key = NULL;

	g_hash_table_insert (e->id_table, old_key ? old_key : g_strdup (id), obj);
}

HTMLObject *
html_engine_get_object_by_id (HTMLEngine *e, const gchar *id)
{
	if (e->id_table == NULL)
		return NULL;

	return (HTMLObject *) g_hash_table_lookup (e->id_table, id);
}

static inline GHashTable *
get_class_table (HTMLEngine *e, const gchar *class_name)
{
	return e->class_data ? g_hash_table_lookup (e->class_data, class_name) : NULL;
}

static inline GHashTable *
get_class_table_sure (HTMLEngine *e, const gchar *class_name)
{
	GHashTable *t;

	if (e->class_data == NULL)
		e->class_data = g_hash_table_new (g_str_hash, g_str_equal);

	t = get_class_table (e, class_name);
	if (!t) {
		t = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (e->class_data, g_strdup (class_name), t);
	}

	return t;
}

static void
html_engine_set_class_data (HTMLEngine *e, const gchar *class_name, const gchar *key, const gchar *value)
{
	GHashTable *t;
	gpointer old_key;
	gpointer old_val;

	printf ("set (%s) %s to %s (%p)\n", class_name, key, value, e->class_data);

	t = get_class_table_sure (e, class_name);

	if (!g_hash_table_lookup_extended (t, key, &old_key, &old_val))
		old_key = NULL;
	else {
		if (strcmp (old_val, value))
			g_free (old_val);
		else
			return;
	}
	g_hash_table_insert (t, old_key ? old_key : g_strdup (key), g_strdup (value));
}

static void
html_engine_clear_class_data (HTMLEngine *e, const gchar *class_name, const gchar *key)
{
	GHashTable *t;
	gpointer old_key;
	gpointer old_val;

	t = get_class_table (e, class_name);

	printf ("clear (%s) %s\n", class_name, key);
	if (t && g_hash_table_lookup_extended (t, key, &old_key, &old_val)) {
		g_hash_table_remove (t, key);
		g_free (old_key);
		g_free (old_val);
	}
}

const gchar *
html_engine_get_class_data (HTMLEngine *e, const gchar *class_name, const gchar *key)
{
	return e->class_data ? g_hash_table_lookup (e->class_data, key) : NULL;
}

static void
set_object_data (gpointer key, gpointer value, gpointer data)
{
	printf ("set %s\n", (const gchar *) key);
	html_object_set_data (HTML_OBJECT (data), g_strdup ((const gchar *) key), g_strdup ((const gchar *) value));
}

static void
html_engine_set_object_data (HTMLEngine *e, HTMLObject *o)
{
	GHashTable *t;

	t = get_class_table (e, html_type_name (HTML_OBJECT_TYPE (o)));
	if (t)
		g_hash_table_foreach (t, set_object_data, o);
}


HTMLEngine *
html_engine_get_top_html_engine (HTMLEngine *e)
{
	while (e->widget->iframe_parent)
		e = GTK_HTML (e->widget->iframe_parent)->engine;

	return e;
}
