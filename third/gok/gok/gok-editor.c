/*
* gok-editor.c
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
#include <config.h>
#endif

#include <gnome.h>
#include <glade/glade.h>
#include "gok-log.h"
#include "gok-keyboard.h"
#include "gok-editor.h"
#include "gok-modifier.h"
#include "gok-glade-helpers.h"

#define TITLE_GOK_EDITOR _("GOK Keyboard Editor")

/* pointer to the window group for the editor */
static GtkWindowGroup* m_pWindowGroup;

/* pointer to the window that displays the key parameters */
static GtkWidget* m_pWindowEditor = NULL;

/* pointer to the glade xml structure for the window that 
 * displays the key parameters */
static GladeXML* m_pEditorXML = NULL;

/* pointer to the window that displays the keyboard */
static GtkWidget* m_pWindowKeyboard;

/* will be TRUE if the current file has been modified */
static gboolean m_bFileModified;

/* name of the keyboard file */
static gchar* m_pFilename;

/* pointer to the keyboard we're currently editing */
static GokKeyboard* m_pKeyboard;

/* pointer to the key we're currently editing */
static GokKey* m_pKey;

/* use this flag when programmatically setting property controls from key properties */
static gboolean m_bSkipUpdate;

/**
* gok_editor_run
* 
* Runs the GOK keyboard editor.
*/
void gok_editor_run()
{
	GtkWidget *fixed1;
	
	m_pWindowGroup = NULL;
	m_pWindowKeyboard = NULL;
	m_bFileModified = FALSE;
	m_pFilename = NULL;
	m_pKeyboard = NULL;
	m_pKey = NULL;
	m_bSkipUpdate = TRUE;
	
	/* create the parameters dialog */
	m_pEditorXML = gok_glade_xml_new("gok.glade2", "windowEditor");
	m_pWindowEditor = glade_xml_get_widget(m_pEditorXML, "windowEditor");
	g_assert(m_pWindowEditor != NULL);
	gtk_widget_show (m_pWindowEditor);
	
	/* create the keyboard window */
	m_pWindowKeyboard = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data (GTK_OBJECT (m_pWindowKeyboard), "m_pWindowKeyboard", m_pWindowKeyboard);
	gtk_window_set_title (GTK_WINDOW (m_pWindowKeyboard), _(TITLE_GOK_EDITOR));

	/* create a window group */
/*	m_pWindowGroup = gtk_window_group_new();	
	g_assert (m_pWindowGroup!= NULL);
	gtk_window_group_add_window(m_pWindowGroup, (GtkWindow*)m_pWindowEditor);
	gtk_window_group_add_window(m_pWindowGroup, (GtkWindow*)m_pWindowKeyboard);
	*/

	/* handle the 'delete' event so the keyboard window will not close */
	gtk_signal_connect (GTK_OBJECT (m_pWindowKeyboard), "delete_event",
                      GTK_SIGNAL_FUNC (on_editor_keyboard_delete_event),
                      NULL);
	
	/* TODO - do not allow the user to resize the keyboard window */

	/* add the 'fixed' container to the window */
	fixed1 = gtk_fixed_new ();
	gtk_widget_ref (fixed1);
	gtk_object_set_data_full (GTK_OBJECT (m_pWindowKeyboard), "fixed1", fixed1,
                            (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (fixed1);
	gtk_container_add (GTK_CONTAINER (m_pWindowKeyboard), fixed1);
	gtk_widget_show (m_pWindowKeyboard);
	
	m_bSkipUpdate = FALSE;
}

/**
* gok_editor_close
* 
* Closes the GOK keyboard editor.
*/
void gok_editor_close()
{
	if (m_pKeyboard != NULL)
	{
		gok_keyboard_delete (m_pKeyboard,FALSE);
	}
	
	if (m_pFilename != NULL)
	{
		g_free (m_pFilename);
	}
}

/**
* gok_editor_on_exit
* 
* The menu item to close the program has been selected so close it.
*/
void gok_editor_on_exit ()
{
	GtkWidget* pDialog;
	gint response;

	if (m_bFileModified == TRUE)
	{
		/* ask if user wants to lose their changes */
		pDialog = gtk_message_dialog_new (GTK_WINDOW(m_pWindowEditor),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_OK_CANCEL,
					_("You have modified the current file.\nDo you want to discard your changes?"));
	
		gtk_window_set_title (GTK_WINDOW (pDialog), _("Keyboard Filename Invalid"));
		response = gtk_dialog_run (GTK_DIALOG (pDialog));
		gtk_widget_destroy (pDialog);
		if (response == GTK_RESPONSE_CANCEL)
		{
			return;
		}
	}

	g_signal_emit_by_name (m_pWindowEditor, "delete_event");
}

/**
* gok_editor_new_file
* 
* Create a new GOK keyboard file.
*/
void gok_editor_new_file ()
{
	GtkWidget* pDialog;
	GokKey* pKey;
	gint response;

	gok_log_enter();
	if (m_bFileModified == TRUE)
	{
		/* ask if user wants to lose their changes */
		pDialog = gtk_message_dialog_new (GTK_WINDOW(m_pWindowEditor),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_OK_CANCEL,
					_("You have modified the current file.\nDo you want to discard your changes?"));
	
		gtk_window_set_title (GTK_WINDOW (pDialog), _("Keyboard Filename Invalid"));
		response = gtk_dialog_run (GTK_DIALOG (pDialog));
		gtk_widget_destroy (pDialog);
		if (response == GTK_RESPONSE_CANCEL)
		{
			return;
		}
	}

	/* delete the previous keyboard */
	if (m_pKeyboard != NULL)
	{
		/* remove any buttons on the keyboard */
		pKey = m_pKeyboard->pKeyFirst;
		while (pKey != NULL)
		{
			if (pKey->pButton != NULL)
			{
				gtk_widget_destroy (pKey->pButton);
			}
			pKey = pKey->pKeyNext;
		}

		gok_keyboard_delete (m_pKeyboard,FALSE);
	}
	m_pKey = NULL;

	/* create a new keyboard */
	m_pKeyboard = gok_keyboard_new();
	gok_keyboard_set_name (m_pKeyboard, _("new"));
	m_pKeyboard->bSupportsWordCompletion = FALSE;
	m_pKeyboard->bSupportsCommandPrediction = FALSE;
	

	gok_editor_add_key();
	gok_editor_add_key();
	gok_editor_add_key();
	gok_editor_add_key();

	/* display the keyboard */
	gok_keyboard_display (m_pKeyboard, NULL, m_pWindowKeyboard, FALSE);

	/* clear the parameters dialog */
	gok_editor_show_parameters (NULL);

	/* clear the filename */
	if (m_pFilename != NULL)
	{
		g_free (m_pFilename);
		m_pFilename = NULL;
	}
	
	gok_editor_touch_file(TRUE);
	gok_log_leave();
}

/**
* gok_editor_open_file
* 
* Opens an existing keyboard file for editing.
*/
void gok_editor_open_file()
{
	GtkWidget* pDialogFilename;
	GtkWidget* pDialog;
	gint response;
	gchar* filename;
	GtkWidget* pEntry;
	GokKey* pKey;
	GtkFileFilter *filter;
	
	if (m_bFileModified == TRUE)
	{
		/* ask if user wants to lose their changes */
		pDialog = gtk_message_dialog_new (GTK_WINDOW(m_pWindowEditor),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_OK_CANCEL,
					_("You have modified the current file.\nDo you want to discard your changes?"));
	
		gtk_window_set_title (GTK_WINDOW (pDialog), _("Keyboard Filename Invalid"));
		response = gtk_dialog_run (GTK_DIALOG (pDialog));
		gtk_widget_destroy (pDialog);
		if (response == GTK_RESPONSE_CANCEL)
		{
			return;
		}
	}

	/* create the file selector dialog */
	pDialogFilename = gtk_file_chooser_dialog_new (_("Select keyboard file for editing"),
						       GTK_WINDOW(m_pWindowEditor),
						       GTK_FILE_CHOOSER_ACTION_OPEN,
						       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						       GTK_STOCK_OPEN, GTK_RESPONSE_OK,
						       NULL);

	/* display only .kbd files */
	/* TODO - this is not working! Why?? */
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _(".kbd files"));
	gtk_file_filter_add_pattern (filter, "*.kbd");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(pDialogFilename), filter);
	
	/* display that file selector dialog */
	response = gtk_dialog_run (GTK_DIALOG(pDialogFilename));

	if (response != GTK_RESPONSE_OK)
	{
		/* destroy the file selector dialog */
		gtk_widget_destroy (pDialogFilename);

		return;
	}
	
	/* get the file name */
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(pDialogFilename));
	
	/* is this a GOK keyboard file? */
	if ((strlen(filename) < 4) ||
		(strcmp(filename + (strlen(filename)-4), ".kbd") != 0))
	{
		/* no, destroy the file selector dialog */
		gtk_widget_destroy (pDialogFilename);

		gok_editor_message_filename_bad (filename);
		g_free (filename);
		return;
	}
	
	/* copy the file name */
	if (m_pFilename != NULL)
	{
		g_free (m_pFilename);
	}
	m_pFilename = filename;
	
	/* destroy the file selector dialog */
	gtk_widget_destroy (pDialogFilename);

	/* delete the previous keyboard */
	if (m_pKeyboard != NULL)
	{
		/* remove any buttons on the keyboard */
		pKey = m_pKeyboard->pKeyFirst;
		while (pKey != NULL)
		{
			if (pKey->pButton != NULL)
			{
				gtk_widget_destroy (pKey->pButton);
			}
			pKey = pKey->pKeyNext;
		}

		gok_keyboard_delete (m_pKeyboard,FALSE);
	}
	m_pKey = NULL;

	/* read the keyboard file */
	m_pKeyboard = gok_keyboard_read (m_pFilename);
	if (m_pKeyboard == NULL)
	{
		return;
	}
	
	/* display info about the keyboard */
	/* keyboard name */
	pEntry = glade_xml_get_widget (m_pEditorXML, "entryKeyboardName");
	g_assert (pEntry != NULL);
	gtk_entry_set_text (GTK_ENTRY(pEntry), gok_keyboard_get_name(m_pKeyboard));
	
	/* TODO - command prediction and word completion flags */
	
	/* edit the first key */
	m_bSkipUpdate = TRUE;
	m_pKey = m_pKeyboard->pKeyFirst;
	gok_editor_show_parameters (m_pKey);
	m_bSkipUpdate = FALSE;
	
	/* display the keyboard */
	gok_keyboard_display (m_pKeyboard, NULL, m_pWindowKeyboard, FALSE);

	/* we now have a fresh keyboard */
	gok_editor_touch_file(FALSE);
}


