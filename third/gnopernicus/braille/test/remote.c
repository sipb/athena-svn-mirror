/* remote.c
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
#include <glib.h>
#include <libxml/parser.h>
#include "brlxmlapi.h"
#include "brlinp.h"
#include "remoteinit.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <gnome.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "util.h"
#include "srintl.h"

#define __REMOTE_DEBUG__

typedef enum
{
    SC_IDLE,
    SC_TEXT,
    SC_QUIT
} ScState;

#define MAX_SOURCE_LENGTH 100

typedef struct
{
    gint eventtype;
    gchar source[MAX_SOURCE_LENGTH];
} BrlPackage;

gint 		brl_sock_init_glib_poll	();
gboolean 	brl_create_server_udp 	();

#define MIN_SERIAL_PORT 1
#define MAX_SERIAL_PORT 4

#define MIN_PORT	1024
#define MAX_PORT	30000
#define	SERV_UDP_PORT	7096
#ifdef INET6
    #define DEFAULT_IP	"::1"
#else
    #define DEFAULT_IP	"127.0.0.1"
#endif


#define BUFFER_SIZE		256
#define DEFAULT_DEVICE		"VARIO"
#define DEFAULT_SERIAL_PORT	1

/*Server start*/
static	gint   		sockfd;

static LincSockLen 	restricted_host_len;

#ifdef INET6
    struct sockaddr_in6	remote_host, 
			server_host;
    struct sockaddr_in6 *restricted_host;
    #define FL_AF_INET	AF_INET6
    #define IP_TYPE	"IPv6"
    #define sockaddrin	sockaddr_in6
#else
    struct sockaddr_in	remote_host, 
			server_host;
    struct sockaddr_in 	*restricted_host;
    #define FL_AF_INET	AF_INET
    #define IP_TYPE	"IPv4"
    #define sockaddrin	sockaddr_in
#endif


static 	gchar 		*text,*oldtext;
static 	gchar 		*hostip;
static	gint		hostport;
static	GIOChannel 	*giochh;
static 	gint 		mon_state;
static	gchar		*device; 	/* devault device */
static	gint   		serial_port; 	


/* GLOBALS */
GMainLoop* g_loop  = NULL;

struct poptOption poptopt[] = 
    {		
	{"device", 	'd', POPT_ARG_STRING, 	&device,  	'd', "Braille Device", 		"device_name"},
	{"serial", 	's', POPT_ARG_INT, 	&serial_port,	's', "Serial port (ttyS)", 	"serial_port_no"},
	{"port", 	'p', POPT_ARG_INT, 	&hostport,  	'p', "UDP port number", 	"port_no"},
	{"ip", 		'i', POPT_ARG_STRING, 	&hostip, 	'i', "Restricted host ip ", 	"ip"},
	{NULL, 		0,0, NULL, 0}
    };



