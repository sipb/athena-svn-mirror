#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/pages.c,v 1.1 1993-10-12 05:34:45 probe Exp $";
#endif lint

/*	This is the file pages.c for the Xmore, a file browsing utility
 *      built upon Xlib and the XToolkit.
 *	It Contains: InitPage() and PrintPage().
 *	
 *	Created: 	November 10, 1987
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/pages.c,v $
 *      $Author: probe $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/pages.c,v 1.1 1993-10-12 05:34:45 probe Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "globals.h"
#include "mit-copyright.h"

#define ADD_MORE_MEM 100
#define CHAR_PER_LINE 40

/*	Function Name: InitPage
 *	Description: This function reads a file and sets it up for 
 *                   display in the scrollbyline widget.
 *	Arguments: widget - the scrollled widget we are initializing.
 *                 file - a file pointer to the file that we are opening.
 *	Returns: none.
 */

void
InitPage(widget,file)
Widget widget;
FILE * file;
{
  char *page,*top_of_page;	/* a pointer to the manpage, 
				   stored in dynamic memory. */
  char **line_pointer,**top_line; /* pointers to beginnings of the 
				     lines of the file. */
  int nlines;			/* the current number of allocated lines. */
  Arg arglist[5];		/* the arglist. */
  Cardinal num_args;		/* the number of arguments. */
  struct stat fileinfo;		/* file information from fstat. */
  MemoryStruct * memory_struct;	/* the memory information to pass to the 
				   next file. */
  
  memory_struct = global_memory_struct;

  if ( memory_struct->top_line != NULL) {
    free(memory_struct->top_line);
    free(memory_struct->top_of_page);
  }

/*
 * Get file size and allocate a chunk of memory for the file to be 
 * copied into.
 */

  if (fstat(fileno(file), &fileinfo)) {
    printf("Failure in fstat\n");
    exit(1);
  }

  page = (char *) malloc(fileinfo.st_size + 1);	/* leave space for the NULL */
  top_of_page = page;

/* 
 * Allocate a space for a list of pointer to the beginning of each line.
 */

  nlines = fileinfo.st_size/CHAR_PER_LINE;
  line_pointer = (char**) malloc( nlines * sizeof(char *) );
  top_line = line_pointer;

  *line_pointer++ = page;

/*
 * Copy the file into memory. 
 */

  fread(page,sizeof(char),fileinfo.st_size,file); 

/* put NULL at end of buffer. */

  *(page + fileinfo.st_size) = '\0';

/*
 * Go through the file setting a line pointer to the character after each 
 * new line.  If we run out of line pointer space then realloc that space
 * with space for more lines.
 */

  while (*page != '\0') {

    if ( *page == '\n' ) {
      *line_pointer++ = page + 1;

      if (line_pointer >= top_line + nlines) {
	top_line = (char **) realloc( top_line, 
			      (nlines + ADD_MORE_MEM) * sizeof(char *) );
	line_pointer = top_line + nlines;
	nlines += ADD_MORE_MEM;
      }
    }
    page++;
  }
   
/*
 *  Realloc the line pointer space to take only the minimum amount of memory
 */

  nlines = line_pointer - top_line - 1;
  top_line = (char **) realloc(top_line,nlines * sizeof(char *));

/*
 * Store the memory pointers into a structure that will be returned with
 * the widget callback.
 */

  memory_struct->top_line = top_line;
  memory_struct->top_of_page = top_of_page;

  num_args = 0;
  XtSetArg(arglist[num_args], XtNlines, nlines);
  num_args++;
    
  XtSetValues(widget,arglist,num_args);
}


/*	Function Name: PrintPage
 *	Description: This function prints a man page in a ScrollByLine widget.
 *	Arguments: w - the ScrollByLine widget.
 *                 structure_pointer - a pointer to the ManpageStruct.
 *                 data - a pointer to the call data.
 *	Returns: none.
 */

