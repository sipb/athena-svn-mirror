/* gok-control.h
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gok-control.h"
#include "gok-settings-dialog.h"
#include "gok-log.h"

/* keep this array in the same order as the enum 'ControlTypes' in gok-control.h*/
static gchar ArrayControlTypeNames [MAX_CONTROL_TYPES][12] = {
"label",	
"hbox",	
"vbox",	
"combobox",	
"seperator",	
"frame",	
"button",	
"checkbutton",	
"radiobutton",	
"spinbutton"	
};

/* keep this array in the same order as the enum 'ControlHandlers' */
static gchar ArrayControlHandlerNames [MAX_CONTROL_HANDLERS][16] = {
"browsesound",
"inverseadvanced"
};

static GokControlCallback* m_pControlCallbackFirst;

/**
* gok_control_new
*
* Creates a new gok_control. It's the caller's responsibility to delete this.
*
* returns: A pointer to the new control.
**/
GokControl* gok_control_new ()
{
	GokControl* pControl;
	
	pControl = (GokControl*)g_malloc (sizeof (GokControl));
	pControl->Name = NULL;
	pControl->String = NULL;
	pControl->StringBackup = NULL;
	pControl->pWidget = NULL;
	pControl->Handler = -1;
	pControl->pControlChild = NULL;
	pControl->pControlNext = NULL;
	pControl->Type = -1;
	pControl->Size = 0;
	pControl->Border = 0;
	pControl->Spacing = 0;
	pControl->Fillwith = -1;
	pControl->Qualifier = -1;
	pControl->Value = 0;
	pControl->ValueBackup = 0;
	pControl->Min = 0;
	pControl->Max = 0;
	pControl->StepIncrement = 0;
	pControl->PageIncrement = 0;
	pControl->PageSize = 0;
	pControl->bGroupStart = FALSE;
	pControl->NameAssociatedControl = NULL;
	pControl->bAssociatedStateActive = TRUE;

	return pControl;
}

/**
* gok_control_delete_all
* @pControl: Pointer to the control that will be deleted.
*
* Deletes the given control and all other controls linked to it.
**/
void gok_control_delete_all (GokControl* pControl)
{
	GokControl* pControlTemp;

	while (pControl != NULL)
	{
		pControlTemp = pControl;
		pControl = pControl->pControlNext;

		gok_control_delete_all (pControlTemp->pControlChild);
		
		if (pControlTemp->String != NULL)
		{
			g_free (pControlTemp->String);
		}
		
		if (pControlTemp->Name != NULL)
		{
			g_free (pControlTemp->Name);
		}
		
		if (pControlTemp->NameAssociatedControl != NULL)
		{
			g_free (pControlTemp->NameAssociatedControl);
		}
		
		g_free (pControlTemp);
	}
}

/**
* gok_control_get_control_type
* @NameControlType: Name of the control type.
*
* returns: The ID number of the control type name. Returns -1 if the
* control type name can't be found.
**/
gint gok_control_get_control_type (gchar* NameControlType)
{
	int x;
	
	for (x = 0; x < MAX_CONTROL_TYPES; x++)
	{
		if (strcmp (ArrayControlTypeNames[x], NameControlType) == 0)
		{
			return x;
		}
	}

	gok_log_x ("Unknown control type '%s'!", NameControlType);
	return -1;
}

/**
* gok_control_get_handler_type
* @NameControlType: Name of the control handler.
*
* returns: The ID number of the control handler name. Returns -1 if the
* control handler name can't be found.
**/
gint gok_control_get_handler_type (gchar* NameControlHandler)
{
	int x;
	
	for (x = 0; x < MAX_CONTROL_HANDLERS; x++)
	{
		if (strcmp (ArrayControlHandlerNames[x], NameControlHandler) == 0)
		{
			return x;
		}
	}

	gok_log_x ("Unknown control handler (%s)!", NameControlHandler);
	return -1;
}

/**
* gok_control_button_callback_open
*
* Initializes the GOK control callback handlers. This must be called
* at the beginning of the program.
**/
void gok_control_button_callback_open ()
{
	m_pControlCallbackFirst = NULL;
}

/**
* gok_control_button_callback_close
*
* Frees any memory used by the control callbacks. This must be called
* at the end of the program.
**/
void gok_control_button_callback_close ()
{
	GokControlCallback* pCallbackTemp;
	
	while (m_pControlCallbackFirst != NULL)
	{
		pCallbackTemp = m_pControlCallbackFirst;
		m_pControlCallbackFirst = m_pControlCallbackFirst->pControlCallbackNext;
		g_free (pCallbackTemp);
	}
}

