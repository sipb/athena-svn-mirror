/* 
* gok-spy.h 
* 
* utility for getting accessible application UI info 
* 
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto 
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
 

#ifndef __GOKSPY_H__ 
#define __GOKSPY_H__ 
 
#include "gok-spy-priv.h" 
 
#ifdef __cplusplus 
extern "C" { 
#endif /* __cplusplus */ 

/*G_LOCK_DEFINE_STATIC(gok_spy_mutex);*/
#define GOKSPY_ROLE_APPLICATION -1
#define GOKSPY_ROLE_EDITABLETEXT -2

typedef enum {
	GOK_SPY_SEARCH_ROLE,
	GOK_SPY_SEARCH_TOOLBARS,
	GOK_SPY_SEARCH_MENU,
	GOK_SPY_SEARCH_UI,
	GOK_SPY_SEARCH_EDITABLE_TEXT,
	GOK_SPY_SEARCH_APPLICATIONS,
	GOK_SPY_SEARCH_CHILDREN,
	GOK_SPY_SEARCH_LISTITEMS,
	GOK_SPY_SEARCH_TABLE_ROWS,
	GOK_SPY_SEARCH_COMBO,
	GOK_SPY_SEARCH_ACTIONABLE,
	GOK_SPY_SEARCH_ALL
} GokSpySearchType;

typedef struct {
	AccessibleNodeFlags match_flags;
} GokSpyMatchCondition;

typedef union 
{
	guint value;
	struct {
		guint menus:1;
		guint toolbars:1;
		guint gui:1;
		guint editable_text:1;
		guint context_menu:1;
	} data;
} GokSpyUIFlags;

void gok_spy_open(void);        /* call this first (user must call SPI_init first) */
void gok_spy_close(void);       /* call this when shutting down */ 
void gok_spy_stop (void);       /* call this first, as soon as you start shutting down */
 
/* listener support */
void gok_spy_register_appchangelistener(AccessibleChangeListener* callback); 
void gok_spy_deregister_appchangelistener(AccessibleChangeListener* callback); 
 
void gok_spy_register_windowchangelistener(AccessibleChangeListener* callback); 
void gok_spy_deregister_windowchangelistener(AccessibleChangeListener* callback); 

void gok_spy_register_objectstatelistener(AccessibleChangeListener* callback); 
void gok_spy_deregister_objectstatelistener(AccessibleChangeListener* callback); 

void gok_spy_register_mousebuttonlistener(MouseButtonListener* callback); 
void gok_spy_deregister_mousebuttonlistener(MouseButtonListener* callback); 

gulong gok_spy_get_modmask (void);

gboolean gok_spy_is_menu_role (AccessibleRole role);
/* gboolean gok_spy_is_desktop(Accessible* pAccessible); */
/* gboolean gok_spy_accessible_is_desktopChild(Accessible* accessible); */
Accessible*     gok_spy_get_active_frame (void );
Accessible*     gok_spy_get_focussed_object (void );
GSList*         gok_spy_get_list (Accessible* paccessible); 
Accessible*     gok_spy_get_list_parent (Accessible* paccessible);
Accessible*     gok_spy_get_editable (Accessible* paccessible);
GSList*         gok_spy_get_children (Accessible* paccessible); 
GSList*         gok_spy_get_actionable_descendants (Accessible* paccessible, GSList *nodes); 
GSList*         gok_spy_get_table_row_nodes (Accessible* paccessible); 
GokSpyUIFlags   gok_spy_update_component_list (Accessible* accessible, GokSpyUIFlags keyboard_ui_flags);
AccessibleNode* gok_spy_get_ui_list (Accessible* paccessible); 
AccessibleNode* gok_spy_refresh (AccessibleNode* plist); 
GSList*         gok_spy_append_node (GSList *nodes, Accessible* pAccessible, AccessibleNodeFlags flags);

void            gok_spy_free (GSList *nodes); /* call this to free up an unneeded list's memory */ 

Accessible*     gok_spy_get_accessibleWithText(void);

void     gok_spy_set_context_menu_accessible (Accessible* accessible, int action_index);
gboolean gok_spy_has_child (Accessible* accessible, GokSpySearchType type, AccessibleRole role); 
gboolean gok_spy_node_match (AccessibleNode *node, GokSpySearchType type);
gboolean gok_spy_check_queues(void);
gboolean gok_spy_check_window(AccessibleRole role);
void gok_spy_add_idle_handler (void);
void gok_spy_keymap_listener (void);

/* for debugging: */
void gok_spy_accessible_ref(Accessible* accessible);
void gok_spy_accessible_implicit_ref(Accessible* accessible);
void gok_spy_accessible_unref(Accessible* accessible);
 
 
#ifdef __cplusplus 
} 
#endif /* __cplusplus */ 
 
#endif /* #ifndef __GOKSPY_H__ */
