/* test.c
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

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>


typedef enum {BRAILLE=0,KEYBOARD=1,MAGNIFIER=2,SPEECH=3,SRCORE=4,SRLOW=5} OBJ;
typedef enum {_INT=0,_FLOAT,_STRING,_BOOL} VAL;

#define PATH "/apps/gnopernicus"
#define BRAILLE_PATH "/apps/gnopernicus/braille"
#define KEYBOARD_PATH "/apps/gnopernicus/kbd_mouse"
#define MAGNIFIER_PATH "/apps/gnopernicus/magnifier"
#define SPEECH_PATH  "/apps/gnopernicus/speech"
#define SRCORE_PATH 	"/apps/gnopernicus/srcore"
#define SRLOW_PATH  	"/apps/gnopernicus/srlow"

#define MAX_STRING_LENGTH 100
#define MAX_PATH_LENGTH   100
#define MAX_NAME           10

GConfClient *client;

void call_br();
void call_kb();
void call_mag();
void call_sp();
void call_core();
void call_low();

int main(int argc, char *argv[])
{ 
 gint i;
 gboolean quit=FALSE;
 /* gtk_init(&argc,&argv); */
 client = gconf_client_get_default();
 gconf_client_add_dir(client,
	PATH,GCONF_CLIENT_PRELOAD_NONE,NULL);
 while(!quit)
 {
  printf("Select...\n\t0=BRAILLE\n\t1=KEYBOARD\n\t2=MAGNIFIER\n\t3=SPEECH\n\t4=SRCORE\n\t5=SRLOW\n\t9=QUIT\n");
  scanf("%d",&i);
  switch(i)
  {
   case 0:{call_br();break;}
   case 1:{call_kb();break;}
   case 2:{call_mag();break;}
   case 3:{call_sp();break;}
   case 4:{call_core();break;}
   case 5:{call_low();break;}
   case 9:quit=TRUE;break;
  }
 }
 gconf_client_remove_dir(client,PATH,NULL);
 /* gtk_main();*/
 g_object_unref(client);
 return 0;    
}

void call_br()
{
 int i,in;
 char name[10];
 char path[100];
 float fl;
 char stri[100];
 printf("Select key name:");
 scanf("%s",name);
 printf("Select key type(0-int,1-float,2-string):");
 scanf("%d",&i);
 sprintf(path,"%s/%s",BRAILLE_PATH,name);
 switch(i)
 {
  case _INT:{
	    printf("value:");
	    scanf("%d",&in);
	    gconf_client_set_int(client,path,in,NULL); 
	    break;
	    }
  case _FLOAT:{
	    printf("value:");
	    scanf("%f",&fl);
	    gconf_client_set_float(client,path,fl,NULL); 
	    break;
	    }
  case _STRING:
	    {
	    printf("value:");
	    scanf("%s",stri);
	    gconf_client_set_string(client,path,stri,NULL);
	    }
 }
}

/**
 * NUM_OF_KEY_CATEGORIES:
 *
 * The number of elements in KeyCategoriesEnum
**/
#define NUM_OF_KEY_CATEGORIES 22

/**
 * key_category_names:
 *
 * A table with the name of each key category. As index for this table can
 * be used a KeyCategoriesEnum. This table is also used as a key name in gconf.
**/
static const gchar *key_category_names[NUM_OF_KEY_CATEGORIES] = {
    "None",
    "Character",
    "Punctuation",
    "Control",
    "Alt",
    "Shift",
    "Caps_lock",
    "Num_lock",
    "Scroll_lock",
    "Super",
    "Menu",
    "Function",
    "Escape",
    "Tab",
    "Return",
    "Space",
    "Backspace",
    "Cursor",
    "Editing",
    "Print",
    "Pause",
    "Pad"
    };

/**
 * key_category_root_in_gconf:
 *
 * The name of the root directory of the key categories in gconf.
**/
static const gchar *key_category_root_in_gconf = "Key_Categories/";

/**
 * ke_mode_values:
 *
 * The KE mode names.
**/
static const gchar *ke_mode_values[4] = {
	"LETTER",
	"WORD",
	"AUTO",
	"NONE"
	};

int select_choice( int min, int max )
{
 int key;
 do
 {
   scanf("%i", &key);
 } while (key<min || key > max);
 return key;
}