/**
* gok_editor_touch_file
* @modified: should the file be marked as modified?
* 
* Marks the file as having modifications (or not)
*/
void gok_editor_touch_file ( gboolean modified )
{
	gok_log_enter();
	m_bFileModified = modified;
	gok_editor_update_title();
	gok_log_leave();
}

/**
* gok_editor_show_parameters
* @pKey: Pointer to the key that will be edited.
* 
* Displays the parameters for the given key on the editor dialog.
*/
void gok_editor_show_parameters (GokKey* pKey)
{
	GtkWidget* pEntryLabel;
	GtkWidget* pSpin;
	
	gok_log_enter();

	/* normal key label */
	pEntryLabel = glade_xml_get_widget (m_pEditorXML, "entryLabel");
	g_assert (pEntryLabel != NULL);

	if (pKey == NULL)
	{
		/* reset all the entry fields */
		gtk_entry_set_text (GTK_ENTRY(pEntryLabel), "");
		
		return;
	}
	
	m_bSkipUpdate = TRUE;

	g_assert(pKey->Left < pKey->Right);
	g_assert(pKey->Top  < pKey->Bottom);

	gtk_entry_set_text (GTK_ENTRY(pEntryLabel), gok_key_get_label (pKey));

	/* location */
	pSpin = glade_xml_get_widget (m_pEditorXML, "spinLeft");
	g_assert (pSpin != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpin), pKey->Left);
	pSpin = glade_xml_get_widget (m_pEditorXML, "spinRight");
	g_assert (pSpin != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpin), pKey->Right);
	pSpin = glade_xml_get_widget (m_pEditorXML, "spinTop");
	g_assert (pSpin != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpin), pKey->Top);
	pSpin = glade_xml_get_widget (m_pEditorXML, "spinBottom");
	g_assert (pSpin != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpin), pKey->Bottom);

	m_bSkipUpdate = FALSE;

	gok_log_leave();
		
}

