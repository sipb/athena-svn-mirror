/* srbrl.c
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
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <signal.h>

#include <glib.h>
#include "libsrconf.h"
#include "srbrl.h"
#include "brlinp.h"
#include <string.h>
#include <unistd.h>
#include "../braille/libbrl/brlxmlapi.h"
#include "srpres.h"
/*#include "sockaddr.h"*/
#include "util.h"

#define MIN_UDP_PORT 		1024
#define MAX_UDP_PORT 		30000


#define REMOTE_CONFIG_PATH 	"config/remote"
#define BRLMON_CONFIG_PATH 	"config/brlmon"


#undef _BRAILLE_DEBUG_

typedef struct
{
    gchar	 *device;
    gchar 	 *style;
    gchar 	 *cursor_style;	
    gchar 	 *translation_table;
    
    gint  	 port;
    gint  	 position_sensor;
    gint  	 optical_sensor;
    gint  	 offset;

}SRCBraille;

static SRCBraille *src_braille = NULL;

extern gboolean src_use_braille;
extern gboolean src_use_braille_monitor;
extern gchar* scroll;
extern void brl_input_event (BRLInEvent *event);
extern gchar *fmt_for_xml (gchar *, gchar *, gchar *);

gint 	brlmon_sd, 	/* BrlMonitor socket descriptor*/
	remote_sd;	/* RemoteBraille socket descriptor*/
gint 	src_brlmon_pid = -1;

/* socket structures */
#ifdef INET6
    static struct sockaddr_in6	client_addr, 
				*remote_brlmon_addr, 
				*remote_braille_addr, 
				rclient_addr;
    #define sockaddrin 	sockaddr_in6
    #define FL_AF_INET 	AF_INET6
    #define IP_TYPE	"IPv6"
#else
    static struct sockaddr_in 	client_addr, 
				*remote_brlmon_addr, 
				*remote_braille_addr, 
				rclient_addr;
    #define sockaddrin 	sockaddr_in
    #define FL_AF_INET 	AF_INET
    #define IP_TYPE	"IPv4"
#endif
GIOChannel 			*udp_giochh;

/* state flag of remote modules */
static gboolean remotesockstate = FALSE;
static gboolean brlmonsockstate = FALSE;

/* length of used ip addresses */
static LincSockLen remote_brlmon_addr_len;
static LincSockLen remote_braille_addr_len;


static gboolean 
udp_sock_glib_cb(GIOChannel *source, 
		GIOCondition condition, 
		gpointer data)
{	
    BRLInEvent event;
    BrlPackage package;
    gint clilen;
    gint n;

/*    if (!remotesockstate) return FALSE; */
    
    clilen = sizeof(rclient_addr);
    
    n = recvfrom(remote_sd, &package,sizeof(package), 0, 
		 (struct sockaddr *) &rclient_addr, &clilen);
    
    if (n < 1) return FALSE;
        
    event.event_type = package.eventtype;

    switch (event.event_type)
    {
	case BIET_KEY:	
	    (event.event_data).key_codes = g_strdup(package.source);
	    break;

	case BIET_SENSOR:
	    (event.event_data).switch_codes = g_strdup(package.source);
	    break;
	
	case BIET_SWITCH:
	    (event.event_data).sensor_codes = g_strdup(package.source);
	    break;

	default:
	    break;
    }
    
    brl_input_event(&event);
        
    return TRUE;
}

#define MAX_NO 255
#define MIN_NO 0
#define MAX_DIGIT 3

