/*
 * xquota -  X window system quota display program.
 *
 * $Athena: handler.c,v 1.1 89/03/05 20:58:19 kit Locked $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   March 5, 1989
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: handler.c,v 1.1 89/03/05 20:58:19 kit Locked $";
#endif

#include <stdio.h>
#include <sys/time.h>

#include <sys/param.h>
#include <ufs/quota.h>

#include <X11/Intrinsic.h>

#include "xquota.h"

#define kb(n)   (howmany(dbtob(n), 1024))

static void DisplayInfo(), SetQuotaDisplay();

/*	Function Name: ShowQuota()
 *	Description: Shows the quota to the user.
 *	Arguments: none.
 *	Returns: none.
 */

void ShowQuota(info)
Info * info;
{
  struct dqblk quota_info;
  int ret_val;
  extern int homeIsAFS;
  int (*qfunc)();
  int getnfsquota();
  int getafsquota();

  SetTimeOut(info);

  qfunc = homeIsAFS ? getafsquota : getnfsquota;
  ret_val = (*qfunc)(info->host, info->path, info->uid, &quota_info);
  if (ret_val == QUOTA_NONE) {
    printf("You do not have a quota on your home file system.\n");
    printf("Filesystem: %s:%s\n", info->host, info->path);
    exit(1);
  }

  if (ret_val == QUOTA_PERMISSION) {
    printf("You do not have permission to get quota information on the\n");
    printf("filesystem: %s:%s.\n", info->host, info->path);
    exit(1);
  }

  if (ret_val == QUOTA_ERROR) {
    printf("Error in quota checking, trying again.\n");
      ret_val = (*qfunc)(info->host, info->path, info->uid, &quota_info);
    if (ret_val != QUOTA_OK) {
      printf("Quota check failed twice in a row, giving up.\n");
      exit(1);
    }
  }

  if (info->numbers && !info->space_saver) {
    char buf[BUFSIZ];
    sprintf(buf, "Quota: %5dK", (int) kb(quota_info.dqb_bsoftlimit));
    SetLabel(info->quota_top, buf);
    sprintf(buf, " Used: %5dK", (int) kb(quota_info.dqb_curblocks));
    SetLabel(info->used_top, buf);
  }
  SetGraph(info, info->graph, (int) kb(quota_info.dqb_bsoftlimit),
	   (int) kb(quota_info.dqb_curblocks));

  DisplayInfo(info, &quota_info);
  if ( quota_info.dqb_curblocks > quota_info.dqb_bsoftlimit) 
    XtPopup(info->info_popup, XtGrabNone);
}

/*	Function Name: DisplayInfo
 *	Description: Displays information.
 *	Arguments: info - the info struct.
 *	Returns: none.
 */

static void
DisplayInfo(info, quota_info)
Info * info;
struct dqblk * quota_info;
{
  long now;
  char buf[BUFSIZ];

  SetQuotaDisplay(info->hard_widget, "Hard Limit:",
		  (int) kb(quota_info->dqb_bhardlimit),
		  (int) quota_info->dqb_fhardlimit);

  SetQuotaDisplay(info->soft_widget, "Soft Limit:", 
		  (int) kb(quota_info->dqb_bsoftlimit),
		  (int) quota_info->dqb_fsoftlimit);

  SetQuotaDisplay(info->used_widget, "Current:", 
		  (int) kb(quota_info->dqb_curblocks),
		  (int) quota_info->dqb_curfiles);

  (void) time(&now);
  strcpy(buf, "Last Quota Check: ");
  strncat(buf, ctime(&now), 16);
  SetLabel(info->last_widget, buf);

/*
 * Determine which message to give.  If you are over both you file and
 * block quota you will only get a message about the blocks, oh well.
 */
  
  if (quota_info->dqb_curblocks > quota_info->dqb_bsoftlimit) {
    int kbytes = (int) kb(quota_info->dqb_curblocks - 
			  quota_info->dqb_bsoftlimit);
    if ( (kb(quota_info->dqb_curblocks) >= 
	                               (kb(quota_info->dqb_bhardlimit) - 5))||
	 (quota_info->dqb_btimelimit <= now) ) {
      sprintf(buf, EXCEEDED_HARD, kbytes, "kilobytes");
    }
    else {
      sprintf(buf, EXCEEDED_SOFT, kbytes, "kilobytes",
 	      ctime(&quota_info->dqb_btimelimit));
    }
    SetTextMessage(info->message_widget, buf);
  }
  else if ( quota_info->dqb_curfiles > quota_info->dqb_fsoftlimit ) {
    int files = quota_info->dqb_curfiles - quota_info->dqb_fsoftlimit;
    if ( (quota_info->dqb_curfiles >= (quota_info->dqb_fhardlimit - 10))  ||
	 (quota_info->dqb_ftimelimit <= now) ) {
      sprintf(buf, EXCEEDED_HARD, files, "files");
    }
    else {
      sprintf(buf, EXCEEDED_SOFT, files, "files",
 	      ctime(&quota_info->dqb_ftimelimit));
    }
    SetTextMessage(info->message_widget, buf);
  }
  else if (info->flipped)
    SetTextMessage(info->message_widget, WARNING_MESSAGE);
  else
    SetTextMessage(info->message_widget, SAFE_MESSAGE);
}

/*	Function Name: SetQuotaDisplay
 *	Description: Sets the Quota Display. 
 *	Arguments: w - the widget to display this in.
 *                 string - string to preface this with.
 *                 bytes - number of bytes.
 *                 files - number of files.
 *	Returns: none
 */

static void
SetQuotaDisplay( w, string, bytes, files )
Widget w;
char * string;
int bytes, files;
{
  char buf[BUFSIZ];
  
  sprintf(buf, "%12s %10d %10d", string, bytes, files);
  SetLabel(w, buf);
}
  