/**
* gok_editor_update_keyboard
* @pKeyboard: Pointer to the keyboard that will be updated.
*
* update the display of the keyboard and recalculate rows and columns
*/
void gok_editor_update_keyboard(GokKeyboard* pKeyboard)
{
	g_assert(pKeyboard != NULL);
	
	gok_keyboard_count_rows_columns(pKeyboard);
	gok_keyboard_display(pKeyboard, NULL, m_pWindowKeyboard, FALSE);
}

/**
* gok_editor_next_key
* 
* Display parameters for the next key on the keyboard.
*/
void gok_editor_next_key ()
{
	if (m_pKey != NULL)
	{
		if (m_pKey->pKeyNext != NULL)
		{
			m_pKey = m_pKey->pKeyNext;
			gok_editor_show_parameters (m_pKey);
		}
		else
		{
		}
	}
}

/**
* gok_editor_previous_key
* 
* Select the previous key properties
*
* returns: void
*/
void gok_editor_previous_key ()
{
	if (m_pKey != NULL)
	{
		if (m_pKey->pKeyPrevious != NULL)
		{
			m_pKey = m_pKey->pKeyPrevious;
			gok_editor_show_parameters (m_pKey);
		}
		else
		{
		}
	}
}

/**
* gok_editor_add_key
* 
* Add a default key to the keyboard.
*/
void gok_editor_add_key ()
{
	GokKey* pKey;
	GokKey* pKeyPrevious;
	GokKey* pNewKey;
	gint row,col;
	
	pKey = NULL;
	pKeyPrevious = NULL;
	pNewKey = NULL;
	
	gok_log_enter();
	
	if (m_pKeyboard != NULL)
	{
		pKey = m_pKeyboard->pKeyFirst;

		col = 0; /* start with the leftmost column */
		row = gok_keyboard_get_number_rows (m_pKeyboard); /* and the last row */
		if (row > 0)
		{
			row--;
		}
		
		while (pKey != NULL)
		{
			/* try to find the first empty column at the end of the last row */
			if ((pKey->Bottom > row) && (pKey->Right > col))
			{
				col = pKey->Right;
			}
			pKeyPrevious = pKey;
			pKey = pKey->pKeyNext;
		}
		
		pNewKey = gok_key_new (pKeyPrevious, NULL, m_pKeyboard);
		pNewKey->Top = row;
		pNewKey->Bottom = row + 1;
		pNewKey->Left = col;
		pNewKey->Right = col + 1;
		gok_log("adding key {top [%d] bottom [%d] left [%d] right [%d]}",pNewKey->Top,pNewKey->Bottom,pNewKey->Left,pNewKey->Right);
		gok_key_add_label (pNewKey, _("label"), 0, 0, NULL);
		
		gok_editor_touch_file(TRUE);
		gok_editor_update_keyboard(m_pKeyboard);
	}
	
	gok_log_leave();
}