static gboolean
brl_is_ipv4(const gchar *ip)
{
    gchar number[MAX_DIGIT + 1];
    gint len;
    gint digit 	= 0;
    gint dots  	= 0;
    gint no    	= 0;
    
    if (!ip) return FALSE;
    
    len = strlen (ip);
    
    while (len != 0)
    {
	if (*ip == '.')
	{
	    number[digit] = '\0';
	    
	    no = atoi(number);
	    
	    if (no < MIN_NO || no > MAX_NO) return FALSE;
	    
	    digit = 0;
	    
	    if (++dots > MAX_DIGIT) return FALSE;
	}
	else
	if (g_ascii_isdigit(*ip))
	{
	    number[digit] = *ip;
	    
	    if (++digit > MAX_DIGIT) return FALSE;
	}
	else
	    return FALSE;
	
	len--;
	ip++;
    }
    
    if (dots != MAX_DIGIT) return FALSE;
    
    number[digit] = '\0';
    
    no = atoi (number);
    
    if (no < MIN_NO || no > MAX_NO) return FALSE;
        
    return TRUE;
}

#ifdef INET6
#define MAX_DOUBLEDOT 	7
#define MAX_HEXA_DIGIT 	4

static gboolean
brl_is_ipv6(const gchar *ip)
{
    gchar *tmp;
    gint adr  = 0;
    gint len;
    gint ddot = 0;
    gboolean compres = FALSE;
    
    if (!ip) return FALSE;
    
    len = strlen (ip);
    
    while (len != 0)
    {
	if (*ip == ':')
	{
	    if (ip - tmp == 1)
	    {
		if (compres) return FALSE;
		compres = TRUE;
	    }
	    tmp = (gchar*)ip;
	    adr = 0;
	    if (++ddot > MAX_DOUBLEDOT) return FALSE;
	}
	else
	if (*ip == '.') 
	{
	    if (ddot >= MAX_DOUBLEDOT)  return FALSE;
	    return brl_is_ipv4 (tmp + 1);
	}
	else
	if (g_ascii_isxdigit(*ip))
	{
	    if (++adr > MAX_HEXA_DIGIT) return FALSE;
	}
	else
	    return FALSE;
	    
	len--;
	ip++;
    }
    
    return TRUE;
}
#endif

static gboolean
brl_valid_ip_address(const gchar *ip)
{
#ifdef INET6
    return brl_is_ipv6 (ip);
#else
    return brl_is_ipv4 (ip);
#endif
}

