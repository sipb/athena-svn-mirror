/* SRObject.h
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

#ifndef _SROBJECT_H
#define _SROBJECT_H

#include <glib.h>
#include <glib-object.h>
#include "SRMessages.h"

/*
#define SRL_DEBUG
*/

#define SR_INDEX_CONTAINER -1

/* an universal type of integer type to comunicate with a #SRObject */
typedef long int SRLong;

typedef enum _SRObjectType
{
    SR_OBJ_TYPE_VISUAL,
    SR_OBJ_TYPE_PROCESSED
}SRObjectType;

typedef enum _SRNavigationDir
{
    SR_NAV_NEXT,
    SR_NAV_PREV,
    SR_NAV_PARENT,
    SR_NAV_CHILD,
    SR_NAV_TITLE,
    SR_NAV_MENU,    
    SR_NAV_GROUP,
    SR_NAV_TOOLBAR,
    SR_NAV_STATUSBAR,
    SR_NAV_CARET,
    SR_NAV_FIRST,
    SR_NAV_LAST,
    SR_NAV_WINDOW
}SRNavigationDir;


typedef enum _SRNavigationMode
{
    SR_NAV_MODE_WINDOW,
    SR_NAV_MODE_APPLICATION,
    SR_NAV_MODE_DESKTOP
}SRNavigationMode;


/* Screen reader point type */
typedef struct _SRPoint SRPoint;

struct _SRPoint
{
    gint32	x;
    gint32	y;
};

/* Screen reader point's methodt's */
gboolean sr_point_get_x (const SRPoint *point, gint32 *x);
gboolean sr_point_get_y (const SRPoint *point, gint32 *y);
gboolean sr_point_set_x (SRPoint *point, gint32 x);
gboolean sr_point_set_y (SRPoint *point, gint32 y);


/* Screen reader rectangle type */
typedef struct _SRRectangle SRRectangle;
struct _SRRectangle
{
    gint32	x;
    gint32	y;
    gint32	width;
    gint32	height;
};

/* Screen reader rectangle's methods */
gboolean sr_rectangle_get_x 	(const SRRectangle *rect, gint32 *x);
gboolean sr_rectangle_get_y 	(const SRRectangle *rect, gint32 *y);
gboolean sr_rectangle_get_width (const SRRectangle *rect, gint32 *width);
gboolean sr_rectangle_get_height(const SRRectangle *rect, gint32 *height);


/* Screen reader coordinate type enumeration */
typedef enum _SRCoordinateType
{
	SR_COORD_TYPE_WINDOW,
	SR_COORD_TYPE_SCREEN
}SRCoordinateType;

#define SR_RELATION_NONE 		(0)
#define	SR_RELATION_CONTROLLED_BY 	(1)
#define	SR_RELATION_CONTROLLER_FOR	(1<<1)
#define	SR_RELATION_MEMBER_OF		(1<<2)
#define	SR_RELATION_EXTENDED		(1<<3)

typedef long SRRelation;

/* Screen reader text atributes type */
typedef gchar * SRTextAttribute;

/* Screen reader text boundary type enumeration */
typedef enum _SRTextBoundaryType
{
    SR_TEXT_BOUNDARY_CHAR,
    SR_TEXT_BOUNDARY_WORD,
    SR_TEXT_BOUNDARY_SENTENCE,
    SR_TEXT_BOUNDARY_LINE
}SRTextBoundaryType;

#define	SR_STATE_INVALID    (0)
#define	SR_STATE_ACTIVE     (1<<0)
#define	SR_STATE_CHECKED    (1<<1)
#define	SR_STATE_COLLAPSED  (1<<2)
#define	SR_STATE_EDITABLE   (1<<3)
#define	SR_STATE_EXPANDED   (1<<4)
#define	SR_STATE_EXPANDABLE (1<<5)
#define	SR_STATE_FOCUSED    (1<<6)
#define	SR_STATE_FOCUSABLE  (1<<7)
#define	SR_STATE_MODAL      (1<<8)
#define	SR_STATE_PRESSED    (1<<9)
#define	SR_STATE_SELECTED   (1<<10)
#define	SR_STATE_VISIBLE    (1<<11)
#define	SR_STATE_SHOWING    (1<<11)
#define	SR_STATE_CHECKABLE  (1<<12)
#define	SR_STATE_MINIMIZED  (1<<13)
#define	SR_STATE_ENABLED    (1<<14)

typedef long SRState;




