/* gok-button.c
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2001 University Of Toronto
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

#include "gok-button.h"
#include "gok-keyboard.h"
#include "gok-log.h"
#include "gok-data.h"
#include "gok-scanner.h"
#include "main.h"
#include <stdio.h>
#include <gnome.h>

static void gok_button_class_init    (GokButtonClass *klass);
static void gok_button_init          (GokButton      *gok_button);
static gboolean gok_button_expose    (GtkWidget      *widget, GdkEventExpose *expose);
GokKey* gok_button_find_key (GtkWidget* pWidget);

static GtkToggleButtonClass *parent_class = NULL;

/* TODO: make image/text relationship (i.e. image on left, label on right) configurable */

/**
* gok_button_get_type
*
* returns: the gok button type
**/
GtkType
gok_button_get_type ()
{
  static GtkType GokButtonType = 0;
 if (!GokButtonType)
    {
      static const GtkTypeInfo GokButtonInfo =
      {
	"GokButton",
	sizeof (GokButton),
	sizeof (GokButtonClass),
	(GtkClassInitFunc) gok_button_class_init,
	(GtkObjectInitFunc) gok_button_init,
	NULL,
     NULL,
	(GtkClassInitFunc) NULL,
      };

      GokButtonType = gtk_type_unique (GTK_TYPE_TOGGLE_BUTTON, &GokButtonInfo);
    }

  return GokButtonType;
}

/**
* gok_button_class_init
* @class: Pointer to the class that will be initialized.
*
**/
static void
gok_button_class_init (GokButtonClass *klass)
{
	GtkWidgetClass   *widget_class;
	widget_class   = (GtkWidgetClass*)   klass;
	parent_class = g_type_class_peek_parent (klass);
	widget_class->expose_event = gok_button_expose;
}

/**
* gok_button_init
* @gok_button: Pointer to the button that will be initialized.
*
* Initializes a GOK button.
**/
static void
gok_button_init (GokButton* gok_button)
{
  gok_button->pBox = g_object_new (GTK_TYPE_HBOX, NULL);
  gtk_container_add (GTK_CONTAINER (gok_button), gok_button->pBox);
  gtk_widget_show (gok_button->pBox);
  gok_button->pLabel = NULL;
  gok_button->pImage = NULL;
  gok_button->indicator_type = "checkbox";
}

/**
* gok_button_new_with_label:
* @pText: Text string for the button's label.
*
* Creates a new GOK button with a label.
*
* Returns: A pointer to the new button, NULL if it could not be created.
*/
GtkWidget* gok_button_new_with_label (const gchar *pText, GokImagePlacementPolicy align)
{
	GokButton* pNewButton = NULL;

	/* create a new GOK button */
	pNewButton = g_object_new (GOK_TYPE_BUTTON, NULL);
	
	if (pNewButton == NULL)
	{
		gok_log_x ("Error: Can't create new GOK button '%s' in gok_button_new_with_label!\n", pText);
	}
	else {
		/* add the label to it */
		pNewButton->pLabel = gtk_label_new (pText);
		gtk_widget_show (GTK_WIDGET (pNewButton->pLabel));
		if (align == IMAGE_PLACEMENT_LEFT)
			gtk_box_pack_end (GTK_BOX (pNewButton->pBox), pNewButton->pLabel, TRUE, TRUE, 0);
		else
			gtk_box_pack_start (GTK_BOX (pNewButton->pBox), pNewButton->pLabel, TRUE, TRUE, 0);
	}
	return GTK_WIDGET (pNewButton);
}

/**
* gok_button_new_with_image:
* @pFilename: Filename for the button's image.
*
* Creates a new GOK button with an image.
*
* Returns: A pointer to the new button, NULL if it could not be created.
**/
GtkWidget* gok_button_new_with_image (GtkWidget *image, GokImagePlacementPolicy align)
{
	GokButton* pNewButton = NULL;
	
	/* create a new GOK button */
	pNewButton = g_object_new (GOK_TYPE_BUTTON, NULL);
	
	if (pNewButton == NULL) {
		gok_log_x ("Error: Can't create new GOK button in gok_button_new_with_image!\n");
	}
	else {
		/* add the image to it */
		pNewButton->pImage = image;
		gtk_widget_show (pNewButton->pImage);
		if (align == IMAGE_PLACEMENT_LEFT)
			gtk_box_pack_start (GTK_BOX (pNewButton->pBox), pNewButton->pImage, FALSE, FALSE, 0);
		else
			gtk_box_pack_end (GTK_BOX (pNewButton->pBox), pNewButton->pImage, FALSE, FALSE, 0);
	}

	return GTK_WIDGET (pNewButton);
}

/**
* gok_button_set_image:
* @button: The GokButton to be changed.
* @image: The image to be associated with the button.
*
* Sets the image/icon displayed on a GOK button.
*
*/
void gok_button_set_image (GokButton *button, GtkImage *image)
{
	if (button->pImage) {
		gtk_container_remove (GTK_CONTAINER (button->pBox), button->pImage);
	}
	button->pImage = GTK_WIDGET (image);
	gtk_widget_show (button->pImage);
	gtk_box_pack_start (GTK_BOX (button->pBox), button->pImage, FALSE, FALSE, 0);
}

/**
* gok_button_set_label:
* @button: The GokButton to be changed.
* @label: The label to be associated with the button.
*
* Sets the text label displayed on a GOK button.
*
*/
void gok_button_set_label (GokButton *button, GtkLabel *label)
{
	if (button->pLabel) {
		gtk_container_remove (GTK_CONTAINER (button->pBox), button->pLabel);
	}
	button->pLabel = GTK_WIDGET (label);
	gtk_widget_show (button->pLabel);
	gtk_box_pack_end (GTK_BOX (button->pBox), button->pLabel, TRUE, TRUE, 0);
}

