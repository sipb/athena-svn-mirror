/* sercomm.c
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
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>

#include <glib.h>

#include "sercomm.h"

#ifndef TTYNAME0
#define TTYNAME0 "/dev/ttys0"
#endif 

#ifndef TTYNAME1
#define TTYNAME1 "/dev/ttys1"
#endif 

/* Globals */
static gint 		fd_serial; 	       /* file descriptor for the serial port */
static BRLSerCallback	client_callback = NULL; /* lient callback */

GIOChannel 		*gioch = NULL;						/* GLIB io channel */
gboolean		 glib_poll = TRUE;

/* Functions */
gint 
brl_ser_open_port (gint port)
{
	
    gchar *pname; /* port name */

    switch (port)
    {
	case 1:
	    pname = TTYNAME0; 	
	break;
    
	case 2:
	    pname = TTYNAME1;
	break;
	
	default:
	    fprintf (stderr, "\nbrl_open_port: Invalid serial port number %d", port);
	    return 0;
	break;
    }

    fd_serial = open(pname, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);		
                   	
    if (fd_serial == -1)
    {
	fprintf (stderr, "\nbrl_open_port: Unable to open the serial port (%s).", pname);
	return 0;
    }

    fcntl (fd_serial, F_SETFL, 0);
	
    return 1;
}

/* for HandyTech devices */
gint 
handy_set_comm_param()
{
    struct termios options;

    /* get the current options from the port */
    tcgetattr (fd_serial, &options);

    /* set the baud rate to 19200 */
    cfsetispeed (&options, B19200);
    cfsetospeed (&options, B19200);
    
    /* set parity odd */
    options.c_cflag |= PARENB;							
    options.c_cflag |= PARODD;
    options.c_iflag &= ~(INPCK | ISTRIP);
	
    /* set character size */
    options.c_cflag &= ~CSIZE;	
    options.c_cflag |= CS8;		

    /* set 1 stop bits */
    options.c_cflag &= ~CSTOPB;
	
    /* disable flow controls */
    options.c_cflag &= ~CRTSCTS;	                /* disable CTS/RTS flow control	 */
    options.c_iflag &= ~(IXON | IXOFF | IXANY);	/* disable XON/XOFF flow control */

    /* choose raw input */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

#if defined(OLCUC)
	options.c_oflag &= ~OLCUC;
#endif
    options.c_oflag &= ~ONLCR;
    options.c_oflag &= ~OCRNL;

    /* enable the receiver and set local mode*/
    options.c_cflag |= (CLOCAL | CREAD);

    /* timeouts */
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;

    /* set the options for the port */
    tcsetattr (fd_serial, TCSANOW, &options);

    return 1;
}

gint 
brl_ser_set_comm_param (glong  baud_rate, 
			gchar  parity, 
			gshort stop_bits, 
			gchar  flow_ctrl)
{
    struct termios options;

    /* get the current options from the port */
    tcgetattr (fd_serial, &options);

    /* set the baud rate */
    switch (baud_rate)
    {
	case 9600:
	    cfsetispeed (&options, B9600);		
	    cfsetospeed (&options, B9600);		
	break;

	case 19200:
	    cfsetispeed (&options, B19200);
	    cfsetospeed (&options, B19200);
	break;

	case 38400:
	    cfsetispeed (&options, B38400);		
	    cfsetospeed (&options, B38400);		
	break;
#ifdef B57600
	case 57600:
	    cfsetispeed (&options, B57600);		
	    cfsetospeed (&options, B57600);		
	break;
#endif
#ifdef B115200
	case 115200:
	    cfsetispeed (&options, B115200);		
	    cfsetospeed (&options, B115200);		
	break;
#endif
	default:
	    return 0;		
	break;	    		
    }

    /* set parity */
    switch (parity)
    {
	case 'N':
	case 'n':
	default:		                  /* none */
	    options.c_cflag &= ~PARENB;	          /* disable parity */
	    options.c_iflag &=  ~INPCK;
	break;

	case 'E': 
	case 'e':	                          /* even parity */
	    options.c_cflag |= PARENB;		  /* enable parity */
	    options.c_cflag &= ~PARODD;	          /* even */
	    options.c_iflag |=  (INPCK | ISTRIP); /* enable checking and stripping of the parityy bit */
	break;

	case 'O': 
	case 'o':	                          /* odd parity */
	    options.c_cflag &= ~PARENB;		  /* mask parity bits						 */
	    options.c_cflag |= PARODD;		  /* odd */
	    options.c_iflag |=  (INPCK | ISTRIP); /* enable checking and stripping of the parityy bit */
	break;
    }
	
    /* set character size */
    options.c_cflag &= ~CSIZE;	                  /* mask character size bits */
    options.c_cflag |= CS8;			  /* set the character size to 8 bit */


    /* set stop bits */
    if (stop_bits < 2)
    {
	options.c_cflag &= ~CSTOPB;	          /* 1 stop bit */
    }
    else
    {
	options.c_cflag |= CSTOPB;	          /* 2 stop bit	!!! TBR !!! */
    }
	
    /* set flow control */
    switch (flow_ctrl)
    {
	case 'N': 
	case 'n': 
	default:
#ifdef CRTSCTS
	    options.c_cflag &= ~CRTSCTS;	 /* disable CTS/RTS flow control	 */
#endif
	    options.c_iflag &= ~(IXON | IXOFF | IXANY);	/* disable XON/XOFF flow control */
	break;
    
	case 'H': 
	case 'h':	/* hardware flow control */
#ifdef CRTSCTS
    	    options.c_cflag |= CRTSCTS;	/* enable CTS/RTS flow control */
#endif
	    options.c_iflag &= ~(IXON | IXOFF | IXANY);	/* disable XON/XOFF flow control */
	break;
    
	case 'S': case 's':	/* software flow control */
#ifdef CRTSCTS
	    options.c_cflag &= ~CRTSCTS;	/* disable CTS/RTS flow control	 */
#endif	
	    options.c_iflag |= IXON | IXOFF | IXANY;	/* enable XON/XOFF flow control */
	break;
    }

    /* choose raw input */
    options.c_lflag &= ~(ICANON |	/* disable canonical input */
			 ECHO |		/* disable echoing of input chars */
			 ECHOE |   	/* disable echoing of erase characters as BS-SP-BS */
			 ISIG);		/* disable the SIGINTR, SIGSUSP, SIGDSUSP and SIGQUIT signals */

	/* choose raw output */
    options.c_oflag &= ~OPOST;

    /* enable the receiver and set local mode*/
    options.c_cflag |= (CLOCAL | CREAD);

    /* set the options for the port */
    tcsetattr (fd_serial, TCSANOW, &options);

    return 1;
}