/* Screen reader object role type enumeration */
typedef enum _SRObjectRole
{
    SR_ROLE_INVALID = 0,
    /* Object is used to alert the user about something */
    SR_ROLE_ALERT,
    /* Object that can be drawn into and is used to trap events */
    SR_ROLE_CANVAS,
    /*
     * A choice that can be checked or unchecked and provides a separate
     * indicator for the current state.
     */
    SR_ROLE_CHECK_BOX,
    /* A specialized dialog that lets the user choose a color. */
    SR_ROLE_COLOR_CHOOSER,
    /* The header for a column of data */
    SR_ROLE_COLUMN_HEADER,
    /* A list of choices the user can select from */
    SR_ROLE_COMBO_BOX,
    /* An inconifed internal frame within a DESKTOP_PANE */
    SR_ROLE_DESKTOP_ICON,
    /*
     * A pane that supports internal frames and iconified versions of those
     * internal frames.
     */
    SR_ROLE_DESKTOP_FRAME,
    /* A top level window with title bar and a border */
    SR_ROLE_DIALOG,
    /*
     * A pane that allows the user to navigate through and select the contents
     * of a directory
     */
    SR_ROLE_DIRECTORY_PANE,
    /*
     * A specialized dialog that displays the files in the directory and lets
     * the user select a file, browse a different directory, or specify a
     * filename.
     */
    SR_ROLE_FILE_CHOOSER,
    /*
     * A object that fills up space in a user interface
     */
    SR_ROLE_FILLER,
    /* A top level window with a title bar, border, menubar, etc. */
    SR_ROLE_FRAME,
    /* A pane that is guaranteed to be painted on top of all panes beneath it */
    SR_ROLE_GLASS_PANE,
    /*
     * A document container for HTML, whose children
     * represent the document content.
     */
    SR_ROLE_HTML_CONTAINER,
    /* A small fixed size picture, typically used to decorate components */
    SR_ROLE_ICON,
    /* A frame-like object that is clipped by a desktop pane. */
    SR_ROLE_INTERNAL_FRAME,
    /* An object used to present an icon or short string in an interface */
    SR_ROLE_LABEL,
    /*
     * A specialized pane that allows its children to be drawn in layers,
     * providing a form of stacking order.
     */
    SR_ROLE_LAYERED_PANE,
    SR_ROLE_LINK,
    /*
     * An object that presents a list of objects to the user and allows the
     * user to select one or more of them.
     */
    SR_ROLE_LIST,
    /* An object that represents an element of a list. */
    SR_ROLE_LIST_ITEM,
    /*
     * An object usually found inside a menu bar that contains a list of
     * actions the user can choose from.
     */
    SR_ROLE_MENU,
    /*
     * An object usually drawn at the top of the primary dialog box of an
     * application that contains a list of menus the user can choose from.
     */
    SR_ROLE_MENU_BAR,
    /*
     * An object usually contained in a menu that presents an action the
     * user can choose.
     */
    SR_ROLE_MENU_ITEM,
    SR_ROLE_CHECK_MENU_ITEM,
    SR_ROLE_RADIO_MENU_ITEM,
    /* A specialized pane whose primary use is inside a DIALOG */
    SR_ROLE_OPTION_PANE,
    /* An object that is a child of a page tab list */
    SR_ROLE_PAGE_TAB,
    /*
     * An object that presents a series of panels (or page tabs), one at a time,
     * through some mechanism provided by the object.
     */
    SR_ROLE_PAGE_TAB_LIST,
    /* A generic container that is often used to group objects. */
    SR_ROLE_PANEL,
    /*
     * A text object uses for passwords, or other places where the text
     * content is not shown visibly to the user.
     */
    SR_ROLE_PASSWORD_TEXT,
    /*
     * A temporary window that is usually used to offer the user a list of
     * choices, and then hides when the user selects one of those choices.
     */
    SR_ROLE_POPUP_MENU,
    /* An object used to indicate how much of a task has been completed. */
    SR_ROLE_PROGRESS_BAR,
    /*
     * An object the user can manipulate to tell the application to do
     * something.
     */
    SR_ROLE_PUSH_BUTTON,
    /*
     * A specialized check box that will cause other radio buttons in the
     * same group to become uncghecked when this one is checked.
     */
    SR_ROLE_RADIO_BUTTON,
    /*
     * A specialized pane that has a glass pane and a layered pane as its
     * children.
     */
    SR_ROLE_ROOT_PANE,
    /* The header for a row of data */
    SR_ROLE_ROW_HEADER,
    /*
     * An object usually used to allow a user to incrementally view a large
     * amount of data.
     */
    SR_ROLE_SCROLL_BAR,
    /*
     * An object that allows a user to incrementally view a large amount
     * of information.
     */
    SR_ROLE_SCROLL_PANE,
    /*
     * An object usually contained in a menu to provide a visible and
     * logical separation of the contents in a menu.
     */
    SR_ROLE_SEPARATOR,
    /* An object that allows the user to select from a bounded range */
    SR_ROLE_SLIDER,
    /* A specialized panel that presents two other panels at the same time. */
    SR_ROLE_SPLIT_PANE,
    /* An object used to rpesent information in terms of rows and columns. */
    SR_ROLE_STATUS_BAR ,
    SR_ROLE_TABLE,
    SR_ROLE_TABLE_CELL,
    SR_ROLE_TABLE_COLUMN_HEADER,
    SR_ROLE_TABLE_ROW_HEADER ,
/*    SR_ROLE_TEAROFF_MENU_ITEM ,*/
    /* An object that presents text to the user */
    SR_ROLE_TEXT_ML,
    SR_ROLE_TEXT_SL,
    /*
     * A specialized push button that can be checked or unchecked, but does
     * not procide a separate indicator for the current state.
     */
    SR_ROLE_TOGGLE_BUTTON,
    /* A bar or palette usually composed of push buttons or toggle buttons */
    SR_ROLE_TOOL_BAR,
    /* An object that provides information about another object */
    SR_ROLE_TOOL_TIP,
    /* An object used to repsent hierarchical information to the user. */
    SR_ROLE_TREE,
    SR_ROLE_TREE_ITEM,
    SR_ROLE_TREE_TABLE ,
    /*
     * The object contains some Accessible information, but its role is
     * not known.
     */
    SR_ROLE_UNKNOWN,
    /* An object usually used in a scroll pane. */
    SR_ROLE_VIEWPORT,
    /* A top level window with no title or border */
    SR_ROLE_WINDOW,
    
	SR_ROLE_ACCELERATOR_LABEL,
	SR_ROLE_ANIMATION ,
	SR_ROLE_ARROW ,
	SR_ROLE_CALENDAR ,
	SR_ROLE_DATE_EDITOR ,
	SR_ROLE_DIAL ,
	SR_ROLE_DRAWING_AREA ,
	SR_ROLE_FONT_CHOOSER ,
	SR_ROLE_IMAGE ,
/*	SR_ROLE_RADIO_MENU_ITEM ,*/
	SR_ROLE_SPIN_BUTTON ,
	SR_ROLE_TERMINAL,
	SR_ROLE_EXTENDED ,
	SR_ROLE_TABLE_LINE,
	SR_ROLE_TABLE_COLUMNS_HEADER,
	SR_ROLE_TITLE_BAR,
	SR_ROLE_EDITBAR,
    
    /* not a valid role, used for finding end of enumeration. */
    SR_ROLE_LAST_DEFINED
}SRObjectRoles;