static gboolean
brl_remote_udp_init()
{
    gint defport 	= DEFAULT_REMOTE_PORT;
    gint brlport;
    gchar *brlip 	= NULL;

    if (!srconf_get_data_with_default ("port",CFGT_INT,(gpointer)&brlport,(gpointer)&defport,REMOTE_CONFIG_PATH)) {
	    g_warning ("Can not get UDP Port number for Remote from ~/.gconf/apps/gnopernicus/config/remote/gconf.xml config file");
	    remotesockstate = FALSE; 
	    return FALSE; 	    
	}

    if (!srconf_get_data_with_default ("ip",CFGT_STRING,(gpointer)&brlip,(gpointer)DEFAULT_REMOTE_IP,REMOTE_CONFIG_PATH)) {
	    g_warning ("Can not get IP addres for Remote from ~/.gconf/apps/gnopernicus/config/remote/gconf.xml config file");
	    remotesockstate = FALSE; 
	    return FALSE; 
	}
	    
    if (brlport	< MIN_UDP_PORT || brlport > MAX_UDP_PORT) {
	    g_warning ("Invalid UDP port number:%d",brlport);
	    brlport = DEFAULT_REMOTE_PORT;
	}


    if (!brl_valid_ip_address (brlip)) {
	    g_warning ("Invalid IP (%s) address:%s",IP_TYPE,brlip);
	    fprintf (stderr,"Use %s localhost address %s\n",IP_TYPE,DEFAULT_REMOTE_IP);
	    free (brlip);
	    brlip = g_strdup (DEFAULT_REMOTE_IP);
	}
	
    remote_braille_addr = (struct sockaddrin *) get_sockaddr (
		brlip,
		g_strdup_printf ("%d",brlport), 
		(LincSockLen*)&remote_braille_addr_len);    

    
    if ((remote_sd = socket (FL_AF_INET, SOCK_DGRAM, 0)) < 0) {
	    g_warning ("Can not open socket");
	    
	    remotesockstate = FALSE;
	    
	    g_free (brlip);	    
	    brlip = NULL;
	    
	    return FALSE;
	}
 
    bzero ((gchar*) &rclient_addr, sizeof (struct sockaddrin));
#ifdef INET6
    rclient_addr.sin6_family 	= FL_AF_INET;
    rclient_addr.sin6_addr	= in6addr_any;
    rclient_addr.sin6_port 	= htons (0);
#else
    rclient_addr.sin_family 	= FL_AF_INET;
    rclient_addr.sin_addr.s_addr= htonl (INADDR_ANY);
    rclient_addr.sin_port 	= htons (0);
#endif

    if (bind (remote_sd, (struct sockaddr *) &rclient_addr, sizeof (rclient_addr)) < 0) {
	    g_warning ("Can not bind Remote host.");
	    
	    remotesockstate = FALSE;
	    
	    g_free (brlip);
	    brlip = NULL;
	    
	    return FALSE;
	}
    
    remotesockstate = TRUE;

    fprintf (stderr,"\n** Remote Braille Device initialization succeded for %s host"
			" on port %d.", brlip, brlport);
    fprintf (stderr, "\n   To see something you must have remote server started.");
    fprintf (stderr, "\n   To change one or more settings use gconf-editor tool."
			    "\n     Changes must be maded in .../apps/gnopernicus/config/Remote.\n");

    g_free(brlip);
    brlip = NULL;

    udp_giochh = g_io_channel_unix_new (remote_sd);
    
    g_io_add_watch (udp_giochh, G_IO_IN | G_IO_PRI, udp_sock_glib_cb, NULL);	
    
    return TRUE;
}

static gboolean 
brl_mon_udp_init()
{
    gint monport;
    gint defport = DEFAULT_BRLMON_PORT;
    gchar *monip = NULL;

    if (!srconf_get_data_with_default ("port",CFGT_INT,(gpointer)&monport,(gpointer)&defport,BRLMON_CONFIG_PATH)){
	    g_warning("Can not get UDP Port number for Braille monitor from ~/.gconf/apps/gnopernicus/config/brlmon/gconf.xml config file%d",monport);
	    brlmonsockstate = FALSE; 
	    return FALSE;
	}
    
    if (!srconf_get_data_with_default ("ip",CFGT_STRING,(gpointer)&monip,(gpointer)DEFAULT_BRLMON_IP,BRLMON_CONFIG_PATH)) {
	    g_warning ("Can not get IP addres for Braille monitor from ~/.gconf/apps/gnopernicus/config/brlmon/gconf.xml config file");
	    brlmonsockstate = FALSE; 
	    return FALSE;
	}
	    
    if (monport	< MIN_UDP_PORT || monport > MAX_UDP_PORT) {
	    g_warning ("Invalid UDP port number:%d",monport);
	    monport = DEFAULT_BRLMON_PORT;
	}

    if (!brl_valid_ip_address (monip)) {
	    g_warning ("Invalid IP (%s) address:%s",IP_TYPE,monip);
	    fprintf (stderr,"Use %s localhost address %s\n",IP_TYPE,DEFAULT_BRLMON_IP);
    	    free (monip);
	    monip = g_strdup (DEFAULT_BRLMON_IP);
	}
	
    remote_brlmon_addr = (struct sockaddrin *) get_sockaddr (
		monip,
		g_strdup_printf ("%d",monport), 
		(LincSockLen*)&remote_brlmon_addr_len);		        

    if ((brlmon_sd = socket (FL_AF_INET, SOCK_DGRAM, 0)) < 0) {
	    g_warning ("Can not open socket");
	    
	    brlmonsockstate = FALSE;
	    
	    g_free (monip);
	    monip = NULL;
	    
	    return FALSE;
	}
	    
    bzero ((gchar*) &client_addr, sizeof (struct sockaddrin));
#ifdef INET6
    client_addr.sin6_family 	= FL_AF_INET;
    client_addr.sin6_addr 	= in6addr_any;
    client_addr.sin6_port 	= htons (0);
#else
    client_addr.sin_family 	= FL_AF_INET;
    client_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    client_addr.sin_port 	= htons (0);
#endif

    if (bind (brlmon_sd, (struct sockaddr *) &client_addr, sizeof (client_addr)) < 0) { 
	    g_warning ("Can not bind Braille Monitor host.");
	    
	    brlmonsockstate = FALSE;
	    
	    g_free (monip);
	    monip = NULL;
	    
	    return FALSE;
	}
    
    fprintf (stderr,"\n** Braille Monitor initialization succeded for %s host"
			" on port %d.", monip,monport);
    fprintf (stderr, "\n   To see something you must have Braille Monitor"
			" server started.");
    fprintf (stderr, "\n   To change one or more settings use gconf-editor tool."
			    "\n     Changes must be maded in .../apps/gnopernicus/config/BrlMon.\n");
    
    g_free(monip);
    
    monip = NULL;
    
    brlmonsockstate = TRUE;
    
    return TRUE;
}

