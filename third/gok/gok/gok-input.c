/* gok-input.c
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

#include <gnome.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gconf/gconf-client.h>
#include "gok-gconf-keys.h"
#include "gok-log.h"
#include "gok-input.h"
#include "gok-gconf.h"
#include "main.h"

#ifdef HAVE_XINPUT
static  XDeviceInfo  *devices = NULL;
#endif

int gok_input_types[N_INPUT_TYPES];

GSList *
gok_input_get_device_list (void)
{
  GSList       *pList = NULL;
  int		i;
  int		num_devices;
  Display      *display;
  GtkWidget    *pMainWindow = gok_main_get_main_window ();

#ifdef HAVE_XINPUT
  if (pMainWindow && pMainWindow->window) {
	  display = GDK_WINDOW_XDISPLAY (pMainWindow->window); 
	  if (devices) {
		XFreeDeviceList (devices);
		devices = NULL;
	  }
 
	  devices = XListInputDevices(display, &num_devices);
	  for (i = 0; i < num_devices; i++) {
		  GokInput *pInput;
		  if (devices[i].use == IsXExtensionDevice) {
			  GSList *sl;
			  GokInput *input;
			  gboolean duplicate;

			  duplicate = FALSE;
			  for (sl = pList; sl; sl = sl->next) {

			      input = (GokInput *)sl->data;
			      if (!strcmp (devices[i].name, input->name)) {
				duplicate = TRUE;
				break;
			      }
			  }
 
			  pInput = g_new0 (GokInput, 1);
			  pInput->name = g_strdup (devices[i].name);
			  pInput->info = &devices[i];
			  if (duplicate) {
			     while (input->next)
				input = input->next; 

			     input->next = pInput;
			  }
                          else  
			      pList = g_slist_prepend (pList, pInput);
		  }
	  }
  }
#endif
  return pList;
}

#ifdef HAVE_XINPUT
static GokInput*
find_input (Display	*display,
	    char	*name,
	    gboolean     extended_only)
{
	int		i;
	int		num_devices;
	GokInput *input = NULL;
	GokInput *temp;
	
	if (devices) {
		XFreeDeviceList (devices);
		devices = NULL;
	}

	devices = XListInputDevices(display, &num_devices);
	
	for(i = 0; i < num_devices; i++) {
		if (extended_only)
			if (devices[i].use != IsXExtensionDevice)
				continue;

		if (!strcmp(devices[i].name, name)) {
			temp = g_new0 (GokInput, 1);
			temp->name = g_strdup (devices[i].name);
			temp->info = &devices[i];

			if (input == NULL)
				input = temp;
			else {
				GokInput *temp1 = input;

				while (temp1->next)
					temp1 = temp1->next;	

				temp1->next = temp;
			}
			
		}
	}
	return input;
}

#endif

gboolean
gok_input_init (GokInput *input, GdkFilterFunc filter_func)
{
#ifdef HAVE_XINPUT
	XEventClass      event_list[40];
	int              i, number = 0; 
	GtkWidget       *window = gok_main_get_main_window ();
	Display         *display;
	GdkWindow       *root;
	GokInput        *temp;

	temp = input;

	while (temp) {
	
		if (!gok_input_open (temp)) {
			g_warning ("Cannot open input device!\n");
			return FALSE;
		}
	
		for (i = 0; i < temp->info->num_classes; i++) {
			switch (temp->device->classes[i].input_class) 
			{
			case KeyClass:
				DeviceKeyPress(temp->device, 
					       gok_input_types[GOK_INPUT_TYPE_KEY_PRESS], 
					       event_list[number]); number++;
				DeviceKeyRelease(temp->device, 
						 gok_input_types[GOK_INPUT_TYPE_KEY_RELEASE], 
						 event_list[number]); number++;
					 break;      
			case ButtonClass:
				DeviceButtonPress(temp->device, 
						  gok_input_types[GOK_INPUT_TYPE_BUTTON_PRESS], 
						  event_list[number]); number++;
				DeviceButtonRelease(temp->device, 
						    gok_input_types[GOK_INPUT_TYPE_BUTTON_RELEASE], 
						    event_list[number]); number++;
				break;
			case ValuatorClass:
				DeviceMotionNotify(temp->device, 
						   gok_input_types[GOK_INPUT_TYPE_MOTION], 
						   event_list[number]); number++;
			}
		}
		temp = temp->next;
	}
	printf ("%d event types available\n", number);
	
	root = gdk_screen_get_root_window (gdk_drawable_get_screen (window->window));
	
	if (XSelectExtensionEvent(GDK_WINDOW_XDISPLAY (root), 
				  GDK_WINDOW_XWINDOW (root),
				  event_list, number)) 
	{
		g_warning ("Can't connect to input device!");
		return FALSE;
	}
	
	gdk_window_add_filter (NULL,
			       filter_func,
			       NULL);
	return TRUE;
#else
	return FALSE;
#endif
}

gboolean
gok_input_set_extension_device_by_name (char *name)
{
	gok_gconf_set_string (gconf_client_get_default(), 
			      GOK_GCONF_INPUT_DEVICE,
			      name);
	return TRUE;
}

gchar *
gok_input_get_extension_device_name (void)
{
        gchar *cp;
        if (gok_gconf_get_string (gconf_client_get_default(), 
				  GOK_GCONF_INPUT_DEVICE,
				  &cp)) {
	        return cp;
	}
	else
	        return NULL;
}

void
gok_input_free (GokInput *pInput)
{
#ifdef HAVE_XINPUT
	if (pInput->next)
		gok_input_free (pInput->next);
#endif
	g_free (pInput->name);
	g_free (pInput);
}

GokInput * 
gok_input_find_by_name (char *name, gboolean extended_only)
{
  GtkWidget *window = gok_main_get_main_window ();
  GokInput *input = NULL;
#ifdef HAVE_XINPUT
  
  if (window && window->window) 
    {
      input = find_input (GDK_WINDOW_XDISPLAY (window->window), name, extended_only);
    }
#else
  g_message ("Warning, XInput extension not present, cannot open input device.");
#endif
  return input;
}

gboolean   
gok_input_open (GokInput *input)
{
  GtkWidget *window = gok_main_get_main_window ();

#ifdef HAVE_XINPUT  
  if (window && window->window && 
      input && input->info && (input->info->use == IsXExtensionDevice))
    {
      input->device = XOpenDevice(GDK_WINDOW_XDISPLAY (window->window), 
				  input->info->id);
      return TRUE;
    }
#endif
  if (input )
    g_warning ("could not open device %s", input);

  return FALSE;
}

GokInput * gok_input_get_current (void)
{
  char *input_device_name;
  static GokInput *input = NULL;

  if (input)
    gok_input_free (input);

  /* was the input device specified on the command line? */
  input_device_name = gok_main_get_inputdevice_name ();
  if (input_device_name == NULL) {
	  /* use gconf setting */
	  gok_gconf_get_string (gconf_client_get_default (), 
				GOK_GCONF_INPUT_DEVICE, 
				&input_device_name);
	  input = gok_input_find_by_name (input_device_name, TRUE);
	  g_free (input_device_name);
  }
  else
  {
      input = gok_input_find_by_name (input_device_name, TRUE);
  }
  return input;
}
