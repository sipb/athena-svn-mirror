/* brlmon.c
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
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include "brlmon.h"
#include "brlmonxmlapi.h"
#include "SRMessages.h"
#include "srintl.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define BUFFER_SIZE	256
#define MIN_LINE 	2
#define MIN_COLUMN 	1
#define MIN_PORT 	1024
#define MAX_PORT	30000

#define INIT_PORT	-1
#define NULL_VALUE	0

#ifdef INET6
    #define	FL_AF_INET AF_INET6
#else
    #define	FL_AF_INET AF_INET
#endif


/**
 *
 * States of UDP server state-machine.
 *
**/
typedef enum
{
    SC_IDLE,
    SC_TEXT,
    SC_QUIT
} Sc_state;



static void  brlmon_initialize_values (void);
static void  brlmon_free_values (void);
static gboolean brlmon_command_params (void);
static gboolean brlmon_create_server_udp (void);
static gboolean brlmon_close_server_udp (void);
static gboolean brlmon_sock_init_glib_poll (void);
static void brlmon_sock_terminate_glib_poll (void);

/**
 *
 * port - UDP port on the server is listen
 *
**/
static	gint		brlmon_port;
static	gint		brlmon_cmd_port;

/**
 *
 * sockfd - UDP socket file descriptor
 * clilen - length of client addres structure
 * rc     -
 * 
**/
static	gint   		brlmon_sockfd;
static  gint 		brlmon_clilen;
static  gint		brlmon_rc;

/**
 *
 * cli_addr - client address structure
 * serv_addr - srever address structure
 *
**/
#ifdef INET6
struct 	sockaddr_in6	brlmon_cli_addr;
struct 	sockaddr_in6	brlmon_serv_addr;
#else
struct 	sockaddr_in	brlmon_cli_addr;
struct 	sockaddr_in	brlmon_serv_addr;
#endif

/**
 *
 * Internal variales.
 *
**/
static 	gchar 		*brlmon_text;
static 	gchar		*brlmon_oldtext;
static 	gchar		*brlmon_currtext;

/**
 *
 * gioch - Socket fd channel listener.
 *
**/
static GIOChannel *brlmon_gioch;

/**
 *
 * UDP server state variable. (state machine)
 *
**/
static gint brlmon_state;

/**
 *
 *  modetype - Display Mode Type Normal|Braille|Dual [default normal]
 *
**/
extern gint brlmon_modetype;

/**
 *
 * line   - number of lines on display [min 2][default 2]
 * column - number of columns in line [min 1][default 40]
 *
**/
static gint brlmon_line;
static gint brlmon_column;

/**
 *
 *  cmd_mode - Display Mode Type Normal|Braille|Dual from command line
 *
**/
static gchar *brlmon_cmd_mode;

/**
 *
 *  cmd_port - UDP port number from command line
 *
**/
static gint brlmon_cmd_port;

extern GdkColor *brlmon_colors;

/**
 *
 * Main arg list.
 *
**/
struct poptOption poptopt[] = 
    {		
	{"line", 	'l', POPT_ARG_INT, &brlmon_line,  'l', "Number of line on display [line_no >= 2].", "line_no"},
	{"column", 	'c', POPT_ARG_INT, &brlmon_column,'c', "Number of column in line on display [column_no >= 1].", "column_no"},
	{"port", 	'p', POPT_ARG_INT, &brlmon_cmd_port,  'p', "UDP port number [1025 - 30000] ", "port_no"},
	{"mode", 	'm', POPT_ARG_STRING, &brlmon_cmd_mode,  'm', "Display Mode[normal|braille|dual] ", "modetype"},
	{NULL, 		0,0, NULL, 0}
    };


gint 
main (gint argc, gchar *argv[])
{
    gboolean rv;
    
#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, GNOPERNICUSLOCALEDIR);
    textdomain (GETTEXT_PACKAGE);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
#endif    
            
    brlmon_initialize_values ();

    gnome_program_init ("brlmonitor", VERSION,
			LIBGNOMEUI_MODULE, 
			argc, argv,
			GNOME_PARAM_POPT_TABLE, poptopt,
			GNOME_PARAM_HUMAN_READABLE_NAME, _("Braille monitor"),
			GNOME_PARAM_APP_DATADIR, DATADIR,
			NULL);
		    
    sru_log_init ();

    
    gtk_init (&argc, &argv);

    rv = brlmon_gconf_client_init ();
    sru_return_val_if_fail (rv, EXIT_FAILURE);

    brlmon_load_colors ();        
    
    rv = brlmon_command_params ();
    rv = rv & brlmon_xml_init ();
    rv = rv & brlmon_load_interface ();
    sru_return_val_if_fail (rv, EXIT_FAILURE);
    
    brlmon_create_text_area (brlmon_line, brlmon_column);
    brlmon_cursor_pos_clean ();
    brlmon_old_pos_clean ();

    rv = brlmon_create_server_udp ();
    sru_return_val_if_fail (rv, EXIT_FAILURE);    
    rv = brlmon_sock_init_glib_poll ();
    sru_return_val_if_fail (rv, EXIT_FAILURE);    

    gtk_main ();
    
    brlmon_xml_terminate ();
    
    rv = brlmon_close_server_udp ();
    sru_return_val_if_fail (rv, EXIT_FAILURE);    
    
    brlmon_free_values ();
    
    brlmon_sock_terminate_glib_poll ();
    
    sru_log_terminate ();
    
    return EXIT_SUCCESS;
}