void 
brl_input_event (BRLInEvent* brl_in_event)
{	
    BrlPackage package;
#ifdef INET6
    gchar restricted_host_addrbuf[INET6_ADDRSTRLEN];
    gchar remote_host_addrbuf[INET6_ADDRSTRLEN];
    gchar *ap = NULL;
#endif
    package.source[0]='\0';
    package.eventtype = brl_in_event->event_type;
	
    switch (brl_in_event->event_type)
    {
	/* NOTE: brl_in_event.event_data.XXX_codes can be NULL ! (check it!) */
	case BIET_KEY:
	    if (brl_in_event->event_data.key_codes)
	    {	
		g_strlcpy(package.source,brl_in_event->event_data.key_codes,100);
#ifdef __REMOTE_DEBUG__
		fprintf (stderr, "1KEY: %s\n", brl_in_event->event_data.key_codes);
#endif
		/* TEST - 6 keys pressed -> exit loop -> terminate app */
		if ((g_strcasecmp(brl_in_event->event_data.key_codes, "DK00DK01DK02DK03DK04DK05") == 0) ||
		    (g_strcasecmp(brl_in_event->event_data.key_codes, "FK00FK01FK02FK03FK04FK05") == 0))
		{
		    g_main_quit	(g_loop);
		}
	    }
	    else
	    {
#ifdef __REMOTE_DEBUG__
		fprintf (stderr, "ALL KEYS RELEASED\n");
#endif
	    }				
	break;
		
	case BIET_SENSOR:
	    if (brl_in_event->event_data.sensor_codes)
	    {
		g_strlcpy(package.source,brl_in_event->event_data.sensor_codes,100);
#ifdef __REMOTE_DEBUG__
		fprintf (stderr, "1SENSOR: %s\n", brl_in_event->event_data.sensor_codes);
#endif
	    }
	    else
	    {
#ifdef __REMOTE_DEBUG__
		fprintf (stderr, "1ALL SENSORS RELEASED\n");
#endif
	    }		
	break;
		
	case BIET_SWITCH:		
	    if (brl_in_event->event_data.switch_codes)
	    {
		g_strlcpy(package.source,brl_in_event->event_data.switch_codes,100);
#ifdef __REMOTE_DEBUG__
		fprintf (stderr, "1SWITCH: %s\n", brl_in_event->event_data.switch_codes);
#endif
	    }
	    else
	    {
#ifdef __REMOTE_DEBUG__
		fprintf (stderr, "1ALL SWITCHED ARE OFF\n");
#endif
	    }		
	break;
		
	default:
#ifdef __REMOTE_DEBUG__
	    fprintf (stderr, "UNKNOWN BRAILLE EVENT");
#endif
	break;
    }

#ifdef INET6	
    ap = (gchar*)inet_ntop(AF_INET6, (void*)&(restricted_host->sin6_addr),
		           restricted_host_addrbuf, sizeof(restricted_host_addrbuf));

    /* fprintf(stderr,"-%s\n",restricted_host_addrbuf); */
    ap = (gchar*)inet_ntop(AF_INET6,(void*)&(remote_host.sin6_addr),
		           remote_host_addrbuf, sizeof(remote_host_addrbuf));
    /* fprintf(stderr,"%s-%s\n",ap,remote_host_addrbuf); */
    if (!strcmp(restricted_host_addrbuf,remote_host_addrbuf) && ap)
#else
    if ((restricted_host->sin_addr.s_addr == remote_host.sin_addr.s_addr) && 
	(remote_host.sin_addr.s_addr != 0))
#endif
    {
	if (sendto(sockfd,&package,sizeof(package), 0,
		   (struct sockaddr *) &remote_host,
		    sizeof(remote_host)) != sizeof(remote_host))
	{
	    g_warning("Can not send a package");
        }
    }
}

/* Braille input callback */
void 
brl_xml_input_proc (gchar* buffer, 
                    gint len)
{	
    /* Braille input received as XML, give it to the parser */
    brl_in_xml_parse (buffer, len);			
}

static gboolean 
brl_sock_glib_cb (GIOChannel   *source, 
		  GIOCondition condition, 
		  gpointer data)
{
    gint n,i;
#ifdef INET6
    gchar restricted_host_addrbuf[INET6_ADDRSTRLEN];
    gchar remote_host_addrbuf[INET6_ADDRSTRLEN];
    gchar *ap = NULL;
#endif

    gchar buff[BUFFER_SIZE];
    gint  cli_len;
        
    cli_len = sizeof(remote_host);
    
    n = recvfrom (sockfd, buff, sizeof(buff), 0, 
		 (struct sockaddr *) &remote_host, &cli_len);
		 
    if (!n) 
	return FALSE;

#ifdef INET6
    ap = (gchar*)inet_ntop (AF_INET6,(void*)&(restricted_host->sin6_addr),
		            restricted_host_addrbuf, sizeof(restricted_host_addrbuf));
    /* fprintf(stderr,"%s-%s\n",ap,restricted_host_addrbuf);		    */
    ap = (gchar*)inet_ntop (AF_INET6,(void*)&(remote_host.sin6_addr),
		            remote_host_addrbuf, sizeof(remote_host_addrbuf));
    /* fprintf(stderr,"%s-%s\n",ap,remote_host_addrbuf);		    */
    if (strcmp (restricted_host_addrbuf,remote_host_addrbuf)) 
	return FALSE;
#else
    if (restricted_host->sin_addr.s_addr != remote_host.sin_addr.s_addr) 
	return FALSE;
#endif

    i=0;
    while( i != BUFFER_SIZE-1)
    {
	switch(mon_state)
	{
	    case SC_IDLE:
		if (buff[i] == 0x02)
		{
		    if (oldtext != NULL) 
		    {
			g_free(oldtext);
			oldtext=NULL;
		    }
		    mon_state = SC_QUIT;
		}
	    break;
	    
	    case SC_QUIT:
		if (buff[i] == 0x03)
		{
		    mon_state = SC_IDLE;
		}
		else
		{
		    oldtext = g_strdup_printf("%c",buff[i]);
		    mon_state = SC_TEXT;
		}
	    break;
	    
	    case SC_TEXT:
		if (buff[i] == 0x03)
		{
		    mon_state = SC_IDLE;
#ifdef __REMOTE_DEBUG__
		    g_message("%s",oldtext); 
#endif
		    if (oldtext)
		    {
			brl_xml_output(oldtext, strlen(oldtext));
			g_free(oldtext);
		    	oldtext=NULL;
		    }
		}
		else if (strncmp (buff,"<QUIT>",6) == 0)
		{
		    if (oldtext != NULL) 
		    {
			g_free(oldtext);
			oldtext = NULL;
		    }
		    g_main_quit(g_loop);
		}
		else
		{
		    if (oldtext == NULL) 
		    {
			oldtext = g_strdup_printf("%c",buff[i]);
		    }
		    else
		    {
			text=g_strdup_printf("%s%c",oldtext,buff[i]);
			g_free(oldtext);
			oldtext = text;
		    }
		}
	    break;
	}
	i++;
    }
    return TRUE;
}