gint 
brl_ser_close_port ()
{
    close (fd_serial);
    return 1;
}

gint 
brl_ser_send_data (gchar  *data, 
		   gint   data_size, 
		   gshort blocking)
{
    gint n;

#if defined(OLCUC)
    n = write (fd_serial, data, data_size);
#else
    gchar tmp[data_size];
    memcpy(tmp, data, data_size);
    for (n = 0; n < data_size; n++)
	if (islower(tmp[n]))
	    tmp[n] = toupper(tmp[n]);
    n = write (fd_serial, tmp, data_size);
#endif
    if (blocking) tcdrain(fd_serial);	
	
    if (n < 0)
    {
   	fprintf (stderr, "bra_ser_send_data: failed sending data\n");		
	return 0;
    }
    return 1;
}

gint 
brl_ser_read_data (gchar *data_buff, 
		   gint  max_len)
{
    fcntl (fd_serial, F_SETFL, O_NONBLOCK);
    return read (fd_serial, data_buff, max_len);
}

void 
brl_ser_sig_alarm (gint sig)
{
    gint n, i;
    guchar data[256];
    n = brl_ser_read_data (&data[0], 256);
	
    /* printf ("brl_ser_sig_alarm\n"); */
	
    for (i = 0; i < n; ++i)
    {
	/* fprintf (stderr, "%c", data[i]);   !!! TEST !!! */
	/* dispatch bytes to client callback if callback available */
	if (client_callback)
	{
	    while (client_callback(data[i]));
	}
    }
}

gint 
brl_ser_start_timer (glong interval)
{
    struct sigaction sa = {{brl_ser_sig_alarm}};
    struct itimerval iv = {{0, 10000}, {0, 10000}}; /* default 10 ms periodic */

    if (interval)
    {
	iv.it_value.tv_sec = 0;
	iv.it_value.tv_usec = interval * 1000;
	iv.it_interval.tv_sec = 0;
	iv.it_interval.tv_usec = interval * 1000;
    }

    sigaction (SIGALRM, &sa, NULL);
    setitimer (ITIMER_REAL, &iv, NULL);

    return 1;	/* !!! TBR !!! should depend on sigaction and setitimer returning success */
}


gint 
brl_ser_stop_timer (glong interval)
{
    struct itimerval iv = {{0, 0}, {0, 0}}; /* default 10 ms periodic	 */
	
    sigaction (SIGALRM, NULL, NULL);	/* disconnect callback !!! TBR !!! OK? */
    setitimer (ITIMER_REAL, &iv, NULL);  /* disable the timer after its next expiration */
	
    return 1;
}

void
 brl_ser_set_callback (BRLSerCallback callback)
{
    client_callback = callback;	
}

/* GLIB input polling */
static gboolean 
brl_ser_glib_cb (GIOChannel   *source, 
		 GIOCondition condition, 
		 gpointer     unused)
{			
    gint n, i;
    guchar data[256];
	
    if (!glib_poll) 
	return FALSE;
	
    /* !!! TBR !!! - read until no more data available */
    n = brl_ser_read_data (&data[0], 256);	/* !!! TBR !!! g_io_channel_read ??? */
	
    for (i = 0; i < n; ++i)
    {
	/* fprintf (stderr, "%c", data[i]);		 !!! TEST !!! */
	/* dispatch bytes to client callback if callback available */
	if (client_callback)
	{
	    while (client_callback(data[i]));
	}
    }	
    return TRUE;	
}

gint
brl_ser_init_glib_poll ()
{
    /* printf ("brl_ser_init_glib_poll\n"); */
    glib_poll = TRUE;
    gioch = g_io_channel_unix_new (fd_serial);
    g_io_add_watch (gioch, G_IO_IN | G_IO_PRI, brl_ser_glib_cb, NULL);
	
    return TRUE;
}

gint 
brl_ser_exit_glib_poll ()
{
    glib_poll = FALSE;
    if (gioch) 
	g_io_channel_unref (gioch);
    return 0;
}
