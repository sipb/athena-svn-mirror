/* gok-page-keysizespace.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glade/glade.h>
#include "gok-page-keysizespace.h"
#include "gok-data.h"
#include "gok-log.h"
#include "gok-settings-dialog.h"

/* settings for the key size/spacing */
static gint m_keywidth, m_keyheight, m_spacing;
static gboolean m_bUseGtkPlusTheme;

/* backup of the settings (in case the "cancel" button is clicked) */
static gint m_keywidthBackup, m_keyheightBackup, m_spacingBackup; 
static gboolean m_bUseGtkPlusThemeBackup;

static GtkWidget *sample_buttons[4];

/**
* gok-settings-page-keysizespace-initialize
* @pWindowSettings: Pointer to the settings dialog window.
*
* Initializes this page of the gok settings dialog. This must be called
* prior to any calls on this page.
*
* returns: TRUE if the page was initialized OK, FALSE if not.
**/
gboolean gok_settings_page_keysizespace_initialize (GladeXML * xml)
{
	GtkWidget* fixedKeySpacing;
	GtkWidget* pWindowSettings;
	GtkWidget* widget;
	GtkTooltips *tooltips;
	gint i;

	g_assert (xml != NULL);

	/* initialize this data */
	m_keywidth = gok_data_get_key_width();
	m_keyheight = gok_data_get_key_height();
	m_spacing = gok_data_get_key_spacing();
	m_bUseGtkPlusTheme = gok_data_get_use_gtkplus_theme();
	
	/* backup the initial settings */
	gok_settings_page_keysizespace_backup();

	tooltips = gtk_tooltips_new ();

	/* create the example buttons for key size/spacing */
	/* I'm doing this here because Glade doesn't know about GokButtons */
	
	/* first, get the fixed area that holds the keys */
	fixedKeySpacing =  glade_xml_get_widget (xml, "fixedKeySpacing");
	g_assert (fixedKeySpacing != NULL);
 	pWindowSettings = glade_xml_get_widget (xml, "dialogSettings");
	g_assert (pWindowSettings != NULL);

	for (i = 0; i < 4; ++i)
	{
		sample_buttons[i] = gok_button_new_with_label (_("Button"), IMAGE_PLACEMENT_LEFT);
		g_assert (sample_buttons[i] != NULL);
		gtk_widget_set_name (sample_buttons[i], "StyleButtonNormal"); 
		gok_settings_page_keysizespace_set_label_name (sample_buttons[i]);
		gtk_container_add (GTK_CONTAINER (fixedKeySpacing), sample_buttons[i]);
		gtk_widget_show (sample_buttons[i]);
	}

	/* get the settings from the gok_data and update the controls */
	gok_settings_page_keysizespace_refresh();

	return TRUE;
}

/**
* gok_settings_page_keysizespace_set_label_name
* @pButton: Pointer to the button that gets the new label name.
*
* Helper function that sets the 'name' of the button label.
**/
void gok_settings_page_keysizespace_set_label_name (GtkWidget* pButton)
{
	GList* pListContainerChildren;

	pListContainerChildren = gtk_container_get_children (GTK_CONTAINER(pButton));
	while (pListContainerChildren != NULL)
	{
		if (GTK_IS_LABEL (pListContainerChildren->data) == TRUE)
		{
			gtk_widget_set_name (GTK_WIDGET(pListContainerChildren->data), "StyleTextNormal"); 
			break;
		}
		pListContainerChildren = pListContainerChildren->next;
	}
	
	g_list_free (pListContainerChildren);
}