/**
 * brlmon_initialize_values
 *
 * Initializes all global variable .
 *
 * return:
**/
void 
brlmon_initialize_values (void)
{
    brlmon_cmd_port 	= INIT_PORT;
    brlmon_cmd_mode 	= NULL;
    brlmon_column 	= -1;
    brlmon_line   	= -1;
    brlmon_oldtext	= NULL;
    brlmon_currtext   	= NULL;
    brlmon_modetype 	= MODE_NORMAL;
    
    putenv ("GTK_MODULES=");
    putenv ("GNOME_ACCESSIBILITY=0");    
}

/**
 * brlmon_sock_terminate_glib_poll
 * 
 * Close socket channel.
 *
 * return:
**/
void
brlmon_sock_terminate_glib_poll (void)
{
    if (brlmon_gioch) 
	g_io_channel_unref (brlmon_gioch);
}

/**
 * brlmon_free_values
 *
 * Free values which need this.
 *
 * return:
**/
void 
brlmon_free_values (void)
{
    g_free (brlmon_oldtext);
    g_free (brlmon_currtext);
    g_free (brlmon_cmd_mode);
    g_free (brlmon_colors);
}

/**
 * brlmon_command_params
 *
 * Interpret command line parameters
 *
 * return: FALSE at invalid argument.
**/
gboolean 
brlmon_command_params (void)
{    
    if (!brlmon_cmd_mode)
	brlmon_cmd_mode = brlmon_get_string_with_default (BRAILLE_MONITOR_MODE_GCONF_KEY, DEFAULT_MODE);    

    brlmon_modetype = brlmon_get_mode_type_from_string (brlmon_cmd_mode);
    
    g_free (brlmon_cmd_mode);
	
    if (brlmon_cmd_port != INIT_PORT && 
	brlmon_cmd_port > MIN_PORT && 
	brlmon_cmd_port <= MAX_PORT)
    {
	brlmon_port = brlmon_cmd_port;
    }
    else
    if (brlmon_cmd_port == INIT_PORT)
    {	    
	brlmon_port = 
	    brlmon_get_int_with_default (BRAILLE_MONITOR_PORT_GCONF_KEY, 
					 DEFAULT_PORT);    
    }
    else
    if (brlmon_cmd_port <= MIN_PORT || brlmon_cmd_port > MAX_PORT)
    {
        sru_warning (_("Invalid port number: %d."), brlmon_cmd_port);
        sru_warning (_("port_no = [1025 - 30000]."));

        return FALSE;
    }
	
    if (brlmon_port <= MIN_PORT || brlmon_port > MAX_PORT)
    {
        sru_warning (_("Invalid port number: %d."), brlmon_port);
	sru_warning (_("port_no = [1025 - 30000]."));
	
	return FALSE;
    }
        
    if (brlmon_line   < MIN_LINE)
	brlmon_line   = brlmon_get_int_with_default (BRAILLE_MONITOR_LINE_GCONF_KEY, 
						    DEFAULT_LINE);    
    if (brlmon_column < MIN_COLUMN)
	brlmon_column = brlmon_get_int_with_default (BRAILLE_MONITOR_COLUMN_GCONF_KEY, 
						    DEFAULT_COLUMN);    
                        	    	    
    return  TRUE;
}