void 
src_brlmon_terminate()
{
    if (brlmonsockstate == TRUE) 
    {
	src_brlmon_quit ();
	close (brlmon_sd);
	brlmonsockstate = FALSE;
    }
}

static void 
brl_remote_udp_terminate()
{
    if (remotesockstate == TRUE) {
	close (remote_sd);
	remotesockstate = FALSE;
	}
}


static void 
brl_send_package(gchar *sendtmp,gint len)
{
    gint rc = 0;
    if (brlmonsockstate)
    {
	rc = sendto (brlmon_sd, sendtmp, len, 0,
		    (struct sockaddr *) remote_brlmon_addr,
		    remote_brlmon_addr_len);
	if ( rc == -1)	g_warning ("Can not send data to Braille Monitor");
    }
    if (remotesockstate)
    {
	rc = sendto (remote_sd, sendtmp, len, 0,
		    (struct sockaddr *) remote_braille_addr,
		    remote_braille_addr_len);
	if ( rc == -1)	g_warning ("Can not send data to Remote Braille");
    }
}

static void 
src_brl_chop_output(gchar *buff)
{
    gint len 		= strlen (buff);
    gint no_of_pack 	= len / (BUFFER_SIZE - 1);
    gint rest 		= len % (BUFFER_SIZE - 1);
    gchar *tmp 		= buff;
    gchar sendtmp[BUFFER_SIZE];
    gint iter;
    
    if (rest != 0) no_of_pack++;
        
    memset(sendtmp, 0x0, sizeof (sendtmp));
    
    sendtmp[0] = 0x02;
    
    brl_send_package(sendtmp, sizeof (sendtmp));
    
    for(iter = 0; iter < no_of_pack; ++iter)
    {
	memset (sendtmp, 0x0, sizeof (sendtmp));
		
	strncpy (sendtmp, tmp, BUFFER_SIZE - 1);
			
	tmp = tmp + (BUFFER_SIZE - 1);
		
	brl_send_package (sendtmp, sizeof (sendtmp));
    }
	
    memset (sendtmp, 0x0, sizeof (sendtmp));
    
    sendtmp[0] = 0x03;
    
    brl_send_package (sendtmp, sizeof (sendtmp));
}


void 
src_brlmon_quit()
{
    src_brlmon_send("<QUIT>");
}

void 
src_brlmon_send (gchar *monoutput)
{
    if (monoutput)
    {
	if (g_utf8_validate (monoutput, -1, NULL))
	    src_brl_chop_output (monoutput);
	else
	    sru_warning ("brlmon output: invalid UTF-8 received");
    }
}

