/*  
 *     Campus Cluster Monitering Program
 *       John Welch and Chris Vanharen
 *            MIT's Project Athena
 *                Summer 1990
 */

/*
 *    check.c contacts the cview daemon to retreive cluster info and 
 *    then displays it in the main window.
 */

#include "xcluster.h"		/* global include file. */

#include <sys/socket.h>		/* socket definitions */
#include <netinet/in.h>		/* random network-type defines */
#include <netdb.h>		/* random network-type defines */
#include <hesiod.h>		/* hesiod lookup functions */
#include "net.h"

#include <Jets.h>
#include <Drawing.h>


/*
 * DEFINES
 */
#define ERRMSG "Unable to find any info on any machines."
#ifndef MAX
#define MAX(a,b)  ( ((a) > (b)) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN(a,b)  ( ((a) < (b)) ? (a) : (b) )
#endif

static char *intsonly[] = {"intsonlyplease3"};
extern char machtypes[][25];
extern int num_machtypes;
static char *headers[] = {"Cluster", "free / total"};
static char *totals = "Totals";
static char *printers = "Printers";
static char *status = "Status";
static char *jobs = "Jobs";
static char *cluster_hdr = "Cluster";
static char *phone_hdr = "Phone";

int xspace = 0;
int height = 0;

struct cluster *find_text(a, b)
     int a, b;
{
  int y_init, y;
  int x = 20;
  int i;
  struct cluster *c;

  y_init = 20 + height+4;
  y = 20 + 2*height+4;

  for (c = cluster_list; c != NULL; c = c->next)
    {
      for(i = 0; strcmp(c->cluster_names[i], "XXXXX"); i++)
	{
	  if (b < y  &&  b > y_init &&
	      a > x  &&  a < x + (num_machtypes+2)*xspace)
	    return c;
	  y += height;
	}
    }
  return c;
}


/*
 *  check_cluster prints out the information for the specified cluster.
 */