typedef enum _SRObjectLayer
{
	SR_LAYER_INVALID,
	SR_LAYER_BACKGROUND,
	SR_LAYER_CANVAS,
	SR_LAYER_WIDGET,
	SR_LAYER_MDI,
	SR_LAYER_POPUP,
	SR_LAYER_OVERLAY,
	SR_LAYER_WINDOW,
	/* not a valid layer, used for finding end of enumeration. */
	SR_LAYER_LAST_DEFINED
}SRObjectLayer;


typedef struct _SRRoleCnt
{
    gchar *role;
    gint cnt;
}SRRoleCnt;

#define SR_TYPE_OBJECT                           (sro_get_type ())
#define SR_OBJECT(obj)                           (G_TYPE_CHECK_INSTANCE_CAST ((obj), SR_TYPE_OBJECT, SRObject))
#define SR_OBJECT_CLASS(klass)                   (G_TYPE_CHECK_CLASS_CAST ((klass), SR_TYPE_OBJECT, SrObjectClass))
#define SR_IS_OBJECT(obj)                        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SR_TYPE_OBJECT))
#define SR_IS_OBJECT_CLASS(klass)                (G_TYPE_CHECK_CLASS_TYPE ((klass), SR_TYPE_OBJECT))
#define SR_OBJECT_GET_CLASS(obj)                 (G_TYPE_INSTANCE_GET_CLASS ((obj), SR_TYPE_OBJECT, SRObjectClass))