/**
* gok_editor_delete_key
* 
* Delete the currently selected key..
*/
void gok_editor_delete_key ()
{
	gok_log_enter();
	if (m_pKey != NULL)
	{
		gok_key_delete(m_pKey, m_pKeyboard, TRUE);
		m_pKey = NULL;
		gok_editor_touch_file(TRUE);
	}
	else
	{
	}
	gok_log_leave();
}

/**
* gok_editor_duplicate_key
* 
* Add a copy of the currently selected key.
*/
void gok_editor_duplicate_key ()
{
	gok_log_enter();
	if (m_pKey != NULL)
	{
		/* gok_editor_touch_file(TRUE); */
	}
	else
	{
	}
	gok_log_leave();
}


/**
* gok_editor_update_key
* 
* Call this if a key property changes.
*/
void gok_editor_update_key()
{
	GtkWidget* pSpin;
	gint top;
	gint bottom;
	gint left;
	gint right;
	GokKey* pKey = m_pKey;
	
	g_assert (pKey != NULL);

	gok_log_enter();
	
	if (m_bSkipUpdate == FALSE)
	{
		/* location */
		pSpin = glade_xml_get_widget (m_pEditorXML, "spinLeft");
		g_assert (pSpin != NULL);
		left = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pSpin));
		
		pSpin = glade_xml_get_widget (m_pEditorXML, "spinRight");
		g_assert (pSpin != NULL);
		right = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pSpin));
		
		pSpin = glade_xml_get_widget (m_pEditorXML, "spinTop");
		g_assert (pSpin != NULL);
		top = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pSpin));
		
		pSpin = glade_xml_get_widget (m_pEditorXML, "spinBottom");
		g_assert (pSpin != NULL);
		bottom = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pSpin));
		
		if (left >= right)
		{
			/* TODO */
		}
		else if (top >= bottom)
		{
			/* TODO */
		}
		else
		{
			gok_log("cell span for [%s] t:%d b:%d l:%d r:%d", gok_key_get_label(pKey),top,bottom,left,right);		
			gok_key_set_cells(pKey, top, bottom, left, right);
			gok_editor_update_keyboard(m_pKeyboard);				
		}
		gok_editor_touch_file(TRUE);
	}
	
	gok_log_leave();
	
}