/**
 * brlmon_sock_glib_cb
 *
 * Socket listener callbacks function and server state-machine
 *
 * @source: socket event source 
 * @condition: event how generat the event
 * @data: user data
 *
 * return: FALSE at discard event.
**/
static gboolean 
brlmon_sock_glib_cb (GIOChannel *source, 
		     GIOCondition condition, 
		     gpointer data)
{
    gint n,i;
    gchar buff[BUFFER_SIZE];
    
    memset (buff,0x0,sizeof(buff));

    brlmon_clilen = sizeof(brlmon_cli_addr);

    n = recvfrom (brlmon_sockfd, buff, sizeof(buff), 0, 
		 (struct sockaddr *) &brlmon_cli_addr, &brlmon_clilen);
	
    if (!n) 
	return FALSE;
    
    i = 0;
	
    while( i != BUFFER_SIZE - 1)
    {
	switch(brlmon_state)
	{
	    case SC_IDLE:
	    	if (buff[i] == 0x02)
	        {
	    	    if (brlmon_oldtext != NULL)
		    {
			g_free (brlmon_oldtext);
			brlmon_oldtext=NULL;
		    }
		    brlmon_state = SC_QUIT;
		    brlmon_cursor_pos_clean ();
		}
		break;
	    case SC_QUIT:
		if (buff[i] == 0x03)
		{
		    brlmon_state = SC_IDLE;
		    brlmon_cursor_pos_clean ();
		}
		else
		{
		    brlmon_oldtext = g_strdup_printf ("%c", buff[i]);
		    brlmon_state = SC_TEXT;
		    brlmon_cursor_pos_clean ();
		    brlmon_clean_panel ();
		}
		break;
	    case SC_TEXT:
		if (buff[i] == 0x03)
		{
		    brlmon_state = SC_IDLE;
		    g_free (brlmon_currtext);
		    brlmon_currtext = g_strdup (brlmon_oldtext);
		    sru_debug ("%s",brlmon_currtext);
    		    brlmon_cursor_pos_clean ();		
		    if (brlmon_oldtext != NULL)
		    {
			brlmon_xml_output ( brlmon_oldtext, 
					    strlen (brlmon_oldtext));
			g_free (brlmon_oldtext);
			brlmon_oldtext = NULL;
		    }
		}
		else
		if (strncmp (buff,"<QUIT>",5) == 0)	
		{
		    if (brlmon_oldtext != NULL) 
		    {
			g_free (brlmon_oldtext);
			brlmon_oldtext=NULL;
		    }
		    gtk_main_quit ();
		}
		else
		{
		    if (brlmon_oldtext == NULL) 
		    {
			brlmon_oldtext = g_strdup_printf ("%c", buff[i]);
		    }
		    else
		    {
			brlmon_text = g_strdup_printf ("%s%c", brlmon_oldtext, buff[i]);
			g_free (brlmon_oldtext);
			brlmon_oldtext = brlmon_text;
		    }
		}
		break;
	}
	
	i++;
    }
    
    return TRUE;
}

/**
 * brlmon_refresh_display
 *
 * @change_mode:
 *
 * Refresh the text on display.
 *
 * return:
**/
extern void 
brlmon_refresh_display (gboolean change_mode)
{
    if (brlmon_currtext != NULL)
	brlmon_xml_output (brlmon_currtext, strlen (brlmon_currtext));
}

/**
 * brlmon_sock_init_glib_poll
 *
 * add notify listener for socket channel
 *
 * return: TRUE is GOOD
 *
**/
gboolean
brlmon_sock_init_glib_poll (void)
{
    brlmon_gioch = g_io_channel_unix_new (brlmon_sockfd);
    
    if (!g_io_add_watch (brlmon_gioch, 
			G_IO_IN | G_IO_PRI, 
			brlmon_sock_glib_cb, NULL))
	return FALSE;
    
    return TRUE;	
}

/**
 * brlmon_close_server_udp
 *
 * Close server.
 *
 * return: TRUE is GOOD.
**/
gboolean
brlmon_close_server_udp (void)
{
    return !close (brlmon_sockfd);
}

/**
 * brlmon_create_server_udp
 *
 * Create UDP server.
 *
 * return: TRUE is GOOD.
**/
gboolean 
brlmon_create_server_udp (void)
{
    if ((brlmon_sockfd = socket (FL_AF_INET, SOCK_DGRAM, 0)) < NULL_VALUE) 
    {
    	sru_warning (_("Can not open socket!"));
	return FALSE;
    }

    /* bind local server port */
#ifdef INET6
    bzero ((gchar*) &brlmon_serv_addr, sizeof (struct sockaddr_in6));
    brlmon_serv_addr.sin6_family 	= AF_INET6;
    brlmon_serv_addr.sin6_addr 		= in6addr_any;
    brlmon_serv_addr.sin6_port 		= htons (brlmon_port);
#else
    bzero ((gchar*) &brlmon_serv_addr, sizeof (struct sockaddr_in));
    brlmon_serv_addr.sin_family 	= FL_AF_INET;
    brlmon_serv_addr.sin_addr.s_addr 	= htonl (INADDR_ANY);
    brlmon_serv_addr.sin_port 		= htons (brlmon_port);
#endif

    brlmon_rc = bind (brlmon_sockfd, 
		     (struct sockaddr *) &brlmon_serv_addr,
		     sizeof(brlmon_serv_addr));
  
    if (brlmon_rc < NULL_VALUE) 
    {
	sru_warning (_("Can not bind port number %d."), brlmon_port);
	return FALSE;
    }

    sru_message (_("Waiting for data on port UDP %u."), brlmon_port);

    return TRUE;
}
