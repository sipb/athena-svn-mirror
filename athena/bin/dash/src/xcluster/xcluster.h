/*  
 *     Campus Cluster Monitering Program
 *       John Welch and Chris Vanharen
 *            MIT's Project Athena
 *                Summer 1990
 */

/*
 *   This is the global include file.
 */

#include <stdio.h>		/* standard io routines */
#include <strings.h>		/* strcpy, strcat, etc... */
#include <X11/Xlib.h>		/* basic X includes */
#include <X11/Xutil.h>		/* basic X includes */
#include <Button.h>

#define BUF_SIZE	1024

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Cluster Structure */

struct cluster
{
  int cluster_number;		/* For internal use... */
  char button_name[25];		/* Name to be shown on the button. */
  ButtonJet btn;		/* Jet for the button. */
  char cluster_names[5][25];	/* Names of clusters in group, as in cmon. */
  char phone_num[5][15];	/* phone number of cluster. */
  int cluster_info[5][20];	/* Machine info for clusters in group. */
  int x_coord;			/* Location on map. */
  int y_coord;			/* ditto. */
  char prntr_name[10][15];	/* Names of printers in cluster group. */
  char prntr_current_status[10][15]; /* Stati of said printers. */
  char prntr_num_jobs[10][5];	/* Number of waiting jobs on said printers. */
  struct cluster *next;		/* Pointer to next cluster */
};


/*
 * GLOBAL VARIABLES
 */
extern char *progname;		/* Slightly obvious . . . */
extern struct cluster *cluster_list;	/* Pointer to list of clusters */
extern Display *display;	/* Pointer to display structure. */
extern int screen;		/* Screen number. */
extern Window window;		/* Window we're creating. */
extern struct cluster *Current;	/* Current cluster being displayed. */