/**
* gok_settings_page_keysizespace_refresh
* 
* Refreshes the key size and key spacing controls on the key size/spacing page 
* from the gok_data.
*
* returns: void.
**/
void gok_settings_page_keysizespace_refresh ()
{
	GtkWidget* pSpinSpacing;
	GtkWidget* pSpinWidth;
	GtkWidget* pSpinHeight;
	GtkWidget* pCheckUseTheme;
	GtkWidget* widget;

	/* initialize the spin control for the key width */
	pSpinWidth = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinKeyWidth");
	g_assert (pSpinWidth != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpinWidth), gok_data_get_key_width());

	/* initialize the spin control for the key height */
	pSpinHeight = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinKeyHeight");
	g_assert (pSpinHeight != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpinHeight), gok_data_get_key_height());

	/* initialize the spin control for the key spacing */
	pSpinSpacing = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinKeySpacing");
	g_assert (pSpinSpacing != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpinSpacing), gok_data_get_key_spacing());

	/* position the example keys to show key size and key spacing */
	gok_settings_page_keysizespace_display_keysizespacing (gok_data_get_key_width(),
							       gok_data_get_key_height(),
							       gok_data_get_key_spacing());

	pCheckUseTheme = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkUseTheme");
	g_assert (pCheckUseTheme != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pCheckUseTheme), gok_data_get_use_gtkplus_theme());

	/* initialize the state of the dock buttons */
	widget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "DockCheckButton");
	g_assert (widget != NULL);
	if (gok_data_get_dock_type () == GOK_DOCK_NONE)
	{
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget), FALSE);
	    gtk_widget_set_sensitive (glade_xml_get_widget (gok_settingsdialog_get_xml (), 
							    "DockTopRadiobutton"),
				      FALSE);
	    gtk_widget_set_sensitive (glade_xml_get_widget (gok_settingsdialog_get_xml (), 
							    "DockBottomRadiobutton"),
				      FALSE);
	}
	else
	{
	    GtkWidget *togglewidget;
	    togglewidget = glade_xml_get_widget (gok_settingsdialog_get_xml (), "DockTopRadiobutton");
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(togglewidget), gok_data_get_dock_type () == GOK_DOCK_TOP);
	    togglewidget = glade_xml_get_widget (gok_settingsdialog_get_xml (), "DockBottomRadiobutton");
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(togglewidget), gok_data_get_dock_type () == GOK_DOCK_BOTTOM);
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget), TRUE);
	}
	widget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "FillCheckButton");
	g_assert (widget != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget), gok_data_get_expand ());
}

/**
* gok_settings_page_keysizespace_apply
* 
* Stores the current settings in the gok_data.
*
* returns: TRUE if any settings have changed, FALSE if no settings have changed.
**/
gboolean gok_settings_page_keysizespace_apply ()
{
	gboolean bDataChanged;
	GtkWidget* pCheckUseTheme;
	
	bDataChanged = FALSE;

	/* update the gok data with any new settings */
	if (gok_data_get_key_spacing() != m_spacing)
	{
		bDataChanged = TRUE;
		gok_data_set_key_spacing (m_spacing);
	}

	if (gok_data_get_key_width() != m_keywidth)
	{
		bDataChanged = TRUE;
		gok_data_set_key_width (m_keywidth);
	}

	if (gok_data_get_key_height() != m_keyheight)
	{
		bDataChanged = TRUE;
		gok_data_set_key_height (m_keyheight);
	}

	pCheckUseTheme = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkUseTheme");
	g_assert (pCheckUseTheme != NULL);
	m_bUseGtkPlusTheme = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(pCheckUseTheme));
	if (gok_data_get_use_gtkplus_theme() != m_bUseGtkPlusTheme)
	{
		bDataChanged = TRUE;
		gok_data_set_use_gtkplus_theme (m_bUseGtkPlusTheme);
	}
	
	return bDataChanged;
}

/**
* gok_settings_page_keysizespace_revert
* 
* Revert to the backup settings for this page and store them in the gok_data.
*
* returns: TRUE if any of the settings have changed, FALSE 
* if they are all still the same.
**/
gboolean gok_settings_page_keysizespace_revert ()
{
	gboolean bDataChanged;

	bDataChanged = FALSE;
	
	/* get the original settings */
	if (m_spacing != m_spacingBackup)
	{
		bDataChanged = TRUE;
		m_spacing = m_spacingBackup;
		gok_data_set_key_spacing (m_spacing);
	}

	if (m_keywidth != m_keywidthBackup)
	{
		bDataChanged = TRUE;
		m_keywidth = m_keywidthBackup;
		gok_data_set_key_width (m_keywidth);
	}

	if (m_keyheight != m_keyheightBackup)
	{
		bDataChanged = TRUE;
		m_keyheight = m_keyheightBackup;
		gok_data_set_key_height (m_keyheight);
	}

	if (m_bUseGtkPlusTheme != m_bUseGtkPlusThemeBackup)
	{
		bDataChanged = TRUE;
		m_bUseGtkPlusTheme = m_bUseGtkPlusThemeBackup;
		gok_data_set_use_gtkplus_theme (m_bUseGtkPlusTheme);
	}

	if (bDataChanged == TRUE)
	{
		gok_settings_page_keysizespace_refresh();
	}
	
	return bDataChanged;
}