void 
src_brlmon_show (gchar *message)
{
    gchar *brlout = NULL;
    gchar *text 	= NULL;
    gchar offset[5];
	
    text = src_xml_format ("TEXT", NULL, message);
    sprintf (offset, "%d", src_braille->offset);
    brlout = g_strconcat (
		"<BRLOUT language=\"", src_braille->translation_table,"\" bstyle=\"", src_braille->style ,"\" clear=\"true\">\n",
		/* main display */
		"<BRLDISP role=\"main\" offset=\"", offset, "\" clear=\"true\">\n",
		text,
		NULL);
    if (scroll && scroll[0])
    {
	brlout = g_strconcat (brlout,
	                      "<SCROLL mode=\"", scroll, "\" /SCROLL>\n",
		              "</BRLDISP>\n",
		              "</BRLOUT>\n",
		              NULL);
    }
    else
    {		
	brlout = g_strconcat (brlout,
		              "</BRLDISP>\n",
		              "</BRLOUT>\n", 
    		              NULL);
    }		    
    		
    src_brlmon_send  (brlout);
    g_free (brlout);						
    g_free(text);
}



void 
src_braille_send (gchar *brloutput)
{
    if (brloutput)
    {
    	if (g_utf8_validate (brloutput, -1, NULL))
	    brl_xml_output(brloutput, strlen(brloutput));
	else
	    sru_warning ("braille output: invalid UTF-8 received");
#ifdef _BRAILLE_DEBUG_
	fprintf (stderr, "\n%s", brloutput); 
#endif
    }
}

void 
src_braille_show (gchar *message)
{
    gchar *brlout = NULL;
    gchar *text 	= NULL;
    gchar offset[5];
	
    text = src_xml_format ("TEXT", NULL, message);
    sprintf (offset, "%d", src_braille->offset);	
    brlout = g_strconcat (
		"<BRLOUT language=\"",src_braille->translation_table,"\" bstyle=\"", src_braille->style,"\" clear=\"true\">\n",
		/* main display */
		"<BRLDISP role=\"main\" offset=\"", offset, "\" clear=\"true\">\n",
		text,
		"</BRLDISP>\n",
		/* nothing to do on state display here*/
		"</BRLOUT>\n", 
		NULL);
	
    src_braille_send (brlout);
    g_free (brlout);						
    g_free(text);
}

gboolean
src_braille_translation_table_exist (const gchar *table)
{
    gchar *fn = NULL;
    gboolean rv = FALSE;			
    
    /* derive the TT file name from the language name */
    fn = g_strdup_printf ("%s.a2b", table);
    /* test the file */
    if (g_file_test (fn, G_FILE_TEST_EXISTS)) 
    {
	rv = TRUE;
    }
    else
    {
	gchar *dir_fn = NULL;
	dir_fn = g_strconcat (BRAILLE_TRANS_TABLES_DIR,fn,NULL);
	if (g_file_test (dir_fn, G_FILE_TEST_EXISTS)) 
	    rv = TRUE;
	g_free (dir_fn);
    }
    g_free (fn);
    
    return rv;
}


gboolean
src_braille_restart ()
{
    if (src_use_braille)
    {
	gchar *device = g_strdup (src_braille->device);
	gint port = src_braille->port;
	/* terminate Braille */
	src_braille_terminate ();
    	/* and then start it again */
	src_braille->device = device;
	src_braille->port = port;	
	src_use_braille = src_braille_init ();
    }
    if (src_use_braille_monitor)
    {
    }
    return src_use_braille;
}

extern void brl_xml_input_proc (gchar *, gint);

static void
src_braille_exited (gint signal)
{
    if (signal == SIGTERM ||
	signal == SIGQUIT ||
	signal == SIGINT  ||
	signal == SIGKILL ||
	signal == SIGSEGV)
    {
	kill (src_brlmon_pid, SIGTERM);
	exit (1);
    }
}

