/* testlibsrconf.c
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
#include <string.h>
#include "libsrconf.h"


static void testfcnt(SREvent *event,unsigned long data)
{
  SRConfigStructure *cfg = (SRConfigStructure *)(event->data);

  printf("EventSource:%i %s \n",cfg->module, cfg->key);
  /*  Get_Braille_Config_Data("key",CT_INT,&ii);*/
  /*  printf("KeyValue:%d\n\n",ii); */


  if((cfg->module == CFGM_SRCORE) && (strcmp(cfg->key,"Exit") == 0)
	&& ((*(gboolean *)(cfg->newvalue)) == TRUE))
  {
    /* g_free(str); */
/*    sru_exit_loop();*/
  }
    /*  g_free(str); */
}

int main(int argc, char *argv[])
{
  gchar *text =  NULL;
  fprintf(stderr, "\ntestlibsrconf started...\n");

  fprintf(stderr, "\nSPI init...\n");
  sru_init();

  fprintf(stderr, "\nsrconf init...\n");
  srconf_init((SRConfCB)testfcnt,NULL, NULL);

/*  fprintf(stderr, "\nTest the sending...\n"); */
/*  fprintf(stderr, "\nChanging the /apps/gnopernicus/Kbd_mouse/KE_Mode key to AUTO...returned %s\n", */
/*    SET_KBD_MOUSE_CONFIG_DATA("KE_Mode", CFGT_STRING, "AUTO" ) ? "TRUE" : "FALSE" ); */
/*  fprintf(stderr, "\nChanging the /apps/gnopernicus/Kbd_mouse/KE_Mode key to LETTER...returned %s\n", */
/*    SET_KBD_MOUSE_CONFIG_DATA("KE_Mode", CFGT_STRING, "LETTER" ) ? "TRUE" : "FALSE" ); */
/*  fprintf(stderr, "\nChanging back the /apps/gnopernicus/Kbd_mouse/KE_Mode key to AUTO...returned %s\n", */
/*    SET_KBD_MOUSE_CONFIG_DATA("KE_Mode", CFGT_STRING, "AUTO" ) ? "TRUE" : "FALSE" ); */

  fprintf(stderr, "\nTest the receiving...\n");
  fprintf(stderr, "\nSPI entry loop...\n");
  
  srconf_get_data_with_default("KE_Mode",CFGT_STRING,&text,"default","Kbd_mouse");
  fprintf(stderr,"BBD:%s\n",text);
  
  GET_KBD_MOUSE_CONFIG_DATA_WITH_DEFAULT("KE_MODe",CFGT_STRING,&text,(gpointer)g_strdup("default string"));
  fprintf(stderr,"BBD:%s\n",text);
  
  g_free(text);
  
  sru_entry_loop();

  fprintf(stderr, "\nsrconf terminate...\n");
  srconf_terminate();

  fprintf(stderr, "\nSPI exit...\n");
  sru_terminate();

  fprintf(stderr, "\ntestlibsrconf end.\n");
  
  return 0;
}