/**
* gok_settings_page_keysizespace_display_keysizespacing
* @KeyWidth: Width of the example keys.
* @KeyHeight: Height of the example keys.
* @Space: Spacing between the example keys.
*
* Displays the example key size and key spacing.
**/
void gok_settings_page_keysizespace_display_keysizespacing (gint KeyWidth, gint KeyHeight, gint Space)
{
	GtkWidget* pFixedKeyspacing;
	GtkWidget* pWindowSettings;
	GtkRequisition requisitionSizeFixed;
	gint i;
	gint left;
	gint top;
	gint width;

	if (KeyWidth < 0)
	{
		KeyWidth = m_keywidth;
	}
	if (KeyHeight < 0)
	{
		KeyHeight = m_keyheight;
	}
	if (Space < 0)
	{
		Space = m_spacing;
	}

	/* keep the values within range */
	if (KeyWidth < MIN_KEY_WIDTH)
	{
		KeyWidth = MIN_KEY_WIDTH;
	}
	else if (KeyWidth > MAX_KEY_WIDTH)
	{
		KeyWidth = MAX_KEY_WIDTH;
	}
	if (KeyHeight < MIN_KEY_HEIGHT)
	{
		KeyHeight = MIN_KEY_HEIGHT;
	}
	else if (KeyHeight > MAX_KEY_HEIGHT)
	{
		KeyHeight = MAX_KEY_HEIGHT;
	}
	if ((Space <0) ||
		(Space > MAX_KEY_SPACING))
	{
		Space = MAX_KEY_SPACING;
	}

	/* get the container that holds the 'keyspace' buttons */
	pFixedKeyspacing =  glade_xml_get_widget (gok_settingsdialog_get_xml(), "fixedKeySpacing");
	g_assert (pFixedKeyspacing != NULL);

	/* get the size of the fixed container */
	gtk_widget_size_request (pFixedKeyspacing, &requisitionSizeFixed);

	/* calculate the positions of the buttons */
	left = (requisitionSizeFixed.width - ((KeyWidth * 2) + Space)) / 2;
	top = (requisitionSizeFixed.height - ((KeyHeight * 2) + Space)) / 2;

	/* calculate the left side based upon the window size */
	pWindowSettings = glade_xml_get_widget (gok_settingsdialog_get_xml(), "dialogSettings");
	g_assert (pWindowSettings != NULL);
			
	if (GDK_IS_DRAWABLE(pWindowSettings->window) == TRUE)
	{
		gdk_window_get_size (pWindowSettings->window, &width, NULL);
		left = ((width - 32) - ((KeyWidth * 2) + Space)) / 2;
	}

	/* resize the buttons */
	for (i = 0; i < 4; ++i)
	{
		gtk_widget_set_size_request (sample_buttons[i], KeyWidth, KeyHeight);
	}

	/* position the buttons */
	gtk_fixed_move (GTK_FIXED(pFixedKeyspacing), sample_buttons[0], left, top);
	gtk_fixed_move (GTK_FIXED(pFixedKeyspacing), sample_buttons[1], left + Space + KeyWidth, top);
	gtk_fixed_move (GTK_FIXED(pFixedKeyspacing), sample_buttons[2], left, top + Space + KeyHeight);
	gtk_fixed_move (GTK_FIXED(pFixedKeyspacing), sample_buttons[3], left + Space + KeyWidth, top + Space + KeyHeight);

	m_keywidth = KeyWidth;
	m_keyheight = KeyHeight;
	m_spacing = Space;
}

/**
* gok_settings_page_keysizespace_backup
* 
* Copies all the member settings to backup.
**/
void gok_settings_page_keysizespace_backup ()
{
	m_spacingBackup = m_spacing;
	m_keywidthBackup = m_keywidth;
	m_keyheightBackup = m_keyheight;
	m_bUseGtkPlusThemeBackup = m_bUseGtkPlusTheme;
}