gboolean 
brl_mon_run_process (void)
{
    signal (SIGTERM, src_braille_exited);
    signal (SIGQUIT, src_braille_exited);
    signal (SIGINT, src_braille_exited);
    signal (SIGKILL, src_braille_exited);
    signal (SIGSEGV, src_braille_exited);

    if (g_file_test ("../brlmon/brlmonitor", G_FILE_TEST_EXISTS) &&
        g_file_test ("../brlmon/brlmonitor", G_FILE_TEST_IS_EXECUTABLE) &&
        g_file_test ("../brlmon/brlmonitor", G_FILE_TEST_IS_REGULAR))
    {
	gchar *args[] = {"../brlmon/brlmonitor", NULL};
	if (!g_spawn_async ( ".", args , NULL , 
			    G_SPAWN_DO_NOT_REAP_CHILD,
			    NULL, NULL, &src_brlmon_pid, NULL))
	{
	    sru_message ("No \"%s\" binary file found.", 
			    "brlmonitor");
	    return FALSE;
	}
    }
    else
    if (g_file_test ("./brlmonitor", G_FILE_TEST_EXISTS) &&
        g_file_test ("./brlmonitor", G_FILE_TEST_IS_EXECUTABLE) &&
        g_file_test ("./brlmonitor", G_FILE_TEST_IS_REGULAR))
    {
	gchar *args[] = {"./brlmonitor", NULL};
	if (!g_spawn_async ( ".", args , NULL , 
			    G_SPAWN_DO_NOT_REAP_CHILD,
			    NULL, NULL, &src_brlmon_pid, NULL))
	{
	    sru_message ("No \"%s\" binary file found.", 
			    "brlmonitor");
	    return FALSE;
	}	
    }
    else
    {
	gchar *args[] = {BRLMON_DIR"brlmonitor", NULL};
	if (!g_spawn_async ( ".", args , NULL , 
			    G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
			    NULL, NULL, &src_brlmon_pid, NULL))
	{
	    sru_message ("No \"%s\" binary file found.",
			    "brlmonitor");
	    return FALSE;
	}
    }
    return TRUE;
}

gboolean
src_braille_monitor_init ()
{
    brl_mon_udp_init ();
    brl_remote_udp_init();
    
    if (!remotesockstate && !brlmonsockstate)
	{
	    fprintf (stderr, "**Can't initialize any module.\n");
	    srconf_unset_key (SRCORE_BRAILLE_MONITOR_SENSITIVE, SRCORE_PATH);
	    SET_SRCORE_CONFIG_DATA (SRCORE_BRAILLE_MONITOR_SENSITIVE, CFGT_BOOL, &brlmonsockstate);
	}
    if (!brl_mon_run_process ())
	{
	    fprintf (stderr, "**Can't initialize any module.\n");
/*	    srconf_unset_key (SRCORE_BRAILLE_MONITOR_SENSITIVE, SRCORE_PATH);
	    SET_SRCORE_CONFIG_DATA (SRCORE_BRAILLE_MONITOR_SENSITIVE, CFGT_BOOL, &brlmons);
*/
	}
    return TRUE;
}

static SRCBraille*
src_braille_setting_new ()
{
    SRCBraille *braille = NULL;
    
    braille = g_new0 (SRCBraille, 1);
    
    return braille;
}