/**
* gok_control_button_handler
*
* @pButton: Pointer to the button that was just pressed.
* @user_data: User data that is associated with the button.
**/
void gok_control_button_handler (GtkButton* pButton, gpointer user_data)
{
	GokControlCallback* pCallback;
	GtkWidget* pDialog;

	/* find the appropriate callback for this button */
	pCallback = m_pControlCallbackFirst;
	while (pCallback != NULL)
	{
		if (pCallback->pWidget == GTK_WIDGET(pButton))
		{
			switch (pCallback->HandlerType)
			{
				case CONTROL_HANDLER_BROWSESOUND:
	
pDialog = gtk_message_dialog_new ((GtkWindow*)gok_settingsdialog_get_window(),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		_("Sorry, browse for sound file not implemented yet."));
	
gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Browse for sound file"));
gtk_dialog_run (GTK_DIALOG (pDialog));
gtk_widget_destroy (pDialog);
				

/*
					pFileSelectorDialog = gtk_file_selection_new (_("Select Sound File"));
					g_signal_connect_swapped (GTK_OBJECT(GTK_FILE_SELECTION(pFileSelectorDialog)->ok_button),
											"clicked", 
											G_CALLBACK (gtk_widget_destroy),
											(gpointer)pFileSelectorDialog);
					g_signal_connect_swapped (GTK_OBJECT(GTK_FILE_SELECTION(pFileSelectorDialog)->cancel_button),
											"clicked", 
											G_CALLBACK (gtk_widget_destroy),
											(gpointer)pFileSelectorDialog);
					gtk_widget_show (pFileSelectorDialog);
*/
					break;
					
				case CONTROL_HANDLER_ADVANCED:
pDialog = gtk_message_dialog_new ((GtkWindow*)gok_settingsdialog_get_window(),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		_("Sorry, advanced settings not implemented yet."));
	
gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Inverse Scanning Advanced"));
gtk_dialog_run (GTK_DIALOG (pDialog));
gtk_widget_destroy (pDialog);
					break;
					
				default:
					gok_log_x ("Default hit!");
					break;
			}
			
			break;
		}
		pCallback = pCallback->pControlCallbackNext;
	}
	
	if (pCallback == NULL)
	{
		gok_log_x ("Can't find this button in our callback list!");
	}
}

/**
* gok_control_add_handler
*
* @pWidget: Pointer to the button gets added to the handler list.
* @HandlerType: Type of handler the button requires.
**/
void gok_control_add_handler (GtkWidget* pWidget, gint HandlerType)
{
	GokControlCallback* pCallbackNew;
	GokControlCallback* pCallback;
	
	/* create a new button handler structure for this control */
	pCallbackNew = (GokControlCallback*)g_malloc (sizeof (GokControlCallback));
	pCallbackNew->pControlCallbackNext = NULL;
	pCallbackNew->HandlerType = HandlerType;
	pCallbackNew->pWidget = pWidget;
	
	/* add the new structure into our list of callback handlers */
	if (m_pControlCallbackFirst == NULL)
	{
		m_pControlCallbackFirst = pCallbackNew;
	}
	else
	{
		pCallback = m_pControlCallbackFirst;
		while (pCallback->pControlCallbackNext != NULL)
		{
			pCallback = pCallback->pControlCallbackNext;
		}
		pCallback->pControlCallbackNext = pCallbackNew;
	}
	
	/* connect a GTK signal handler to the widget */
	gtk_signal_connect (GTK_OBJECT (pWidget), "clicked",
			GTK_SIGNAL_FUNC (gok_control_button_handler), NULL);

}

/**
* gok_control_find_by_widget
* @pWidget: Pointer to the widget you want to find.
* @pControl: Pointer to a control.
*
* Utility function that finds a control given a widget. Checks through
* the list of controls and their children.
* Returns a pointer to the control that holds the given widget. Returns
* NULL if not found.
**/
GokControl* gok_control_find_by_widget (GtkWidget* pWidget, GokControl* pControl)
{
	GokControl* pControlFound;
	
	while (pControl != NULL)
	{
		if (pControl->pWidget == pWidget)
		{
			return pControl;
		}
		
		if (pControl->pControlChild != NULL)
		{
			pControlFound = gok_control_find_by_widget (pWidget, pControl->pControlChild);
			if (pControlFound != NULL)
			{
				return pControlFound;
			}
		}

		pControl = pControl->pControlNext;	
	}
	
	return NULL;
}

/**
* gok_control_find_by_name
* @pWidget: Pointer to the widget you want to find.
* @pControl: Pointer to a control.
*
* Utility function that finds a control given a name. Checks through
* the list of controls and their children.
* Returns a pointer to the control that has the given name. Returns
* NULL if not found.
**/
GokControl* gok_control_find_by_name (gchar* Name, GokControl* pControl)
{
	GokControl* pControlFound;
	
	while (pControl != NULL)
	{
		if ((pControl->Name != NULL) &&
			(strcmp (pControl->Name, Name) == 0))
		{
			return pControl;
		}
		
		if (pControl->pControlChild != NULL)
		{
			pControlFound = gok_control_find_by_name (Name, pControl->pControlChild);
			if (pControlFound != NULL)
			{
				return pControlFound;
			}
		}

		pControl = pControl->pControlNext;	
	}
	
	return NULL;
}
