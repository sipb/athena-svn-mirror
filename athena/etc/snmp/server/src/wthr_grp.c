/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the Weather portion of the mib.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Tom Coppeto
 * MIT Network Operations
 * 24 January 1992
 *
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/wthr_grp.c,v $
 *    $Author: ghudson $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 *    Revision 1.1  1993/06/18 14:33:19  tom
 *    Initial revision
 *
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/wthr_grp.c,v 1.2 1997-02-27 06:48:04 ghudson Exp $";
#endif

#ifdef WEATHER
#include "include.h"
#include <sys/termios.h>

#include <mit-copyright.h>

#ifdef MIT

struct 
{
  unsigned long time;
  int temperature;
  int humidity;
  int pressure;
  int wspeed;
  int wdirection;
} wstat;

int tty = -1;

/*
 * Function:    lu_afs()
 * Description: Top level callback. Supports the following:
 *                  N_AFSCACHESIZE- (INT) afs cache size
 * Notes:       Because the cell names are so long, listings of cell names
 *              are indexed with integers instead of the cell name itself.
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_weather(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char *c;
  char *tstr;
  unsigned long t;

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /*
   * Build reply
   */

  memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */

  
  switch(varnode->offset)
    {
    case N_WRTIME:      
      t = time(0);
      tstr = ctime(&t);
       c   = rindex(tstr, '\n');
      *c  = '\0';
      return(make_str(&(repl->val), tstr));
    case N_WRLOCATION:      
      return(make_str(&(repl->val), weather_location));
    case N_WRTEMP:
      repl->val.type = INT;
      repl->val.value.intgr = get_weather() - 27300;
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_temp: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}


get_weather()
{
  unsigned char buf[8];
  fd_set ttyfdset;
  static struct timeval ttytout;
  int i, cnt, status, len;
  float dk = 0;

  if(init_port() < 0)
    return(BUILD_ERR);
  ttytout.tv_sec =  1;
  ttytout.tv_usec = 0;
  FD_ZERO(&ttyfdset);

  for (cnt = 0; cnt < 3; cnt++)   
    {
      /* flush input buffer */
      FD_SET(tty, &ttyfdset);
      if (select(FD_SETSIZE, &ttyfdset, NULL, NULL, &ttytout) > 0)
	read(tty, buf, 1);
      *buf = '\r';
      if (write(tty, buf, 1) != 1)
	syslog (LOG_ERR, "lu_temp: unable to send command");
      FD_SET(tty, &ttyfdset);

      for (i = 0; i < 8; i++)
	if((status = select(FD_SETSIZE, &ttyfdset, NULL, NULL, &ttytout) == 1))
	  {
	    read(tty, buf+i, 1);
	    buf[i] = buf[i] & 0x7f;
	    if (buf[i] == '\r') 
	      break;
	  }
	else
	  break;

      if ((i == 8) || (status < 1)) 
	continue;

      /* validate response */
      status = 0;
      len = i - 1;

      for (i = 0; i < len; i++)
	status = status ^ buf[i];
      if ((status + 63) != buf[len]) 
	continue;
      
      /* overrange (hard error)? */
      if ((buf[0] == '+') || (buf[0] == '-')) 
	break;      
      buf[len] = '\0';
  
      dk = -1;
      if (sscanf(buf, "%f", &dk) != 1)
	dk = -1;
      close(tty);
      return((int) dk * 100);
    }
  return(BUILD_ERR);
}
      
      
init_port()
{
  struct termios ttyargs;

  if((tty = open(weather_tty, O_RDWR)) < 0)
    {
      syslog(LOG_ERR, "lu_temp: can't open tty '%s'.\n", weather_tty);
      return(BUILD_ERR);
    }

  if(tcgetattr(tty, &ttyargs) < 0)
    {
      syslog(LOG_ERR, "lu_temp: cannot do tcgetattr on tty %s", weather_tty);
      return(BUILD_ERR);
    }

  ttyargs.c_iflag = 0;
  ttyargs.c_oflag = 0;
  ttyargs.c_cflag = (B9600 | CS8 | CREAD | CLOCAL);
  ttyargs.c_lflag = 0;

  if(tcsetattr(tty, TCSANOW, &ttyargs) < 0)
    {
      syslog(LOG_ERR, "lu_temp: can't do tcsetattr on tty: %s", weather_tty);
      close(tty);
      return(BUILD_ERR);
    }

  dtr(TRUE);
  return(BUILD_SUCCESS);
}


dtr(state)
  int state;
{
  int bits;

  if(state)
    {
      bits = (TIOCM_DTR | TIOCM_RTS);
      if ((ioctl(tty, TIOCMBIS, &bits)) < 0)
	syslog(LOG_ERR, 
	       "lu_temp: cannot set DTR and RTS -- ioctl TIOCMBIS failed.");
    }
  else
    {
      bits = (TIOCM_DTR | TIOCM_RTS);
      if ((ioctl(tty, TIOCMBIC, &bits)) < 0)
	syslog(LOG_ERR, 
	       "lu_temp: cannot  clear DTR and RTS -- ioctl TIOCMBIS failed.");
    }
}

#endif MIT
#endif WEATHER