static void
src_braille_load_values (SRCBraille *braille)
{
    sru_assert (braille);
    
    if (!braille->device)
    {
	gchar *default_device = DEFAULT_BRAILLE_DEVICE;
	GET_BRAILLE_CONFIG_DATA_WITH_DEFAULT (BRAILLE_DEVICE, CFGT_STRING, 
					      &braille->device, 
					      default_device);
    }
        
    if (MIN_BRAILLE_PORT > braille->port || 
	MAX_BRAILLE_PORT < braille->port)
    {
	gint default_port = DEFAULT_BRAILLE_PORT_NO;
	GET_BRAILLE_CONFIG_DATA_WITH_DEFAULT (BRAILLE_PORT_NO, CFGT_INT, 
					      &braille->port, 
					      &default_port);
    
	if (MIN_BRAILLE_PORT > braille->port || 
	    MAX_BRAILLE_PORT < braille->port) 
	{
    	    braille->port = DEFAULT_BRAILLE_PORT_NO;
    	    SET_BRAILLE_CONFIG_DATA (BRAILLE_PORT_NO, CFGT_INT, &braille->port);
	}					  
    }    
    
    gchar *default_braille_transaltion_table = DEFAULT_BRAILLE_TRANSLATION;
    GET_BRAILLE_CONFIG_DATA_WITH_DEFAULT (BRAILLE_TRANSLATION, CFGT_STRING, 
					  &braille->translation_table, 
					  default_braille_transaltion_table);		
    if (!src_braille_translation_table_exist (braille->translation_table))
    {
        srconf_set_config_data (BRAILLE_TRANSLATION, CFGT_STRING, 
				DEFAULT_BRAILLE_TRANSLATION, 
				CFGM_BRAILLE);
    }
	
    
    gchar *default_braille_style = DEFAULT_BRAILLE_STYLE;
    GET_BRAILLE_CONFIG_DATA_WITH_DEFAULT (BRAILLE_STYLE, CFGT_STRING, 
					  &braille->style, 
					  default_braille_style);
					  
    gchar *default_braille_cursor_style = DEFAULT_BRAILLE_CURSOR_STYLE;
    GET_BRAILLE_CONFIG_DATA_WITH_DEFAULT (BRAILLE_CURSOR_STYLE, CFGT_STRING, 
					  &braille->cursor_style, 
					  default_braille_cursor_style);
					  
    gint default_pos_sensor = DEFAULT_BRAILLE_POSITION_SENSOR;
    GET_BRAILLE_CONFIG_DATA_WITH_DEFAULT (BRAILLE_POSITION_SENSOR, CFGT_INT, 
					  &braille->position_sensor, 
					  &default_pos_sensor);
    
    gint default_opt_sensor = DEFAULT_BRAILLE_OPTICAL_SENSOR;
    GET_BRAILLE_CONFIG_DATA_WITH_DEFAULT (BRAILLE_OPTICAL_SENSOR, CFGT_INT, 
					  &braille->optical_sensor, 
					  &default_opt_sensor);
	
}

void
src_braille_get_defaults ()
{
    src_braille = src_braille_setting_new ();
    src_braille_load_values (src_braille);
}

gboolean 
src_braille_init ()
{
    gboolean brlval = FALSE;
    brl_in_xml_init (brl_input_event);
    
    sru_assert (src_braille && src_braille->device);

    if (!brl_xml_init(src_braille->device, src_braille->port, brl_xml_input_proc)) /* take Braille display from Gnopi... */
    {
	brlval = FALSE;
	fprintf(stderr, "\n** BRAILLE initialization failed for %s on port %d",
					src_braille->device, src_braille->port);
	fprintf (stderr, "\n   To change one or more settings use gnopernicus UI\n");
    }
    else
    {
	brlval = TRUE;
	fprintf (stderr, "\n** BRAILLE initialization succeded for %s device"
				    " on port %d.", src_braille->device, src_braille->port);
	fprintf (stderr, "\n   To see something you must have a Braille device"
				    " connected to your computer.");
	fprintf (stderr, "\n   To change one or more settings use gnopernicus UI.\n");
	
    }
    
    srconf_unset_key (SRCORE_BRAILLE_SENSITIVE, SRCORE_PATH);
    SET_SRCORE_CONFIG_DATA (SRCORE_BRAILLE_SENSITIVE, CFGT_BOOL, &brlval);
    
    return brlval;
}