/**
* on_editor_keyboard_delete_event
* 
* Prevents the keyboard window from closing.
*
* returns: TRUE so the keyboard window will NOT close.
*/
gboolean on_editor_keyboard_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	return TRUE;
}

/**
* gok_editor_keyboard_key_press
* 
* The user has just pressed a key on the keyboard.
*/
void gok_editor_keyboard_key_press (GtkWidget* widget)
{
	GokKey* pKey;
	
	g_assert (m_pKeyboard != NULL);

	pKey = m_pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey->pButton == widget)
		{
			m_pKey = pKey;
			gok_editor_show_parameters (pKey);
			
			break;
		}
		pKey = pKey->pKeyNext;
	}

	if (pKey == NULL)
	{
		gok_log_x ("Can't find key!");
	}
}

/**
* gok_editor_save_keyboard
* 
* Saves the given keyboard to the given file.
*
* @pKeyboard: Pointer to the keyboard that will be saved to disk.
* @Filename: Name of the file where the keyboard will be saved.
*
* returns: TRUE if the keyboard was saved, FALSE if keyboard was not saved.
*/
gboolean gok_editor_save_keyboard (GokKeyboard* pKeyboard, gchar* Filename)
{
	FILE* pFile;
	gchar yes[]="yes";
	gchar no[]="no";
	gchar* pCommandYesNo;
	gchar* pCompletionYesNo;
	GokKey* pKey;
	GokKeyLabel* pLabel;
	gchar* typeKey;
	gchar branchTarget[101];
	gchar modifierName[101];

	pFile = fopen (Filename, "w");
	if (pFile == NULL)
	{
		gok_log_x (_("Can't save file: %s\n"), Filename);
		return FALSE;
	}

	fputs ("<?xml version=\"1.0\"?>\n", pFile);
	fputs ("<GOK:GokFile xmlns:GOK=\"http://www.gnome.org/GOK\">\n\n", pFile);

	pCommandYesNo = (pKeyboard->bSupportsCommandPrediction == TRUE) ? yes : no;
	pCompletionYesNo = (pKeyboard->bSupportsWordCompletion == TRUE) ? yes : no;
	fprintf (pFile, "<GOK:keyboard name=\"%s\" commandprediction=\"%s\" wordcompletion=\"%s\">\n",
					pKeyboard->Name, pCommandYesNo, pCompletionYesNo);
	
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		branchTarget[0] = 0;
		modifierName[0] = 0;
		
		switch (pKey->Type)
		{
			case KEYTYPE_NORMAL:
				typeKey = "normal";
				break;
				
			case KEYTYPE_MODIFIER:
				typeKey = "modifier";
				sprintf (modifierName, " modifier=\"%s\"", pKey->ModifierName);
				break;
				
			case KEYTYPE_BRANCH:
				typeKey = "branch";
				sprintf (branchTarget, " target=\"%s\"", pKey->Target);
				break;
				
			case KEYTYPE_BRANCHBACK:
				typeKey = "branchBack";
				break;
				
			case KEYTYPE_BRANCHMENUS:
				typeKey = "branchMenus";
				break;
				
			case KEYTYPE_BRANCHTOOLBARS:
				typeKey = "branchToolbars";
				break;
				
			case KEYTYPE_BRANCHMENUITEMS:
				typeKey = "branchMenuItems";
				break;
				
			case KEYTYPE_BRANCHALPHABET:
				typeKey = "branchAlphabet";
				break;
				
			case KEYTYPE_BRANCHGUI:
				typeKey = "branchGUI";
				break;
				
			case KEYTYPE_SETTINGS:
				typeKey = "settings";
				break;
				
			case KEYTYPE_BRANCHCOMPOSE:
				typeKey = "branchEditText";
				break;
				
			default:
				break;
		}
		
		fprintf (pFile, "\t<GOK:key left=\"%d\" right=\"%d\" top=\"%d\" bottom=\"%d\" type=\"%s\"",
					pKey->Left, pKey->Right, pKey->Top, pKey->Bottom, typeKey);
		
		if (branchTarget[0] != 0)
		{
			fputs (branchTarget, pFile);
		}
		
		if (modifierName[0] != 0)
		{
			fputs (modifierName, pFile);
			if (gok_modifier_get_type (pKey->ModifierName) == MODIFIER_TYPE_TOGGLE)
			{
				fputs (" modifiertype=\"toggle\"", pFile);
			}
		}
		
		if (pKey->FontSizeGroup != 0)
		{
			fprintf (pFile, " fontsizegroup=\"%d\"", pKey->FontSizeGroup);
		}
		
		fputs (">\n", pFile);
		
		/* print the key labels */
		pLabel = pKey->pLabel;
		while (pLabel != NULL)
		{
			fputs ("\t\t<GOK:label", pFile);
			if (pLabel->level != 0)			{
				fprintf (pFile, " level=\"%d\"", 
					 pLabel->level);
			}
			fprintf (pFile, ">%s</GOK:label>\n", pLabel->Text);
			
			pLabel = pLabel->pLabelNext;
		}

		/* print the wrapper outputs */
		if (pKey->pOutputWrapperPre != NULL)
		{
			fputs ("\t\t<GOK:wrapper type=\"pre\">\n", pFile);
			gok_editor_print_outputs (pFile, pKey->pOutputWrapperPre, TRUE);
			fputs ("\t\t</GOK:wrapper>\n", pFile);
		}

		if (pKey->pOutputWrapperPost != NULL)
		{
			fputs ("\t\t<GOK:wrapper type=\"post\">\n", pFile);
			gok_editor_print_outputs (pFile, pKey->pOutputWrapperPost, TRUE);
			fputs ("\t\t</GOK:wrapper>\n", pFile);
		}

		/* print the key outputs */
		gok_editor_print_outputs (pFile, pKey->pOutput, FALSE);
		
		/* print the end end tag */
		fputs ("\t</GOK:key>\n", pFile);

		/* get next key */
		pKey = pKey->pKeyNext;
	}
	
	fputs ("</GOK:keyboard>\n", pFile);
	fputs ("</GOK:GokFile>\n", pFile);

	fclose (pFile);
	
	gok_editor_touch_file(TRUE);
	
	return TRUE;
}