/* Screen reader object type */
typedef struct _SRObject SRObject ;
typedef struct _SRObjectClass SRObjectClass ;

GType            sro_get_type   (void);


/* Screen reader object's methods */
void 		sro_add_reference (SRObject *obj);
void 		sro_release_reference (SRObject *obj);

gboolean sro_is_action		(const SRObject *obj, SRLong index);
gboolean sro_is_component	(const SRObject *obj, SRLong index);
gboolean sro_is_editable_text	(const SRObject *obj, SRLong index);
gboolean sro_is_hypertext	(const SRObject *obj, SRLong index);
gboolean sro_is_image		(const SRObject *obj, SRLong index);
gboolean sro_is_selection	(const SRObject *obj, SRLong index);
gboolean sro_is_table		(const SRObject *obj, SRLong index);
gboolean sro_is_text		(const SRObject *obj, SRLong index);
gboolean sro_is_value		(const SRObject *obj, SRLong index);

gboolean sro_get_reason 	(const SRObject *obj, gchar **reason);
gboolean sro_get_role 		(const SRObject *obj, SRObjectRoles *role, SRLong index);
gboolean sro_get_state 		(const SRObject *obj, SRState *state, SRLong index);
gboolean sro_get_relation	(const SRObject *obj, SRRelation *relation, SRLong index);
gboolean sro_get_role_name	(const SRObject *obj, gchar **role_name,SRLong index);
gboolean sro_get_name 		(const SRObject *obj, gchar **name, SRLong index);
gboolean sro_get_column_header 	(const SRObject *obj, gchar **header_name, SRLong index);
gboolean sro_get_row_header     (const SRObject *obj, gchar **header_name, SRLong index);
gboolean sro_get_cell           (const SRObject *obj, gchar **cell, SRLong index);
gboolean sro_get_description 	(const SRObject *obj, gchar **description, SRLong index);
gboolean sro_get_parent 	(const SRObject *obj, SRObject **parent);
gboolean sro_get_index_in_parent(const SRObject *obj, guint32 *index);
gboolean sro_get_children_count	(const SRObject *obj, guint32 *children_count);
gboolean sro_get_i_child	(const SRObject *obj, SRLong index, 
					    SRObject **child);
					    
gboolean sro_get_app_name       (SRObject *obj, gchar **name, SRLong index); 
gboolean sro_get_window_name    (SRObject *obj, gchar **role, gchar **name, SRLong index);
gboolean sro_get_location	(SRObject *obj, SRCoordinateType type, 
					    SRRectangle *location, SRLong index);
gboolean sro_get_layer 		(SRObject *obj, SRObjectLayer *layer, SRLong index);
gboolean sro_get_MDIZOrder 	(SRObject *obj, short *MDIZOrder, SRLong index);
gboolean sro_get_shortcut	(SRObject *obj, gchar **shortcut, SRLong index);
gboolean sro_get_accelerator	(SRObject *obj, gchar **accelerator, SRLong index);
gboolean sro_get_objs_for_relation(SRObject *obj, SRRelation type,
					SRObject ***targets, SRLong index);
gboolean sro_manages_descendants(SRObject *obj);
gboolean sro_get_index_in_group (SRObject *obj, SRLong *index, SRLong index_obj);

gboolean sro_action_get_count 	 	(SRObject *obj, SRLong *count, SRLong index);
gboolean sro_action_get_name 	 	(SRObject *obj, SRLong index,
						gchar **name, SRLong index_obj);
gboolean sro_action_get_description 	(SRObject *obj, SRLong index,
						gchar **description, SRLong index_obj);
gboolean sro_action_get_key 	 	(SRObject *obj, SRLong index, 
						gchar **key, SRLong index_obj);
gboolean sro_action_do_action 	 	(SRObject *obj, gchar *action, SRLong index_obj);


gboolean sro_value_get_min_val	(SRObject *obj, gdouble *min, SRLong index);
gboolean sro_value_get_max_val	(SRObject *obj, gdouble *max, SRLong index);
gboolean sro_value_get_crt_val	(SRObject *obj, gdouble *crt, SRLong index);

gboolean sro_image_get_description	(SRObject *obj, 
						gchar **description, SRLong index);
gboolean sro_image_get_location		(SRObject *obj, 
						SRCoordinateType type ,
						SRRectangle *location, SRLong index);


gboolean sro_text_get_caret_offset		(SRObject *obj,
						    SRLong *line_offset, SRLong index);
gboolean sro_text_get_caret_location		(SRObject *obj,
						    SRCoordinateType type ,
						    SRRectangle *location, 
						    SRLong index);