gint
brl_sock_init_glib_poll ()
{
    giochh = g_io_channel_unix_new (sockfd);
    
    g_io_add_watch (giochh, G_IO_IN | G_IO_PRI, brl_sock_glib_cb, NULL);	
    
    return TRUE;	
}


#define MAX_NO 255
#define MIN_NO 0
#define MAX_DIGIT 3
static gboolean
brl_is_ipv4(const gchar *ip)
{
    gchar number[MAX_DIGIT + 1];
    gint len 	= strlen(ip);
    gint digit 	= 0;
    gint dots  	= 0;
    gint no    	= 0;
    gint last_digit = 0;
    
    while (len != 0)
    {
	if (*ip == '.')
	{
	    number[digit] = '\0';
	    
	    no = atoi(number);
	    
	    if (no < MIN_NO || no > MAX_NO) 
		return FALSE;
	    
	    digit = 0;
	    
	    if (++dots > MAX_DIGIT) 
		return FALSE;
	    
	    last_digit = 1;
	}
	else if (g_ascii_isdigit(*ip))
	{
	    number[digit] = *ip;
	    
	    if (++digit > MAX_DIGIT) 
		return FALSE;
	    
	    last_digit = 2;
	}
	else
	    return FALSE;
	
	len--;
	ip++;
    }
    
    if (last_digit != 2) 
	return FALSE;
    if (dots != MAX_DIGIT) 
	return FALSE;
    
    number[digit] = '\0';
    
    no = atoi(number);
    
    if (no < MIN_NO || no > MAX_NO) 
	return FALSE;
        
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
    gint len  = strlen(ip);
    gint ddot = 0;
    gboolean compres = FALSE;
    gint last_digit = 0;
    
    while (len != 0)
    {
	if (*ip == ':')
	{
	    if (ip - tmp == 1)
	    {
		if (compres) 
		    return FALSE;
		compres = TRUE;
	    }
	    tmp = (gchar*)ip;
	    adr = 0;
	    if (++ddot > MAX_DOUBLEDOT) 
		return FALSE;
	    last_digit = 1;
	}
	else
	if (*ip == '.') 
	{
	    if (ddot >= MAX_DOUBLEDOT)  
		return FALSE;
	    return brl_is_ipv4(tmp + 1);
	}
	else
	if (g_ascii_isxdigit(*ip))
	{
	    if (++adr > MAX_HEXA_DIGIT) 
		return FALSE;
	    last_digit = 2;
	}
	else
	    return FALSE;
	    
	len--;
	ip++;
    }
    
    if (last_digit != 2) 
	return FALSE;
    
    return TRUE;
}
#endif