/**
* gok_button_enter_notify:
* @widget: Pointer to the widget that has just been entered.
* @event: Not sure?
*
* This handler is called whenever a widget on the keyboard is entered.
*
* Returns: TRUE if the given widget is associated with a GOK key, FALSE if
* the given button is not associated with a GOK key.
*/
gint gok_button_enter_notify   (GtkWidget *widget, GdkEventCrossing   *event)
{
	GokKey* pKey;

	pKey = gok_button_find_key (widget);
	if (pKey != NULL)
	{
		if (!strcmp (gok_data_get_name_accessmethod (), "dwellselection")) 
		{
		     gok_main_warn_if_corepointer_mode (_("You appear to be using GOK in \'core pointer\' mode."), TRUE);
		}
		gok_scanner_on_key_enter (pKey);
	}

	return FALSE;
}

/**
* gok_button_leave_notify:
* @widget: Pointer to the widget that has just been left.
* @event: Not sure?
*
* This handler is called whenever a widget on the keyboard has been left.
*
* Returns: TRUE if the given widget is associated with a GOK key, FALSE if
* the given button is not associated with a GOK key.
*/
gint gok_button_leave_notify   (GtkWidget *widget, GdkEventCrossing   *event)
{
	GokKey* pKey;
	gint x, y;

	if (GOK_IS_BUTTON (widget))
	{
		gdk_drawable_get_size (GTK_BUTTON (widget)->event_window,
				       &x, &y);
		if (event->x <= 0 || event->x >= x ||
		    event->y <= 0 || event->y >= y)
		{
			pKey = gok_button_find_key (widget);
			if (pKey != NULL)
			{
				gok_scanner_on_key_leave (pKey);
			}	
		}
	}	
	return FALSE;
}

/**
* gok_button_state_changed:
* @widget: The button that has just changed state.
* @state: State requested (not necessarily the state we set it).
* @user_data: Any user data associated with the widget (ignored by us).
*
* This is called each time the button state is changed. We handle this call
* and make sure the button is set to the state we want.
*/
void gok_button_state_changed (GtkWidget *widget, GtkStateType state, gpointer user_data)
{
	GokKey* pKey;

	if (GOK_IS_BUTTON (widget)) 
	{
	        pKey = gok_button_find_key (widget);
		if (pKey)
			gtk_widget_set_state (widget, pKey->State);
	}

}

/**
* gok_button_find_key
* @pWidget: Pointer to the widget that you need the GOK key for.
*
* Finds the GOK key given a pointer to a widget.
*
* Returns: A pointer to the key that holds the widget, NULL if not found.
**/
GokKey* gok_button_find_key (GtkWidget* pWidget)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey->pButton == pWidget)
		{
			return pKey;
		}
		pKey = pKey->pKeyNext;
	}

	return NULL;
}

static gboolean
gok_button_expose (GtkWidget      *widget,
		   GdkEventExpose *event)
{
 	gboolean retval = 
		(* GTK_WIDGET_CLASS (parent_class)->expose_event) (widget, event);
	
	if (GTK_WIDGET_DRAWABLE (widget) && GOK_BUTTON (widget)->pImage) {
		if (GOK_BUTTON (widget)->indicator_type != NULL) {
			GtkWidget *image = GOK_BUTTON (widget)->pImage;
			GdkRectangle rect;
			gint indicator_size = 13, indicator_spacing = 2;
			/* TODO: must get this from something other than a GtkToggleButton
			   gtk_widget_style_get (image, "indicator_size", &indicator_size, 
			   "indicator_spacing", &indicator_spacing, NULL);
			*/
			rect.x = image->allocation.x + (image->allocation.width - indicator_size)/2;
			/* "20" is size of standard status images we use. kludge. */
			rect.y = image->allocation.y + (image->allocation.height - indicator_size)/2;
			rect.width = indicator_size;
			rect.height = indicator_size;
			if (*(GOK_BUTTON (widget)->indicator_type) == 'r') {
				gtk_paint_option (gtk_widget_get_style (image),
						  gtk_widget_get_parent_window (image),
						  GTK_WIDGET_STATE (image),
						  (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) ? 
						  GTK_SHADOW_IN : GTK_SHADOW_OUT,
						  &rect,
						  image,
						  GOK_BUTTON (widget)->indicator_type,
						  rect.x, rect.y, rect.width, rect.height);
			} 
			else if (*(GOK_BUTTON (widget)->indicator_type) == 's') {
				GdkPoint points[5];
				points[4].x = points[0].x = rect.x + 1;
				points[4].y = points[0].y = rect.y + rect.height;
				points[3].x = points[1].x = rect.x + rect.width/2;
				points[1].y = rect.y;
				points[2].x = rect.x + rect.width - 1;
				points[2].y = rect.y + rect.height;
				points[3].y = rect.y + rect.height - 2;
				gtk_paint_polygon (gtk_widget_get_style (widget),
						   gtk_widget_get_parent_window (image),
						   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ?
						   GTK_STATE_PRELIGHT : GTK_WIDGET_STATE (widget),
						   GTK_SHADOW_IN,
						   &rect,
						   image,
						   GOK_BUTTON (widget)->indicator_type,
						   points,
						   5,
						   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ?
					           TRUE : FALSE);
			}
			else {
				gtk_paint_check (gtk_widget_get_style (image),
						 gtk_widget_get_parent_window (image),
						 GTK_WIDGET_STATE (image),
						 (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) ? 
						 GTK_SHADOW_IN : GTK_SHADOW_OUT,
						 &rect,
						 image,
						 GOK_BUTTON (widget)->indicator_type,
						 rect.x, rect.y, rect.width, rect.height);
			}
		}
	}

	return retval;
}
