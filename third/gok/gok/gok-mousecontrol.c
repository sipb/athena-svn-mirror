/* gok-mousecontrol.c
*
* Copyright 2003 Sun Microsystems, Inc.,
* Copyright 2003 University Of Toronto
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gok-mousecontrol.h"
#include "gok-log.h"

typedef enum {
	MOUSE_COMMAND_MOVE,
	MOUSE_COMMAND_BUTTON,
	MOUSE_COMMAND_TOGGLE_LATCH,
	MOUSE_COMMAND_DOUBLECLICK
} GokMouseCommandType;

typedef enum {
	MOUSE_BUTTON_1,
	MOUSE_BUTTON_2,
	MOUSE_BUTTON_3
} GokMouseButton;

typedef struct {
	GokMouseCommandType type;
	union {
		GokKeyboardDirection dir;
		GokMouseButton button;
		gboolean latch_state;
	} detail;	
} GokMouseCommand;

void
gok_mouse_control_init (GokKey *key, xmlNode *node)
{
	GokMouseCommand *command = g_new0 (GokMouseCommand, 1);
	xmlChar *attribute;
	key->pGeneral = command;
	attribute = xmlGetProp (node, (const xmlChar *) "dir");
	if (attribute != NULL)
	{
		command->type = MOUSE_COMMAND_MOVE;
		command->detail.dir = 
			gok_keyboard_parse_direction ((const char *)attribute);
	}
	else {
		attribute = xmlGetProp (node, (const xmlChar *) "action");
		if (attribute != NULL) {
			if (xmlStrcmp (attribute, (const xmlChar *) "latch") == 0) 
				command->type = MOUSE_COMMAND_TOGGLE_LATCH;
			else if (xmlStrncmp (attribute, (const xmlChar *) "button", 6) == 0) 
				command->type = MOUSE_COMMAND_BUTTON;
			else if (xmlStrncmp (attribute, (const xmlChar *) "double", 6) == 0) 
				command->type = MOUSE_COMMAND_DOUBLECLICK;
			command->detail.button = MOUSE_BUTTON_1; /* for now. */
		}
	}
}

void
gok_mouse_control (GokKey *key) 
{
	GokMouseCommand *command = (GokMouseCommand *) key->pGeneral;
	char *action;
	long x = -1, y = -1;
	long dx = 0, dy = 0;
	long epsilon = 7;
	fprintf (stderr, "mouse command %x, %x", command->type, command->detail.dir);
	switch (command->type) {
	case MOUSE_COMMAND_MOVE:
		action = "rel";
		switch (command->detail.dir) {
		case GOK_DIRECTION_W:
			dx = -epsilon;
			break;
		case GOK_DIRECTION_N:
			dy = -epsilon;
			break;
		case GOK_DIRECTION_NW:
			dx = -epsilon;
			dy = -epsilon;
			break;
		case GOK_DIRECTION_E:
			dx = epsilon;
			break;
		case GOK_DIRECTION_NE:
			dx = epsilon;
			dy = -epsilon;
			break;
		case GOK_DIRECTION_SW:
			dx = -epsilon;
			dy = epsilon;
			break;
		case GOK_DIRECTION_S:
			dy = epsilon;
			break;
		case GOK_DIRECTION_SE:
			dx = epsilon;
			dy = epsilon;
			break;
		default:
			break;
		}
		x = dx;
		y = dy;
		break;
	case MOUSE_COMMAND_BUTTON:
		switch (command->detail.button) {
		case MOUSE_BUTTON_2:
			action = "b2c";
			break;
		case MOUSE_BUTTON_3:
			action = "b3c";
			break;
		case MOUSE_BUTTON_1:
		default:
			action = "b1c";
			break;
		}
		break;
	case MOUSE_COMMAND_DOUBLECLICK:
		action = "b1d";
		break;
	default:
		return;
		break;
	}
	SPI_generateMouseEvent (x, y, action);
}