void call_kb()
{
 int i,in;
 gchar *path = NULL;
 printf("\nSelect key name:\n\t1. KE_Mode\n\t2. KE_Timeout\n\t3. %s\n\t4. return to main menu\n",
 	key_category_root_in_gconf);
 switch (select_choice(1,4))
 {
 case 1:
   path = g_strdup_printf("%s/KE_Mode",KEYBOARD_PATH);
   printf("\nSelect KE working mode (current mode: %s):\n\t1. LETTER\n\t2. WORD\n\t3. AUTO\n\t4. NONE\n\t5. return to main menu\n",
	gconf_client_get_string(client, path, NULL));
   i = select_choice(1,5);
   if (i < 5)
     gconf_client_set_string(client, path, ke_mode_values[i-1] ,NULL);
   break;
 case 2:
   path = g_strdup_printf("%s/KE_Timeout", KEYBOARD_PATH);
   printf( "Enter timeout value (in miliseconds: 1 -> 1500, current value is %ld))",
    (unsigned long)gconf_client_get_int( client, path, NULL));
   i = select_choice(1, 1500);
   gconf_client_set_int( client, path, i, NULL);
   break;
 case 3:
   printf( "Select key category...\n" );
   for( i = 1; i < NUM_OF_KEY_CATEGORIES; i++ )
   {
        path = g_strdup_printf("%s/%s%s",
     		KEYBOARD_PATH,
     		key_category_root_in_gconf,
		key_category_names[i]);
   	printf( "\t%2i. %s\t\t%s%s\n", i, key_category_names[i],
	    (strlen(key_category_names[i]) < 4) ? "\t" : "",
	    gconf_client_get_bool(client, path, NULL) ? "Enabled" : "Disabled");
	g_free (path);
	path = NULL;
   }
   printf( "\t%2i. return to main menu\n", i);
   i = select_choice(1, NUM_OF_KEY_CATEGORIES);
   if (i < NUM_OF_KEY_CATEGORIES)
   {
     path = g_strdup_printf("%s/%s%s",
     		KEYBOARD_PATH,
     		key_category_root_in_gconf,
		key_category_names[i]);
     printf("Enter the new value...\n\t1. Enable\n\t2. Disable\n");
     in = select_choice (1, 2);
     gconf_client_set_bool(client, path, ((in == 1) ? TRUE : FALSE), NULL);
   }
   break;
 }
 if (path) g_free (path);

}

void call_mag()
{
 int i,in;
 char name[MAX_NAME];
 char path[MAX_PATH_LENGTH];
 float fl;
 char stri[MAX_STRING_LENGTH];

 printf("Select key name:");
 scanf("%s",name);
 printf("Select key type(0-int,1-float,2-string):");
 scanf("%d",&i);
 sprintf(path,"%s/%s",MAGNIFIER_PATH,name);
 switch(i)
 {
  case _INT:{
	    printf("value:");
	    scanf("%d",&in);
	    gconf_client_set_int(client,path,in,NULL); 
	    break;
	    }
  case _FLOAT:{
	    printf("value:");
	    scanf("%f",&fl);
	    gconf_client_set_float(client,path,fl,NULL); 
	    break;
	    }
  case _STRING:
	    {
	    printf("value:");
	    /*adi begin */
	    getchar();
	    fgets(stri, MAX_STRING_LENGTH-1, stdin);
	    /*adi end */
	    gconf_client_set_string(client,path,stri,NULL); 
	    }
 }
}
void call_sp()
{
 int i,in;
 char name[10];
 char path[100];
 float fl;
 char stri[100];
 printf("Select key name:");
 scanf("%s",name);
 printf("Select key type(0-int,1-float,2-string):");
 scanf("%d",&i);
 sprintf(path,"%s/%s",SPEECH_PATH,name);
 switch(i)
 {
  case _INT:{
	    printf("value:");
	    scanf("%d",&in);
	    gconf_client_set_int(client,path,in,NULL); 
	    break;
	    }
  case _FLOAT:{
	    printf("value:");
	    scanf("%f",&fl);
	    gconf_client_set_float(client,path,fl,NULL); 
	    break;
	    }
  case _STRING:
	    {
	    printf("value:");
	    scanf("%s",stri);
	    gconf_client_set_string(client,path,stri,NULL); 
	    }
 }
}
void call_core()
{
 int i,in;
 char name[10];
 char path[100];
 float fl;
 char stri[100];
 printf("Select key name:");
 scanf("%s",name);
 printf("Select key type(0-int,1-float,2-string,3-bool):");
 scanf("%d",&i);
 sprintf(path,"%s/%s",SRCORE_PATH,name);
 switch(i)
 {
  case _INT:{
	    printf("value:");
	    scanf("%d",&in);
	    gconf_client_set_int(client,path,in,NULL); 
	    break;
	    }
  case _FLOAT:{
	    printf("value:");
	    scanf("%f",&fl);
	    gconf_client_set_float(client,path,fl,NULL); 
	    break;
	    }
  case _STRING:
	    {
	    printf("value:");
	    scanf("%s",stri);
	    gconf_client_set_string(client,path,stri,NULL); 
	    }
    case _BOOL:
	    {
	    printf("value(1/0):");
	    scanf("%d",&in);
	    if (in)  gconf_client_set_bool(client,path,TRUE,NULL); 
		else gconf_client_set_bool(client,path,FALSE,NULL); 
	    }
 }
}
void call_low()
{
 int i,in;
 char name[10];
 char path[100];
 float fl;
 char stri[100];
 printf("Select key name:");
 scanf("%s",name);
 printf("Select key type(0-int,1-float,2-string):");
 scanf("%d",&i);
 sprintf(path,"%s/%s",SRLOW_PATH,name);
 switch(i)
 {
  case _INT:{
	    printf("value:");
	    scanf("%d",&in);
	    gconf_client_set_int(client,path,in,NULL); 
	    break;
	    }
  case _FLOAT:{
	    printf("value:");
	    scanf("%f",&fl);
	    gconf_client_set_float(client,path,fl,NULL); 
	    break;
	    }
  case _STRING:
	    {
	    printf("value:");
	    scanf("%s",stri);
	    gconf_client_set_string(client,path,stri,NULL); 
	    }
 }
}