void
PrintPage(w,struct_pointer,data)
Widget w;
caddr_t struct_pointer,data;
{
  MemoryStruct * info;
  ScrollByLineStruct * scroll_info;
  register char *bufp;
  char **line;
  int current_line;
  char buf[BUFSIZ],*c;
  int col,height,width,cur_pos;
  int italicflag = 0;
  int y_loc;			/* vertical postion of text on screen. */
  GC normal_gc,italic_gc;
  Display * disp;
  Window window;
  Pixel fg;			/* The foreground color. */
  Arg arglist[1];		/* An argument list. */
  Cardinal num_args;		/* The number of arguments. */

  scroll_info = (ScrollByLineStruct *) data;
  info = (MemoryStruct *) struct_pointer;

  current_line = scroll_info->start_line;

/* Use get values to get the foreground colors from the widget. */

  num_args = 0;
  XtSetArg(arglist[num_args], XtNforeground, &fg);
  num_args++;
  XtGetValues(w,arglist,num_args);

  line = scroll_info->start_line + info->top_line;
  c = *line;

  disp = XtDisplay(w);
  window = XtWindow(XtScrollByLineWidget(w));

  /* find out how tall the font is. */

  height = (fonts.normal->max_bounds.ascent + 
		   fonts.normal->max_bounds.descent); 

  normal_gc = XCreateGC(disp,window,0,NULL);
  XSetForeground(disp,normal_gc,(unsigned long) fg);
  XSetFont(disp,normal_gc,fonts.normal->fid);

  italic_gc = XCreateGC(disp,window,0,NULL);
  XSetForeground(disp,italic_gc,(unsigned long) fg);
  XSetFont(disp,italic_gc,fonts.italic->fid);

  /*
   * Ok, here's the more than mildly heuristic page formatter.
   * We put chars into buf until either a font change or newline
   * occurs (at which time we flush it to the screen.)
   */

/*
 * Because XDrawString uses the bottom of the text as a position
 * reference, add one font height to the ScollByLine position reference.
 */

  y_loc = scroll_info->location + height;

  for(buf[0] = '\0',bufp = buf,col=INDENT;;) {

    switch(*c) {

    case '\0':		      /* If we reach the end of the file then return */
      XFreeGC(disp,normal_gc);
      XFreeGC(disp,italic_gc);
      return;

    case '\n':
      *bufp = '\0';
      if (*bufp != buf[0]) {
	if(italicflag)		/* print the line as italic. */
	  XDrawString(disp,window,italic_gc,col,y_loc,buf,strlen(buf));
	else {
	  XDrawString(disp,window,normal_gc,col,y_loc,buf,
		      strlen(buf));
	}
      }
      if (current_line++ == scroll_info->start_line +
	                    scroll_info->num_lines ) {
	XFreeGC(disp,normal_gc);
	XFreeGC(disp,italic_gc);
	return;
      }
      col = INDENT;		
      bufp = buf;
      *bufp = '\0';
      italicflag = 0;
      y_loc += height;
      break;

    case '\t':			/* TAB */
      *bufp = '\0';

      if (italicflag)
	XDrawString(disp,window,italic_gc,col,y_loc,buf,strlen(buf));
      else
	XDrawString(disp,window,normal_gc,col,y_loc,buf,strlen(buf));
      bufp = buf; 
      italicflag = 0;
      cur_pos = XTextWidth(fonts.normal,buf,strlen(buf)) + col;
      width =  XTextWidth(fonts.normal,"        ",8);
      col = cur_pos - (cur_pos % width) + width;
      break;

    case '\033':		/* ignore esc sequences for now */
      c++;			/* should always be esc-x */
      break;

   case '_':			/* look for underlining [italicize] */
      c++;
      if(*c != BACKSPACE) {

	/* oops real underscore, fall through to default. */
	c--;
      }
      else {
	if(!italicflag) {	/* font change? */
	  *bufp = '\0';
	  XDrawString(disp,window,normal_gc,col,y_loc,
		      buf,strlen(buf));
	  col += XTextWidth(fonts.normal,buf,strlen(buf));
	  bufp = buf;
	  *bufp = '\0';
	  italicflag = 1;
	}
	c++;
	*bufp++ = *c;
	break;
      }

    default:
      if(italicflag) {			/* font change? */
	*bufp = '\0';
	XDrawString(disp,window,italic_gc,col,y_loc,buf,strlen(buf));
	col += XTextWidth(fonts.italic,buf,strlen(buf));	
	bufp = buf;
	*bufp = '\0';
	italicflag = 0;
      }
      *bufp++ = *c;
      break;
    }
    c++;
  }
}
