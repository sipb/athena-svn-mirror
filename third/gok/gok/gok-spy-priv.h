/* gok-spy-priv.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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

#ifndef __GOK_SPY_PRIV_H__
#define __GOK_SPY_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cspi/spi.h>
#include <libspi/Accessibility.h>
#include <libspi/accessible.h>
#include <libspi/application.h>

enum gokevents {
GOKSPY_FOCUS_EVENT,
GOKSPY_DEFUNCT_EVENT,
GOKSPY_WINDOW_ACTIVATE_EVENT,
GOKSPY_WINDOW_DEACTIVATE_EVENT,
GOKSPY_STATE_EVENT,
GOKSPY_CONTAINER_EVENT,
GOKSPY_KEYMAP_EVENT
};

typedef union
{
	guint value;
	struct {
		guint is_link:1;
		guint is_ui:1;
		guint is_menu:1;
		guint has_context_menu:1;
		guint is_toolbar_item:1;
	} data;
} AccessibleNodeFlags;

/* this structure used in the creation of a linked list of Accessible*, char* (name) pairs */
typedef struct AccessibleNode
{
    Accessible* paccessible;
    AccessibleNodeFlags flags;
    guint link;
    char* pname;
} AccessibleNode;

/* this structure intended for use with a GQueue */
typedef struct EventNode
{
	const AccessibleEvent *event;
	gint type; /* easier than dealing with string data */
} EventNode;

/* callback types */
typedef void (*ListChangeListenerCB)     (AccessibleNode* plist); 
typedef void (*AccessibleChangeListenerCB) 	(Accessible* paccessible);
typedef void (*MouseButtonListenerCB)	(gint mouseButton, gint state, long x, long y);
typedef void ListChangeListener;
typedef void AccessibleChangeListener;
typedef void MouseButtonListener;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef__GOK_SPY_PRIV_H__ */
