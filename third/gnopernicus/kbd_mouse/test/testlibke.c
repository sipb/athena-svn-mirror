/* testlibke.c
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
#include <stdio.h>
#include "libke.h"
#include "SREvent.h"
#include "libsrconf.h"
#include <string.h>
#if 0
static void my_cb_fnc(SREvent *event, unsigned long data)
{
  gchar *str = NULL;
  SRHotkeyData *srhotkey_data = NULL;
  
  switch(event->type)
  { 
  case SR_EVENT_CONFIG_CHANGED:
	switch(((SRConfigStructure *)(event->data))->module)
	{
	case CFGM_BRAILLE:
	    break;
	case CFGM_GNOPI:
	    break;
	case CFGM_KBD_MOUSE:
	    ke_config_changed(event->data);
	    break;
	case CFGM_MAGNIFIER:
	    break;
	case CFGM_SPEECH:
	    break;
	case CFGM_SRCORE:
	    break;
	case CFGM_SRLOW:
	    break;
	case CFGM_KEY_PAD:
	    break;
	case CFGM_PRESENTATION:
	    break;
	}
	break;
  case SR_EVENT_KEYBOARD_ECHO:
	str = (char *)event->data;
	printf("\"%s\"\n",str);
	break;
  case SR_EVENT_HOTKEY:
	srhotkey_data = (SRHotkeyData *) event->data;
	printf("\nSR_EVENT_HOTKEY\tKeyID = %ld\t%s%s%s%s\n", 
			srhotkey_data->keyID,
			srhotkey_data->modifiers & SRHOTKEY_ALT ? "ALT + " : "",
			srhotkey_data->modifiers & SRHOTKEY_CTRL ? "CTRL + " : "",
			srhotkey_data->modifiers & SRHOTKEY_SHIFT ? "SHIFT + ": "",
			srhotkey_data->keystring);	
	if(strcmp(srhotkey_data->keystring,"q") == 0)
	{
	    fprintf(stderr, "before sru_exit_loop");
	    sru_exit_loop();
	}
	else if(strcmp(srhotkey_data->keystring,"d") == 0)
	{
	    ke_set_default_configuration();
	}
	break;
  case SR_EVENT_KEY:
	srhotkey_data = (SRHotkeyData *) event->data;
	printf("\nSR_EVENT_KEY\tKeyID = %ld\t%s%s%s%s\n", 
			srhotkey_data->keyID,
			srhotkey_data->modifiers & SRHOTKEY_ALT ? "ALT + " : "",
			srhotkey_data->modifiers & SRHOTKEY_CTRL ? "CTRL + " : "",
			srhotkey_data->modifiers & SRHOTKEY_SHIFT ? "SHIFT + ": "",
			srhotkey_data->keystring);	
	break;
    default:
	break;
  }
}
#endif
int
main(int argc, char **argv)
{
/*  int i;*/
  printf("\nKeyboard echo tester\nPress:\n\tCTRL + ALT + d for setting the default configuration\n\tCTRL + ALT + q to quit\n\n");
/*
  gchar s[] = "`1234567890-=~!@#$%^&*()_+[]{};\':\",./\\<>?| \t\n\r";  
  fprintf(stderr,"\n**************************************************\n");
  for( i = 0; i < strlen(s); i++ )
    fprintf(stderr,"\'%c\'\tispunct = %4i\tisspace = %4i\n",s[i],g_ascii_ispunct(s[i]),g_ascii_isspace(s[i]));
  fprintf(stderr,"**************************************************\n\n");
*/  
  sru_init();
#if 0
  srconf_init((SRConfCB)my_cb_fnc, NULL);
  
  i = ke_init((KeyboardEchoCB)my_cb_fnc);

  sru_entry_loop();

  ke_terminate();

  srconf_terminate();
#endif
  sru_terminate();
  
  printf("\nKeyboard echo tester finished.\n");
  return 0;
}