gboolean sro_text_set_caret_offset		(SRObject *obj,
						    SRLong line_offset, SRLong index);
gboolean sro_text_get_text_from_caret	   	(SRObject *obj, 
						    SRTextBoundaryType type,
						    gchar **text, SRLong index); 
gboolean sro_text_get_text_attr_from_caret	(SRObject *obj, 
						    SRTextBoundaryType type,
						    SRTextAttribute **attr, SRLong index); 
gboolean sro_text_get_text_location_from_caret	(SRObject *obj, 
						    SRTextBoundaryType type_text,
						    SRCoordinateType type_coord,
						    SRRectangle *location, SRLong index); 

gboolean sro_text_get_line_offset_from_point	(SRObject *obj,
						    const SRPoint *point,
						    SRCoordinateType type,
						    SRLong *line_offset, SRLong index);
gboolean sro_text_get_text_from_point	   	(SRObject *obj, 
						    const SRPoint *point,
						    SRCoordinateType type_coord,
						    SRTextBoundaryType type_text,
						    gchar **text, SRLong index); 
gboolean sro_text_get_text_attr_from_point	(SRObject *obj,
						    const SRPoint *point, 
						    SRCoordinateType type_coord,
						    SRTextBoundaryType type_text,
						    SRTextAttribute **attr,
						    SRLong index); 
gboolean sro_text_get_text_location_from_point	(SRObject *obj, 
						    const SRPoint *point,
						    SRCoordinateType type_coord,
						    SRTextBoundaryType type_text,
						    SRRectangle *location, SRLong index); 
gboolean sro_text_get_selections	(SRObject *obj,
					    gchar ***selections, SRLong index);

gboolean sro_tree_item_get_level	(SRObject *obj,
					    SRLong *level, SRLong index);
					    
gboolean sra_get_attribute_value	(SRTextAttribute text_attr,
					    gchar *attr,
					    gchar **val);
gboolean sra_get_attribute_values_string   (SRTextAttribute text_attr,
					        gchar *attr,
						gchar **val);


gboolean sro_text_get_attributes_at_index (SRObject *obj, SRLong index,
					    SRTextAttribute **index_attr, SRLong index_obj);

gboolean sro_text_get_location_at_index (SRObject *obj, SRLong index,
					    SRRectangle *location, SRLong index_obj);
gboolean sro_text_get_char_at_index (SRObject *obj, SRLong index,
					    gchar *chr, SRLong index_obj);

gboolean sro_text_get_difference (SRObject *obj, gchar **difference,
					    SRLong index);

gboolean sro_text_get_abs_offset (SRObject *obj, SRLong *offset,
					    SRLong index);
gboolean sro_is_word_navigation (SRObject *obj, SRLong crt_offset, 
                                           SRLong last_offset, SRLong index);					    
gboolean sro_text_is_same_line (SRObject *obj, SRLong offset,
					    SRLong index);

gboolean sro_get_sro (SRObject *obj1, SRNavigationDir dir, SRObject **obj2, SRNavigationMode mode);

gboolean sro_get_surroundings (SRObject *obj, GArray **surroundings);
/*gboolean sro_get_window_hierarchy (SRObject *obj, GNode **hierarchy);*/
gboolean sro_get_next_text (SRObject *obj, gchar *text, SRObject **next, SRNavigationMode mode);
gboolean sro_get_next_image (SRObject *obj, SRObject **next, SRNavigationMode mode);
gboolean sro_get_next_attributes (SRObject *obj, gchar *attr, SRObject **next, SRNavigationMode mode);

gboolean sro_get_sros_from_rectangle (SRRectangle *area, GArray **array, gboolean intersect);
gboolean sro_alert_get_info (SRObject *obj, gchar **title, gchar **text, gchar **button);

#ifdef SRL_DEBUG
void SR_freeString 	(gchar *str);
void SR_strfreev 	(gchar **str);
#define srl_debug	srl_assert
#else
#define SR_freeString 	g_free
#define SR_strfreev 	g_strfreev
#define srl_debug(X)	
#endif
#define sra_free(X)	if (X) SR_strfreev (X)

#define srl_assert 			sru_assert
#define srl_assert_not_reached 		sru_assert_not_reached
#define srl_return_if_fail		sru_return_if_fail
#define srl_return_val_if_fail		sru_return_val_if_fail
#define srl_message			sru_message
#define srl_warning			sru_warning
#define srl_error			sru_error

#endif