void
src_braille_monitor_terminate ()
{
    src_brlmon_terminate ();
    brl_remote_udp_terminate();
}

void
src_braille_terminate ()
{
    brl_xml_terminate ();
    brl_in_xml_terminate ();
	
    g_free (src_braille->device);
    src_braille->port = -1;
}

static SRCBraille* 
src_braille_get_global_settings ()
{
    sru_assert (src_braille);
    return src_braille;
}

gboolean
src_braille_set_device (gchar *device)
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    sru_assert (device && braille && braille->device);
    
    if (strcmp (device, braille->device) != 0)
    {
	g_free (braille->device);
	braille->device = g_strdup (device);
	src_use_braille = src_braille_restart ();
	return TRUE ;
    }
    return FALSE;
}

gboolean
src_braille_set_port_no (gint port)
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    if (braille->port != port)
    {
	braille->port = port;
	src_use_braille = src_braille_restart ();
	return TRUE;
    }    
    return FALSE;
}

gboolean
src_braille_set_style (gchar *style)
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    sru_assert (style && braille->style);
    
    if (strcmp (style, braille->style) != 0)
    {
	g_free (braille->style);
	braille->style = g_strdup (style);
	
	return TRUE;
    }
    return FALSE;
}

gboolean
src_braille_set_cursor_style (gchar *cursor_style)
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    sru_assert (cursor_style && braille->cursor_style);
    
    if (strcmp (cursor_style, braille->cursor_style) != 0)
    {
	g_free (braille->cursor_style);
	braille->cursor_style = g_strdup (cursor_style);
	
	return TRUE;
    }
    return FALSE;
}

gboolean
src_braille_set_translation_table (gchar *translation_table)
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    sru_assert (translation_table && braille->translation_table);
    if (strcmp (translation_table, braille->translation_table) != 0)
    {
	if (src_braille_translation_table_exist (translation_table))
	{
	    g_free (braille->translation_table);
	    braille->translation_table = g_strdup (translation_table);
	
	    src_use_braille = src_braille_restart ();
	}
	else
	{
	    gchar *value = DEFAULT_BRAILLE_TRANSLATION;
	    srconf_set_config_data (BRAILLE_TRANSLATION,
			            CFGT_STRING,
			            value,
			    	    CFGM_BRAILLE);
	}
    }
    return FALSE; 
}

gboolean
src_braille_set_position_sensor (gint position_sensor)
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    if (position_sensor != braille->position_sensor)
    {
	braille->position_sensor = position_sensor;
    }

    return TRUE;
}

gboolean
src_braille_set_optical_sensor (gint optical_sensor)
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    if (optical_sensor != braille->optical_sensor)
    {
	braille->optical_sensor = optical_sensor;
    }

    return TRUE;
}

gboolean
src_braille_set_offset (gint offset)
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    if (offset != braille->offset)
    {
	braille->offset = offset;
    }

    return TRUE;
}

gint
src_braille_get_offset ()
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    return braille->offset;
}

gint
src_braille_get_optical_sensor ()
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    return braille->optical_sensor;
}

gint
src_braille_get_position_sensor ()
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    return braille->position_sensor;
}

gint
src_braille_get_port_no ()
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille);
    
    return braille->port;
}

gchar*
src_braille_get_translation_table ()
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille && braille->translation_table);
    
    return braille->translation_table;
}

gchar*
src_braille_get_cursor_style ()
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille && braille->cursor_style);
    
    return braille->cursor_style;
}

gchar*
src_braille_get_style ()
{
    SRCBraille *braille;
    
    braille = src_braille_get_global_settings ();
    sru_assert (braille && braille->style);
    
    return braille->style;
}

gchar*
src_braille_get_device ()
{
    SRCBraille *braille;
        
    braille = src_braille_get_global_settings ();
    sru_assert (braille && braille->device);
    
    return braille->device;
}