int check_cluster(me, foo, data)
     DrawingJet me;
     int foo;
     caddr_t data;
{
  char buf[BUF_SIZE];		/* temporary storage buffer. */
  char *ptr;			/* pointer to string. */
  int x = 20, y = 20;		/* location to begin printing. */
  int ht;			/* height of area used for phone numbers */
  int len, len2;		/* Lengths of buffers (pixels). */
  int i, j;			/* Counters */
  static long time_cached = 0;	/* time info was cached. */
  int s;			/* Socket to connect to cview daemon. */
  FILE *f;			/* stream for reading data from cviewd. */
  struct cluster *c;
  static struct cluster *old_c = NULL;
  char name[15], stat[15], number[5];
  static int init = 0, plen = 0, slen = 0, jlen =0;
  static struct cluster total;
  static int err = 0;

  if (!init)
    {
      init = 1;

      strcpy(total.cluster_names[0], totals);

      height = me->drawing.font->max_bounds.ascent
	+ me->drawing.font->max_bounds.descent;

      plen = XTextWidth(me->drawing.font, printers, strlen(printers));
      slen = XTextWidth(me->drawing.font, status, strlen(status)) + 10;
      jlen = XTextWidth(me->drawing.font, jobs, strlen(jobs));


      for (j = 0; j <= num_machtypes + 1; j++)
	{
	  len = MAX(XTextWidth(me->drawing.font, headers[j ? 1 : 0],
			       strlen(headers[j ? 1 : 0])),
		    XTextWidth(me->drawing.font, machtypes[j],
			       strlen(machtypes[j])));
	  if (len > xspace)
	    xspace = len;
	}


      for (c = cluster_list; c != NULL; c = c->next)
	{
	  for(i = 0; ptr=c->cluster_names[i], strcmp(ptr, "XXXXX"); i++)
	    {  
	      len = XTextWidth(me->drawing.font, ptr, strlen(ptr));
	      if (len > xspace)
		xspace = len;
	    }
	  for(i=0; ptr=c->prntr_name[i], strcmp(ptr, "XXXXX"); i++)
	    {
	      len = XTextWidth(me->drawing.font, ptr, strlen(ptr));
	      if (len > plen)
		plen = len;
	    }
	}

      xspace += 5;
      plen += 5;
    }


  if (time_cached + 60 < time(0)) /* cache info for 60 seconds */
    {
      time_cached = time(0);
      s = net(progname, 1, intsonly);

      if (s < 1)
	{
	  XClearArea(XjDisplay(me), XjWindow(me), x, y, 0, 0, 0);
	  ptr = "Error while contacting server.";
	  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
		      x, y, ptr, strlen(ptr));
	  XFlush(XjDisplay(me));
	  err = 1;
	  return 0;
	}

      f = fdopen(s, "r");

      for (j=0; j < (num_machtypes+1)*2; j++)
	total.cluster_info[0][j] = 0;

      while(fscanf(f, "%s", buf) != EOF)
	{
	  if (!strcmp(buf, "cluster"))
	    {
	      (void) fscanf(f, "%s", buf);
	      for (c = cluster_list; c != NULL; c = c->next)
		{
		  for(i = 0; strcmp(c->cluster_names[i], "XXXXX"); i++)
		    if (!strcmp(buf, c->cluster_names[i]))
		      {
			int free, inuse;
			int tot_free=0, tot_inuse=0;

			for (j=0; j < num_machtypes; j++)
			  {
			    fscanf(f, "%d %d", &free, &inuse);
			    c->cluster_info[i][j*2] = free;
			    c->cluster_info[i][j*2+1] = inuse;
			    total.cluster_info[0][j*2] += free;
			    total.cluster_info[0][j*2+1] += inuse;
			    tot_free += free;
			    tot_inuse += inuse;
			  }
			c->cluster_info[i][j*2] = tot_free;
			c->cluster_info[i][j*2+1] = tot_inuse;
			total.cluster_info[0][j*2] += tot_free;
			total.cluster_info[0][j*2+1] += tot_inuse;
		      }
		}
	    }

	  if (!strcmp(buf, "printer"))
	    {
	      (void) fscanf(f, "%s %s %s", name, stat, number);
	      for (c = cluster_list; c != NULL; c = c->next)
		{
		  for(i=0;
		      ptr=c->prntr_name[i], strcmp(ptr, "XXXXX");
		      i++)
		    {
		      if (!strcmp(name, ptr))
			{
			  strcpy(c->prntr_current_status[i], stat);
			  strcpy(c->prntr_num_jobs[i], number);
			}
		    }
		}
	    }
	}

      (void) fclose(f);
      XClearArea(XjDisplay(me), XjWindow(me), x+xspace, y+height+3, 0, 0, 0);
    }


  /*
   *  display the data
   */

  if (err)
    {
      XClearArea(XjDisplay(me), XjWindow(me), x, y, 0, 0, 0);
      err = 0;
    }

  for (j = 0; j <= num_machtypes + 1; j++)
    {
      len = XTextWidth(me->drawing.font, headers[j ? 1 : 0],
		       strlen(headers[j ? 1 : 0]))
	- XTextWidth(me->drawing.font, machtypes[j], strlen(machtypes[j]));
      XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc, 
		  x + xspace*j + len/2, y, machtypes[j],
		  strlen(machtypes[j]));
      XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
		  x + xspace*j, y + height, headers[j ? 1 : 0],
		  strlen(headers[j ? 1 : 0]));
    }

  XDrawLine(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	    x, y+height+2, x + (num_machtypes+2)*xspace, y+height+2);
  y += 2*height + 2;

  /*
   * Check for multiple clusters in the same area
   */

  len = XTextWidth(me->drawing.font, "free ", 5);

  for (c = cluster_list; c != NULL; c = c->next)
    {
      for(i = 0; strcmp(c->cluster_names[i], "XXXXX"); i++)
	{  
	  GC gc;

	  if (c == Current)
	    {
	      XFillRectangle(XjDisplay(me), XjWindow(me),
			     me->drawing.foreground_gc,
		x, y+2-height, (num_machtypes+2)*xspace, height);
	      gc = me->drawing.background_gc;
	    }
	  else
	    gc = me->drawing.foreground_gc;

	  if (old_c != Current  &&  c == old_c)
	    {
	      XFillRectangle(XjDisplay(me), XjWindow(me),
			     me->drawing.background_gc,
		x, y+2-height, (num_machtypes+2)*xspace, height);
	    }

	  strcpy(buf, c->cluster_names[i]);
	  XDrawString(XjDisplay(me), XjWindow(me), gc,
		      x, y, buf, strlen(buf));

	  for (j = 0; j < num_machtypes+1; j++)
	    {
	      if (c->cluster_info[i][2*j] == 0
		  && c->cluster_info[i][2*j+1] == 0)
		{
		  XDrawString(XjDisplay(me), XjWindow(me), gc,
			      x+xspace*(j+1) + len, y, "-", 1);
		}
	      else
		{
		  sprintf(buf, "%d ",
			  c->cluster_info[i][2*j]);
		  len2 = XTextWidth(me->drawing.font, buf, strlen(buf));
		  sprintf(buf, "%d / %d",
			  c->cluster_info[i][2*j],
			  c->cluster_info[i][2*j] + c->cluster_info[i][2*j+1]);
		  XDrawString(XjDisplay(me), XjWindow(me), gc, 
			      x+xspace*(j+1) + len - len2,
			      y, buf, strlen(buf));
		}
	    }
	  
	  y += height;
	  XFlush(XjDisplay(me));
	}
    }

  /*
   *   print out totals...
   */

  strcpy(buf, total.cluster_names[0]);
  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	      x, y, buf, strlen(buf));

  for (j = 0; j < num_machtypes+1; j++)
    {
      if (total.cluster_info[0][2*j] == 0
	  && total.cluster_info[0][2*j+1] == 0)
	{
	  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
		      x+xspace*(j+1) + len, y, "-", 1);
	}
      else
	{
	  sprintf(buf, "%d ",
		  total.cluster_info[0][2*j]);
	  len2 = XTextWidth(me->drawing.font, buf, strlen(buf));
	  sprintf(buf, "%d / %d",
		  total.cluster_info[0][2*j],
		  total.cluster_info[0][2*j] + total.cluster_info[0][2*j+1]);
	  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc, 
		      x+xspace*(j+1) + len - len2,
		      y, buf, strlen(buf));
	}
    }

  /*
   *  print out phone number(s) for the specified cluster.
   */

  x = me->core.width - plen - slen - jlen - 20;
  y -= (2 + 3*height);
  ht = y - height;

  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	      x, y, cluster_hdr, strlen(cluster_hdr));
  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	      x+MAX(xspace,plen), y, phone_hdr, strlen(phone_hdr));
  y += 2;
  XDrawLine(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	    x, y, x+plen+slen+jlen, y);

  if (old_c != Current)
    XClearArea(XjDisplay(me), XjWindow(me), x, y+1, 0, 0, 0);

  for (c = cluster_list; c != NULL; c = c->next)
    {
      for(i = 0; strcmp(c->cluster_names[i], "XXXXX"); i++)
	{  
	  if (c == Current)
	    {
	      y += height;
	      strcpy(buf, c->cluster_names[i]);
	      XDrawString(XjDisplay(me), XjWindow(me),
			  me->drawing.foreground_gc,
			  x, y, buf, strlen(buf));
	      strcpy(buf, c->phone_num[i]);
	      XDrawString(XjDisplay(me), XjWindow(me),
			  me->drawing.foreground_gc,
			  x+MAX(xspace,plen), y, buf, strlen(buf));
	    }
	}
    }

  /*
   *  print out printer information for the specified cluster.
   */

  x = me->core.width - plen - slen - jlen - 20;
  y = 20 + height;

  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	      x, y, printers, strlen(printers));
  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	      x+plen, y, status, strlen(status));
  XDrawString(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	      x+plen+slen, y, jobs, strlen(jobs));
  y += 2;
  XDrawLine(XjDisplay(me), XjWindow(me), me->drawing.foreground_gc,
	    x, y, x+plen+slen+jlen, y);

  if (old_c != Current)
    XClearArea(XjDisplay(me), XjWindow(me), x, y+1, 0, ht - (y+1), 0);

  if (Current != NULL  &&  strcmp(Current->prntr_name[0], "XXXXX"))
    {
      for(i=0;
	  ptr=Current->prntr_name[i], strcmp(ptr, "XXXXX");
	  i++)
	{
	  /* display printer info */

	  y += height;
	  XDrawString(XjDisplay(me), XjWindow(me),
		      me->drawing.foreground_gc,
		      x, y, ptr, strlen(ptr));
	  ptr = Current->prntr_current_status[i];
	  XDrawString(XjDisplay(me), XjWindow(me),
		      me->drawing.foreground_gc,
		      x+plen+5, y, ptr, strlen(ptr));
	  ptr = Current->prntr_num_jobs[i];
	  XDrawString(XjDisplay(me), XjWindow(me),
		      me->drawing.foreground_gc,
		      x+plen+slen+5, y, ptr, strlen(ptr));
	}
    }



  old_c = Current;

  return 0;
}