static gboolean
brl_valid_ip_address(const gchar *ip)
{
#ifdef INET6
    return brl_is_ipv6(ip);
#else
    return brl_is_ipv4(ip);
#endif
}

gboolean 
brl_create_server_udp()
{
/*    LINCProtocolInfo *proto;*/
            
    if (!brl_valid_ip_address (hostip)) 
    {
	g_warning("Invalid IP (%s) address:%s",IP_TYPE,hostip);
	fprintf(stderr,"Use %s localhost address %s\n",IP_TYPE,DEFAULT_IP);
	free(hostip);
	hostip = g_strdup(DEFAULT_IP);
	brl_set_string(hostip,"IP");
    }

    fprintf(stderr,"Remote ip %s",hostip);

    restricted_host = (struct sockaddrin *) get_sockaddr (hostip,
							  g_strdup_printf ("%d",hostport), 
							  (LincSockLen*)&restricted_host_len);		        

    if (restricted_host == NULL) 
	return FALSE;    
                
    if ((sockfd = socket(FL_AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
	fprintf(stderr,"Can not open socket \n");
	return FALSE;
    }

#ifdef INET6
    server_host.sin6_family 	= FL_AF_INET;
    server_host.sin6_addr 	= in6addr_any;
    server_host.sin6_port 	= htons(hostport);
#else
    server_host.sin_family 	= FL_AF_INET;
    server_host.sin_addr.s_addr = htonl(INADDR_ANY);
    server_host.sin_port 	= htons(hostport);
#endif

     /* bind local server port */
    if (bind (sockfd, (struct sockaddr *) &server_host,sizeof(server_host)) < 0)
    {
	fprintf(stderr,"Can not bind host");
	return FALSE;
    }

    g_message("ServerUDP:Waiting for data on port UDP %u\n",hostport);    
    
    return TRUE;
}

/* Main */
gint 
main (gint argc, gchar** argv)
{
#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, GNOPERNICUSLOCALEDIR);
    textdomain (GETTEXT_PACKAGE);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
#endif    

    serial_port = -1;
	
    hostport = -1;
    	
    gnome_program_init ("Braille_remote", VERSION,
		        LIBGNOMEUI_MODULE, argc, argv,
		        GNOME_PARAM_POPT_TABLE, poptopt,
		        GNOME_PARAM_HUMAN_READABLE_NAME, _("Braille remote"),
		        NULL);

    if (!brl_gconf_client_init()) 
	return -1;
		
    printf ("Braille remote begin.\n");			
		
    if (device == NULL)
	device = g_strdup(DEFAULT_DEVICE);
	    
    if (serial_port < MIN_SERIAL_PORT || serial_port > MAX_SERIAL_PORT)
	serial_port = DEFAULT_SERIAL_PORT;
	    
    if (hostport < MIN_PORT || hostport > MAX_PORT)
    {
	hostport = brl_get_int_with_default("Port",SERV_UDP_PORT);
		
	if (hostport < MIN_PORT || hostport > MAX_PORT)
	{
	    hostport = SERV_UDP_PORT;
			
	    brl_set_int(hostport,"Port");
	}
    }
	
    if (hostip == NULL)
    {
	hostip = (gchar*)brl_get_string_with_default ("IP",DEFAULT_IP);	
    }	
		 		
    if (!brl_create_server_udp()) 
	return -1;
		
    brl_in_xml_init (brl_input_event);
		
    oldtext = NULL; 
    		
    if (brl_xml_init (device, serial_port, brl_xml_input_proc))
    {						
	brl_sock_init_glib_poll ();
			
 	g_loop = g_main_new (TRUE);
			
 	if (g_loop)
 	{
 	    printf ("Entering the GNOME loop...\n");
 	    g_main_run (g_loop);		
 	    printf ("GNOME loop exited.\n"); 	    	
 	    g_main_destroy (g_loop);
 	    g_loop = NULL;
 	}
 	else
 	{
 	    fprintf (stderr, "Error on creating GNOME loop\n");
	}
 	brl_xml_terminate (); 		 		 		
			
	close(sockfd);	
    }
		
    brl_in_xml_terminate (); 		
	
    g_free(hostip);

    printf ("Braille remote end.\n");	
	
    return 0;
}