/**
* gok_editor_print_outputs
* @pFile: Pointer to the XML file that gets the outputs.
* @pOutput: Pointer to the first output that gets written.
* 
* Prints the outputs in XML format to the given file.
*
* returns: TRUE if the outputs were written, FALSE if not.
*/
gboolean gok_editor_print_outputs (FILE* pFile, GokOutput* pOutput, gboolean bWrapper)
{
	while (pOutput != NULL)
	{
		if (bWrapper == TRUE)
		{
			/* wrapper outputs are indented one more tab */
			fputs ("\t\t\t<GOK:output type=\"", pFile);
		}
		else
		{
			fputs ("\t\t<GOK:output type=\"", pFile);
		}
		
			
		if (pOutput->Type == OUTPUT_KEYSYM)
		{
			fputs ("keysym\"", pFile);
		}
		else
		{
			fputs ("keycode\" flag=\"", pFile);
			if (pOutput->Flag == SPI_KEY_PRESS)
			{
				fputs ("press\"", pFile);
			}
			else if (pOutput->Flag == SPI_KEY_RELEASE)
			{
				fputs ("release\"", pFile);
			}
			else
			{
				fputs ("pressrelease\"", pFile);
			}
		}
			
		fprintf (pFile, ">%s</GOK:output>\n", pOutput->Name);
			
		pOutput = pOutput->pOutputNext;
	}
		
	return TRUE;
}

/**
* gok_editor_save_current_keyboard
* 
* Saves the current keyboard back to its original file.
*
* returns: TRUE if the keyboard was saved, FALSE if keyboard was not saved.
*/
gboolean gok_editor_save_current_keyboard ()
{
	/* make sure there is a keyboard loaded and it's been modified */
	if ((m_pKeyboard != NULL) &&
		(m_bFileModified == TRUE))
	{
		/* get a filename if we don't have one (a new keyboard) */
		if (m_pFilename == NULL)
		{
			if (gok_editor_save_current_keyboard_as() == TRUE)
			{
				gok_editor_touch_file(FALSE);
				return TRUE;
			}
		}
		else
		{
			if (gok_editor_validate_keyboard (m_pKeyboard) == TRUE)
			{
				if (gok_editor_save_keyboard (m_pKeyboard, m_pFilename) == TRUE)
				{
					gok_editor_touch_file(FALSE);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/**
* gok_editor_save_current_keyboard_as
* 
* Saves the current keyboard under a new filename.
*
* returns: TRUE if the keyboard was saved, FALSE if keyboard was not saved.
*/
gboolean gok_editor_save_current_keyboard_as ()
{
	GtkWidget* pDialogFilename;
	gint response;
	gchar* filename;
	GtkFileFilter *filter;
	
	if (m_pKeyboard != NULL)
	{
		if (gok_editor_validate_keyboard (m_pKeyboard) == TRUE)
		{
			/* get name of keyboard filename */
			/* create the file selector dialog */
			pDialogFilename = gtk_file_chooser_dialog_new (_("Save keyboard file as"),
								       GTK_WINDOW(m_pWindowEditor),
								       GTK_FILE_CHOOSER_ACTION_SAVE,
								       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								       GTK_STOCK_SAVE, GTK_RESPONSE_OK,
								       NULL);

			/* display only .kbd files */
			/* TODO - this is not working! Why?? */
			filter = gtk_file_filter_new ();
			gtk_file_filter_set_name (filter, _(".kbd files"));
			gtk_file_filter_add_pattern (filter, "*.kbd");
			gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(pDialogFilename), filter);
	
			/* display that file selector dialog */
			response = gtk_dialog_run (GTK_DIALOG(pDialogFilename));
		
			if (response != GTK_RESPONSE_OK)
			{
				/* destroy the file selector dialog */
				gtk_widget_destroy (pDialogFilename);
				return FALSE;
			}
			
			/* get the file name */
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (pDialogFilename));
			
			/* is this a GOK keyboard file? */
			if ((strlen(filename) < 4) ||
				(strcmp(filename + (strlen(filename)-4), ".kbd") != 0))
			{
				/* no, destroy the file selector dialog */
				gtk_widget_destroy (pDialogFilename);
				
				gok_editor_message_filename_bad (filename);
				g_free (filename);
				return FALSE;
			}
			
			/* copy the file name */
			if (m_pFilename != NULL)
			{
				g_free (m_pFilename);
			}
			m_pFilename = filename;
			
			/* destroy the file selector dialog */
			gtk_widget_destroy (pDialogFilename);

			/* save the keyboard under this filename */
			gok_editor_save_keyboard (m_pKeyboard, filename);
		}
	}
	
	return TRUE;
}

/**
* gok_editor_validate_keyboard
* 
* Validates the current keyboard to make sure it is OK.
*
* returns: TRUE if the keyboard is OK, FALSE if not.
*/
gboolean gok_editor_validate_keyboard (GokKeyboard* pKeyboard)
{
	return TRUE;
}

/**
* gok_editor_message_filename_bad
* @Filename: Pointer to the file name.
*
* Inform the user that the selected file name is not a valid 
* GOK keyboard file name.
*/
void gok_editor_message_filename_bad (gchar* Filename)
{
	GtkWidget* pDialog;
	gchar buffer[101];

	sprintf (buffer, _("This is not a valid keyboard filename:\n%s"), Filename);

	pDialog = gtk_message_dialog_new (GTK_WINDOW(m_pWindowEditor),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_(buffer));

	gtk_window_set_title (GTK_WINDOW (pDialog), _("Keyboard Filename Invalid"));
	gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);
}

/**
* gok_editor_update_title
* 
* Displays the filename and modification status in the titlebar.
*
* returns: void
*/
void gok_editor_update_title ()
{
	gchar buffer[200];
	gchar* pNameOnly;
	
	gok_log_enter();
	
	/* if no filename then this is a new file */
	if (m_pFilename == NULL)
	{
		pNameOnly = _("(new)");
	}
	else
	{
		/* remove the path name from the file name */
		pNameOnly = strrchr (m_pFilename, '/');
		if (pNameOnly == NULL)
		{
			pNameOnly = m_pFilename;
		}
		else
		{
			pNameOnly++;
		}
	}
	
	sprintf (buffer, "%s - %s", TITLE_GOK_EDITOR, pNameOnly);
	if (m_bFileModified == TRUE)
	{
		strcat (buffer, "*");
	}
	gok_log("window title is %s", buffer);
	
	gtk_window_set_title (GTK_WINDOW(m_pWindowEditor), buffer);

	gok_log_leave();
}




